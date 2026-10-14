"""Helpers shared by the oracle tests: build C pieces into a shared library
and load them with ctypes."""
import ctypes, os, subprocess, sys, tempfile
ROOT = os.path.join(os.path.dirname(__file__), '..')
sys.path.insert(0, os.path.join(ROOT, 'tools'))
SCRATCH = os.environ.get('SCRATCH', '/tmp')


def build(name, sources, extra=()):
    out = os.path.join(SCRATCH, 'lib%s.so' % name)
    cmd = ['gcc', '-std=gnu11', '-shared', '-fPIC', '-O1', '-g', '-fsigned-char', '-fwrapv', '-fno-strict-aliasing',
           '-w', '-I', os.path.join(ROOT, 'include'), '-I', os.path.join(ROOT, 'audio'), '-o', out]
    cmd += [os.path.join(ROOT, s) if not os.path.isabs(s) else s for s in sources] + list(extra)
    subprocess.check_call(cmd)
    return ctypes.CDLL(out)


def build_game(name='game'):
    """The whole game (headless host) as a shared library."""
    import glob
    srcs = sorted(glob.glob(os.path.join(ROOT, 'src/*.c'))) + sorted(glob.glob(os.path.join(ROOT, 'src/rooms/*.c'))) + sorted(glob.glob(os.path.join(ROOT, 'audio/*.c'))) + [os.path.join(ROOT, 'audio/nuked/opl3.c')] \
        + [os.path.join(ROOT, 'platform/plat_files.c'), os.path.join(ROOT, 'platform/plat_null.c'),
           os.path.join(ROOT, 'platform/plat_audio_null.c')]
    return build(name, srcs, ['-lm'])
