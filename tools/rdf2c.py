#!/usr/bin/env python3
"""Room scripts to C: src/rdf/<room>.asm (and the original .RDF) -> src/rooms/<room>.c

    tools/rdf2c.py ROOM... | --all [--out DIR]

The scripts in the .RDF files are 16-bit machine code.  This translates
each script, helper and the code they share into a C function that does
exactly what the machine code does: registers become locals, the pushed
arguments of the game call become the arguments of a call to the matching
svc_ macro, the room state is the `room` structure, and the file's own
variables, strings and tables are read and written through the loaded image
(rooms.h) at the offsets the assembly source names.

The names (scripts, labels, variables) come from the symbol table of an
assembly listing of src/rdf/<room>.asm; the comments above each script from
the source.  The analysis is tools/rdf.py's.  The result is meant to be
tidied by hand from here on: this is a one-shot translator, like
tools/port_types.py.
"""

from __future__ import annotations

import os
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

import iced_x86 as ix

sys.path.insert(0, str(Path(__file__).parent))
import rdf as R  # noqa: E402
from rdf import (Room, load_room, room_flags, room_names, w16, FLOW_END, SERVICE, MSG_NAMES)  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build/rdf"
Reg = ix.Register
Mn = ix.Mnemonic

R16 = {Reg.AX: "ax", Reg.BX: "bx", Reg.CX: "cx", Reg.DX: "dx", Reg.SI: "si", Reg.DI: "di", Reg.BP: "bp",
       Reg.SP: "sp"}
R8 = {Reg.AL: ("ax", 0), Reg.AH: ("ax", 1), Reg.BL: ("bx", 0), Reg.BH: ("bx", 1), Reg.CL: ("cx", 0),
      Reg.CH: ("cx", 1), Reg.DL: ("dx", 0), Reg.DH: ("dx", 1)}
GP = ("ax", "bx", "cx", "dx", "si", "di")

# RoomState (include/room.h): offset -> (member, element size, count)
ROOM_FIELDS = [(0x00, "timers", 2, 8), (0x10, "saved_x", 2, 1), (0x12, "saved_y", 2, 1),
               (0x14, "stun", 2, 4), (0x1C, "no_walk", 1, 1), (0x1D, "busy", 1, 1),
               (0x1E, "redshirt_dead", 1, 1), (0x1F, "action", 1, 1), (0x20, "target", 1, 1),
               (0x21, "target2", 1, 1), (0x22, "unk22", 1, 1), (0x23, "more", 1, 1), (0x24, "flags", 1, 1),
               (0x25, "fall", 1, 4)]
VARS = 0x29

RC = ["RC_ANIM0", "RC_STOPANIM", "RC_ANIM2", "RC_TALK", "RC_GIVEITEM", "RC_LEVERS", "RC_GOTOROOM",
      "RC_TAKEITEM", "RC_WALK", "RC_LOADMAP", "RC_FLASHBMP", "RC_HAVEITEM", "RC_ITEMCASE", "RC_ACTORPOS",
      "RC_RANDOM", "RC_BRIDGESFX", "RC_MUSIC", "RC_BEAMUP", "RC_GAMEOVER", "RC_INPUTCODE", "RC_TURBOLIFT",
      "RC_SFX", "RC_BAD", "RC_DIGIOFF"]


class Unsupported(Exception):
    pass


def read_inc():
    """Names for items, objects and messages from include/rdf.inc."""
    items, objs, msgs = {}, {}, {}
    for l in (ROOT / "include/rdf.inc").read_text().splitlines():
        m = re.match(r"^(\w+)\s+EQU\s+([0-9A-Fa-f]+)h?\b", l)
        if not m:
            continue
        n, v = m.group(1), int(m.group(2), 16)
        if n.startswith("IT_"):
            items[v] = n
        elif n.startswith("OBJ_") and n != "OBJ_INVENTORY":
            objs[v] = n
        elif n.startswith("MSG_") and n != "MSG_TICK":
            msgs[v] = n
    return items, objs, msgs


ITEMS, OBJS, MSGS = read_inc()


# ------------------------------------------------------------------ listing

def assemble_listing(name: str) -> Path:
    """MASM src/rdf/<name>.asm with a listing, for its symbol table."""
    src = ROOT / "src/rdf" / f"{name.lower()}.asm"
    BUILD.mkdir(parents=True, exist_ok=True)
    lst = BUILD / f"{name.lower()}.lst"
    obj = BUILD / f"{name.lower()}.obj"

    def dos(p: Path) -> str:
        return "D:\\" + "\\".join(p.resolve().relative_to(ROOT).parts).upper()

    for old in BUILD.iterdir():
        if old.name.lower() in (lst.name, obj.name):
            old.unlink()
    masm = ROOT / "tools/MASM50"
    env = dict(os.environ, EMU2_DRIVE_C=str(masm), EMU2_DRIVE_D=str(ROOT))
    cmd = [os.environ.get("EMU2", "emu2"), str(masm / "BIN/MASM.EXE"), "/Ml", "/t", "/ID:\\INCLUDE", "/Z",
           f"{dos(src)},{dos(obj)},{dos(lst)};", "--", "INCLUDE=C:\\INC"]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True, errors="replace")
    if res.returncode != 0 or not lst.exists():
        raise SystemExit(f"{name}: MASM failed\n{res.stdout}{res.stderr}")
    return lst


SYM = re.compile(r"^(\S+)\s+(?:\.\s)*\.?\s*\t(L \w+|NUMBER)\t([0-9A-F]+)")


def read_symbols(lst: Path):
    """labels: addr -> [names]; equs: name -> value (from the listing's symbol table)."""
    labels: dict[int, list[str]] = defaultdict(list)
    kinds: dict[str, str] = {}
    equs: dict[str, int] = {}
    on = False
    for line in lst.read_text(errors="replace").splitlines():
        if line.startswith("Symbols:"):
            on = True
            continue
        if not on:
            continue
        m = SYM.match(line)
        if not m:
            continue
        name, kind, val = m.group(1), m.group(2), int(m.group(3), 16)
        if kind == "NUMBER":
            equs[name] = val
        else:
            labels[val].append(name)
            kinds[name] = kind
    return labels, kinds, equs


def source_equ_order(src: str) -> list[str]:
    return [m.group(1) for m in re.finditer(r"^(\w+)\s+EQU\b", src, re.M)]


def script_comments(src: str) -> dict[str, list[str]]:
    """Comment lines directly above each label line of the source: label -> lines."""
    out: dict[str, list[str]] = {}
    lines = src.splitlines()
    for i, line in enumerate(lines):
        m = re.match(r"^(\w+):", line)
        if not m:
            continue
        j = i - 1
        block = []
        while j >= 0 and lines[j].startswith(";"):
            block.append(lines[j][1:].strip())
            j -= 1
        if block:
            out[m.group(1)] = list(reversed(block))
    return out


def file_header(src: str) -> list[str]:
    out = []
    for line in src.splitlines():
        if line.startswith(";"):
            out.append(line[1:].strip())
        else:
            break
    return out


# ------------------------------------------------------------------ constants

class K:
    """A known constant: its value and how to write it in C."""
    __slots__ = ("v", "text")

    def __init__(self, v: int, text: str | None = None):
        self.v = v & 0xFFFF
        self.text = text or (f"0x{self.v:X}" if self.v > 9 else str(self.v))

    def __repr__(self):
        return f"K({self.text})"


def hx(v: int) -> str:
    return f"0x{v:X}" if v > 9 else str(v)


def reg_of(r):
    """(16 bit register name, byte shift or None) of an iced register."""
    if r in R16:
        return R16[r], None
    if r in R8:
        return R8[r]
    raise Unsupported(f"register {r}")


def ins_size(ins, i):
    k = ins.op_kind(i)
    if k == ix.OpKind.REGISTER:
        return 1 if ins.op_register(i) in R8 else 2
    if k == ix.OpKind.MEMORY:
        return 1 if ins.memory_size in (ix.MemorySize.UINT8, ix.MemorySize.INT8) else 2
    if k == ix.OpKind.IMMEDIATE8:
        return 1
    return 2


JCC = {Mn.JE: "e", Mn.JNE: "ne", Mn.JL: "l", Mn.JGE: "ge", Mn.JLE: "le", Mn.JG: "g", Mn.JB: "b",
       Mn.JAE: "ae", Mn.JBE: "be", Mn.JA: "a", Mn.JS: "s", Mn.JNS: "ns"}
ARITH = {Mn.ADD: "+", Mn.SUB: "-", Mn.AND: "&", Mn.OR: "|", Mn.XOR: "^"}


# ------------------------------------------------------------------ the room

def is_service(ins) -> bool:
    return Room.is_service(ins)


class RoomC:
    def __init__(self, name: str):
        self.name = name.upper()
        self.lname = name.lower()
        flags = room_flags()[self.name]
        dead = [int(f[5:], 16) for f in flags if f.startswith("code=")]
        self.r = Room(self.name, load_room(self.name), dead)
        self.code = self.r.code
        self.d = self.r.d
        self.src = (ROOT / "src/rdf" / f"{self.lname}.asm").read_text()
        self.labels, self.kinds, self.equs = read_symbols(assemble_listing(self.name))
        self.comments = script_comments(self.src)
        self.header = file_header(self.src)
        self.setup_names()
        self._tt: dict[int, list[int]] = {}
        self.succ: dict[int, list[int]] = {}
        self.build_succ()
        self.global_depth()
        self.partition()
        self.macros: dict[str, str] = {}        # name -> definition, for the file's variables
        self.enums: dict[str, int] = {}         # name -> offset, for strings and tables
        self.warnings: list[str] = []

    # ------------------------------------------------------------ names

    GENERATED = re.compile(r"^(E|L|H)_?[0-9A-Fa-f]{4}$")

    def setup_names(self) -> None:
        # code labels and data symbols by address
        self.code_names: dict[int, str] = {}
        self.data_names: dict[int, list[str]] = defaultdict(list)
        for a, names in self.labels.items():
            for n in names:
                if self.kinds[n] == "L NEAR":
                    cur = self.code_names.get(a)
                    if cur is None or (self.GENERATED.match(cur) and not self.GENERATED.match(n)) or \
                            (n.startswith("H_") is False and cur.startswith("H_")):
                        self.code_names[a] = n
                else:
                    self.data_names[a].append(n)
        # room variable names: value -> name, in source order
        order = source_equ_order(self.src)
        self.var_names: dict[int, str] = {}
        for n in order:
            v = self.equs.get(n)
            if v is not None and VARS <= v < 0x129 and not re.match(r"^(RS|IT|OBJ|MSG|RC)_|^ANY$", n):
                self.var_names.setdefault(v, n)

    def label(self, a: int) -> str:
        n = self.code_names.get(a)
        if n is None or n.startswith("H_"):
            n = f"L_{a:04X}"
        return n

    def func_name(self, a: int) -> str:
        n = self.code_names.get(a)
        kind = self.r.helpers.get(a)
        if n is None or n.startswith("rdf_") or self.GENERATED.match(n):
            n = {"say": "Say", "talk": "Talk"}.get(kind)
        if n is None:
            n = f"E_{a:04X}" if a in self.r.entries else f"part_{a:04X}"
        return f"{self.lname}_{n}"

    # ------------------------------------------------------------ control flow

    def table_targets(self, jmp_addr: int) -> list[int]:
        """Targets of a jmp cs:[reg+table], in table order.  The scripts fill the table
        themselves, and each fills it for its own jump: the nearest stores before it."""
        if jmp_addr in self._tt:
            return self._tt[jmp_addr]
        ins = self.code[jmp_addr]
        table = ins.memory_displacement & 0xFFFF
        tgt: dict[int, int] = {}
        seen = {jmp_addr}
        work = [jmp_addr]
        while work:
            a = work.pop(0)
            for p in self.preds.get(a, []):
                if p in seen:
                    continue
                seen.add(p)
                pi = self.code[p]
                if pi.mnemonic == Mn.MOV and pi.op0_kind == ix.OpKind.MEMORY and pi.segment_prefix == Reg.CS \
                        and pi.memory_base == Reg.NONE and pi.op1_kind == ix.OpKind.IMMEDIATE16 \
                        and table <= (pi.memory_displacement & 0xFFFF) < table + 64 \
                        and self.r.valid_ptr(pi.immediate16):
                    k = ((pi.memory_displacement & 0xFFFF) - table) // 2
                    tgt.setdefault(k, pi.immediate16)
                work.append(p)
        res = [tgt[k] for k in sorted(tgt)]
        self._tt[jmp_addr] = res
        return res

    def build_succ(self) -> None:
        for a, ins in self.code.items():
            s = []
            if ins.mnemonic == Mn.JMP and ins.op0_kind == ix.OpKind.NEAR_BRANCH16:
                s.append(self.tgt(ins))
            elif ins.flow_control == ix.FlowControl.CONDITIONAL_BRANCH:
                s.append(self.tgt(ins))
                s.append(self.fall(a))
            elif ins.flow_control not in FLOW_END and ins.flow_control != ix.FlowControl.INDIRECT_BRANCH:
                s.append(self.fall(a))
            self.succ[a] = s
        # the indirect jumps' targets are the table stores before them
        self.preds = defaultdict(list)
        for a, ss in self.succ.items():
            for t in ss:
                self.preds[t].append(a)
        for a, ins in self.code.items():
            if ins.flow_control == ix.FlowControl.INDIRECT_BRANCH:
                self.succ[a] = self.table_targets(a)

    def fall(self, a: int) -> int:
        """Where the code after the instruction at a goes on: a script header in the way
        is six bytes the machine ran as (harmless) code, then the script's body."""
        n = a + self.code[a].len
        if n in self.r.hdr_bytes:
            n = max(h for h, _, _ in self.r.headers if h <= n) + 6
        return n

    def tgt(self, ins) -> int:
        t = ins.near_branch16
        if t in self.r.hdr_bytes:
            t = max(h for h, _, _ in self.r.headers if h <= t) + 6
        return t

    def partition(self) -> None:
        """Split the code into functions with one entry each: the scripts
        and helpers, and the code shared between them."""
        E = set(self.r.entries) | set(self.r.calls)
        self.helpers = set(self.r.calls)
        entries = sorted(a for a in self.r.entries if a in self.code)
        while True:
            reach: dict[int, set[int]] = {}
            for e in sorted(E):
                if e not in self.code:
                    continue
                seen = set()
                todo = [e]
                while todo:
                    a = todo.pop()
                    if a in seen or a not in self.code:
                        continue
                    if a != e and a in E:
                        continue
                    seen.add(a)
                    todo.extend(self.succ[a])
                reach[e] = seen
            cover: dict[int, frozenset] = {}
            for e, s in reach.items():
                for a in s:
                    cover[a] = cover.get(a, frozenset()) | {e}
            heads = set()
            for a, c in cover.items():
                if len(c) > 1:
                    for p, ss in self.succ.items():
                        if a in ss and cover.get(p) != c and p in cover:
                            heads.add(a)
                            break
            heads -= E
            heads -= self.noheads
            if not heads:
                break
            E |= heads
        self.E = E
        self.reach = reach
        for a in sorted(self.code):
            if a not in cover and a not in self.r.dead:
                pass

    def global_depth(self) -> None:
        """The stack depth at every instruction, from the scripts and helpers (at 0)."""
        gd: dict[int, int] = {}
        conflict: set[int] = set()
        work = []
        for e in sorted(self.r.entries | self.r.calls):
            if e in self.code:
                gd[e] = 0
                work.append(e)
        while work:
            a = work.pop()
            ins = self.code[a]
            d = gd[a]
            m = ins.mnemonic
            if m == Mn.PUSH:
                d += 1
            elif m == Mn.POP:
                d -= 1
            elif m == Mn.ADD and ins.op0_kind == ix.OpKind.REGISTER and ins.op0_register == Reg.SP:
                d -= (ins.immediate(1) & 0xFFFF) // 2
            d = max(d, 0)
            for t in self.succ[a]:
                if t in self.code and t not in gd:
                    gd[t] = d
                    work.append(t)
                elif t in gd and gd[t] != d:
                    conflict.add(t)
        self.gdepth = gd
        # code reached with different stacks cannot be shared as a function
        self.noheads = set()
        todo = list(conflict)
        while todo:
            a = todo.pop()
            if a in self.noheads or a not in self.code:
                continue
            self.noheads.add(a)
            todo.extend(self.succ[a])

    # ------------------------------------------------------------ output pieces

    # ------------------------------------------------------------ symbols

    def data_sym(self, a: int) -> str:
        names = self.data_names.get(a)
        n = names[0] if names else f"D_{a:04X}"
        self.enums[n] = a
        return n

    def cs_var(self, disp: int, size: int) -> str:
        """The C lvalue of the file's variable at disp."""
        base = ("CS_B" if size == 1 else "CS_W")
        names = [n for n in self.data_names.get(disp, []) if not n.startswith(("N_", "T_", "S_", "C_"))]
        if not names:
            return f"{base}({hx(disp)})"
        n = f"fv_{names[0]}"
        cur = self.macros.get(n)
        want = f"{base}({hx(disp)})"
        if cur is None:
            self.macros[n] = want
            return n
        if cur == want:
            return n
        n2 = n + ("_b" if size == 1 else "_w")
        self.macros[n2] = want
        return n2

    def room_var(self, off: int, size: int, index: str | None = None) -> str:
        """The C lvalue of RoomState byte offset off (plus an optional index register)."""
        for base, member, esz, count in ROOM_FIELDS:
            if base <= off < base + esz * count and esz == size and (off - base) % esz == 0:
                k = (off - base) // esz
                if count == 1:
                    return f"room.{member}" if index is None else f"ROOM_{'B' if size == 1 else 'W'}({hx(off)} + {index})"
                if index is None:
                    return f"room.{member}[{k}]"
                if k == 0 and size == 1:
                    return f"room.{member}[{index}]"
                return f"room.{member}[{k} + {index}]"
        macro = "ROOM_B" if size == 1 else "ROOM_W"
        if index is None and off >= VARS and off in self.var_names:
            n = f"rv_{self.var_names[off]}"
            want = f"{macro}({hx(off)})"
            cur = self.macros.get(n)
            if cur is None:
                self.macros[n] = want
                return n
            if cur == want:
                return n
            n2 = n + ("_b" if size == 1 else "_w")
            self.macros[n2] = want
            return n2
        return f"{macro}({hx(off)}{' + ' + index if index else ''})"



# ------------------------------------------------------------------ one function

class Func:
    def __init__(self, entry: int, body: list[int], helper: bool):
        self.entry = entry
        self.body = body
        self.bodyset = set(body)
        self.helper = helper
        self.name = ""
        self.li: frozenset = frozenset()       # registers read before written
        self.df: frozenset = frozenset()       # registers it may write
        self.ret: frozenset = frozenset()      # registers its callers read after it
        self.dyn = False                       # a real stack instead of variables
        self.entry_depth = 0                   # stack words it is called with (shared code)
        self.facts: dict[int, tuple[bool, bool]] = {}
        self.depth: dict[int, int] = {}
        self.rd: dict[int, frozenset] = {}
        self.kill: dict[int, frozenset] = {}
        self.live_after: dict[int, frozenset] = {}
        self.call_live: dict[int, frozenset] = {}      # helper call site -> live after it
        self.max_slots = 0
        self.lines: list[str] = []


class Slot:
    __slots__ = ("expr", "k", "vol", "regs")

    def __init__(self, expr, k=None, vol=False, regs=frozenset()):
        self.expr, self.k, self.vol, self.regs = expr, k, vol, frozenset(regs)


class Pend:
    __slots__ = ("kind", "a", "b", "size", "regs")

    def __init__(self, kind, a, b, size, regs):
        self.kind, self.a, self.b, self.size, self.regs = kind, a, b, size, frozenset(regs)


NEG = {"e": "ne", "ne": "e", "l": "ge", "ge": "l", "le": "g", "g": "le", "b": "ae", "ae": "b", "be": "a",
       "a": "be", "s": "ns", "ns": "s"}


class Gen:
    """Translate one function's instructions to C statements.  Run first in
    analysis mode (no output; which registers each instruction reads and
    writes), then for real with the liveness that gives."""

    def __init__(self, rc: RoomC, f: Func, analysis: bool, infos: dict[int, Func]):
        self.rc, self.f, self.analysis, self.infos = rc, f, analysis, infos
        self.code = rc.code
        self.out: list[str] = []
        self.ntemp = 0
        self.temps: list[str] = []
        self.regs_used: set[str] = set()
        self.cur_rd: set[str] = set()
        self.cur_df: set[str] = set()
        self.cur_kill: set[str] = set()
        self.cur: int = f.entry
        self.targets: set[int] = set()
        self.df_rec: dict[int, frozenset] = {}
        self.pend_at: dict[int, list] = defaultdict(list)
        self.npred: dict[int, int] = defaultdict(int)
        for a in f.body:
            for t in rc.succ[a]:
                if t in f.bodyset:
                    self.npred[t] += 1
        for a in f.body:
            ins = rc.code[a]
            if ins.mnemonic == Mn.JMP and ins.op0_kind == ix.OpKind.NEAR_BRANCH16 \
                    or ins.flow_control == ix.FlowControl.CONDITIONAL_BRANCH:
                if rc.tgt(ins) in f.bodyset:
                    self.targets.add(rc.tgt(ins))
            elif ins.flow_control == ix.FlowControl.INDIRECT_BRANCH:
                self.targets.update(t for t in rc.table_targets(a) if t in f.bodyset)
        # emit the entry's code first: the function starts there, and code before it in
        # the file (reached by jumps) follows
        runs: list[list[int]] = []
        for a in f.body:
            if runs and runs[-1][-1] + rc.code[runs[-1][-1]].len == a:
                runs[-1].append(a)
            else:
                runs.append([a])
        first, rest = [], []
        for r in runs:
            if f.entry in r:
                k = r.index(f.entry)
                first, head = r[k:], r[:k]
                if head:
                    rest.append(head)
            else:
                rest.append(r)
        rest.sort(key=lambda r: r[0])
        self.order = first + [a for r in rest for a in r]
        for i, a in enumerate(self.order):
            ins = rc.code[a]
            nxt = rc.fall(a)
            if ins.flow_control not in FLOW_END and nxt in f.bodyset \
                    and (i + 1 >= len(self.order) or self.order[i + 1] != nxt):
                self.targets.add(nxt)
        self.reset(None)

    # ---------------------------------------------------------------- state

    def reset(self, a):
        self.cv: dict[str, K] = {}
        self.slots: list[Slot] = []
        self.pend: Pend | None = None
        self.si_room, self.bx_room = (True, False) if a is None else self.f.facts.get(a, (True, False))
        if a is not None and not self.f.dyn:
            self.slots = [Slot(f"s{i}") for i in range(self.f.depth.get(a, 0))]
        if a is not None and a != self.f.entry:
            lst = self.pend_at.get(a)
            if lst and len(lst) == self.npred[a] and all(x is not None for x in lst):
                key = lambda p: (p.kind, p.a, p.b, p.size)
                if all(key(x) == key(lst[0]) for x in lst):
                    self.pend = lst[0]

    def emit(self, s: str):
        if not self.analysis:
            self.out.append(s)

    def temp(self, expr: str, ty="uint16_t") -> str:
        n = f"t{self.ntemp}"
        self.ntemp += 1
        self.temps.append(f"{ty} {n};")
        self.emit(f"{n} = {expr};")
        return n

    def rd(self, reg: str):
        self.cur_rd.add(reg)
        self.regs_used.add(reg)

    def df(self, reg: str, kill: bool = True):
        self.cur_df.add(reg)
        self.regs_used.add(reg)
        if kill:
            self.cur_kill.add(reg)

    def dead(self, reg: str) -> bool:
        """The value written to reg is never read."""
        return (not self.analysis) and reg not in self.f.live_after.get(self.cur, frozenset(GP))

    def use_slot(self, s: Slot) -> str:
        for r in s.regs:
            self.rd(r)
        return s.expr

    def freeze_dep(self, reg: str):
        """reg is about to change: materialize what still refers to its value."""
        for i, s in enumerate(self.slots):
            if reg in s.regs:
                self.slots[i] = Slot(self.temp(self.use_slot(s)), s.k)
        p = self.pend
        if p and reg in p.regs:
            ty = "uint8_t" if p.size == 1 else "uint16_t"
            for r in p.regs:
                self.rd(r)
            a = self.temp(p.a, ty)
            b = self.temp(p.b, ty) if p.b is not None else None
            self.pend = Pend(p.kind, a, b, p.size, ())

    def freeze_vol(self):
        """memory is about to change: materialize what was read from it."""
        for i, s in enumerate(self.slots):
            if s.vol:
                self.slots[i] = Slot(self.temp(self.use_slot(s)), s.k)
        p = self.pend
        if p and getattr(p, "vol", False):
            pass

    def spill(self):
        """at a block end: the stack goes into its variables."""
        if self.f.dyn:
            return
        for i, s in enumerate(self.slots):
            if s.expr != f"s{i}" or s.regs:
                self.emit(f"s{i} = {self.use_slot(s)};")
                self.slots[i] = Slot(f"s{i}")
            self.f.max_slots = max(self.f.max_slots, i + 1)

    # ---------------------------------------------------------------- registers

    def rdreg(self, r):
        """(expr, K or None) of a register's value."""
        name, sh = reg_of(r)
        k = self.cv.get(name)
        if sh is None:
            if k is not None:
                return k.text, k
            self.rd(name)
            return name, None
        if k is not None:
            v = (k.v >> (8 * sh)) & 0xFF
            return hx(v), K(v)
        self.rd(name)
        return (f"HI({name})" if sh else f"LO({name})"), None

    def setreg(self, r, expr: str, k: K | None, skippable=True) -> None:
        name, sh = reg_of(r)
        if name == "sp":
            raise Unsupported("write to sp")
        self.freeze_dep(name)
        if name in ("si", "bx"):
            if name == "si":
                self.si_room = False
            else:
                self.bx_room = False
        if sh is None:
            self.df(name)
            if skippable and self.dead(name):
                pass
            else:
                self.emit(f"{name} = {expr};")
            if k is not None:
                self.cv[name] = k
            else:
                self.cv.pop(name, None)
        else:
            self.rd(name)
            self.df(name, kill=False)
            if k is not None and name in self.cv:
                old = self.cv[name].v
                v = (old & (0x00FF if sh else 0xFF00)) | (k.v << (8 * sh))
                self.cv[name] = K(v)
            else:
                self.cv.pop(name, None)
            if skippable and self.dead(name):
                pass
            else:
                self.emit(f"{'SETHI' if sh else 'SETLO'}({name}, {expr});")

    # ---------------------------------------------------------------- operands

    def imm(self, ins, i) -> K:
        v = ins.immediate(i)
        k = ins.op_kind(i)
        if k == ix.OpKind.IMMEDIATE8:
            return K(v & 0xFF)
        v &= 0xFFFF
        t = self.rc.r.ptr_imm.get(ins.ip)
        if t is not None and t == v and t not in self.code:
            return K(v, self.rc.data_sym(v))
        return K(v)

    def mem(self, ins, size: int):
        """The C lvalue of a memory operand."""
        base, idx = ins.memory_base, ins.memory_index
        disp = ins.memory_displacement & 0xFFFF
        regs = [r for r in (base, idx) if r != Reg.NONE]
        if ins.segment_prefix == Reg.CS:
            terms = []
            for r in regs:
                name, _ = reg_of(r)
                k = self.cv.get(name)
                if k is not None:
                    disp = (disp + k.v) & 0xFFFF
                else:
                    self.rd(name)
                    terms.append(name)
            if not terms:
                return self.rc.cs_var(disp, size)
            return f"{'CS_B' if size == 1 else 'CS_W'}({hx(disp)} + {' + '.join(terms)})"
        # the room state: SI (or BX, loaded from rdf_state) plus maybe an index
        room = [r for r in regs if (r == Reg.SI and self.si_room) or (r == Reg.BX and self.bx_room)]
        rest = [r for r in regs if r not in room[:1]]
        if not room or len(rest) > 1:
            if ins.segment_prefix == Reg.NONE and ins.memory_base in (Reg.SI, Reg.BX):
                # si or bx holds a table index here, not the room: the machine reads and writes
                # some other part of the game's data (the script's mistake, which did no harm)
                idx = [reg_of(r)[0] for r in regs]
                for n in idx:
                    self.rd(n)
                return f"STRAY_{'B' if size == 1 else 'W'}({hx(disp)} + {' + '.join(idx)})"
        index = None
        if rest:
            name, _ = reg_of(rest[0])
            k = self.cv.get(name)
            if k is not None:
                disp = (disp + k.v) & 0xFFFF
            else:
                self.rd(name)
                index = name
        return self.rc.room_var(disp, size, index)

    def room_field(self, ins, i) -> int | None:
        """The RoomState offset of a plain room operand."""
        if ins.op_kind(i) != ix.OpKind.MEMORY or ins.segment_prefix == Reg.CS:
            return None
        regs = [r for r in (ins.memory_base, ins.memory_index) if r != Reg.NONE]
        if len(regs) == 1 and ((regs[0] == Reg.SI and self.si_room) or (regs[0] == Reg.BX and self.bx_room)):
            return ins.memory_displacement & 0xFFFF
        return None

    def named(self, off, k: K | None, expr: str) -> str:
        """A constant compared with or stored in the room's action or target, by name."""
        if k is None or off is None or k.text != expr:
            return expr
        if off in (0x20, 0x21):
            return ITEMS.get(k.v) or OBJS.get(k.v) or expr
        if off == 0x1F:
            return MSGS.get(k.v, expr)
        return expr

    def is_cs4(self, ins) -> bool:
        return (ins.op1_kind == ix.OpKind.MEMORY and ins.segment_prefix == Reg.CS
                and ins.memory_base == Reg.NONE and ins.memory_index == Reg.NONE
                and (ins.memory_displacement & 0xFFFF) == 4)

    def value(self, ins, i):
        """(expr, K or None, volatile) of an operand."""
        k = ins.op_kind(i)
        if k == ix.OpKind.REGISTER:
            e, kk = self.rdreg(ins.op_register(i))
            return e, kk, False
        if k in (ix.OpKind.IMMEDIATE8, ix.OpKind.IMMEDIATE16, ix.OpKind.IMMEDIATE8TO16):
            kk = self.imm(ins, i)
            return kk.text, kk, False
        if k == ix.OpKind.MEMORY:
            return self.mem(ins, ins_size(ins, i)), None, True
        raise Unsupported(f"operand {ins}")

    def store(self, ins, i, expr: str, k: K | None = None, skippable=True):
        kind = ins.op_kind(i)
        if kind == ix.OpKind.REGISTER:
            self.setreg(ins.op_register(i), expr, k, skippable)
        elif kind == ix.OpKind.MEMORY:
            self.freeze_vol()
            self.emit(f"{self.mem(ins, ins_size(ins, i))} = {expr};")
        else:
            raise Unsupported(f"store {ins}")

    # ---------------------------------------------------------------- flags

    def set_pend(self, kind, a, b, size, regs):
        self.pend = Pend(kind, a, b, size, regs)

    def cond(self, jc: str) -> str:
        p = self.pend
        if p is None:
            raise Unsupported(f"jump without flags at {self.cur:04X}")
        sty = "int8_t" if p.size == 1 else "int16_t"
        uty = "uint8_t" if p.size == 1 else "uint16_t"
        for r in p.regs:
            self.rd(r)
        if p.kind == "cmp":
            a, b = p.a, p.b
            m = {"e": f"{a} == {b}", "ne": f"{a} != {b}",
                 "l": f"({sty}){a} < ({sty}){b}", "ge": f"({sty}){a} >= ({sty}){b}",
                 "le": f"({sty}){a} <= ({sty}){b}", "g": f"({sty}){a} > ({sty}){b}",
                 "b": f"({uty}){a} < ({uty}){b}", "ae": f"({uty}){a} >= ({uty}){b}",
                 "be": f"({uty}){a} <= ({uty}){b}", "a": f"({uty}){a} > ({uty}){b}",
                 "s": f"({sty})({a} - {b}) < 0", "ns": f"({sty})({a} - {b}) >= 0"}
            return m[jc]
        val = f"({p.a} & {p.b})" if p.kind == "test" else p.a
        sv = f"({sty}){val}"
        m = {"e": f"{val} == 0", "ne": f"{val} != 0", "s": f"{sv} < 0", "ns": f"{sv} >= 0",
             "l": f"{sv} < 0", "ge": f"{sv} >= 0", "le": f"{sv} <= 0", "g": f"{sv} > 0",
             "b": "0", "ae": "1", "be": f"{val} == 0", "a": f"{val} != 0"}
        return m[jc]

    # ---------------------------------------------------------------- calls

    def callee_args(self, info: Func) -> list[str]:
        return [self.rdreg_name(r) for r in sorted(info.li, key=GP.index)]

    def rdreg_name(self, name: str) -> str:
        k = self.cv.get(name)
        if k is not None:
            return k.text
        self.rd(name)
        return name

    def after_call(self, defs, note=True):
        for r in defs:
            self.freeze_dep(r)
            self.df(r)
            self.cv.pop(r, None)
        self.pend = None
        if "bx" in defs:
            self.bx_room = False

    def call_helper(self, t: int):
        info = self.infos.get(t)
        if info is None:
            raise Unsupported(f"call to {t:04X} which is not a function")
        self.spill_vol_for_call()
        args = self.callee_args(info)
        self.f.call_live[self.cur] = self.f.live_after.get(self.cur, frozenset())
        defs = set(info.df)
        live = self.f.live_after.get(self.cur, frozenset(GP))
        call = f"{info.name}({', '.join(args)})"
        self.after_call(defs)
        if info.ret and not self.analysis:
            if "dx" in info.ret and "dx" in live:
                self.emit(f"{{ uint32_t r = {call}; ax = (uint16_t)r; dx = (uint16_t)(r >> 16); }}")
            elif "ax" in live:
                self.emit(f"ax = (uint16_t){call};")
            else:
                self.emit(f"{call};")
        else:
            self.emit(f"{call};")

    def spill_vol_for_call(self):
        self.freeze_vol()

    ARITY = {0: 5, 1: 1, 2: 5, 3: 1, 4: 1, 5: 0, 6: 2, 7: 1, 8: 4, 9: 1, 10: 1, 11: 1, 12: 1, 13: 1, 14: 2,
             15: 1, 16: 2, 17: 3, 18: 0, 19: 1, 20: 2, 21: 1, 22: 0, 23: 0}
    SVC_NAMES = {0: "Anim0", 1: "StopAnim", 2: "Anim", 3: "Talk", 4: "GiveItem", 5: "Levers", 6: "GotoRoom",
                 7: "TakeItem", 8: "Walk", 9: "LoadMap", 10: "FlashBitmap", 11: "HaveItem", 12: "ItemCase",
                 13: "ActorPos", 14: "Random", 15: "BridgeSfx", 16: "Music", 17: "BeamUp", 18: "GameOver",
                 19: "InputCode", 20: "Turbolift", 21: "Sfx", 22: "Bad", 23: "DigiOff"}

    def service(self, a: int, ins):
        nxt = self.code.get(a + ins.len)
        if self.f.dyn:
            self.freeze_vol()
            call = "ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5])"
        else:
            cmd = self.slots[-1] if self.slots else None
            if nxt is not None and nxt.mnemonic == Mn.ADD and nxt.op0_register == Reg.SP:
                n = (nxt.immediate(1) & 0xFFFF) // 2 - 1
            elif cmd is not None and cmd.k is not None and cmd.k.v in self.ARITY:
                n = self.ARITY[cmd.k.v]            # a call that does not return: no add sp
            else:
                raise Unsupported(f"service call without add sp at {a:04X}")
            if cmd is None or n < 0 or n + 1 > len(self.slots):
                raise Unsupported(f"service call with {n} arguments at {a:04X}")
            avail = len(self.slots) - 1
            ar = self.ARITY.get(cmd.k.v, 5) if cmd.k is not None else 5
            # the engine reads all of the command's arguments; what the script did not push
            # is whatever was on the stack, and what an earlier push left there counts
            args = [self.sarg(self.slots[-2 - j]) if j < avail else "STALE_ARG" for j in range(ar)]
            if cmd.k is not None and cmd.k.v in (4, 7, 11) and args and self.slots[-2].k is not None \
                    and self.slots[-2].k.v in ITEMS:
                args[0] = ITEMS[self.slots[-2].k.v]
            name = self.SVC_NAMES.get(cmd.k.v) if cmd.k is not None else None
            if name is None:
                # the command was a variable: call the dispatcher as it is
                words = [self.sarg(self.slots[-1 - j]) for j in range(min(n + 1, 6))]
                words += ["0"] * (6 - len(words))
                call = f"ScriptService({', '.join(words)})"
            else:
                call = f"svc_{name}({', '.join(args)})"
            self.freeze_vol_except(n + 1)
        self.after_call(("ax", "dx", "bx", "cx"))
        live = self.f.live_after.get(a, frozenset(GP))
        if "dx" in live:
            self.emit(f"{{ int32_t r = {call}; ax = (uint16_t)r; dx = (uint16_t)(r >> 16); }}")
        elif "ax" in live:
            self.emit(f"ax = (uint16_t){call};")
        else:
            self.emit(f"{call};")

    def sarg(self, s: Slot) -> str:
        """A service argument: constants of 8000h and over are negative numbers."""
        if s.k is not None and s.k.v >= 0x8000 and s.k.text.startswith("0x"):
            return str(s.k.v - 0x10000)
        return self.use_slot(s)

    def freeze_vol_except(self, top: int):
        """before a call: slots below the arguments that were read from memory."""
        keep = len(self.slots) - top
        for i, s in enumerate(self.slots[:keep]):
            if s.vol:
                self.slots[i] = Slot(self.temp(self.use_slot(s)), s.k)

    # ---------------------------------------------------------------- the instructions

    def goto(self, t: int):
        if t in self.f.bodyset:
            self.rc.used_labels.add(t)
            return f"goto {self.rc.label(t)};"
        info = self.infos.get(t)
        if info is None:
            raise Unsupported(f"jump to {t:04X} which is not code of this function")
        stack_args = []
        if not self.f.dyn:
            if len(self.slots) != info.entry_depth:
                raise Unsupported(f"jump to function {t:04X} with {len(self.slots)} stack words, wants {info.entry_depth}")
            stack_args = [self.use_slot(s) for s in self.slots]
        args = stack_args + self.callee_args(info)
        call = f"{info.name}({', '.join(args)})"
        if self.f.ret and info.ret:
            return f"return {call};"
        return f"{{ {call}; return; }}"

    def run(self):
        f = self.f
        prev_end = None
        for idx, a in enumerate(self.order):
            ins = self.code[a]
            self.cur = a
            self.cur_rd, self.cur_df, self.cur_kill = set(), set(), set()
            if a in f.leaders or prev_end != a:
                self.reset(a)
            if a in self.targets:
                self.emit(f"{self.rc.label(a)}:;")
            self.refs: set[str] = set()
            self.h(a, ins)
            f.rd[a] = frozenset(self.cur_rd)
            f.kill[a] = frozenset(self.cur_kill)
            self.df_rec[a] = frozenset(self.cur_df)
            nxt = self.rc.fall(a)
            prev_end = a + ins.len
            if ins.flow_control not in FLOW_END:
                if nxt not in self.code:
                    self.emit("/* the original runs on into the next script's header */")
                    self.emit("return;")
                    prev_end = None
                elif nxt not in f.bodyset or (idx + 1 >= len(self.order) or self.order[idx + 1] != nxt):
                    self.spill()
                    self.pend_at[nxt].append(self.pend)
                    self.emit(self.goto(nxt))
                    prev_end = None
                elif nxt in f.leaders:
                    self.spill()
                    if ins.flow_control != ix.FlowControl.CONDITIONAL_BRANCH:
                        self.pend_at[nxt].append(self.pend)

    def h(self, a, ins):
        m = ins.mnemonic
        if m == Mn.MOV:
            return self.h_mov(a, ins)
        if m == Mn.PUSH:
            return self.h_push(ins)
        if m == Mn.POP:
            return self.h_pop(ins)
        if m in ARITH:
            return self.h_arith(a, ins, ARITH[m])
        if m in (Mn.INC, Mn.DEC):
            return self.h_incdec(a, ins)
        if m in (Mn.SHL, Mn.SHR):
            return self.h_shift(a, ins)
        if m in (Mn.CMP, Mn.TEST):
            return self.h_cmp(ins, m == Mn.TEST)
        if m in JCC:
            return self.h_jcc(ins)
        if m == Mn.JMP:
            return self.h_jmp(a, ins)
        if m == Mn.CALL:
            if is_service(ins):
                return self.service(a, ins)
            return self.call_helper(self.rc.tgt(ins))
        if m in (Mn.RET, Mn.RETF):
            return self.h_ret(ins)
        raise Unsupported(f"instruction {ins} at {a:04X}")

    def h_mov(self, a, ins):
        d0 = ins.op0_kind
        if d0 == ix.OpKind.REGISTER and ins.op0_register in (Reg.SI, Reg.BX) and self.is_cs4(ins):
            if ins.op0_register == Reg.SI:
                self.si_room = True
            else:
                self.bx_room = True
            return
        expr, k, vol = self.value(ins, 1)
        if d0 == ix.OpKind.REGISTER:
            self.setreg(ins.op0_register, expr, k)
        else:
            expr = self.named(self.room_field(ins, 0), k, expr)
            self.store(ins, 0, expr, k)

    def h_push(self, ins):
        self.refs = set()
        expr, k, vol = self.value(ins, 0)
        if self.f.dyn:
            self.freeze_vol()
            self.emit(f"stk[--sp_] = {expr};")
            return
        self.slots.append(Slot(expr, k, vol, self.refs))

    def h_pop(self, ins):
        if self.f.dyn:
            self.setreg(ins.op0_register, "stk[sp_++]", None)
            return
        if not self.slots:
            raise Unsupported("pop from an empty stack")
        s = self.slots.pop()
        self.setreg(ins.op0_register, self.use_slot(s), s.k)

    def h_addsp(self, ins):
        n = (ins.immediate(1) & 0xFFFF) // 2
        if self.f.dyn:
            self.emit(f"sp_ += {n};")
            return
        if n > len(self.slots):
            raise Unsupported("add sp below the stack")
        del self.slots[len(self.slots) - n:]

    def next_is_jcc(self, a):
        n = self.code.get(a + self.code[a].len)
        return n is not None and n.mnemonic in JCC

    def h_arith(self, a, ins, op):
        if ins.op0_register == Reg.SP:
            return self.h_addsp(ins)
        size = ins_size(ins, 0)
        mask = 0xFF if size == 1 else 0xFFFF
        d0 = ins.op0_kind
        self.refs = set()
        if d0 == ix.OpKind.REGISTER and ins.op1_kind == ix.OpKind.REGISTER and \
                ins.op0_register == ins.op1_register and op in ("-", "^"):
            k = K(0)
            self.setreg(ins.op0_register, "0", k, skippable=not self.next_is_jcc(a))
            self.set_pend("res", "0", None, size, ())
            return
        dexpr, dk, _ = self.value(ins, 0)
        sexpr, sk, _ = self.value(ins, 1)
        regs = set(self.refs)
        if d0 == ix.OpKind.REGISTER:
            name, sh = reg_of(ins.op0_register)
            if dk is not None and sk is not None:
                v = {"+": dk.v + sk.v, "-": dk.v - sk.v, "&": dk.v & sk.v, "|": dk.v | sk.v,
                     "^": dk.v ^ sk.v}[op] & mask
                res = K(v)
                self.setreg(ins.op0_register, hx(v), res, skippable=not self.next_is_jcc(a))
                self.set_pend("res", hx(v), None, size, ())
                return
            self.setreg(ins.op0_register, f"{dexpr} {op} {sexpr}", None, skippable=not self.next_is_jcc(a))
            cur, _ = self.rdreg(ins.op0_register)
            self.set_pend("res", cur, None, size, {name})
        else:
            self.freeze_vol()
            lv = self.mem(ins, size)
            self.emit(f"{lv} {op}= {sexpr};")
            self.set_pend("res", lv, None, size, regs)

    def h_incdec(self, a, ins):
        size = ins_size(ins, 0)
        op = "+" if ins.mnemonic == Mn.INC else "-"
        if ins.op0_kind == ix.OpKind.REGISTER:
            self.refs = set()
            dexpr, dk, _ = self.value(ins, 0)
            if dk is not None:
                v = (dk.v + (1 if op == "+" else -1)) & (0xFF if size == 1 else 0xFFFF)
                self.setreg(ins.op0_register, hx(v), K(v), skippable=not self.next_is_jcc(a))
                self.set_pend("res", hx(v), None, size, ())
                return
            name, _ = reg_of(ins.op0_register)
            self.setreg(ins.op0_register, f"{dexpr} {op} 1", None, skippable=not self.next_is_jcc(a))
            cur, _ = self.rdreg(ins.op0_register)
            self.set_pend("res", cur, None, size, {name})
        else:
            self.freeze_vol()
            self.refs = set()
            lv = self.mem(ins, size)
            self.emit(f"{lv}{op}{op};")
            self.set_pend("res", lv, None, size, set(self.refs))

    def h_shift(self, a, ins):
        sexpr, sk, _ = self.value(ins, 1)
        if sk is None:
            raise Unsupported("variable shift")
        size = ins_size(ins, 0)
        self.refs = set()
        dexpr, dk, _ = self.value(ins, 0)
        c = "<<" if ins.mnemonic == Mn.SHL else ">>"
        if dk is not None:
            v = ((dk.v << sk.v) if c == "<<" else (dk.v >> sk.v)) & (0xFF if size == 1 else 0xFFFF)
            self.setreg(ins.op0_register, hx(v), K(v))
            return
        name, _ = reg_of(ins.op0_register)
        self.setreg(ins.op0_register, f"{dexpr} {c} {sexpr}", None, skippable=True)
        cur, _ = self.rdreg(ins.op0_register)
        self.set_pend("res", cur, None, size, {name})

    def h_cmp(self, ins, test: bool):
        size = ins_size(ins, 0)
        self.refs = set()
        aexpr, ak, _ = self.value(ins, 0)
        bexpr, bk, _ = self.value(ins, 1)
        bexpr = self.named(self.room_field(ins, 0), bk, bexpr)
        self.set_pend("test" if test else "cmp", aexpr, bexpr, size, set(self.refs))

    def h_jcc(self, ins):
        self.spill()
        c = self.cond(JCC[ins.mnemonic])
        t = self.rc.tgt(ins)
        self.pend_at[t].append(self.pend)
        self.pend_at[self.cur + ins.len].append(self.pend)
        g = self.goto(t)
        cn = self.cond(NEG[JCC[ins.mnemonic]])
        self.emit(f"\x00IF{c}\x01{cn}\x01{g}")

    def h_jmp(self, a, ins):
        self.spill()
        if ins.flow_control == ix.FlowControl.INDIRECT_BRANCH:
            # jmp [table + si]: the scripts store code addresses into the table before
            # they jump through it (and what an earlier script left there stays)
            table = ins.memory_displacement & 0xFFFF
            tg = sorted({t for st, t in self.rc.r.code_ptrs.items()
                         if table <= (self.code[st].memory_displacement & 0xFFFF) < table + 64
                         and (t in self.f.bodyset or t in self.infos)})
            target = self.mem(ins, 2)
            lines = [f"switch ({target}) {{"]
            for t in tg:
                lines.append(f"case 0x{t:04X}: {self.goto(t)}")
            lines.append("default: return;")
            lines.append("}")
            for l in lines:
                self.emit(l)
            return
        self.pend_at[self.rc.tgt(ins)].append(self.pend)
        self.emit(self.goto(self.rc.tgt(ins)))

    def h_ret(self, ins):
        if ins.mnemonic == Mn.RETF or not self.f.ret:
            self.emit("return;")
            return
        for r in self.f.ret:
            self.rd(r)
        if "dx" in self.f.ret:
            self.emit("return (uint32_t)ax | ((uint32_t)dx << 16);")
        else:
            self.emit("return ax;")


# ------------------------------------------------------------------ the whole room

def liveness(f: Func, rc: RoomC, ret_live: frozenset):
    """Backward dataflow of the registers over the function.  Sets
    f.live_after; returns the registers live at the entry."""
    body = f.body
    live_in: dict[int, frozenset] = {a: frozenset() for a in body}
    live_out: dict[int, frozenset] = {a: frozenset() for a in body}
    changed = True
    while changed:
        changed = False
        for a in reversed(body):
            out = frozenset()
            for t in rc.succ[a]:
                if t in f.bodyset:
                    out |= live_in[t]
            ins = rc.code[a]
            nin = f.rd.get(a, frozenset()) | (out - f.kill.get(a, frozenset()))
            if nin != live_in[a] or out != live_out[a]:
                live_in[a], live_out[a] = nin, out
                changed = True
    f.live_after = live_out
    return live_in[f.entry]


def compute_facts(f: Func, rc: RoomC) -> None:
    """si and bx as the room pointer, and the stack depth, at each instruction."""
    code = rc.code
    f.entry_depth = rc.gdepth.get(f.entry, 0)
    facts = {f.entry: (True, False)}
    depth = {f.entry: f.entry_depth}
    work = [f.entry]
    dyn = False
    while work:
        a = work.pop()
        ins = code[a]
        sr, br = facts[a]
        d = depth[a]
        m = ins.mnemonic
        d0 = ins.op0_register if ins.op0_kind == ix.OpKind.REGISTER else None
        if m == Mn.PUSH:
            d += 1
        elif m == Mn.POP:
            d -= 1
            if d0 == Reg.SI:
                sr = False
            if d0 == Reg.BX:
                br = False
        elif m == Mn.ADD and d0 == Reg.SP:
            d -= (ins.immediate(1) & 0xFFFF) // 2
        elif d0 in (Reg.SI, Reg.BX) and m not in (Mn.CMP, Mn.TEST):
            room = m == Mn.MOV and ins.op1_kind == ix.OpKind.MEMORY and ins.segment_prefix == Reg.CS \
                and ins.memory_base == Reg.NONE and (ins.memory_displacement & 0xFFFF) == 4
            if d0 == Reg.SI:
                sr = room
            else:
                br = room
        if m == Mn.CALL:
            br = False
        if d < 0:
            dyn = True
            d = 0
        for t in rc.succ[a]:
            if t not in f.bodyset:
                continue
            old = facts.get(t)
            new = (sr, br) if old is None else (old[0] and sr, old[1] and br)
            if old is None or old != new or depth.get(t) is None:
                facts[t] = new
                if t in depth and depth[t] != d:
                    dyn = True
                    depth[t] = min(depth[t], d)
                else:
                    depth[t] = d
                work.append(t)
            elif depth[t] != d:
                dyn = True
    f.facts, f.depth, f.dyn = facts, depth, dyn
    leaders = {f.entry}
    for a in f.body:
        ins = code[a]
        if ins.flow_control not in (ix.FlowControl.NEXT, ix.FlowControl.CALL, ix.FlowControl.INDIRECT_CALL):
            leaders.add(a + ins.len)
            for t in rc.succ[a]:
                leaders.add(t)
        elif rc.fall(a) != a + ins.len:
            leaders.add(rc.fall(a))
    f.leaders = leaders


def build(rc: RoomC):
    rc.used_labels = set()
    funcs: dict[int, Func] = {}
    for e in sorted(rc.E):
        if e not in rc.reach or not rc.reach[e]:
            continue
        f = Func(e, sorted(rc.reach[e]), e in rc.helpers)
        f.name = rc.func_name(e)
        funcs[e] = f
        compute_facts(f, rc)
    rc.funcs = funcs
    for it in range(20):
        changed = False
        need: dict[int, set] = defaultdict(set)
        for f in funcs.values():
            f.call_live = {}
            g = Gen(rc, f, True, funcs)
            g.run()
            li = liveness(f, rc, frozenset())
            df = frozenset().union(*g.df_rec.values()) if g.df_rec else frozenset()
            if li != f.li or df != f.df:
                f.li, f.df = li, df
                changed = True
            for site, live in f.call_live.items():
                need[rc.tgt(rc.code[site])] |= set(live) & {"ax", "dx"}
        for f in funcs.values():
            ret = frozenset(need[f.entry]) & (f.df | {"ax"}) if f.helper else frozenset()
            if ret and "dx" not in ret and "dx" in f.df and "dx" in need[f.entry]:
                ret |= {"dx"}
            if ret != f.ret:
                f.ret = ret
                changed = True
        if not changed:
            break
    return funcs


def func_text(rc: RoomC, f: Func) -> list[str]:
    g = Gen(rc, f, False, rc.funcs)
    g.df_rec = {}
    g.run()
    params = [f"s{i}" for i in range(f.entry_depth)] + sorted(f.li, key=GP.index)
    rt = "uint32_t" if "dx" in f.ret else "uint16_t" if f.ret else "void"
    sig = f"static {rt} {f.name}({', '.join('uint16_t ' + p for p in params) or 'void'})"
    lines = []
    cmt = rc.comments.get(rc.code_names.get(f.entry, ""), None)
    if f.entry in rc.r.entries:
        hdr = f.entry - 6
        c2 = rc.comments.get(f"H_{hdr:04X}")
        key = rc.d[hdr:hdr + 4]
        kt = MSG_NAMES.get(key[0], str(key[0]))
        cmt = (c2 or []) if c2 else (cmt or [])
        cmt = list(cmt) + [f"[{kt} {key[1]:02X} {key[2]:02X} {key[3]:02X}]"]
    if cmt:
        lines.append("/* " + "\n * ".join(cmt) + " */")
    lines.append(sig)
    lines.append("{")
    text = "\n".join(structure(g.out))
    decl = [r for r in GP if re.search(rf"\b{r}\b", text) and r not in params]
    if decl:
        lines.append("    uint16_t " + ", ".join(f"{r} = 0" for r in decl) + ";")
    for i in range(f.entry_depth, f.max_slots):
        lines.append(f"    uint16_t s{i};")
    if f.dyn:
        lines.append("    uint16_t stk[32 + 8] = { 0 };       /* the top 8 are what a service call reads past the stack */")
        lines.append("    int sp_ = 32;")
    for t in g.temps:
        lines.append("    " + t)
    for l in structure(g.out):
        if l.endswith(":;") and not l.startswith(" "):
            lines.append(l)
        else:
            lines.append("    " + l)
    lines.append("}")
    return lines


def room_file(rc: RoomC) -> str:
    funcs = build(rc)
    bodies = []
    for e in sorted(funcs):
        bodies.append(func_text(rc, funcs[e]))
    out = []
    out.append(f"/* {rc.name}.RDF: the room's scripts, translated from the machine code in the room file")
    out.append(" * (src/rdf/%s.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the" % rc.lname)
    out.append(" * room state (`room`), and call the game through the svc_ macros of rooms.h.")
    out.append(" *")
    for l in rc.header:
        out.append((" * " + l).rstrip())
    out.append(" */")
    out.append("")
    out.append('#include "rooms.h"')
    out.append("")
    if rc.enums:
        out.append("/* where the file keeps its strings, conversations and tables */")
        out.append("enum {")
        for n, v in sorted(rc.enums.items(), key=lambda x: x[1]):
            out.append(f"    {n} = 0x{v:04X},")
        out.append("};")
        out.append("")
    if rc.macros:
        out.append("/* the room's variables: in the room state, and in the file itself */")
        for n, v in sorted(rc.macros.items()):
            out.append(f"#define {n} {v}")
        out.append("")
    out.append("/* functions */")
    for e in sorted(funcs):
        f = funcs[e]
        params = [f"s{i}" for i in range(f.entry_depth)] + sorted(f.li, key=GP.index)
        rt = "uint32_t" if "dx" in f.ret else "uint16_t" if f.ret else "void"
        out.append(f"static {rt} {f.name}({', '.join('uint16_t ' + p for p in params) or 'void'});")
    out.append("")
    for b in bodies:
        out.extend(b)
        out.append("")
    stubs = {}
    for hdr, key, nxt in rc.r.headers:
        e = hdr + 6
        if e not in funcs and e in rc.r.hdr_bytes:
            nh = e                                   # the next script's header
            body = nh + 6
            if body in funcs:
                name = f"{rc.lname}_E_{e:04X}"
                stubs[e] = name
                out.append(f"/* [{MSG_NAMES.get(key[0], key[0])} {key[1]:02X} {key[2]:02X} {key[3]:02X}] This script has no code of its own: its entry is the next\n * script's header, whose six bytes the machine ran as code (with no effect), and then that script. */")
                out.append(f"static void {name}(void)")
                out.append("{")
                out.append(f"    {funcs[body].name}();")
                out.append("}")
                out.append("")
    out.append("/* the scripts, by the offset of their code in the room file */")
    out.append(f"const RoomScript {rc.lname}_scripts[] = {{")
    for hdr, key, nxt in rc.r.headers:
        e = hdr + 6
        if e in funcs and (funcs[e].li or funcs[e].entry_depth or funcs[e].ret):
            out.append(f"    /* {e:04X}: a helper, not a script */")
        elif e in funcs:
            kt = MSG_NAMES.get(key[0], str(key[0]))
            out.append(f"    {{ 0x{e:04X}, {funcs[e].name} }},    /* {kt} {key[1]:02X} {key[2]:02X} {key[3]:02X} */")
    for e, name in sorted(stubs.items()):
        out.append(f"    {{ 0x{e:04X}, {name} }},")
    out.append("    { 0, 0 }")
    out.append("};")
    out.append("")
    return "\n".join(out)


def write_index(outdir: Path) -> None:
    names = [n.lower() for n in room_names()]
    out = ["/* The scripts of every room (tools/rdf2c.py). */", "", '#include "rooms.h"', ""]
    for n in names:
        out.append(f"extern const RoomScript {n}_scripts[];")
    out += ["", "static const struct {", "    const char *name;", "    const RoomScript *scripts;", "} rooms[] = {"]
    for n in names:
        out.append(f'    {{ "{n}", {n}_scripts }},')
    out += ["    { 0, 0 }", "};", "",
            "const RoomScript *FindRoomScripts(const char *room_name)", "{", "    int i;", "",
            "    for (i = 0; rooms[i].name; i++)", "        if (strcasecmp(rooms[i].name, room_name) == 0)",
            "            return rooms[i].scripts;", "    return 0;", "}", ""]
    (outdir / "index.c").write_text("\n".join(out))


def main():
    args = sys.argv[1:]
    outdir = ROOT / "src/rooms"
    if "--out" in args:
        i = args.index("--out")
        outdir = Path(args[i + 1])
        del args[i:i + 2]
    names = room_names() if "--all" in args else [a.upper() for a in args if not a.startswith("--")]
    outdir.mkdir(parents=True, exist_ok=True)
    bad = 0
    for n in names:
        try:
            rc = RoomC(n)
            text = room_file(rc)
        except Unsupported as e:
            print(f"{n}: {e}")
            bad += 1
            continue
        (outdir / f"{n.lower()}.c").write_text(text)
        print(f"{n}: ok")
    if "--all" in sys.argv[1:]:
        write_index(outdir)
    sys.exit(1 if bad else 0)



# ------------------------------------------------------------------ structure

LABEL = re.compile(r"^(\w+):;$")
GOTO = re.compile(r"^goto (\w+);$")


def structure(lines: list[str]) -> list[str]:
    """Turn the gotos of a function into if/else where the code between a jump and its
    target can only be entered from the jump.  Returns the lines, indented."""
    nodes = []
    for l in lines:
        if l.startswith("\x00IF"):
            c, cn, act = l[3:].split("\x01")
            m = GOTO.match(act)
            nodes.append(("if", c, cn, act, m.group(1) if m else None))
            continue
        m = LABEL.match(l)
        if m:
            nodes.append(("label", m.group(1)))
            continue
        m = GOTO.match(l)
        if m:
            nodes.append(("goto", m.group(1)))
            continue
        nodes.append(("s", l))
    pos = {n[1]: i for i, n in enumerate(nodes) if n[0] == "label"}
    refs: dict[str, set[int]] = defaultdict(set)
    for i, n in enumerate(nodes):
        if n[0] == "goto":
            refs[n[1]].add(i)
        elif n[0] == "if" and n[4]:
            refs[n[4]].add(i)
        elif n[0] == "s":
            for m in re.finditer(r"goto (\w+);", n[1]):
                refs[m.group(1)].add(i)

    def single_entry(lo: int, hi: int) -> bool:
        for name, p in pos.items():
            if lo <= p < hi:
                if any(not (lo <= r < hi) for r in refs.get(name, ())):
                    return False
        return True

    out: list[str] = []
    done_refs: set[int] = set()

    def ref_count(name):
        return len([r for r in refs.get(name, ()) if r not in done_refs])

    def gen(lo, hi, ind):
        i = lo
        pad = "    " * ind
        while i < hi:
            n = nodes[i]
            if n[0] == "if" and n[4] and n[4] in pos and i < pos[n[4]] <= hi and n[4] not in ():
                j = pos[n[4]]
                if single_entry(i + 1, j):
                    L = n[4]
                    last = nodes[j - 1] if j - 1 > i else None
                    # if / else: the region ends by jumping over the code at the target
                    if last and last[0] == "goto" and last[1] in pos and pos[last[1]] > j \
                            and pos[last[1]] <= hi and single_entry(j + 1, pos[last[1]]) \
                            and refs[L] == {i}:
                        k = pos[last[1]]
                        done_refs.add(i)
                        done_refs.add(j - 1)
                        out.append(f"{pad}if ({n[2]}) {{")
                        gen(i + 1, j - 1, ind + 1)
                        saved = out[:]
                        del out[:]
                        gen(j + 1, k, ind + 1)
                        sub = out[:]
                        out[:] = saved
                        base = "    " * (ind + 1)
                        top = [x for x in sub if (x.startswith(base) and not x.startswith(base + " ")
                               and not x.startswith(base + "}")) or x.startswith("\x00L")]
                        if len(top) == 1 and top[0].startswith(base + "if (") and top[0].endswith("{") \
                                and sub[0] == top[0]:
                            # else { if ... } is else if ...
                            out.append(f"{pad}}} else {sub[0].strip()}")
                            out.extend(x if x.startswith("\x00L") else x[4:] for x in sub[1:])
                        else:
                            out.append(f"{pad}}} else {{")
                            out.extend(sub)
                            out.append(f"{pad}}}")
                        i = k
                        continue
                    done_refs.add(i)
                    out.append(f"{pad}if ({n[2]}) {{")
                    gen(i + 1, j, ind + 1)
                    out.append(f"{pad}}}")
                    i = j
                    continue
            if n[0] == "if":
                out.append(f"{pad}if ({n[1]})")
                out.append(f"{pad}    {n[3]}")
            elif n[0] == "goto":
                out.append(f"{pad}goto {n[1]};")
            elif n[0] == "label":
                out.append(f"\x00L{n[1]}")
            else:
                for sub in n[1].split("\n"):
                    out.append(f"{pad}{sub}")
            i += 1

    gen(0, len(nodes), 0)
    # drop gotos to the very next label and labels nothing refers to any more
    live = defaultdict(int)
    for l in out:
        for m in re.finditer(r"goto (\w+);", l):
            live[m.group(1)] += 1
    res = []
    for k, l in enumerate(out):
        m = GOTO.match(l.strip())
        if m:
            nxt = next((x for x in out[k + 1:] if True), "")
            if nxt == f"\x00L{m.group(1)}":
                live[m.group(1)] -= 1
                continue
        res.append(l)
    final = []
    for l in res:
        if l.startswith("\x00L"):
            name = l[2:]
            if live.get(name, 0) > 0:
                final.append(f"{name}:;")
        else:
            final.append(l)
    return final


if __name__ == "__main__":
    main()
