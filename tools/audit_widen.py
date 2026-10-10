#!/usr/bin/env python3
"""Find places where 16 bit arithmetic is widened to 32 bits after the fact.

The original compiler multiplied two ints in 16 bits (wrapping) and then
sign-extended the product when it was stored in a long; C with a 32 bit int
keeps the full product.  This lists statements that assign a `*` or `<<`
expression of 16 bit operands to a 32 bit lvalue without a cast to a 32 bit
type, for review.  Crude: a regex over the source, with declarations scanned
file-wide.
"""
import re, sys, glob

DECL = re.compile(r'^\s*(?:extern\s+|static\s+)?(int16_t|uint16_t|int32_t|uint32_t|unsigned char|char)\s+([^;(]*);')


def types_of(src):
    t = {}
    for line in src.split('\n'):
        m = DECL.match(line)
        if not m:
            continue
        for part in m.group(2).split(','):
            nm = re.match(r'\s*(\*?)\s*(\w+)', part)
            if nm and not nm.group(1):
                t[nm.group(2)] = m.group(1)
    return t


def main():
    for f in sorted(glob.glob('src/seg*.c')):
        src = open(f).read()
        ty = types_of(src)
        for ln, line in enumerate(src.split('\n'), 1):
            m = re.match(r'\s*(?:\w+->|\w+\.)*(\w+)(?:\[[^\]]*\])?\s*([-+]?=)\s*(.+);', line)
            if not m or m.group(1) not in ty or ty[m.group(1)] not in ('int32_t', 'uint32_t'):
                continue
            rhs = m.group(3)
            if re.search(r'\((?:u)?int32_t\)|\(long\)|L\b|\d+UL|0x[0-9A-Fa-f]{5,}', rhs):
                continue
            if not re.search(r'[*]|<<', rhs):
                continue
            ids = re.findall(r'\b[A-Za-z_]\w*\b', rhs)
            known = [i for i in ids if i in ty]
            if known and all(ty[i] in ('int16_t', 'uint16_t', 'char', 'unsigned char') for i in known):
                print('%s:%d: %s' % (f, ln, line.strip()))


main()
