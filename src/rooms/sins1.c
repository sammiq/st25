/* SINS1.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/sins1.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * SINS1.RDF - "Sins of Omission", inside the security door: a huge inner
 * door with a keypad and display, and a lens above it. The right code
 * opens it; a wrong but meaningful one fires an anti-intruder laser.
 * Scott reports a virus that has taken the Enterprise's phasers and
 * tractor beams.
 *
 * Actors: 8 the door, 9 the laser.
 * Objects: 8 the lock, 20h the door, 21h the keypad, 22h the display,
 * 23h the lens. Exits: 0 room 2, 1 room 0.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - Two scripts answer talking to Spock; the second never runs.
 * - The conversation at 00D8 is built at run time.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_sscans = 0x00CA,
    N_mscans = 0x00D1,
    talk_buf = 0x00D8,
    N_Capt_Kirk = 0x012D,
    N_Dr_McCoy = 0x0138,
    N_Mr_Spock = 0x0142,
    N_Ensign_Mosher = 0x014C,
    N_Lt_Uhura = 0x015A,
    N_Mr_Scott = 0x0164,
    N_SIN1LOOP = 0x0207,
    N_s1dro = 0x0231,
    N_sins12 = 0x024E,
    N_sins1 = 0x0266,
    N_s1ndro = 0x0281,
    T_SIN1N001 = 0x02E2,
    T_SIN1N009 = 0x032D,
    T_SIN1N010 = 0x037D,
    T_SIN1N003 = 0x03F6,
    T_SIN1N011 = 0x0441,
    T_SIN1_020 = 0x048A,
    N_sscann = 0x0506,
    T_SIN1_010 = 0x053D,
    N_sscann_2 = 0x0577,
    T_SIN1_021 = 0x05AE,
    N_sscann_3 = 0x05EC,
    T_SIN1_009 = 0x0623,
    N_sscann_4 = 0x065B,
    T_SIN1_019 = 0x0692,
    T_SIN1_024 = 0x0709,
    N_sscann_5 = 0x0735,
    T_SIN1_022 = 0x076C,
    T_SIN1_023 = 0x07BC,
    T_SIN1N007 = 0x0811,
    T_SIN1_028 = 0x0893,
    T_SIN1_012 = 0x08DC,
    T_SIN1_001 = 0x0930,
    N_01210 = 0x099A,
    N_EFX14S = 0x09D9,
    N_susehn = 0x09F1,
    N_EFX14S_2 = 0x0A31,
    N_susehn_2 = 0x0A49,
    N_EFX14S_3 = 0x0A68,
    N_susehn_3 = 0x0A80,
    N_sins12_2 = 0x0ACE,
    N_s1door = 0x0AE4,
    N_HUGEDOO2 = 0x0B02,
    T_SIN1_011 = 0x0B2C,
    T_SIN1_017 = 0x0BDA,
    N_s1ph1 = 0x0CA0,
    T_SIN1_016 = 0x0CE5,
    T_SIN1_018 = 0x0D64,
    T_SIN1_S10 = 0x0DFF,
    T_SIN1_007 = 0x0E4B,
    T_SIN1_S18 = 0x0E7A,
    T_SIN1_005 = 0x0F33,
    T_SIN1_S85 = 0x0F86,
    T_SIN1_003 = 0x1005,
    T_SIN1_S17 = 0x1054,
    T_SIN1_006 = 0x10BB,
    T_SIN1_027 = 0x110D,
    T_SIN1_025 = 0x1167,
    T_SIN1_015 = 0x11E1,
    T_SIN1_030 = 0x1269,
    T_SIN1_026 = 0x12CD,
    T_SIN1_029 = 0x136A,
    T_SIN1N005 = 0x139F,
    T_SIN1N004 = 0x140B,
    T_SIN1N006 = 0x1492,
    T_SIN1N002 = 0x14F6,
    T_SIN1N008 = 0x1565,
    T_SIN1N000 = 0x15AC,
    T_SIN1_008 = 0x1610,
    T_SIN1_S51 = 0x1646,
    T_SIN1_002 = 0x16FF,
    T_SIN1_S20 = 0x1780,
    T_SIN1_004 = 0x17D4,
    T_SIN1U070 = 0x1819,
    T_SIN1_014 = 0x1871,
    T_SIN1_013 = 0x1924,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0xD8)
#define rv_door_open1 ROOM_B(0x3D)
#define rv_door_open2 ROOM_B(0x3E)
#define rv_know_lens ROOM_B(0x34)
#define rv_laser_fired ROOM_B(0x33)
#define rv_music_started ROOM_B(0x50)
#define rv_scott_virus ROOM_B(0x43)
#define rv_unused08 ROOM_B(0x31)
#define rv_unused09 ROOM_B(0x32)
#define rv_virus_found ROOM_B(0x44)

/* functions */
static void sins1_Talk(uint16_t ax);
static void sins1_Say(uint16_t ax, uint16_t bx);
static void sins1_StartRoom(void);
static void sins1_LookAtScene(void);
static void sins1_LookAtLock(void);
static void sins1_LookAtDoor(void);
static void sins1_LookAtDisplay(void);
static void sins1_LookAtKeypad(void);
static void sins1_TalkToSpock(void);
static void sins1_SpockScansDisplay(void);
static void sins1_SpockScansKeypad(void);
static void sins1_SpockScansLock(void);
static void sins1_SpockScansDoor(void);
static void sins1_SpockScans(void);
static void sins1_SpockScansLens(void);
static void sins1_RockOnDoor(void);
static void sins1_MosherOnKeypad(void);
static void sins1_McCoyOnKeypad(void);
static void sins1_KirkOnKeypad(void);
static void sins1_SpockOnKeypad(void);
static void sins1_EnterCode(void);
static void sins1_DoorOpens(void);
static void sins1_DoorOpened(void);
static void sins1_WrongCode(void);
static void sins1_LaserFires(void);
static void sins1_E_0CD2(void);
static void sins1_PhaserDoor(void);
static void sins1_LaserFired(void);
static void sins1_ScottOnVirus(void);
static void sins1_TalkToKirk(void);
static void sins1_TalkToSpockNever(void);
static void sins1_TalkToMcCoy(void);
static void sins1_TalkToRedshirt(void);
static void sins1_LookAtKirk(void);
static void sins1_LookAtSpock(void);
static void sins1_LookAtMcCoy(void);
static void sins1_LookAtRedshirt(void);
static void sins1_LookAtDoorway(void);
static void sins1_LookAtLens(void);
static void sins1_UseCommunicator(void);
static void sins1_E_184E(void);
static void sins1_MedkitOnSpock(void);
static void sins1_MedkitOnMcCoy(void);
static void sins1_MedkitOnRedshirt(void);
static void sins1_E_18E0(void);
static void sins1_E_18E8(void);
static void sins1_E_18F0(void);
static void sins1_McCoyScansRedshirt(void);
static void sins1_WalkInside(void);
static void sins1_WalkInside2(void);

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void sins1_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void sins1_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0xDA) = bx;
    CS_W(0xDC) = 0;
    sins1_Talk(talk_buf);
    return;
}

/* The room starts: the door, open or shut. Until Scott's call (tick 40)
 * the party can't move.
 * [MSG_TICK 01 00 00] */
static void sins1_StartRoom(void)
{
    svc_Sfx(N_SIN1LOOP);
    if (rv_scott_virus == 0) {
        room.busy = 1;
    }
    if (rv_door_open1 == 1) {
        svc_Anim(8, N_s1dro, 0, 0, 0);
        svc_LoadMap(N_sins12);
    } else {
        svc_LoadMap(N_sins1);
    }
    if (rv_door_open2 == 1) {
        svc_Anim(8, N_s1ndro, 0, 0, 0);
    }
    rv_unused09 = 1;
    rv_unused08 = 1;
    svc_Music(0x1B, -3);
    if (rv_music_started != 1) {
        svc_Music(1, -2);
        rv_music_started = 1;
    }
    return;
}

/* Looking and talking.
 * [MSG_LOOK FF 00 00] */
static void sins1_LookAtScene(void)
{
    sins1_Say(0, T_SIN1N001);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void sins1_LookAtLock(void)
{
    sins1_Say(0, T_SIN1N009);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void sins1_LookAtDoor(void)
{
    sins1_Say(0, T_SIN1N010);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void sins1_LookAtDisplay(void)
{
    sins1_Say(0, T_SIN1N003);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void sins1_LookAtKeypad(void)
{
    sins1_Say(0, T_SIN1N011);
    return;
}

/* [MSG_TALK 01 00 00] */
static void sins1_TalkToSpock(void)
{
    sins1_Say(N_Mr_Spock, T_SIN1_020);
    return;
}

/* Tricorders.
 * [MSG_USE 44 22 00] */
static void sins1_SpockScansDisplay(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    sins1_Say(N_Mr_Spock, T_SIN1_010);
    return;
}

/* [MSG_USE 44 21 00] */
static void sins1_SpockScansKeypad(void)
{
    svc_Anim(1, N_sscann_2, -1, -1, 0);
    svc_BridgeSfx(4);
    sins1_Say(N_Mr_Spock, T_SIN1_021);
    return;
}

/* [MSG_USE 44 08 00] */
static void sins1_SpockScansLock(void)
{
    svc_Anim(1, N_sscann_3, -1, -1, 0);
    svc_BridgeSfx(4);
    sins1_Say(N_Mr_Spock, T_SIN1_009);
    return;
}

/* [MSG_USE 44 20 00] */
static void sins1_SpockScansDoor(void)
{
    if (rv_door_open1 == 0) {
        svc_Anim(1, N_sscann_4, -1, -1, 0);
        svc_BridgeSfx(4);
        sins1_Say(N_Mr_Spock, T_SIN1_019);
        goto L06D6;
    }
L06D6:;
    return;
}

/* [MSG_USE 44 FF 00] */
static void sins1_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins1_Say(N_Mr_Spock, T_SIN1_024);
    return;
}

/* [MSG_USE 44 23 00] */
static void sins1_SpockScansLens(void)
{
    if (rv_know_lens == 0) {
        svc_Anim(1, N_sscann_5, -1, -1, 0);
        svc_BridgeSfx(4);
        sins1_Say(N_Mr_Spock, T_SIN1_022);
        return;
    }
    sins1_Say(N_Mr_Spock, T_SIN1_023);
    return;
}

/* A rock on the door.
 * [MSG_USE 74 20 00] */
static void sins1_RockOnDoor(void)
{
    sins1_Say(0, T_SIN1N007);
    return;
}

/* The keypad: only Spock (msg 6). He enters the code typed (InputCode):
 * "10200" (99 in the Lucrs' base three) opens the door (anim msgs 8, 1);
 * "01210" or "1210" (100 in the Sofs' base four) fires the laser over the
 * door (msgs 9, 2); anything else does nothing (msg 7).
 * [MSG_USE 03 21 00] */
static void sins1_MosherOnKeypad(void)
{
    if (rv_door_open1 == 0) {
        sins1_Say(N_Ensign_Mosher, T_SIN1_028);
        goto L08C4;
    }
L08C4:;
    return;
}

/* [MSG_USE 02 21 00] */
static void sins1_McCoyOnKeypad(void)
{
    if (rv_door_open1 == 0) {
        sins1_Say(N_Dr_McCoy, T_SIN1_012);
        goto L0918;
    }
L0918:;
    return;
}

/* [MSG_USE 00 21 00] */
static void sins1_KirkOnKeypad(void)
{
    if (rv_door_open1 == 0) {
        sins1_Say(N_Capt_Kirk, T_SIN1_001);
        { sins1_SpockOnKeypad(); return; }
    }
    return;
}

/* [MSG_USE 01 21 00] */
static void sins1_SpockOnKeypad(void)
{
    if (rv_door_open1 == 0) {
        room.fall[1] = 0;
        svc_Walk(1, 0xFD, 0xAD, 6);
    }
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void sins1_EnterCode(void)
{
    uint16_t ax = 0;
    ax = (uint16_t)svc_InputCode(N_01210);
    if (ax != 1) {
        if (ax != 2) {
            if (ax == 3) {
                goto L0A66;
            }
            svc_Sfx(N_EFX14S);
            svc_Anim(1, N_susehn, -1, -1, 7);
            return;
        }
    }
    svc_Sfx(N_EFX14S_2);
    svc_Anim(1, N_susehn_2, -1, -1, 9);
    return;
L0A66:;
    svc_Sfx(N_EFX14S_3);
    svc_Anim(1, N_susehn_3, -1, -1, 8);
    return;
}

/* [MSG_ANIM_DONE 08 00 00] */
static void sins1_DoorOpens(void)
{
    room.fall[1] = 3;
    svc_Walk(1, 0xF3, 0xAD, 0);
    rv_door_open1 = 1;
    svc_LoadMap(N_sins12_2);
    svc_Anim(8, N_s1door, 0, 0, 1);
    svc_Sfx(N_HUGEDOO2);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void sins1_DoorOpened(void)
{
    sins1_Say(N_Mr_Spock, T_SIN1_011);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void sins1_WrongCode(void)
{
    room.fall[1] = 3;
    svc_Walk(1, 0xF3, 0xAD, 0);
    sins1_Say(0, T_SIN1_017);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void sins1_LaserFires(void)
{
    room.fall[1] = 3;
    svc_Walk(1, 0xF3, 0xAD, 0);
    svc_Anim(9, N_s1ph1, 0, 0, 2);
    svc_BridgeSfx(6);
    return;
}

/* Phasers on the door: useless.
 * [MSG_USE 40 20 00] */
static void sins1_E_0CD2(void)
{
    { sins1_PhaserDoor(); return; }
}

/* [MSG_USE 41 20 00] */
static void sins1_PhaserDoor(void)
{
    sins1_Say(N_Mr_Spock, T_SIN1_016);
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void sins1_LaserFired(void)
{
    svc_StopAnim(9);
    sins1_Say(N_Mr_Spock, T_SIN1_018);
    rv_laser_fired = 1;
    rv_know_lens = 1;
    return;
}

/* Tick 40, once: Scott reports a virus in the computer; no phasers.
 * [MSG_TICK 28 00 00] */
static void sins1_ScottOnVirus(void)
{
    if (rv_scott_virus != 0) {
    } else {
        sins1_Say(N_Mr_Scott, T_SIN1_S10);
        sins1_Say(N_Capt_Kirk, T_SIN1_007);
        sins1_Say(N_Mr_Scott, T_SIN1_S18);
        sins1_Say(N_Capt_Kirk, T_SIN1_005);
        sins1_Say(N_Mr_Scott, T_SIN1_S85);
        sins1_Say(N_Capt_Kirk, T_SIN1_003);
        sins1_Say(N_Mr_Scott, T_SIN1_S17);
        rv_scott_virus = 1;
        room.busy = 0;
    }
    return;
}

/* [MSG_TALK 00 00 00] */
static void sins1_TalkToKirk(void)
{
    sins1_Say(N_Capt_Kirk, T_SIN1_006);
    sins1_Say(N_Mr_Spock, T_SIN1_027);
    return;
}

/* [MSG_TALK 01 00 00] */
static void sins1_TalkToSpockNever(void)
{
    sins1_Say(N_Mr_Spock, T_SIN1_025);
    return;
}

/* [MSG_TALK 02 00 00] */
static void sins1_TalkToMcCoy(void)
{
    sins1_Say(N_Dr_McCoy, T_SIN1_015);
    return;
}

/* [MSG_TALK 03 00 00] */
static void sins1_TalkToRedshirt(void)
{
    sins1_Say(N_Ensign_Mosher, T_SIN1_030);
    sins1_Say(N_Mr_Spock, T_SIN1_026);
    sins1_Say(N_Ensign_Mosher, T_SIN1_029);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void sins1_LookAtKirk(void)
{
    sins1_Say(0, T_SIN1N005);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void sins1_LookAtSpock(void)
{
    sins1_Say(0, T_SIN1N004);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void sins1_LookAtMcCoy(void)
{
    sins1_Say(0, T_SIN1N006);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void sins1_LookAtRedshirt(void)
{
    sins1_Say(0, T_SIN1N002);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void sins1_LookAtDoorway(void)
{
    sins1_Say(0, T_SIN1N008);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void sins1_LookAtLens(void)
{
    sins1_Say(0, T_SIN1N000);
    return;
}

/* [MSG_USE 47 FF 00] */
static void sins1_UseCommunicator(void)
{
    if (rv_virus_found != 0) {
    } else {
        sins1_Say(N_Capt_Kirk, T_SIN1_008);
        sins1_Say(N_Mr_Scott, T_SIN1_S51);
        sins1_Say(N_Capt_Kirk, T_SIN1_002);
        sins1_Say(N_Mr_Scott, T_SIN1_S20);
        sins1_Say(N_Capt_Kirk, T_SIN1_004);
        goto L1808;
    }
L1808:;
    if (rv_virus_found == 1) {
        sins1_Say(N_Lt_Uhura, T_SIN1U070);
        goto L1847;
    }
L1847:;
    return;
}

/* [MSG_USE 7F 00 00] */
static void sins1_E_184E(void)
{
    { sins1_MedkitOnSpock(); return; }
}

/* [MSG_USE 7F 01 00] */
static void sins1_MedkitOnSpock(void)
{
    { sins1_MedkitOnMcCoy(); return; }
}

/* [MSG_USE 7F 02 00] */
static void sins1_MedkitOnMcCoy(void)
{
    { sins1_MedkitOnRedshirt(); return; }
}

/* [MSG_USE 7F 03 00] */
static void sins1_MedkitOnRedshirt(void)
{
    sins1_Say(N_Dr_McCoy, T_SIN1_014);
    return;
}

/* [MSG_USE 45 00 00] */
static void sins1_E_18E0(void)
{
    { sins1_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 01 00] */
static void sins1_E_18E8(void)
{
    { sins1_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 02 00] */
static void sins1_E_18F0(void)
{
    { sins1_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 03 00] */
static void sins1_McCoyScansRedshirt(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins1_Say(N_Dr_McCoy, T_SIN1_013);
    return;
}

/* Walking through the open door (room 2).
 * [MSG_WALK 20 00 00] */
static void sins1_WalkInside(void)
{
    if (rv_door_open1 == 1) {
        svc_Walk(0, 0x98, 0x9E, 5);
    }
    return;
}

/* [MSG_WALK 08 00 00] */
static void sins1_WalkInside2(void)
{
    if (rv_door_open1 == 1) {
        svc_Walk(0, 0x98, 0x9E, 5);
    }
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript sins1_scripts[] = {
    { 0x0205, sins1_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x02D7, sins1_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0322, sins1_LookAtLock },    /* MSG_LOOK 08 00 00 */
    { 0x0372, sins1_LookAtDoor },    /* MSG_LOOK 20 00 00 */
    { 0x03EB, sins1_LookAtDisplay },    /* MSG_LOOK 22 00 00 */
    { 0x0436, sins1_LookAtKeypad },    /* MSG_LOOK 21 00 00 */
    { 0x047F, sins1_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0504, sins1_SpockScansDisplay },    /* MSG_USE 44 22 00 */
    { 0x0575, sins1_SpockScansKeypad },    /* MSG_USE 44 21 00 */
    { 0x05EA, sins1_SpockScansLock },    /* MSG_USE 44 08 00 */
    { 0x0653, sins1_SpockScansDoor },    /* MSG_USE 44 20 00 */
    { 0x06DD, sins1_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x072D, sins1_SpockScansLens },    /* MSG_USE 44 23 00 */
    { 0x0806, sins1_RockOnDoor },    /* MSG_USE 74 20 00 */
    { 0x0882, sins1_MosherOnKeypad },    /* MSG_USE 03 21 00 */
    { 0x08CB, sins1_McCoyOnKeypad },    /* MSG_USE 02 21 00 */
    { 0x091F, sins1_KirkOnKeypad },    /* MSG_USE 00 21 00 */
    { 0x0973, sins1_SpockOnKeypad },    /* MSG_USE 01 21 00 */
    { 0x09B2, sins1_EnterCode },    /* MSG_WALK_DONE 06 00 00 */
    { 0x0AA3, sins1_DoorOpens },    /* MSG_ANIM_DONE 08 00 00 */
    { 0x0B21, sins1_DoorOpened },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0BAE, sins1_WrongCode },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0C7E, sins1_LaserFires },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x0CD2, sins1_E_0CD2 },    /* MSG_USE 40 20 00 */
    { 0x0CDA, sins1_PhaserDoor },    /* MSG_USE 41 20 00 */
    { 0x0D49, sins1_LaserFired },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x0DEB, sins1_ScottOnVirus },    /* MSG_TICK 28 00 00 */
    { 0x10B0, sins1_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x115C, sins1_TalkToSpockNever },    /* MSG_TALK 01 00 00 */
    { 0x11D6, sins1_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x125E, sins1_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x1394, sins1_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x1400, sins1_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x1487, sins1_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x14EB, sins1_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x155A, sins1_LookAtDoorway },    /* MSG_LOOK 20 00 00 */
    { 0x15A1, sins1_LookAtLens },    /* MSG_LOOK 23 00 00 */
    { 0x15FC, sins1_UseCommunicator },    /* MSG_USE 47 FF 00 */
    { 0x184E, sins1_E_184E },    /* MSG_USE 7F 00 00 */
    { 0x1856, sins1_MedkitOnSpock },    /* MSG_USE 7F 01 00 */
    { 0x185E, sins1_MedkitOnMcCoy },    /* MSG_USE 7F 02 00 */
    { 0x1866, sins1_MedkitOnRedshirt },    /* MSG_USE 7F 03 00 */
    { 0x18E0, sins1_E_18E0 },    /* MSG_USE 45 00 00 */
    { 0x18E8, sins1_E_18E8 },    /* MSG_USE 45 01 00 */
    { 0x18F0, sins1_E_18F0 },    /* MSG_USE 45 02 00 */
    { 0x18F8, sins1_McCoyScansRedshirt },    /* MSG_USE 45 03 00 */
    { 0x1967, sins1_WalkInside },    /* MSG_WALK 20 00 00 */
    { 0x1990, sins1_WalkInside2 },    /* MSG_WALK 08 00 00 */
    { 0, 0 }
};
