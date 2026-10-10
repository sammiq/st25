#!/usr/bin/env python3
"""Run routines of the original 16-bit program under Unicorn, to check the
portable C replacements against the real machine code.

    from oracle import Oracle
    o = Oracle()
    ax, dx = o.call(0x3A45C, [3, 4])        # FixMul(3, 4), by IDA linear address

The unpacked image (build/exe/STARTREK.EXE) is loaded at paragraph 0x1000, so
IDA linear addresses are real linear addresses.  DGROUP is paragraph 0x45A5.
Arguments are pushed as 16 bit words (cdecl); the routine is called FAR
unless far=False.  The VGA segment (A000) is plain RAM.
"""
import os, struct
from unicorn import *
from unicorn.x86_const import *

LOAD = 0x10000
DGROUP = 0x45A5
STACK_SEG = DGROUP      # SS = DS = DGROUP, as in the program (the C code restores DS from SS)
TRAP = 0x00500          # linear address of the return stub (a HLT)
ROOT = os.path.join(os.path.dirname(__file__), '..')


class Oracle:
    def __init__(self, exe=None):
        exe = exe or os.path.join(ROOT, 'build/exe/STARTREK.EXE')
        data = open(exe, 'rb').read()
        (sig, last, pages, nreloc, hdrsize, minalloc, maxalloc, ss, sp, csum,
         ip, cs, relocoff, ovl) = struct.unpack_from('<14H', data, 0)
        img = bytearray(data[hdrsize * 16:])
        for i in range(nreloc):
            off, seg = struct.unpack_from('<HH', data, relocoff + 4 * i)
            p = seg * 16 + off
            v = struct.unpack_from('<H', img, p)[0]
            struct.pack_into('<H', img, p, (v + LOAD // 16) & 0xFFFF)
        self.image = bytes(img)
        self.uc = Uc(UC_ARCH_X86, UC_MODE_16)
        self.uc.mem_map(0, 0x100000)
        self.reset()

    def reset(self):
        """Fresh program state: the image as loaded (bss included, zeroed)."""
        self.uc.mem_write(0, bytes(0x100000))
        self.uc.mem_write(LOAD, self.image)
        self.uc.mem_write(TRAP, b'\xf4')

    # --- memory ---
    def write(self, linear, data):
        self.uc.mem_write(linear, bytes(data))

    def read(self, linear, n):
        return bytes(self.uc.mem_read(linear, n))

    def dg(self, off):
        return DGROUP * 16 + off

    def sym(self, name):
        if not hasattr(self, '_syms'):
            self._syms = {}
            for l in open(os.path.join(ROOT, 'config/symbols.txt')):
                f = l.split()
                if len(f) >= 4 and not l.startswith('#'):
                    self._syms[f[3]] = int(f[0], 16)
        return self._syms[name]

    def code_seg(self, linear):
        """Paragraph of the code segment holding a linear address: modules
        start on a paragraph, and config/units.txt gives where their text starts."""
        if not hasattr(self, '_units'):
            self._units = []
            for l in open(os.path.join(ROOT, 'config/units.txt')):
                f = l.split()
                if len(f) >= 4 and f[1] == '.text' and not l.startswith('#'):
                    self._units.append((int(f[2], 16), int(f[3], 16)))
        for a, b in self._units:
            if a <= linear < b:
                return a >> 4
        return linear >> 4

    # --- calls ---
    def call(self, linear, args=(), ds=DGROUP, far=True, regs=None, es=0, seg=None):
        uc = self.uc
        sp = 0xFF00         # above the program's bss (ends at DGROUP:EDD0)
        ss = STACK_SEG
        for a in reversed(args):
            sp -= 2
            uc.mem_write(ss * 16 + sp, struct.pack('<H', a & 0xFFFF))
        if far:
            sp -= 4
            uc.mem_write(ss * 16 + sp, struct.pack('<HH', TRAP & 0xF, TRAP >> 4))
        else:
            sp -= 2
            uc.mem_write(ss * 16 + sp, struct.pack('<H', TRAP & 0xF))
        cs = seg if seg is not None else self.code_seg(linear)
        ip = linear - cs * 16
        uc.reg_write(UC_X86_REG_SS, ss)
        uc.reg_write(UC_X86_REG_SP, sp)
        uc.reg_write(UC_X86_REG_DS, ds)
        uc.reg_write(UC_X86_REG_ES, es)
        uc.reg_write(UC_X86_REG_CS, cs)
        uc.reg_write(UC_X86_REG_FLAGS, 0x0002)
        for r in ('AX', 'BX', 'CX', 'DX', 'SI', 'DI', 'BP'):
            uc.reg_write(getattr(__import__('unicorn.x86_const', fromlist=['x']), 'UC_X86_REG_' + r), 0)
        for k, v in (regs or {}).items():
            uc.reg_write(getattr(__import__('unicorn.x86_const', fromlist=['x']), 'UC_X86_REG_' + k.upper()), v)
        # run from cs:ip until HLT at the trap
        uc.emu_start(cs * 16 + ip, TRAP + 1, count=50_000_000)
        ax = uc.reg_read(UC_X86_REG_AX)
        dx = uc.reg_read(UC_X86_REG_DX)
        return ax, dx

    def run_range(self, cs, start, end, regs=None, ds=DGROUP, es=0, ss=DGROUP, count=50_000_000):
        """Run the code cs:start up to cs:end (a block inside a function), with
        the given registers.  Returns nothing; read registers/memory after."""
        uc = self.uc
        uc.reg_write(UC_X86_REG_SS, ss)
        uc.reg_write(UC_X86_REG_DS, ds)
        uc.reg_write(UC_X86_REG_ES, es)
        uc.reg_write(UC_X86_REG_CS, cs)
        uc.reg_write(UC_X86_REG_FLAGS, 0x0002)
        import unicorn.x86_const as xc
        for k, v in (regs or {}).items():
            uc.reg_write(getattr(xc, 'UC_X86_REG_' + k.upper()), v)
        uc.emu_start(cs * 16 + start, cs * 16 + end, count=count)

    def reg(self, name):
        return self.uc.reg_read(getattr(__import__('unicorn.x86_const', fromlist=['x']), 'UC_X86_REG_' + name.upper()))


def s16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def u32(lo, hi):
    return (lo & 0xFFFF) | ((hi & 0xFFFF) << 16)


if __name__ == '__main__':
    o = Oracle()
    print([hex(x) for x in o.call(0x3A45C, [3 << 14, 0x2000])])   # FixMul(3.0, 0.5)
