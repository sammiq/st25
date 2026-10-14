/* SINS0.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/sins0.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * SINS0.RDF - "Sins of Omission", the moon of Proxtrey, before a security
 * door built into a hill: the way into the old installation that the
 * party has come to search. Meanwhile, Scott reports a fault in the
 * transporter.
 *
 * Actors: 8 the door.
 * Objects: 8/20h the door, 21h rocks, 22h stone faces, 23h the planet,
 * 24h the stars. Exit 0: room 1.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - Two scripts answer talking to Mosher; the second never runs.
 * - The conversation at 00D8 is built at run time.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_sscans = 0x00CA,
    N_mscans = 0x00D1,
    talk_buf = 0x00D8,
    N_Capt_Kirk = 0x0121,
    N_Dr_McCoy = 0x012C,
    N_Mr_Spock = 0x0136,
    N_Ensign_Mosher = 0x0140,
    N_Lt_Uhura = 0x014E,
    N_Mr_Scott = 0x0158,
    N_SIN0LOOP = 0x01FE,
    N_s0dr2 = 0x0228,
    N_s0dr1 = 0x024D,
    T_SIN0N009 = 0x0288,
    T_SIN0N005 = 0x02D9,
    T_SIN0N008 = 0x0324,
    T_SIN0_009 = 0x039C,
    T_SIN0_011 = 0x041E,
    T_SIN0_024 = 0x04AD,
    T_SIN0N006 = 0x04E4,
    N_kpickw = 0x051B,
    T_SIN0N010 = 0x0561,
    T_SIN0_018 = 0x05C4,
    T_SIN0_017 = 0x064D,
    T_SIN0_016 = 0x06D6,
    T_SIN0U082 = 0x077B,
    T_SIN0_003 = 0x07C2,
    T_SIN0_S48 = 0x07E7,
    T_SIN0_007 = 0x083C,
    T_SIN0_S50 = 0x087F,
    T_SIN0_005 = 0x0934,
    T_SIN0_S49 = 0x097E,
    T_SIN0_001 = 0x09F0,
    T_SIN0_026 = 0x0A4A,
    T_SIN0_022 = 0x0A9B,
    T_SIN0_006 = 0x0AC6,
    T_SIN0N011 = 0x0B18,
    T_SIN0_015 = 0x0B50,
    T_SIN0_021 = 0x0BA2,
    T_SIN0_027 = 0x0C0E,
    T_SIN0_014 = 0x0C8E,
    T_SIN0N007 = 0x0D0D,
    T_SIN0N004 = 0x0D85,
    T_SIN0N002 = 0x0E0A,
    T_SIN0N003 = 0x0E6E,
    T_SIN0N000 = 0x0EDD,
    T_SIN0N001 = 0x0F9A,
    T_SIN0_019 = 0x1024,
    T_SIN0_025 = 0x1089,
    T_SIN0_013 = 0x110A,
    T_SIN0_028 = 0x1145,
    T_SIN0_008 = 0x1199,
    T_SIN0_023 = 0x11F3,
    T_SIN0_012 = 0x1286,
    T_SIN0_010 = 0x133A,
    T_SIN0_004 = 0x140B,
    T_SIN0_S01 = 0x1450,
    T_SIN0_002 = 0x14C5,
    T_SIN0U069 = 0x150D,
    rdf_scripts_end = 0x1582,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0xD8)
#define rv_door_open1 ROOM_B(0x3D)
#define rv_door_open2 ROOM_B(0x3E)
#define rv_rocks_known ROOM_B(0x39)
#define rv_scanned_stones ROOM_B(0x48)
#define rv_score ROOM_W(0x52)
#define rv_scott_called ROOM_B(0x42)
#define rv_transporter_fixed ROOM_B(0x43)

/* functions */
static void sins0_Talk(uint16_t ax);
static void sins0_Say(uint16_t ax, uint16_t bx);
static void sins0_StartRoom(void);
static void sins0_LookAtScene(void);
static void sins0_LookAtOpenDoor(void);
static void sins0_LookAtDoor(void);
static void sins0_TalkToSpock(void);
static void sins0_TalkToMcCoy(void);
static void sins0_TalkToMosher(void);
static void sins0_LookAtRocks(void);
static void sins0_GetRock(void);
static void sins0_TookRock(void);
static void sins0_SpockScans(void);
static void sins0_SpockScansRocks(void);
static void sins0_SpockScansPlanet(void);
static void sins0_ScottCalls(void);
static void sins0_TalkToMosherNever(void);
static void sins0_LookAtStones(void);
static void sins0_LookAtPlanet(void);
static void sins0_LookAtStars(void);
static void sins0_LookAtKirk(void);
static void sins0_LookAtSpock(void);
static void sins0_LookAtMcCoy(void);
static void sins0_LookAtRedshirt(void);
static void sins0_SpockScansDoor(void);
static void sins0_SpockScansStones(void);
static void sins0_E_1263(void);
static void sins0_MedkitOnSpock(void);
static void sins0_MedkitOnMcCoy(void);
static void sins0_MedkitOnRedshirt(void);
static void sins0_E_12F5(void);
static void sins0_E_12FD(void);
static void sins0_E_1305(void);
static void sins0_McCoyScansRedshirt(void);
static void sins0_UseCommunicator(void);
static void sins0_WalkToDoor(void);
static void sins0_WalkToOpenDoor(void);

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void sins0_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void sins0_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0xDA) = bx;
    CS_W(0xDC) = 0;
    sins0_Talk(talk_buf);
    return;
}

/* The room starts: the door, open or not. The first time, the party can't
 * move until Scott's call (tick 40).
 * [MSG_TICK 01 00 00] */
static void sins0_StartRoom(void)
{
    svc_Sfx(N_SIN0LOOP);
    if (rv_scott_called == 0) {
        room.busy = 2;
    }
    if (rv_door_open2 == 1) {
        svc_Anim(8, N_s0dr2, 0, 0, 0);
    } else {
        if (rv_door_open1 == 1) {
            svc_Anim(8, N_s0dr1, 0, 0, 0);
        }
    }
    svc_Music(0x1B, -3);
    return;
}

/* Looking and talking.
 * [MSG_LOOK FF 00 00] */
static void sins0_LookAtScene(void)
{
    sins0_Say(0, T_SIN0N009);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void sins0_LookAtOpenDoor(void)
{
    sins0_Say(0, T_SIN0N005);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void sins0_LookAtDoor(void)
{
    sins0_Say(0, T_SIN0N008);
    return;
}

/* [MSG_TALK 01 00 00] */
static void sins0_TalkToSpock(void)
{
    sins0_Say(N_Mr_Spock, T_SIN0_009);
    return;
}

/* [MSG_TALK 02 00 00] */
static void sins0_TalkToMcCoy(void)
{
    sins0_Say(N_Dr_McCoy, T_SIN0_011);
    return;
}

/* Mosher. TalkToMosherNever has the same key and the first doesn't set
 * RS_MORE, so it never runs.
 * [MSG_TALK 03 00 00] */
static void sins0_TalkToMosher(void)
{
    sins0_Say(N_Ensign_Mosher, T_SIN0_024);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void sins0_LookAtRocks(void)
{
    sins0_Say(0, T_SIN0N006);
    return;
}

/* Rocks (anim msg 1): tri-phosphorate silver, by Spock's tricorder.
 * [MSG_GET 21 00 00] */
static void sins0_GetRock(void)
{
    room.busy = 1;
    svc_Anim(0, N_kpickw, -1, -1, 1);
    rv_rocks_known |= 1;
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void sins0_TookRock(void)
{
    room.busy = 0;
    svc_StopAnim(0);
    sins0_Say(0, T_SIN0N010);
    svc_GiveItem(IT_S8ROCKS);
    return;
}

/* [MSG_USE 44 FF 00] */
static void sins0_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Mr_Spock, T_SIN0_018);
    return;
}

/* [MSG_USE 44 21 00] */
static void sins0_SpockScansRocks(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Mr_Spock, T_SIN0_017);
    rv_rocks_known |= 1;
    return;
}

/* [MSG_USE 44 23 00] */
static void sins0_SpockScansPlanet(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Mr_Spock, T_SIN0_016);
    return;
}

/* Tick 40, once: Scott reports a fault in the transporters.
 * [MSG_TICK 28 00 00] */
static void sins0_ScottCalls(void)
{
    if (rv_scott_called != 0) {
    } else {
        room.busy = 0;
        sins0_Say(N_Lt_Uhura, T_SIN0U082);
        sins0_Say(N_Capt_Kirk, T_SIN0_003);
        sins0_Say(N_Mr_Scott, T_SIN0_S48);
        sins0_Say(N_Capt_Kirk, T_SIN0_007);
        sins0_Say(N_Mr_Scott, T_SIN0_S50);
        sins0_Say(N_Capt_Kirk, T_SIN0_005);
        sins0_Say(N_Mr_Scott, T_SIN0_S49);
        sins0_Say(N_Capt_Kirk, T_SIN0_001);
        rv_scott_called = 1;
    }
    return;
}

/* [MSG_TALK 03 00 00] */
static void sins0_TalkToMosherNever(void)
{
    sins0_Say(N_Ensign_Mosher, T_SIN0_026);
    sins0_Say(N_Mr_Spock, T_SIN0_022);
    sins0_Say(N_Capt_Kirk, T_SIN0_006);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void sins0_LookAtStones(void)
{
    sins0_Say(0, T_SIN0N011);
    sins0_Say(N_Dr_McCoy, T_SIN0_015);
    sins0_Say(N_Mr_Spock, T_SIN0_021);
    sins0_Say(N_Ensign_Mosher, T_SIN0_027);
    sins0_Say(N_Dr_McCoy, T_SIN0_014);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void sins0_LookAtPlanet(void)
{
    sins0_Say(0, T_SIN0N007);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void sins0_LookAtStars(void)
{
    sins0_Say(0, T_SIN0N004);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void sins0_LookAtKirk(void)
{
    sins0_Say(0, T_SIN0N002);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void sins0_LookAtSpock(void)
{
    sins0_Say(0, T_SIN0N003);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void sins0_LookAtMcCoy(void)
{
    sins0_Say(0, T_SIN0N000);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void sins0_LookAtRedshirt(void)
{
    sins0_Say(0, T_SIN0N001);
    return;
}

/* Tricorders and the medkit.
 * [MSG_USE 44 20 00] */
static void sins0_SpockScansDoor(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Mr_Spock, T_SIN0_019);
    sins0_Say(N_Ensign_Mosher, T_SIN0_025);
    sins0_Say(N_Dr_McCoy, T_SIN0_013);
    sins0_Say(N_Ensign_Mosher, T_SIN0_028);
    return;
}

/* [MSG_USE 44 22 00] */
static void sins0_SpockScansStones(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Mr_Spock, rdf_scripts_end);
    sins0_Say(N_Capt_Kirk, T_SIN0_008);
    sins0_Say(N_Mr_Spock, T_SIN0_023);
    if (rv_scanned_stones != 1) {
        rv_scanned_stones = 1;
        rv_score += 1;
    }
    return;
}

/* [MSG_USE 7F 00 00] */
static void sins0_E_1263(void)
{
    { sins0_MedkitOnSpock(); return; }
}

/* [MSG_USE 7F 01 00] */
static void sins0_MedkitOnSpock(void)
{
    { sins0_MedkitOnMcCoy(); return; }
}

/* [MSG_USE 7F 02 00] */
static void sins0_MedkitOnMcCoy(void)
{
    { sins0_MedkitOnRedshirt(); return; }
}

/* [MSG_USE 7F 03 00] */
static void sins0_MedkitOnRedshirt(void)
{
    sins0_Say(N_Dr_McCoy, T_SIN0_012);
    return;
}

/* [MSG_USE 45 00 00] */
static void sins0_E_12F5(void)
{
    { sins0_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 01 00] */
static void sins0_E_12FD(void)
{
    { sins0_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 02 00] */
static void sins0_E_1305(void)
{
    { sins0_McCoyScansRedshirt(); return; }
}

/* [MSG_USE 45 03 00] */
static void sins0_McCoyScansRedshirt(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    sins0_Say(N_Dr_McCoy, T_SIN0_010);
    return;
}

/* The communicator.
 * [MSG_USE 47 FF 00] */
static void sins0_UseCommunicator(void)
{
    if (rv_transporter_fixed != 0) {
    } else {
        sins0_Say(N_Capt_Kirk, T_SIN0_004);
        sins0_Say(N_Mr_Scott, T_SIN0_S01);
        sins0_Say(N_Capt_Kirk, T_SIN0_002);
        goto L14FC;
    }
L14FC:;
    if (rv_transporter_fixed == 1) {
        sins0_Say(N_Lt_Uhura, T_SIN0U069);
        goto L153B;
    }
L153B:;
    return;
}

/* The door (room 1).
 * [MSG_WALK 20 00 00] */
static void sins0_WalkToDoor(void)
{
    svc_Walk(0, 0x5F, 0x8C, 0);
    return;
}

/* [MSG_WALK 08 00 00] */
static void sins0_WalkToOpenDoor(void)
{
    svc_Walk(0, 0x5F, 0x8C, 0);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript sins0_scripts[] = {
    { 0x01FC, sins0_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x027D, sins0_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x02CE, sins0_LookAtOpenDoor },    /* MSG_LOOK 08 00 00 */
    { 0x0319, sins0_LookAtDoor },    /* MSG_LOOK 20 00 00 */
    { 0x0391, sins0_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0413, sins0_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x04A2, sins0_TalkToMosher },    /* MSG_TALK 03 00 00 */
    { 0x04D9, sins0_LookAtRocks },    /* MSG_LOOK 21 00 00 */
    { 0x0515, sins0_GetRock },    /* MSG_GET 21 00 00 */
    { 0x0542, sins0_TookRock },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0598, sins0_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x0621, sins0_SpockScansRocks },    /* MSG_USE 44 21 00 */
    { 0x06A9, sins0_SpockScansPlanet },    /* MSG_USE 44 23 00 */
    { 0x0763, sins0_ScottCalls },    /* MSG_TICK 28 00 00 */
    { 0x0A3F, sins0_TalkToMosherNever },    /* MSG_TALK 03 00 00 */
    { 0x0B0D, sins0_LookAtStones },    /* MSG_LOOK 22 00 00 */
    { 0x0D02, sins0_LookAtPlanet },    /* MSG_LOOK 23 00 00 */
    { 0x0D7A, sins0_LookAtStars },    /* MSG_LOOK 24 00 00 */
    { 0x0DFF, sins0_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0E63, sins0_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0ED1, sins0_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0F8F, sins0_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0FF8, sins0_SpockScansDoor },    /* MSG_USE 44 20 00 */
    { 0x1164, sins0_SpockScansStones },    /* MSG_USE 44 22 00 */
    { 0x1263, sins0_E_1263 },    /* MSG_USE 7F 00 00 */
    { 0x126B, sins0_MedkitOnSpock },    /* MSG_USE 7F 01 00 */
    { 0x1273, sins0_MedkitOnMcCoy },    /* MSG_USE 7F 02 00 */
    { 0x127B, sins0_MedkitOnRedshirt },    /* MSG_USE 7F 03 00 */
    { 0x12F5, sins0_E_12F5 },    /* MSG_USE 45 00 00 */
    { 0x12FD, sins0_E_12FD },    /* MSG_USE 45 01 00 */
    { 0x1305, sins0_E_1305 },    /* MSG_USE 45 02 00 */
    { 0x130D, sins0_McCoyScansRedshirt },    /* MSG_USE 45 03 00 */
    { 0x13F7, sins0_UseCommunicator },    /* MSG_USE 47 FF 00 */
    { 0x1542, sins0_WalkToDoor },    /* MSG_WALK 20 00 00 */
    { 0x1565, sins0_WalkToOpenDoor },    /* MSG_WALK 08 00 00 */
    { 0, 0 }
};
