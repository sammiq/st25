#!/usr/bin/env python3
"""seg027 (fixed point maths) C port against the original machine code."""
import ctypes, random, struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import *
from oracle import Oracle, s16, u32

STUB = os.path.join(os.path.dirname(__file__), 'stub_platform.c')
lib = build('seg027', ['src/seg027.c', STUB])
o = Oracle()
rnd = random.Random(1)
c = ctypes
bad = 0


def check(name, got, want, args):
    global bad
    if got != want:
        bad += 1
        if bad < 20:
            print('MISMATCH', name, [hex(a & 0xFFFFFFFF) for a in args], 'C', got, 'orig', want)


A = {n: o.sym('_' + n) for n in ('FixMul', 'FixMul2', 'FixDiv', 'FixSqrt', 'FixLength', 'Project',
                                  'sub_3A35E', 'MatMul', 'MatVecMul32', 'MatTVecMul32', 'Vec32FromScreen',
                                  'Vec32Add', 'Vec32Sub', 'proj_dist')}

lib.FixMul.restype = c.c_int16; lib.FixMul.argtypes = [c.c_int16] * 2
lib.FixDiv.restype = c.c_int16; lib.FixDiv.argtypes = [c.c_int32] * 2
lib.FixMul2.restype = c.c_int32; lib.FixMul2.argtypes = [c.c_int32, c.c_int16, c.c_int32, c.c_int16]
lib.FixSqrt.restype = c.c_int16; lib.FixSqrt.argtypes = [c.c_int32]
lib.FixLength.restype = c.c_int16; lib.FixLength.argtypes = [c.c_int16] * 3
lib.sub_3A35E.restype = c.c_int32; lib.sub_3A35E.argtypes = [c.c_int32] * 3
lib.Project.restype = c.c_int16; lib.Project.argtypes = [c.c_int16] * 2
c.c_int32.in_dll(lib, 'proj_dist').value = 0x1234

def w32(v): return [v & 0xFFFF, (v >> 16) & 0xFFFF]

edge = [0, 1, -1, 0x7FFF, -0x8000, 0x4000, -0x4000, 0x2000, 12345, -12345]
def rv():
    return rnd.choice(edge) if rnd.random() < 0.2 else rnd.randint(-0x8000, 0x7FFF)

N = 3000
for _ in range(N):
    a, b = rv(), rv()
    check('FixMul', lib.FixMul(a, b), s16(o.call(A['FixMul'], [a, b])[0]), [a, b])
    d = rv() or 1
    # the original traps on quotient overflow; only compare when it fits
    q = int(((a << 14)) / d) if d else 0
    if -0x8000 <= q <= 0x7FFF:
        check('FixDiv', lib.FixDiv(a, d), s16(o.call(A['FixDiv'], w32(a) + w32(d))[0]), [a, d])
    x, y = rv(), rv()
    lo, hi = o.call(A['FixMul2'], w32(a) + [b] + w32(x) + [y])
    check('FixMul2', lib.FixMul2(a, b, x, y), s16(u32(lo, hi) & 0xFFFF) if False else c.c_int32(u32(lo, hi)).value, [a, b, x, y])
    z = rv()
    # FixSqrt: any 32 bit value
    v = rnd.getrandbits(32) if rnd.random() < 0.5 else rnd.getrandbits(rnd.randint(1, 31))
    ax = o.call(A['FixSqrt'], w32(v))[0]
    check('FixSqrt', lib.FixSqrt(c.c_int32(v).value) & 0xFFFF, ax, [v])
    ax = o.call(A['FixLength'], [a, b, z], seg=0x3A22)[0]
    check('FixLength', lib.FixLength(a, b, z) & 0xFFFF, ax, [a, b, z])
    lo, hi = o.call(A['sub_3A35E'], w32(a) + w32(b) + w32(z))
    check('len3', lib.sub_3A35E(a, b, z), c.c_int32(u32(lo, hi)).value, [a, b, z])
    # Project: original traps on overflow
    pd = 0x1234
    zz = rv() or 1
    o.write(o.dg(A['proj_dist'] - 0x45A50), struct.pack('<i', pd))
    q = int(a * pd / zz)
    if -0x8000 <= q <= 0x7FFF:
        check('Project', lib.Project(a, zz), s16(o.call(A['Project'], [a, 0, zz])[0]), [a, zz])

# exhaustive-ish sqrt around perfect squares
for r in list(range(0, 65536, 97)) + [65535, 65534, 46340, 46341]:
    for dv in (-1, 0, 1):
        v = (r * r + dv) & 0xFFFFFFFF
        ax = o.call(A['FixSqrt'], w32(v))[0]
        check('FixSqrt', lib.FixSqrt(c.c_int32(v).value) & 0xFFFF, ax, [v])


print('seg027 scalar: %d mismatches' % bad)

# --- vectors and matrices, in DGROUP at offset BASE ---
BASE = 0x9000
def rm():
    return [rv() for _ in range(9)]
def mem(off, n): return o.read(o.dg(off), n)
lib.MatMul.argtypes = [c.c_void_p] * 3
lib.MatVecMul32.argtypes = [c.c_void_p] * 3
lib.MatTVecMul32.argtypes = [c.c_void_p] * 3
lib.Vec32Add.argtypes = [c.c_void_p] * 2
lib.Vec32Sub.argtypes = [c.c_void_p] * 2
lib.Vec32FromScreen.argtypes = [c.c_void_p, c.c_int16, c.c_int16, c.c_int16]
def cbuf(b):
    return c.create_string_buffer(bytes(b), len(b))
bad2 = 0
for it in range(1500):
    a = struct.pack('<9h', *rm()); b = struct.pack('<9h', *rm())
    o.write(o.dg(BASE), a); o.write(o.dg(BASE + 32), b)
    o.write(o.dg(BASE + 64), bytes(18))
    o.call(A['MatMul'], [BASE, BASE + 32, BASE + 64])
    want = mem(BASE + 64, 18)
    ca, cb, co = cbuf(a), cbuf(b), cbuf(bytes(18))
    lib.MatMul(ca, cb, co)
    if co.raw != want: bad2 += 1; print('MatMul', a.hex(), b.hex())
    # aliasing: out = a
    o.write(o.dg(BASE), a); o.write(o.dg(BASE + 32), b)
    o.call(A['MatMul'], [BASE, BASE + 32, BASE])
    want = mem(BASE, 18)
    ca = cbuf(a); lib.MatMul(ca, cbuf(b), ca)
    if ca.raw != want: bad2 += 1; print('MatMul alias a', a.hex(), b.hex())
    o.write(o.dg(BASE), a); o.write(o.dg(BASE + 32), b)
    o.call(A['MatMul'], [BASE, BASE + 32, BASE + 32])
    want = mem(BASE + 32, 18)
    cb = cbuf(b); lib.MatMul(cbuf(a), cb, cb)
    if cb.raw != want: bad2 += 1; print('MatMul alias b')
    # vectors (Vec32: three int32 whose low words are what is used)
    v = struct.pack('<3i', *[rv() for _ in range(3)])
    m = struct.pack('<9h', *rm())
    for name, order in (('MatVecMul32', 'mv'), ('MatTVecMul32', 'vm')):
        o.write(o.dg(BASE), m); o.write(o.dg(BASE + 32), v); o.write(o.dg(BASE + 64), bytes(12))
        if order == 'mv':
            o.call(A[name], [BASE, BASE + 32, BASE + 64])
            cm, cv, cr = cbuf(m), cbuf(v), cbuf(bytes(12)); getattr(lib, name)(cm, cv, cr)
        else:
            o.call(A[name], [BASE + 32, BASE, BASE + 64])
            cm, cv, cr = cbuf(m), cbuf(v), cbuf(bytes(12)); getattr(lib, name)(cv, cm, cr)
        if cr.raw != mem(BASE + 64, 12): bad2 += 1; print(name, m.hex(), v.hex())
    # add / sub with 32 bit wrap
    va = struct.pack('<3i', *[rnd.getrandbits(32) - 2**31 for _ in range(3)])
    vb = struct.pack('<3i', *[rnd.getrandbits(32) - 2**31 for _ in range(3)])
    for name in ('Vec32Add', 'Vec32Sub'):
        o.write(o.dg(BASE), va); o.write(o.dg(BASE + 32), vb)
        o.call(A[name], [BASE, BASE + 32])
        ca, cb = cbuf(va), cbuf(vb); getattr(lib, name)(ca, cb)
        if ca.raw != mem(BASE, 12): bad2 += 1; print(name)
    x, y, z = rv(), rv(), rv() or 1
    pd = rnd.randint(1, 0x7FFF)
    o.write(o.dg(A['proj_dist'] - 0x45A50), struct.pack('<i', pd))
    c.c_int32.in_dll(lib, 'proj_dist').value = pd
    qx, qy = int(x * z / pd), int(y * z / pd)
    if -0x8000 <= qx <= 0x7FFF and -0x8000 <= qy <= 0x7FFF:
        o.write(o.dg(BASE), bytes(12))
        o.call(A['Vec32FromScreen'], [BASE, x, y, z])
        cr = cbuf(bytes(12)); lib.Vec32FromScreen(cr, x, y, z)
        if cr.raw != mem(BASE, 12): bad2 += 1; print('Vec32FromScreen', x, y, z, pd)
print('seg027 vectors: %d mismatches' % bad2)
sys.exit(1 if (bad or bad2) else 0)
