#!/usr/bin/env python3
"""Make a contact sheet PNG from several PPM screenshots: ppmsheet.py out.png cols a.ppm b.ppm ..."""
import sys, zlib, struct

def load(p):
    d = open(p, 'rb').read()
    parts = d.split(b'\n', 3)
    w, h = map(int, parts[1].split())
    return w, h, parts[3]

def main():
    out, cols = sys.argv[1], int(sys.argv[2])
    imgs = [load(p) for p in sys.argv[3:]]
    w, h = imgs[0][0], imgs[0][1]
    rows = (len(imgs) + cols - 1) // cols
    W, H = w * cols + (cols - 1) * 2, h * rows + (rows - 1) * 2
    canvas = [bytearray(b'\x40\x40\x40' * W) for _ in range(H)]
    for i, (iw, ih, pix) in enumerate(imgs):
        ox, oy = (i % cols) * (w + 2), (i // cols) * (h + 2)
        for y in range(ih):
            canvas[oy + y][ox * 3:(ox + iw) * 3] = pix[y * iw * 3:(y + 1) * iw * 3]
    raw = b''.join(b'\0' + bytes(r) for r in canvas)
    def chunk(t, d):
        c = struct.pack('>I', len(d)) + t + d
        return c + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 2, 0, 0, 0)) \
        + chunk(b'IDAT', zlib.compress(raw, 6)) + chunk(b'IEND', b'')
    open(out, 'wb').write(png)

main()
