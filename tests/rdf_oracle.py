"""Run a room script's original machine code under Unicorn, with the game call
replaced by a recorder, for tests/test_rooms.py."""
import os, struct, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'tools'))
from unicorn import *
from unicorn.x86_const import *

ROOM_SIZE = 0x129
BASE = 0x100                                  # where the room state sits in the scripts' data segment
IMG_SEG, DS_SEG, SS_SEG, STUB_SEG = 0x6000, 0x3000, 0x4000, 0x2000
SEED = [0]

# words a call of each command reads (command first)
ARITY = {0: 5, 1: 1, 2: 5, 3: 1, 4: 1, 5: 0, 6: 2, 7: 1, 8: 4, 9: 1, 10: 1, 11: 1, 12: 1, 13: 1, 14: 2,
         15: 1, 16: 2, 17: 3, 18: 0, 19: 1, 20: 2, 21: 1, 22: 0, 23: 0}


def svc_result(n):
    """What the n-th game call of a run returns: small (it is often a menu choice)."""
    return ((n * 2654435761 + 12345 + SEED[0] * 977) >> 7) & 3


def run_uc(image, entry, room):
    """-> (calls, image after, room after); raises UcError if the machine does not come back."""
    uc = Uc(UC_ARCH_X86, UC_MODE_16)
    uc.mem_map(0, 0x200000)
    img = bytearray(image)
    struct.pack_into('<HH', img, 0, 0, STUB_SEG)           # the game call: STUB_SEG:0
    struct.pack_into('<H', img, 4, BASE)                   # rdf->state
    uc.mem_write(IMG_SEG * 16, bytes(img))
    uc.mem_write(STUB_SEG * 16, b'\xCB')                    # retf
    TRAP = 0xFFFF * 16 + 0xFFFF
    uc.mem_write(TRAP, b'\xF4')
    uc.mem_write(DS_SEG * 16 + BASE, bytes(room))
    uc.mem_write(SS_SEG * 16, bytes([0xA5]) * 0x10000)
    sp = 1024 - 4
    uc.mem_write(SS_SEG * 16 + sp, struct.pack('<HH', 0xFFFF, 0xFFFF))
    calls = []

    def hook(uc_, addr, size, ud):
        spv = uc.reg_read(UC_X86_REG_SP)
        words = struct.unpack('<6H', bytes(uc.mem_read(SS_SEG * 16 + spv + 4, 12)))
        calls.append(tuple(words))
        if words[0] == 6:                      # GotoRoom: the room is left
            uc.emu_stop()
            return
        r = svc_result(len(calls))
        uc.reg_write(UC_X86_REG_AX, r & 0xFFFF)
        uc.reg_write(UC_X86_REG_DX, r >> 16)

    uc.hook_add(UC_HOOK_CODE, hook, begin=STUB_SEG * 16, end=STUB_SEG * 16)
    # CallScript's state: bx = si = the room, ax = ds, es = ds
    for reg, v in ((UC_X86_REG_DS, DS_SEG), (UC_X86_REG_ES, DS_SEG), (UC_X86_REG_SS, SS_SEG),
                   (UC_X86_REG_SP, sp), (UC_X86_REG_SI, BASE), (UC_X86_REG_BX, BASE), (UC_X86_REG_CS, IMG_SEG),
                   (UC_X86_REG_AX, DS_SEG), (UC_X86_REG_CX, 0), (UC_X86_REG_DX, 0), (UC_X86_REG_DI, 0)):
        uc.reg_write(reg, v)
    uc.reg_write(UC_X86_REG_FLAGS, 2)
    uc.emu_start(IMG_SEG * 16 + entry, TRAP, count=60_000)
    if uc.reg_read(UC_X86_REG_CS) * 16 + uc.reg_read(UC_X86_REG_IP) != TRAP and not (calls and calls[-1][0] == 6):
        raise UcError(0)                                     # did not come back
    out = bytearray(uc.mem_read(IMG_SEG * 16, len(image)))
    out[0:6] = image[0:6]
    return calls, bytes(out), bytes(uc.mem_read(DS_SEG * 16 + BASE, len(room)))
