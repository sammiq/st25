#!/usr/bin/env python3
"""Compile a C file with Microsoft C 6.00A (or assemble a .asm file with
MASM 5.0) under emu2 and convert the result to ELF.

    cc.py SRC OUT.o [--omf OUT.obj] -- [CL flags...]

The project root is mapped to D: and tools/C600 to C:, so every path the
compiler sees must be inside the project and use 8.3 file names.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import omf  # noqa: E402
import omf2elf  # noqa: E402
from project import ROOT  # noqa: E402

MSC = ROOT / "tools/C600"
MASM = ROOT / "tools/MASM50"
EMU2 = os.environ.get("EMU2", "emu2")


def dos_path(p: Path) -> str:
    rel = p.resolve().relative_to(ROOT)
    for part in rel.parts:
        stem, _, ext = part.partition(".")
        if len(stem) > 8 or len(ext) > 3:
            sys.exit(f"cc: '{rel}' is not a DOS 8.3 path")
    return "D:\\" + "\\".join(rel.parts).upper()


def main() -> None:
    argv = sys.argv[1:]
    flags: list[str] = []
    if "--" in argv:
        i = argv.index("--")
        argv, flags = argv[:i], argv[i + 1:]
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("src")
    ap.add_argument("out")
    ap.add_argument("--omf", help="where to keep the OMF object (default: OUT with .obj)")
    args = ap.parse_args(argv)

    src = Path(args.src)
    out = Path(args.out)
    obj = Path(args.omf) if args.omf else out.with_suffix(".obj")
    obj.parent.mkdir(parents=True, exist_ok=True)
    out.parent.mkdir(parents=True, exist_ok=True)
    # emu2 creates lower-case file names
    produced = obj.parent / obj.name.lower()
    produced.unlink(missing_ok=True)

    if src.suffix.lower() == ".asm":
        # /Ml: case sensitive names, /t: quiet unless there are errors
        env = dict(os.environ, EMU2_DRIVE_C=str(MASM), EMU2_DRIVE_D=str(ROOT))
        cmd = [EMU2, str(MASM / "BIN/MASM.EXE"), "/Ml", "/t", *flags,
               f"{dos_path(src)},{dos_path(obj)};", "--", "INCLUDE=C:\\INC"]
    else:
        env = dict(os.environ, EMU2_DRIVE_C=str(MSC), EMU2_DRIVE_D=str(ROOT))
        cmd = [EMU2, str(MSC / "BIN/CL.EXE"), "/c", "/nologo", *flags,
               f"/Fo{dos_path(obj)}", dos_path(src),
               "--", "INCLUDE=C:\\INCLUDE", "PATH=C:\\BIN", "TMP=D:\\BUILD"]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True, errors="replace")
    # CL echoes the source file name; only show real diagnostics
    msgs = [ln for ln in res.stdout.splitlines() if ln.strip() and ln.strip() != dos_path(src)]
    if msgs:
        print("\n".join(msgs), file=sys.stderr)
    if res.returncode != 0 or not produced.exists():
        sys.exit(res.returncode or 1)
    if produced != obj:
        produced.rename(obj)

    mod = omf.parse_obj(obj.read_bytes())
    omf2elf.convert(mod).write(str(out))


if __name__ == "__main__":
    main()
