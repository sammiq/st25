#!/usr/bin/env python3
"""src/qsort.c (the MSC runtime's qsort) against the original machine code: same
result and the same sequence of comparisons, ties and all."""
import ctypes, random, struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import *
from oracle import Oracle, DGROUP
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import *

c = ctypes
lib = build('qsort', ['src/qsort.c'])
o = Oracle()
rnd = random.Random(5)
QS = o.sym('_qsort')
CMP_SEG, CMP_OFF = 0x60, 0
# push bp; mov bp,sp; mov bx,[bp+6]; mov ax,[bx]; mov bx,[bp+8]; cmp ax,[bx]; jl -1; jg +1; xor ax,ax; pop bp; retf
CMP = bytes([0x55, 0x89, 0xE5, 0x8B, 0x5E, 0x06, 0x8B, 0x07, 0x8B, 0x5E, 0x08, 0x3B, 0x07,
             0x7C, 0x0B, 0x7F, 0x04, 0x31, 0xC0, 0x5D, 0xCB,       # equal
             0xB8, 0x01, 0x00, 0x5D, 0xCB,                         # greater
             0xB8, 0xFF, 0xFF, 0x5D, 0xCB])                        # less
log_o = []


def hook(uc, addr, size, user):
    ss = uc.reg_read(UC_X86_REG_SS) * 16
    sp = uc.reg_read(UC_X86_REG_SP)
    a1, a2 = struct.unpack('<HH', bytes(uc.mem_read(ss + sp + 4, 4)))
    log_o.append((a1, a2))


o.uc.hook_add(UC_HOOK_CODE, hook, begin=CMP_SEG * 16, end=CMP_SEG * 16)
BASE = 0x8000
log_c = []


@c.CFUNCTYPE(c.c_int, c.c_void_p, c.c_void_p)
def ccmp(a, b):
    ka = c.c_int16.from_address(a).value
    kb = c.c_int16.from_address(b).value
    log_c.append((a - arr_addr, b - arr_addr))
    return -1 if ka < kb else 1 if ka > kb else 0


bad = 0
for trial in range(4000):
    n = rnd.choice([0, 1, 2, 3, 5, 8, 9, 16, 33, 100, rnd.randint(2, 200)])
    keys = rnd.choice([2, 3, 5, 50, 1000])
    elems = [(rnd.randrange(keys), i) for i in range(n)]
    if rnd.random() < 0.2:
        elems.sort()                    # already sorted
    if rnd.random() < 0.1:
        elems.sort(reverse=True)
    data = b''.join(struct.pack('<hH', k, t) for k, t in elems)
    o.reset()
    o.write(CMP_SEG * 16 + CMP_OFF, CMP)
    o.write(o.dg(BASE), data)
    log_o.clear()
    o.call(QS, [BASE, n, 4, CMP_OFF, CMP_SEG])
    want = bytes(o.read(o.dg(BASE), len(data)))
    buf = c.create_string_buffer(data, len(data) + 8)
    arr_addr = c.addressof(buf)
    log_c.clear()
    lib.ms_qsort(buf, c.c_size_t(n), c.c_size_t(4), ccmp)
    got = buf.raw[:len(data)]
    lo = [(a - BASE, b - BASE) for a, b in log_o]
    if got != want or lo != log_c:
        bad += 1
        if bad < 5:
            print('MISMATCH n', n, 'keys', keys, 'result', got == want, 'calls', len(lo), len(log_c))
print('qsort:', trial + 1, 'arrays,', bad, 'mismatches')
