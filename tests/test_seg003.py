#!/usr/bin/env python3
"""seg003 decompression (Unpack, UnpackChunk) C port against the original."""
import ctypes, random, struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import *
from oracle import Oracle

c = ctypes
lib = build('seg003', ['src/seg003.c', 'src/seg013.c', 'platform/plat_files.c', 'tests/stub_platform.c'])
o = Oracle()
lib.UnpackChunk.restype = c.c_int16
lib.Unpack.argtypes = [c.c_void_p, c.c_void_p, c.c_uint16]
lib.UnpackChunk.argtypes = [c.c_void_p, c.c_void_p, c.c_uint16]
rnd = random.Random(7)
A = {n: o.sym('_' + n) for n in ('Unpack', 'UnpackChunk')}


def compress(data, lit_bias=0.3):
    """The game's LZSS: 8 items per flag byte, set bit = literal, clear = 16 bit
    back reference (12 bit distance, 4 bit length - 3)."""
    out = bytearray()
    items = []
    i = 0
    n = len(data)
    while i < n:
        best = (0, 0)
        if i >= 2 and rnd.random() > lit_bias:
            for dist in range(2, min(i, 4095) + 1):
                l = 0
                while l < 18 and i + l < n and data[i + l - dist] == data[i + l]:
                    l += 1
                if l > best[0]:
                    best = (l, dist)
        if best[0] >= 3:
            items.append(('m', best[1], best[0]))
            i += best[0]
        elif i >= 1 and n - i >= 3 and data[i:i + 3] == bytes([data[i - 1]]) * 3 and rnd.random() < 0.5:
            l = 3
            while l < 18 and i + l < n and data[i + l] == data[i - 1]:
                l += 1
            items.append(('m', 1, l))          # fill with the previous byte
            i += l
        else:
            items.append(('l', data[i]))
            i += 1
    for k in range(0, len(items), 8):
        grp = items[k:k + 8]
        flags = 0
        body = bytearray()
        for j, it in enumerate(grp):
            if it[0] == 'l':
                flags |= 1 << j
                body.append(it[1])
            else:
                body += struct.pack('<H', (it[1] << 4) | (it[2] - 3))
        out.append(flags)
        out += body
    return bytes(out)


def gen(n):
    kind = rnd.randint(0, 3)
    if kind == 0:
        return bytes(rnd.getrandbits(8) for _ in range(n))
    if kind == 1:
        return bytes((i // rnd.randint(1, 9)) & 0xFF for i in range(n))
    if kind == 2:
        base = bytes(rnd.getrandbits(8) for _ in range(rnd.randint(4, 40)))
        return (base * (n // len(base) + 1))[:n]
    return bytes(rnd.choice([0, 0, 0, 1, 255, rnd.getrandbits(8)]) for _ in range(n))


SRC, DST, PDST = 0x7000, 0x8000, 0x9000     # segments / DGROUP offset
bad = 0
for it in range(300):
    data = gen(rnd.randint(20, 3000))
    comp = compress(data, rnd.choice([0.0, 0.3, 0.8]))
    if len(comp) < 20:
        continue
    # whole-stream Unpack
    o.reset()
    o.write(SRC * 16, comp)
    o.call(A['Unpack'], [0, SRC, 0, DST, len(comp)])
    want = o.read(DST * 16, len(data))
    src = c.create_string_buffer(comp, len(comp) + 64)
    dst = c.create_string_buffer(len(data) + 64)
    lib.Unpack(c.addressof(src), c.addressof(dst), len(comp))
    if dst.raw[:len(data)] != want:
        bad += 1
        print('Unpack mismatch', it, len(comp))
    if want != data:
        print('NOTE: original disagrees with the compressor model', it)
    # UnpackChunk on a prefix
    n = rnd.randint(17, len(comp))
    o.reset()
    o.write(SRC * 16, comp)
    o.write(o.dg(PDST), struct.pack('<HH', 0, DST))
    ax, _ = o.call(A['UnpackChunk'], [0, SRC, PDST, n])
    off, seg = struct.unpack('<HH', o.read(o.dg(PDST), 4))
    produced = (seg * 16 + off) - DST * 16
    want_out = o.read(DST * 16, produced)
    dst = c.create_string_buffer(len(data) + 4096)
    pd = c.c_void_p(c.addressof(dst))
    got = lib.UnpackChunk(c.addressof(src), c.addressof(pd), n)
    gotprod = pd.value - c.addressof(dst)
    if got != ax or gotprod != produced or dst.raw[:produced] != want_out:
        bad += 1
        print('UnpackChunk mismatch', it, n, 'consumed', got, ax, 'produced', gotprod, produced)
print('seg003: %d mismatches' % bad)
sys.exit(1 if bad else 0)
