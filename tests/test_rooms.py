#!/usr/bin/env python3
"""The room scripts translated to C (src/rooms, from tools/rdf2c.py) against
the machine code in the room files: every script of each room is run under
Unicorn and as C with the same room state and game call results, and the
calls made, the room state and the file image left behind are compared.

    tests/test_rooms.py [ROOM ...]
"""
import ctypes, os, random, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(__file__))
from oracle_lib import ROOT, SCRATCH
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import rdf_oracle as ro
from rdf import load_room, room_names
from unicorn import UcError

c = ctypes
rnd = random.Random(5)


def build_room(name):
    out = os.path.join(SCRATCH, 'room_%s.so' % name.lower())
    cmd = ['gcc', '-std=gnu11', '-shared', '-fPIC', '-O1', '-w', '-fsigned-char', '-fwrapv', '-fno-strict-aliasing',
           '-I', os.path.join(ROOT, 'include'), '-DTABLE=%s_scripts' % name.lower(), '-o', out,
           os.path.join(ROOT, 'src/rooms/%s.c' % name.lower()), os.path.join(ROOT, 'tests/rooms_stub.c')]
    subprocess.check_call(cmd)
    return c.CDLL(out)


def room_state():
    r = bytearray(ro.ROOM_SIZE)
    for i in range(ro.ROOM_SIZE):
        r[i] = rnd.choice([0, 0, 1, 1, 2, 3, rnd.getrandbits(8)])
    return bytes(r)


def run_c(lib, idx, image, room, seed):
    img = c.create_string_buffer(bytes(image).ljust(0x10000, b'\0'), 0x10000)
    rm = c.create_string_buffer(bytes(room), len(room))
    lib.run_script(idx, c.addressof(img), c.addressof(rm), seed)
    n = c.c_int.in_dll(lib, 'log_n').value
    words = (c.c_int16 * 6 * 512).in_dll(lib, 'log_words')
    calls = [tuple(w & 0xFFFF for w in words[i]) for i in range(min(n, 512))]
    return calls, img.raw[:len(image)], rm.raw


def same_calls(a, b):
    if len(a) != len(b):
        return False
    for x, y in zip(a, b):
        n = ro.ARITY.get(x[0], 5) + 1
        if any(u != v and u != 0x7FFF for u, v in zip(x[:n], y[:n])):
            return False
    return True


def test_room(name):
    lib = build_room(name)
    lib.run_script.argtypes = [c.c_int, c.c_void_p, c.c_void_p, c.c_int]
    image = load_room(name)
    image = bytearray(image)
    n = lib.nscripts()
    ok = bad = skipped = 0
    for i in range(n):
        entry = lib.script_offset(i)
        for trial in range(3):
            room = room_state()
            want = None
            for seed in range(trial * 8, trial * 8 + 8):
                ro.SEED[0] = seed
                try:
                    want = ro.run_uc(image, entry, room)
                    break
                except UcError:
                    pass
            if want is None:
                skipped += 1
                continue
            got = run_c(lib, i, image, room, ro.SEED[0])
            if not same_calls(got[0], want[0]) or got[1] != want[1] or got[2] != want[2]:
                bad += 1
                print('MISMATCH %s %04X trial %d: calls %d/%d image %s room %s' % (
                    name, entry, trial, len(got[0]), len(want[0]), got[1] != want[1], got[2] != want[2]))
                if got[2] != want[2]:
                    print('   room', [(k, got[2][k], want[2][k]) for k in range(len(room)) if got[2][k] != want[2][k]][:6])
                if got[1] != want[1]:
                    print('   image', [(hex(k), got[1][k], want[1][k]) for k in range(len(image)) if got[1][k] != want[1][k]][:6])
                if not same_calls(got[0], want[0]):
                    print('   C', got[0][:4], '\n   U', want[0][:4])
            else:
                ok += 1
    print('%s: %d scripts, %d runs ok, %d mismatches, %d runs the machine would not survive' % (name, n, ok, bad, skipped))
    return bad


def main():
    names = [a.upper() for a in sys.argv[1:]] or room_names()
    bad = sum(test_room(n) for n in names)
    sys.exit(1 if bad else 0)


main()
