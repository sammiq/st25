#!/usr/bin/env python3
"""One-shot converter: the original's 16-bit C types to fixed width ones.

    int -> int16_t   unsigned -> uint16_t   long -> int32_t
    unsigned long -> uint32_t   far/near/_far/_near/_interrupt -> (removed)
    %ld/%lu/%lx in string literals -> %d/%u/%x

Comments, string literals and `_asm { }` blocks are left alone otherwise.
Run it on files that still carry the original types; it is idempotent.
"""
import re, sys

TOKEN = re.compile(r'''
    (?P<comment>/\*.*?\*/|//[^\n]*)
  | (?P<string>"(?:\\.|[^"\\])*")
  | (?P<char>'(?:\\.|[^'\\])+')
  | (?P<ident>[A-Za-z_][A-Za-z_0-9]*)
  | (?P<ws>\s+)
  | (?P<other>.)
''', re.S | re.X)

DROP = {'far', '_far', '__far', 'near', '_near', '__near', 'huge', '_huge', '_interrupt', 'interrupt', '_cdecl', 'cdecl'}


def convert(src):
    toks = [(m.lastgroup, m.group()) for m in TOKEN.finditer(src)]
    out = []
    i = 0
    n = len(toks)

    def nxt(j):
        """index of next non-ws, non-comment token at/after j"""
        while j < n and toks[j][0] in ('ws', 'comment'):
            j += 1
        return j

    while i < n:
        kind, text = toks[i]
        if kind == 'string':
            text = re.sub(r'%(-?\d*)l([dxuXi])', r'%\1\2', text)
            out.append(text)
        elif kind == 'ident':
            if text in ('_asm', '__asm'):
                # copy the whole block (or single statement) verbatim
                j = nxt(i + 1)
                if toks[j][1] == '{':
                    depth = 0
                    while True:
                        if toks[j][1] == '{': depth += 1
                        elif toks[j][1] == '}':
                            depth -= 1
                            if depth == 0: break
                        j += 1
                    out.append(''.join(t for _, t in toks[i:j + 1]))
                    i = j
                else:
                    out.append(text)
            elif text in DROP:
                # swallow one following space so `char far *p` -> `char *p`
                j = i + 1
                if j < n and toks[j][0] == 'ws':
                    i = j
                # no output
            elif text == 'unsigned':
                j = nxt(i + 1)
                nt = toks[j][1] if j < n else ''
                if nt == 'long':
                    j2 = nxt(j + 1)
                    if j2 < n and toks[j2][1] == 'int':
                        j = j2
                    out.append('uint32_t'); i = j
                elif nt == 'char':
                    out.append(text)
                elif nt == 'short':
                    j2 = nxt(j + 1)
                    if j2 < n and toks[j2][1] == 'int':
                        j = j2
                    out.append('uint16_t'); i = j
                elif nt == 'int':
                    out.append('uint16_t'); i = j
                else:
                    out.append('uint16_t')
            elif text == 'long':
                j = nxt(i + 1)
                if j < n and toks[j][1] == 'int':
                    i = j
                out.append('int32_t')
            elif text == 'short':
                j = nxt(i + 1)
                if j < n and toks[j][1] == 'int':
                    i = j
                out.append('int16_t')
            elif text == 'int':
                out.append('int16_t')
            else:
                out.append(text)
        else:
            out.append(text)
        i += 1
    return ''.join(out)


def main():
    for p in sys.argv[1:]:
        s = open(p).read()
        t = convert(s)
        if t != s:
            open(p, 'w').write(t)


if __name__ == '__main__':
    main()
