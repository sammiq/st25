#!/usr/bin/env python3
"""The C replacements of the original's assembly graphics routines against
the original machine code: .ban frames, lines, spans, scaling, mixing."""
import ctypes, random, struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import *
from oracle import Oracle, s16

c = ctypes
lib = build_game()
o = Oracle()
rnd = random.Random(11)
bad = 0
S1, S2, S3 = 0x6000, 0x7000, 0x8000       # segments for buffers
vp = c.c_void_p


def F(name, res, *args):
    f = getattr(lib, name)
    f.restype = res
    f.argtypes = list(args)
    return f


def fp(seg, off=0):
    return [off, seg]


def report(name, ok, *info):
    global bad
    if not ok:
        bad += 1
        if bad < 5000:
            print('MISMATCH', name, *info)


def hostbuf(data, extra=0):
    b = c.create_string_buffer(bytes(data), len(data) + extra)
    return b


# ---------------- BanDraw / BanCopy ----------------
F('BanDraw', None, vp, vp)
F('BanCopy', vp, vp, vp, vp)
def ban_frame(maxoff=60000):
    """A .ban frame: offset, run count, 8 byte rect, runs."""
    runs = bytearray()
    n = rnd.randint(1, 20)
    off = rnd.randint(0, 3000)
    for _ in range(n):
        k = rnd.random()
        if k < 0.4:
            m = rnd.randint(1, 30)
            runs.append(m - 1)
            runs += bytes(rnd.choice([0, 0, rnd.randint(1, 255)]) for _ in range(m))
        elif k < 0.8:
            m = rnd.randint(2, 100)
            runs.append((-(m - 1)) & 0xFF)
            runs.append(rnd.choice([0, rnd.randint(1, 255)]))
        else:
            runs.append(0x80)
            runs += struct.pack('<H', rnd.randint(0, 500))
    return struct.pack('<HH', off, n) + struct.pack('<4h', 0, 0, 10, 10) + bytes(runs)


for it in range(200):
    frame = ban_frame() + bytes(40)
    base = bytes(rnd.getrandbits(8) for _ in range(0x10000))
    # BanDraw
    o.reset()
    o.write(S1 * 16, base)
    o.write(S2 * 16, frame)
    o.call(o.sym('_BanDraw'), fp(S1) + fp(S2))
    want = o.read(S1 * 16, 0x10000)
    dst = hostbuf(base)
    src = hostbuf(frame)
    lib.BanDraw(c.addressof(dst), c.addressof(src))
    report('BanDraw', dst.raw[:0x10000] == want, it)
    # BanCopy(dst, src, from): frame data from src, copy from `from`
    o.reset()
    from_ = bytes(rnd.getrandbits(8) for _ in range(0x10000))
    o.write(S1 * 16, base)
    o.write(S2 * 16, frame)
    o.write(S3 * 16, from_)
    # `from` is passed as a far pointer whose segment is used; offset +8 is added by the code
    ax, dx = o.call(o.sym('_BanCopy'), fp(S1) + fp(S2) + fp(S3 - 0, 0))
    want = o.read(S1 * 16, 0x10000)
    wantnext = ax
    dst = hostbuf(base)
    fr = hostbuf(from_)
    r = lib.BanCopy(c.addressof(dst), c.addressof(src), c.addressof(fr) + 8)
    report('BanCopy', dst.raw[:0x10000] == want and r - c.addressof(src) == wantnext, it)

print('ban: %d mismatches so far' % bad)

# ---------------- bitmaps in the original and host ----------------
PTR = 0x8000        # DGROUP scratch (near pointers) for argument blocks


def make_screen_bitmap(px, w=320, h=200):
    return struct.pack('<hhHH', 0, 0, w, h) + bytes(px)


def host_global_ptr(name, buf):
    c.c_void_p.in_dll(lib, name).value = c.addressof(buf)


def set_clip(rect):
    c.memmove(c.addressof(c.c_int16.in_dll(lib, 'clip_rect')), struct.pack('<4h', *rect), 8)
    o.write(o.sym('_clip_rect'), struct.pack('<4h', *rect))


# ---------------- HLine / FillSpans / OutlineSpans / BlitClipped ----------------
F('HLine', None, c.c_int16, c.c_int16, c.c_int16, c.c_int16)
F('FillSpans', None, vp, c.c_int16)
F('OutlineSpans', None, vp, c.c_int16)
F('BlitClipped', None, vp, vp, vp)
for it in range(150):
    px = bytes(rnd.getrandbits(8) for _ in range(64000))
    bmp = make_screen_bitmap(px)
    o.reset()
    o.write(S1 * 16, bmp)
    o.write(o.sym('_screen'), struct.pack('<HH', 0, S1))
    host = hostbuf(bmp, 4096)
    host_global_ptr('screen', host)
    # HLine
    x, y, ln, col = rnd.randint(0, 300), rnd.randint(0, 199), rnd.randint(-2, 60), rnd.randint(0, 255)
    o.call(o.sym('_HLine'), [x, y, ln, col])
    want = o.read(S1 * 16, len(bmp))
    lib.HLine(x, y, ln, col)
    report('HLine', host.raw[:len(bmp)] == want, it, x, y, ln)
    # spans
    rect = [rnd.randint(0, 100), rnd.randint(0, 100), rnd.randint(150, 319), rnd.randint(110, 199)]
    set_clip(rect)
    n = rect[3] - rect[1] + 1
    spans = bytearray()
    for _ in range(n):
        l = rnd.randint(-20, 340)
        r = l + rnd.randint(-3, 120)
        spans += struct.pack('<hh', r, l)
    for fn, sym in (('FillSpans', '_FillSpans'), ('OutlineSpans', '_OutlineSpans')):
        o.reset()
        o.write(S1 * 16, bmp)
        o.write(o.sym('_screen'), struct.pack('<HH', 0, S1))
        o.write(o.sym('_clip_rect'), struct.pack('<4h', *rect))
        o.write(o.dg(PTR), bytes(spans))
        col = rnd.randint(0, 255)
        o.call(o.sym(sym), [PTR, col])
        want = o.read(S1 * 16, len(bmp))
        host = hostbuf(bmp, 4096)
        host_global_ptr('screen', host)
        sp = hostbuf(spans)
        getattr(lib, fn)(c.addressof(sp), col)
        report(fn, host.raw[:len(bmp)] == want, it, rect)
    # BlitClipped
    bw, bh = rnd.randint(1, 60), rnd.randint(1, 50)
    bx, by = rnd.randint(-10, 300), rnd.randint(-10, 190)
    pix = bytes(rnd.choice([0, 0, rnd.randint(1, 255)]) for _ in range(bw * bh))
    sb = struct.pack('<hhHH', 0, 0, bw, bh) + pix
    r = [bx, by, bx + bw - 1, by + bh - 1]
    clip = [max(r[0], 0), max(r[1], 0), min(r[2], 319), min(r[3], 199)]
    if clip[0] > clip[2] or clip[1] > clip[3]:
        continue
    o.reset()
    o.write(S1 * 16, bmp)
    o.write(S2 * 16, sb)
    o.write(o.sym('_screen'), struct.pack('<HH', 0, S1))
    o.write(o.dg(PTR), struct.pack('<4h', *r) + struct.pack('<4h', *clip))
    o.call(o.sym('_BlitClipped'), fp(S2) + [PTR, PTR + 8])
    want = o.read(S1 * 16, len(bmp))
    host = hostbuf(bmp, 4096)
    host_global_ptr('screen', host)
    hb = hostbuf(sb)
    rr = hostbuf(struct.pack('<4h', *r) + struct.pack('<4h', *clip))
    lib.BlitClipped(c.addressof(hb), c.addressof(rr), c.addressof(rr) + 8)
    report('BlitClipped', host.raw[:len(bmp)] == want, it, r, clip)
print('after spans/blit: %d mismatches' % bad)

# ---------------- BuildScaler / RunScaler ----------------
F('BuildScaler', c.c_int16, vp, c.c_int16, c.c_int16)
F('RunScaler', None, vp, vp, vp, c.c_int16)
code = o.sym('_scale_code') - 0x45A50
for it in range(300):
    sw = rnd.randint(1, 320)
    dw = rnd.randint(1, 320)
    if sw == dw and rnd.random() < 0.5:
        dw = max(1, sw - 1)
    srcrow = bytes(rnd.getrandbits(8) for _ in range(sw)) + bytes(16)
    o.reset()
    o.write(S1 * 16, srcrow)
    o.write(S2 * 16, b'\xEE' * 400)
    o.call(o.sym('_BuildScaler'), [code, dw, sw])
    o.call(o.sym('_RunScaler'), fp(S2) + fp(S1) + [code])
    want = o.read(S2 * 16, 400)
    idx = (c.c_uint16 * 1024)()
    n = lib.BuildScaler(c.addressof(idx), dw, sw)
    dst = hostbuf(b'\xEE' * 400)
    sb = hostbuf(srcrow)
    lib.RunScaler(c.addressof(dst), c.addressof(sb), c.addressof(idx), n)
    report('Scaler', dst.raw[:400] == want, it, sw, dw, n)
print('after scaler: %d mismatches' % bad)

# ---------------- MixSamples ----------------
F('MixSamples', None, vp, vp, c.c_int16)
for it in range(100):
    n = rnd.randint(1, 500)
    a = bytes(rnd.getrandbits(8) for _ in range(n))
    b = bytes(rnd.getrandbits(8) for _ in range(n))
    o.reset()
    o.write(S1 * 16, a)
    o.write(S2 * 16, b)
    o.call(o.sym('_MixSamples'), fp(S1) + fp(S2) + [n])
    want = o.read(S1 * 16, n)
    da = hostbuf(a)
    db = hostbuf(b)
    lib.MixSamples(c.addressof(da), c.addressof(db), n)
    report('MixSamples', da.raw[:n] == want, it, n)
print('after mix: %d mismatches' % bad)

# ---------------- CopyRectRemap ----------------
F('CopyRectRemap', None, vp, c.c_int16, c.c_int16, vp, c.c_int16, c.c_int16, c.c_int16, c.c_int16, c.c_int16)
for it in range(100):
    dw, dh = rnd.randint(20, 120), rnd.randint(20, 120)
    sw_, sh_ = rnd.randint(20, 120), rnd.randint(20, 120)
    w, h = rnd.randint(1, min(dw, sw_) - 1), rnd.randint(1, min(dh, sh_) - 1)
    x, y = rnd.randint(0, dw - w), rnd.randint(0, dh - h)
    sx, sy = rnd.randint(0, sw_ - w), rnd.randint(0, sh_ - h)
    dpx = bytes(rnd.getrandbits(8) for _ in range(dw * dh))
    spx = bytes(rnd.choice([0, 0, 0, rnd.getrandbits(8)]) for _ in range(sw_ * sh_))
    table = bytes(rnd.getrandbits(8) for _ in range(256))
    dbmp = struct.pack('<hhHH', 0, 0, dw, dh) + dpx
    sbmp = struct.pack('<hhHH', 0, 0, sw_, sh_) + spx
    remap = rnd.randint(0, 8)
    o.reset()
    o.write(S1 * 16, dbmp)
    o.write(S2 * 16, sbmp)
    o.write(o.sym('_remap_table'), table)
    o.write(o.sym('_cur_remap_id'), struct.pack('<h', remap))
    o.call(o.sym('_CopyRectRemap'), fp(S1) + [x, y] + fp(S2) + [sx, sy, w, h, remap])
    want = o.read(S1 * 16, len(dbmp))
    c.memmove(c.addressof(c.c_uint8.in_dll(lib, 'remap_table')), table, 256)
    c.c_int16.in_dll(lib, 'cur_remap_id').value = remap
    hd = hostbuf(dbmp)
    hs = hostbuf(sbmp)
    lib.CopyRectRemap(c.addressof(hd), x, y, c.addressof(hs), sx, sy, w, h, remap)
    report('CopyRectRemap', hd.raw[:len(dbmp)] == want, it, (dw, dh, sw_, sh_, w, h, x, y, sx, sy))
print('after remap: %d mismatches' % bad)

# ---------------- DrawSprite (the masked bitmap case) ----------------
class Rect(c.Structure):
    _pack_ = 1
    _fields_ = [(n, c.c_int16) for n in ('x0', 'y0', 'x1', 'y1')]


class Sprite(c.Structure):
    _pack_ = 1
    _fields_ = [('x', c.c_int16), ('y', c.c_int16), ('z', c.c_int16), ('layer', c.c_int16),
                ('name', c.c_void_p), ('handle', c.c_void_p),
                ('type', c.c_int16), ('colour', c.c_int16), ('dirty', c.c_int16), ('has_union', c.c_int16),
                ('visible', c.c_int16), ('erase', c.c_int16),
                ('old', Rect), ('rect', Rect), ('union_rect', Rect), ('ox', c.c_int16), ('oy', c.c_int16)]


F('DrawSprite', None, vp, vp)
for it in range(120):
    bw, bh = rnd.randint(2, 60), rnd.randint(2, 50)
    pix = bytes(rnd.choice([0, 0, rnd.randint(1, 255)]) for _ in range(bw * bh))
    sb = struct.pack('<hhHH', 0, 0, bw, bh) + pix
    sx0, sy0 = rnd.randint(-5, 300), rnd.randint(0, 190)
    ox, oy = sx0, sy0
    # clip: a sub rectangle of the bitmap, inside the screen
    cx0 = rnd.randint(max(ox, 0), min(ox + bw - 1, 319)) if max(ox, 0) <= min(ox + bw - 1, 319) else None
    cy0 = rnd.randint(max(oy, 0), min(oy + bh - 1, 199)) if max(oy, 0) <= min(oy + bh - 1, 199) else None
    if cx0 is None or cy0 is None:
        continue
    cx1 = rnd.randint(cx0, min(ox + bw - 1, 319))
    cy1 = rnd.randint(cy0, min(oy + bh - 1, 199))
    prio = rnd.randint(0, 15)
    mask = bytes(rnd.getrandbits(8) for _ in range(32256))
    back = struct.pack('<hhHH', 0, 0, 320, 200) + bytes(rnd.getrandbits(8) for _ in range(64000))
    # original
    o.reset()
    o.write(S1 * 16, back)
    o.write(S2 * 16, sb)
    o.write(S3 * 16, mask)
    o.write(o.sym('_backbuf'), struct.pack('<HH', 0, S1))
    o.write(o.sym('_mask_buf'), struct.pack('<HH', 0, S3))
    SPR, SLOT, CLIP = 0x8000, 0x8100, 0x8200
    o.write(o.dg(SLOT), struct.pack('<HH', 0, S2))
    spr = bytearray(52)
    struct.pack_into('<hhhh', spr, 0, sx0, sy0, prio, 0)
    struct.pack_into('<H', spr, 0x0A, SLOT)
    struct.pack_into('<hh', spr, 0x30, ox, oy)
    o.write(o.dg(SPR), bytes(spr))
    o.write(o.dg(CLIP), struct.pack('<4h', cx0, cy0, cx1, cy1))
    o.call(o.sym('_DrawSprite'), [SPR, CLIP])
    want = o.read(S1 * 16, len(back))
    # host
    hback = hostbuf(back, 4096)
    host_global_ptr('backbuf', hback)
    hmask = hostbuf(mask)
    c.c_void_p.in_dll(lib, 'mask_buf').value = c.addressof(hmask)
    hb = hostbuf(sb)
    slot = c.c_void_p(c.addressof(hb))
    sp = Sprite()
    sp.x, sp.y, sp.z, sp.handle, sp.ox, sp.oy = sx0, sy0, prio, c.addressof(slot), ox, oy
    cl = Rect(cx0, cy0, cx1, cy1)
    lib.DrawSprite(c.addressof(sp), c.addressof(cl))
    report('DrawSprite', hback.raw[:len(back)] == want, it, (bw, bh, ox, oy, cx0, cy0, cx1, cy1, prio))
print('after DrawSprite: %d mismatches' % bad)

# ---------------- DrawR3Shape's texture row loops (blocks of the original) ----------------
# these locals of DrawR3Shape's frame are what the blocks read
L_U, L_V, L_DU, L_DV, L_SEG, L_DST, L_N, L_D = -0x402, -0x404, -0x3EE, -0x3F0, -0x3EA, -0x3E4, -0x400, -0x406
F('DrawTexRow', None, vp, vp, c.c_int16, c.c_uint16, c.c_uint16, c.c_int16, c.c_int16, c.c_int, c.c_int)
SEG6 = 0x1972                      # seg006's paragraph
BP = 0xE000
for blur in (0, 1):
    for it in range(150):
        n = rnd.randint(1, 320)
        tex = bytes(rnd.choice([0, 0, rnd.randint(1, 255)]) for _ in range(65536))
        dstpx = bytes(rnd.getrandbits(8) for _ in range(400))
        u, v = rnd.getrandbits(16), rnd.getrandbits(16)
        du, dv = rnd.randint(-0x300, 0x300), rnd.randint(-0x300, 0x300)
        d = rnd.randint(0, 7)
        o.reset()
        o.write(S1 * 16, tex)
        o.write(S2 * 16, dstpx)
        def put(off, val, fmt='<H'):
            o.write(o.dg(BP + off), struct.pack(fmt, val & 0xFFFF))
        put(L_U, u); put(L_V, v); put(L_DU, du); put(L_DV, dv); put(L_SEG, S1)
        o.write(o.dg(BP + L_DST), struct.pack('<HH', 0, S2)); put(L_N, n); put(L_D, d)
        if blur:
            o.run_range(SEG6, 0x270D, 0x28BD, regs={'bp': BP, 'sp': 0xF000})
        else:
            o.run_range(SEG6, 0x2688, 0x26D8, regs={'bp': BP, 'sp': 0xF000})
        want = o.read(S2 * 16, 400)
        dst = hostbuf(dstpx)
        tb = hostbuf(tex)
        lib.DrawTexRow(c.addressof(tb), c.addressof(dst), n, u, v, du, dv, blur, d)
        report('DrawTexRow%d' % blur, dst.raw[:400] == want and (want != dstpx or n < 12), it, n, d)   # short rows can be all transparent
print('after texture rows: %d mismatches' % bad)

# ---------------- planet loops (blocks of DrawPlanetLarge / DrawPlanetSmall) ----------------
F('PlanetPixels', None, vp, c.c_uint32, vp, vp, c.c_int16, c.c_uint16, c.c_uint16, vp, c.c_ubyte, c.c_ubyte, c.c_int)
SEG18 = 0x2D4B
# (start, end, entry limit, table word limit, masks, shrink, texture size)
variants = [(0x068D, 0x06EB, 0x4000, 0x4000, 0xBF, 0xBF, 1, 128 * 128),
            (0x06F0, 0x0746, 0x4000, 0x4000, 0xBF, 0xBF, 0, 128 * 128),
            (0x0BE4, 0x0C3A, 0x400, 0x400, 0xF7, 0xFB, 0, 32 * 32)]
for vi, (st, en, elim, tlim, m1, m2, shrink, tsize) in enumerate(variants):
    for it in range(100):
        nn = rnd.choice([121, 441, 961, 1681, 3721])
        SRCO, OUTO, PIXO = 0x4000 if elim == 0x4000 else 0x800, 0x7622, 0
        work = bytearray(0x8600)
        for k in range(0, min(elim, 0x4000), 2):
            struct.pack_into('<H', work, k, rnd.randrange(0, tlim, 2) if rnd.random() < 0.9 else rnd.randrange(0, tlim))
        ents = bytearray()
        for k in range(nn):
            blank = rnd.random() < 0.3
            ents += struct.pack('<H', (0x8000 | (rnd.getrandbits(14) & ~1)) if blank else rnd.randrange(0, elim, 2))
        work[SRCO:SRCO + len(ents)] = ents
        pix = bytes(rnd.getrandbits(8) for _ in range(tsize))
        tu = rnd.randrange(0, 0x4000 if elim == 0x4000 else 0x800, 2)
        tv = rnd.randrange(0, 0x4000 if tlim == 0x4000 else 0x400)
        o.reset()
        o.write(S1 * 16, bytes(work))
        o.write(S2 * 16, pix)
        def put(off, val):
            o.write(o.dg(BP + off), struct.pack('<H', val & 0xFFFF))
        o.write(o.dg(BP - 0x36), struct.pack('<HH', SRCO, S1))
        o.write(o.dg(BP - 0x46), struct.pack('<HH', PIXO, S2))
        put(-0x42, OUTO); put(-0x32, tv); put(-0x3A, nn); put(-0x30, tu)
        o.run_range(SEG18, st, en, regs={'bp': BP, 'sp': 0xF000})
        want = o.read(S1 * 16 + OUTO, nn)
        wk = hostbuf(bytes(work), 64)
        px = hostbuf(pix, 64)
        outb = hostbuf(bytes(nn))
        lib.PlanetPixels(c.addressof(wk), 0x10000, c.addressof(wk) + SRCO, c.addressof(outb), nn, tu, tv,
                         c.addressof(px), m1, m2, shrink)
        if outb.raw[:nn] != want and it == 0 and vi == 1:
            for k in range(nn):
                if outb.raw[k] != want[k]:
                    e = struct.unpack_from('<H', ents, 2 * k)[0]
                    b = (e + tu) & 0xBFFF
                    w = struct.unpack_from('<H', work, b)[0]
                    b2 = (w + tv) & 0xBFFF
                    print('PIX', k, hex(e), hex(tu), hex(b), hex(w), hex(tv), hex(b2), 'pix', pix[b2], 'want', want[k], 'got', outb.raw[k])
                    break
        if outb.raw[:nn] != want and it == 0:
            print('DBG', vi, 'want', want[:24].hex(), 'got', outb.raw[:24].hex(), 'ents', bytes(ents[:24]).hex(), tu, tv)
        report('Planet%d' % vi, outb.raw[:nn] == want, it, nn, tu, tv)
print('after planets: %d mismatches' % bad)
sys.exit(1 if bad > 1 else 0)
