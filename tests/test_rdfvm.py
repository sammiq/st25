#!/usr/bin/env python3
"""The room script VM (src/rdfvm.c) against the real x86 machine code: run
every script of every .rdf from DATA.001 under Unicorn and in the VM with the
same room state and game call results, and compare the calls made and the
memory left behind.

    tests/test_rdfvm.py [data_dir]     (the unpacked files come from tools/extract_data.py)
"""
import ctypes, glob, os, random, struct, sys
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import *
from unicorn import *
from unicorn.x86_const import *
import extract_data

c = ctypes
lib = build('rdfvm', ['src/rdfvm.c', 'tests/stub_platform.c'])
ROOM_SIZE = 0x129
BASE = 0x100                                  # RDF_ROOM_BASE
IMG_SEG, DS_SEG, SS_SEG, STUB_SEG, TRAP_SEG = 0x6000, 0x3000, 0x4000, 0x2000, 0x2100
rnd = random.Random(3)

CB = c.CFUNCTYPE(c.c_int32, c.POINTER(c.c_int16))
lib.RdfRun.restype = c.c_int
lib.RdfRun.argtypes = [c.c_void_p, c.c_uint32, c.c_uint16, c.c_void_p, c.c_uint16, CB]
lib.RdfVerify.argtypes = [c.c_void_p, c.c_uint32, c.c_uint16]


SEED = [0]


def svc_result(n):
    return ((n * 2654435761 + 12345 + SEED[0] * 977) >> 7) & 3       # small: it is often a menu choice used as a table index


def run_c(image, entry, room):
    img = c.create_string_buffer(bytes(image), len(image))
    rm = c.create_string_buffer(bytes(room), len(room))
    calls = []

    def svc(args):
        calls.append(tuple(args[i] & 0xFFFF for i in range(6)))
        return c.c_int32(svc_result(len(calls)) if False else svc_result(len(calls))).value

    cb = CB(svc)
    err = lib.RdfRun(c.addressof(img), len(image), entry, c.addressof(rm), len(room), cb)
    if err:
        return None
    return calls, img.raw, rm.raw


def run_uc(image, entry, room):
    uc = Uc(UC_ARCH_X86, UC_MODE_16)
    uc.mem_map(0, 0x200000)
    img = bytearray(image)
    struct.pack_into('<HH', img, 0, 0, STUB_SEG)           # the game call: STUB_SEG:0
    uc.mem_write(IMG_SEG * 16, bytes(img))
    uc.mem_write(STUB_SEG * 16, b'\xCB')                    # retf
    TRAP = 0xFFFF * 16 + 0xFFFF            # the VM's sentinel return address FFFF:FFFF
    uc.mem_write(TRAP, b'\xF4')
    uc.mem_write(DS_SEG * 16 + BASE, bytes(room))
    uc.mem_write(SS_SEG * 16, bytes([0xA5]) * 0x10000)
    sp = 0xF000 + 0x400
    sp = 0x400 * 2 // 2
    sp = 1024                                              # top of the VM's 512 word stack
    # same layout as the VM: stack top at 0x400, sentinel far return
    uc.mem_write(SS_SEG * 16 + sp - 4, struct.pack('<HH', 0xFFFF, 0xFFFF))
    sp -= 4
    calls = []

    def hook(uc_, addr, size, ud):
        spv = uc.reg_read(UC_X86_REG_SP)
        words = struct.unpack('<6H', bytes(uc.mem_read(SS_SEG * 16 + spv + 4, 12)))
        calls.append(tuple(words))
        r = svc_result(len(calls))
        uc.reg_write(UC_X86_REG_AX, r & 0xFFFF)
        uc.reg_write(UC_X86_REG_DX, r >> 16)

    uc.hook_add(UC_HOOK_CODE, hook, begin=STUB_SEG * 16, end=STUB_SEG * 16)
    for reg, v in ((UC_X86_REG_DS, DS_SEG), (UC_X86_REG_ES, DS_SEG), (UC_X86_REG_SS, SS_SEG),
                   (UC_X86_REG_SP, sp), (UC_X86_REG_SI, BASE), (UC_X86_REG_BX, BASE), (UC_X86_REG_CS, IMG_SEG),
                   (UC_X86_REG_AX, 0), (UC_X86_REG_CX, 0), (UC_X86_REG_DX, 0), (UC_X86_REG_DI, 0)):
        uc.reg_write(reg, v)
    uc.reg_write(UC_X86_REG_FLAGS, 2)
    uc.emu_start(IMG_SEG * 16 + entry, TRAP, count=60_000)
    if uc.reg_read(UC_X86_REG_CS) * 16 + uc.reg_read(UC_X86_REG_IP) != TRAP:
        raise UcError(0)                                     # did not come back
    out = bytearray(uc.mem_read(IMG_SEG * 16, len(image)))
    out[0:4] = image[0:4]                                   # the call pointer we put there
    return calls, bytes(out), bytes(uc.mem_read(DS_SEG * 16 + BASE, len(room)))


def scripts(rdf):
    sc, se = struct.unpack_from('<HH', rdf, 0x0E)
    p = sc
    out = []
    while p < se:
        key, nxt = struct.unpack_from('<IH', rdf, p)
        if nxt <= p or nxt > len(rdf):         # the list runs on into the file's subroutines: not scripts
            break
        out.append((key, p + 6))
        p = nxt
    return out


def room_state():
    r = bytearray(ROOM_SIZE)
    for i in range(ROOM_SIZE):
        r[i] = rnd.choice([0, 0, 1, 1, 2, 3, rnd.getrandbits(8)])
    return bytes(r)


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'orig/CD/TREKCD')
    data = open(os.path.join(src, 'DATA.001'), 'rb').read()
    import struct as st
    nscripts = nbad = nverify = nfail = 0
    for name, ext, pos in extract_data.read_dir(src):
        if ext != 'RDF':
            continue
        size, packed = st.unpack_from('<HH', data, pos)
        rdf = extract_data.unpack(data[pos + 4:pos + 4 + packed], size) if packed != size else data[pos + 4:pos + 4 + packed]
        rdf = bytearray(rdf)
        struct.pack_into('<H', rdf, 4, BASE)               # rdf->state
        vbuf = c.create_string_buffer(bytes(rdf), len(rdf))
        for key, entry in scripts(rdf):
            nscripts += 1
            off = lib.RdfVerify(c.addressof(vbuf), len(rdf), entry)
            if off:
                nverify += 1
                print('UNSUPPORTED', name, hex(entry), hex(off))
                continue
            for trial in range(3):
                room = room_state()
                want = None
                for SEED[0] in range(trial * 8, trial * 8 + 8):     # the game call results steer menus
                    try:
                        want = run_uc(rdf, entry, room)
                        break
                    except UcError as e:
                        err = e
                if want is None:
                    print('UC error', name, hex(entry), err)
                    continue
                if os.environ.get("VERB"): print("run", name, hex(entry), room.hex()[:40], flush=True)
                got = run_c(rdf, entry, room)
                if got is None:
                    nfail += 1
                    continue
                if got[0] != want[0] or got[1] != want[1] or got[2] != want[2]:
                    nbad += 1
                    if got[2] != want[2]:
                        print('  room diff at', [(i, got[2][i], want[2][i], room[i]) for i in range(len(room)) if got[2][i] != want[2][i]])
                    if got[0] != want[0]:
                        print('  calls C', got[0], '\n  calls U', want[0])
                    print('MISMATCH', name, hex(entry), 'calls', len(got[0]), len(want[0]),
                          'img', got[1] != want[1], 'room', got[2] != want[2])
                    if nbad > 12:
                        sys.exit(1)
    print('rdfvm: %d scripts, %d unsupported, %d mismatches, %d runs the machine would not survive' % (nscripts, nverify, nbad, nfail))
    sys.exit(1 if nbad or nverify else 0)


main()
