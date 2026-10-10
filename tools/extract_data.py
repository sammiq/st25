#!/usr/bin/env python3
"""Dump every file in DATA.001 (via DATA.DIR / DATA.RUN), unpacked.

    tools/extract_data.py orig/CD/TREKCD out_dir

Also usable as a module (unpack(), read_dir()) by the port's test tools.
"""
import os, struct, sys


def unpack(src, size):
    """The game's LZSS: flag byte per 8 items (set bit = literal, clear = 16 bit
    back reference: 12 bit distance, 4 bit length - 3); a 0xFF flag byte is 8
    literals.  Distance 0/1 is a fill with the previous byte."""
    out = bytearray()
    i = 0
    n = len(src)
    while i < n and len(out) < size:
        flags = src[i]; i += 1
        for bit in range(8):
            if i >= n:
                break
            if flags & (1 << bit):
                out.append(src[i]); i += 1
            else:
                code = src[i] | (src[i + 1] << 8); i += 2
                cnt = (code & 15) + 3
                dist = code >> 4
                if dist < 2:
                    out.extend(bytes([out[-1]]) * cnt)
                else:
                    for _ in range(cnt):
                        out.append(out[-dist])
            if len(out) >= size:
                break
    return bytes(out[:size])


def read_dir(path):
    d = open(os.path.join(path, 'DATA.DIR'), 'rb').read()
    run = open(os.path.join(path, 'DATA.RUN'), 'rb').read()
    ents = []
    for o in range(0, len(d) // 14 * 14, 14):
        name = d[o:o + 8].split(b'\0')[0].decode('latin1')
        ext = d[o + 8:o + 11].decode('latin1').rstrip('\0')
        off, hi = struct.unpack_from('<HB', d, o + 11)
        ents.append((name, ext, off, hi))
    files = []  # (name, ext, pos)
    for name, ext, off, hi in ents:
        if hi & 0x80:
            cnt = hi & 0x7F
            pos = struct.unpack_from('<I', run, off)[0] & 0x7FFFFE
            for i in range(cnt):
                files.append((name[:-1] + chr(ord(name[-1]) + i), ext, pos))
                if i + 1 < cnt:
                    pos += struct.unpack_from('<H', run, off + 3 + 2 * i)[0] & 0xFFFE
        else:
            files.append((name, ext, ((hi << 16) + off) & 0x7FFFFFFE))
    return files


def main():
    src, dst = sys.argv[1:3]
    os.makedirs(dst, exist_ok=True)
    data = open(os.path.join(src, 'DATA.001'), 'rb').read()
    for name, ext, pos in read_dir(src):
        size, packed = struct.unpack_from('<HH', data, pos)
        body = data[pos + 4:pos + 4 + packed]
        try:
            out = unpack(body, size) if packed != size else body
        except IndexError:
            print('bad', name, ext, file=sys.stderr)
            continue
        open(os.path.join(dst, f'{name}.{ext}'), 'wb').write(out)


if __name__ == '__main__':
    main()
