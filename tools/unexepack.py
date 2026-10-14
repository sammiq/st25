#!/usr/bin/env python3
"""Unpack an EXEPACK-compressed DOS EXE (the original STARTREK.EXE) into the plain
image the oracle tests load:  unexepack.py packed.exe unpacked.exe

The packed program is one image whose last segment is the EXEPACK header and
unpack stub; the code and data before it are compressed backwards, as a run of
commands (fill byte / copy bytes) each led by a 16 bit length."""
import struct
import sys


def unpack(d: bytes) -> bytes:
    sig, last, pages, nreloc, hdrpar, minalloc, maxalloc, ss, sp, csum, ip, cs, reloff, ovl = struct.unpack('<2s13H', d[:28])
    if sig not in (b'MZ', b'ZM'):
        raise SystemExit('not an EXE')
    base = hdrpar * 16
    end = (pages - 1) * 512 + (last or 512)
    hdr = base + cs * 16                       # the EXEPACK header, at the code segment of the entry point
    real_ip, real_cs, _mem, stub_size, real_sp, real_ss, dest_len = struct.unpack('<7H', d[hdr:hdr + 14])
    if d[hdr + 14:hdr + 16] != b'RB':
        raise SystemExit('not an EXEPACK file')
    out = bytearray(dest_len * 16)
    src = hdr
    while d[src - 1] == 0xFF:                  # padding
        src -= 1
    opos = len(out)
    while True:
        op = d[src - 1]
        n = d[src - 3] | d[src - 2] << 8
        src -= 3
        kind = op & 0xFE
        if kind == 0xB0:                       # fill
            fill = d[src - 1]
            src -= 1
            out[opos - n:opos] = bytes([fill]) * n
        elif kind == 0xB2:                     # copy
            out[opos - n:opos] = d[src - n:src]
            src -= n
        else:
            raise SystemExit('bad command %02X' % op)
        opos -= n
        if op & 1:
            break
    out[:opos] = d[base:base + opos]          # what precedes the compressed part is stored as it is
    # the relocation table follows the stub: 16 blocks of (count, offsets), one per 4K of segment
    rel = None
    for cand in range(hdr + 0x10, min(hdr + stub_size + 0x40, len(d)) - 32):
        p = cand
        ok = True
        for c in range(16):
            if p + 2 > len(d):
                ok = False
                break
            p += 2 + struct.unpack('<H', d[p:p + 2])[0] * 2
        if ok and p == end:
            rel = cand
            break
    if rel is None:
        raise SystemExit('relocation table not found')
    relocs = []
    p = rel
    for blk in range(16):
        cnt = struct.unpack('<H', d[p:p + 2])[0]
        p += 2
        for i in range(cnt):
            off = struct.unpack('<H', d[p:p + 2])[0]
            p += 2
            relocs.append((off, blk * 0x1000))
    hdrsize = (28 + 4 * len(relocs) + 15) // 16 * 16
    hdrsize = max(hdrsize, 32)
    total = hdrsize + len(out)
    h = struct.pack('<2s13H', b'MZ', total % 512, (total + 511) // 512, len(relocs), hdrsize // 16, minalloc, maxalloc,
                    real_ss, real_sp, 0, real_ip, real_cs, 28, 0)
    h += b''.join(struct.pack('<HH', off, seg) for off, seg in relocs)
    h += bytes(hdrsize - len(h))
    return bytes(h) + bytes(out)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    open(sys.argv[2], 'wb').write(unpack(open(sys.argv[1], 'rb').read()))
