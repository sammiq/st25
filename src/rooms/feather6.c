/* FEATHER6.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/feather6.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * FEATHER6.RDF - "The Feathered Serpent", the cave beyond the river. A
 * cluster of raw dilithium crystals grows under unstable stalactites: the
 * knife pries a crystal loose; hammering it with a rock, or throwing one at
 * the stalactites, brings a stalactite down on Kirk, and a strange force
 * carries his body on to room 7.
 *
 * Actors: 8 the crystal, 9 the stalactite, 10 (0Ah) the thrown rock.
 * Objects: 8 the crystal, 20h the way back, 21h rocks, 22h the
 * stalactites, 23h the stalagmites. Exit 3: room 7. Region 0: the way
 * back to the river.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - Two scripts share the key MSG_USE, IT_ROCK, ANY: the second never runs.
 * - The script chain runs on into code at 083F: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s5r1kt = 0x00CB,
    N_s5r6kr = 0x00D2,
    N_s5r6kp = 0x00D9,
    N_s5r6kd = 0x00E0,
    N_s5r1ru = 0x00E7,
    N_s5r6cx = 0x00EE,
    N_s5r6ts = 0x00F5,
    N_s5r6tm = 0x00FC,
    N_s5r6tf = 0x0103,
    N_sscann = 0x010A,
    N_sscans = 0x0111,
    N_sscane = 0x0118,
    N_mscane = 0x011F,
    N_FEA6LOOP = 0x012E,
    T_FEA6_011 = 0x0450,
    N_Capt_Kirk = 0x0916,
    N_Dr_McCoy = 0x0921,
    N_Mr_Spock = 0x092B,
    N_Lt_Stragey = 0x0935,
    S_0941 = 0x0941,
    T_FEA6_020 = 0x0942,
    T_FEA6_009 = 0x098D,
    T_FEA6_040 = 0x09C8,
    T_FEA6N000 = 0x0A29,
    T_FEA6_001 = 0x0A7F,
    T_FEA6N009 = 0x0ABA,
    T_FEA6_021 = 0x0ADA,
    T_FEA6N010 = 0x0B13,
    T_FEA6_039 = 0x0B33,
    T_FEA6N015 = 0x0BF3,
    T_FEA6N001 = 0x0C4B,
    T_FEA6_019 = 0x0C81,
    T_FEA6_028 = 0x0CE7,
    T_FEA6N005 = 0x0D4C,
    T_FEA6N016 = 0x0D7E,
    T_FEA6N003 = 0x0E2F,
    T_FEA6N014 = 0x0E62,
    T_FEA6N012 = 0x0EBA,
    T_FEA6N013 = 0x0EEF,
    T_FEA6_034 = 0x0F2B,
    T_FEA6_017 = 0x0FCB,
    T_FEA6_033 = 0x0FF6,
    T_FEA6_032 = 0x111C,
    T_FEA6_036 = 0x118C,
    T_FEA6_037 = 0x11F0,
    T_FEA6N007 = 0x1223,
    T_FEA6N011 = 0x1296,
    T_FEA6N002 = 0x12C4,
    T_FEA6N008 = 0x1306,
    T_FEA6_042 = 0x136C,
    T_FEA6_002 = 0x13CB,
    T_FEA6_041 = 0x1416,
    T_FEA6_016 = 0x146B,
    T_FEA6_043 = 0x14D0,
    T_FEA6_035 = 0x1501,
    T_FEA6_038 = 0x157A,
    T_FEA6_029 = 0x177E,
    T_FEA6_030 = 0x179E,
    T_FEA6_031 = 0x181C,
    T_FEA6_013 = 0x1867,
    T_FEA6_012 = 0x18AB,
    T_FEA6_015 = 0x192D,
    T_FEA6_024 = 0x1989,
    T_FEA6_022 = 0x19BE,
    T_FEA6_025 = 0x1A18,
    T_FEA6_026 = 0x1A79,
    T_FEA6_018 = 0x1ADA,
    T_FEA6_027 = 0x1B83,
    T_FEA6N017 = 0x1BD5,
    T_FEA6N020 = 0x1C11,
    T_FEA6_014 = 0x1C49,
    T_FEA6N004 = 0x1C8B,
    T_FEA6_023 = 0x1CCA,
    talk_buf = 0x1CF5,
    KirkOnArguing = 0x1CFB,
    KirkOnQuetzecoatl = 0x1D05,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_blows CS_B(0xCA)
#define fv_talk_buf CS_W(0x1CF5)
#define rv_died_in_cave ROOM_B(0x29)
#define rv_score ROOM_W(0x36)

/* functions */
static void feather6_StartRoom(void);
static void feather6_BackToRiver(void);
static void feather6_SpockOnCrystal(void);
static void feather6_McCoyOnCrystal(void);
static void feather6_StrageyOnCrystal(void);
static void feather6_E_01C6(void);
static void feather6_RockAtStalactites(void);
static void feather6_KirkThrows(void);
static void feather6_StalactiteFalls(void);
static void feather6_KirkKilled(void);
static void feather6_RockOnCrystal(void);
static void feather6_KirkHammers(void);
static void feather6_Rumbles(void);
static void feather6_FewMoreBlows(void);
static void feather6_StalactiteLoose(void);
static void feather6_RockAtStalagmites(void);
static void feather6_RockAtEntrance(void);
static void feather6_UseRock(void);
static void feather6_UseRockNever(void);
static void feather6_KnifeOnStalagmites(void);
static void feather6_UseKnife(void);
static void feather6_KnifeOnMcCoy(void);
static void feather6_KnifeOnSpock(void);
static void feather6_KnifeOnStragey(void);
static void feather6_UseCrystal(void);
static void feather6_UseSnake(void);
static void feather6_UseMedkit(void);
static void feather6_StunAnything(void);
static void feather6_KillAnything(void);
static void feather6_LookAtEntrance(void);
static void feather6_LookAtScene(void);
static void feather6_LookAtRocks(void);
static void feather6_LookAtStalagmites(void);
static void feather6_LookAtCrystal(void);
static void feather6_LookAtKirk(void);
static void feather6_LookAtSpock(void);
static void feather6_LookAtMcCoy(void);
static void feather6_LookAtRedshirt(void);
static void feather6_LookAtStalactites(void);
static void feather6_TalkToMcCoy(void);
static void feather6_TalkToRedshirt(void);
static void feather6_TalkToSpock(void);
static void feather6_McCoyScansEntrance(void);
static void feather6_McCoyScansRocks(void);
static void feather6_SpockScansEntrance(void);
static void feather6_SpockScans(void);
static void feather6_SpockScansRocks(void);
static void feather6_SpockScansCrystal(void);
static void feather6_SpockScansStalactites(void);
static void feather6_SpockScansStalagmites(void);
static void feather6_GetCrystal(void);
static void feather6_KnifeOnCrystal(void);
static void feather6_KirkPries(void);
static void feather6_TookCrystal(void);
static void feather6_CrystalGone(void);
static uint16_t feather6_Talk(uint16_t ax);
static void feather6_Say(uint16_t ax, uint16_t bx);

/* The room starts: the crystal, the stalactite.
 * [MSG_TICK 01 00 00] */
static void feather6_StartRoom(void)
{
    svc_Sfx(N_FEA6LOOP);
    svc_Music(0x1B, STALE_ARG);
    svc_Anim0(8, N_s5r6cx, 0x8A, 0x91, 0);
    svc_Anim0(9, N_s5r6ts, 0x90, 0x25, 0);
    return;
}

/* Region 0: McCoy won't go back across the river.
 * [MSG_REGION 00 00 00] */
static void feather6_BackToRiver(void)
{
    feather6_Say(N_Dr_McCoy, T_FEA6_014);
    return;
}

/* The crew on the crystal: it needs prying loose.
 * [MSG_USE 01 08 00] */
static void feather6_SpockOnCrystal(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_020);
    return;
}

/* [MSG_USE 02 08 00] */
static void feather6_McCoyOnCrystal(void)
{
    feather6_Say(N_Dr_McCoy, T_FEA6_009);
    return;
}

/* [MSG_USE 03 08 00] */
static void feather6_StrageyOnCrystal(void)
{
    feather6_Say(N_Lt_Stragey, T_FEA6_040);
    return;
}

/* A rock at the stalactites (msg 7): one falls and kills Kirk (anim msg
 * 2): -3, but a strange force takes him on to room 7.
 * [MSG_USE 43 09 00] */
static void feather6_E_01C6(void)
{
    { feather6_RockAtStalactites(); return; }
}

/* [MSG_USE 43 22 00] */
static void feather6_RockAtStalactites(void)
{
    room.busy = 1;
    svc_Walk(0, 0x94, 0x97, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void feather6_KirkThrows(void)
{
    svc_Anim(0, N_s5r1kt, -1, -1, 0);
    svc_Anim0(0xA, N_s5r1ru, 0x96, 0x4E, 7);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void feather6_StalactiteFalls(void)
{
    svc_BridgeSfx(0xB);
    svc_Anim(9, N_s5r6tf, -1, -1, 0);
    svc_Anim(0, N_s5r6kd, -1, -1, 2);
    svc_Music(1, STALE_ARG);
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void feather6_KirkKilled(void)
{
    rv_score += 0xFFFD;
    rv_died_in_cave = 1;
    room.busy = 0;
    feather6_Say(S_0941, T_FEA6N000);
    feather6_Say(S_0941, T_FEA6N004);
    svc_GotoRoom(7, 5);
    return;
}

/* Hammering the crystal with a rock (msgs 4): the first blow loosens the
 * stalactite (timer 2, rumbling), the second brings it down (msg 9).
 * [MSG_USE 43 08 00] */
static void feather6_RockOnCrystal(void)
{
    room.busy = 1;
    svc_Walk(0, 0x9A, 0x97, 4);
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void feather6_KirkHammers(void)
{
    room.timers[2] = 0xAE;
    svc_Anim(9, N_s5r6tm, -1, -1, 0);
    if (fv_blows == 0) {
        fv_blows++;
        svc_Anim(0, N_s5r6kr, -1, -1, 0xA);
        return;
    }
    svc_Anim(0, N_s5r6kr, -1, -1, 9);
    return;
}

/* Every tick: rumbles as timer 2 counts down after a blow.
 * [MSG_TICK FF FF FF] */
static void feather6_Rumbles(void)
{
    if (room.timers[2] != 0x37) {
        if (room.timers[2] != 0x59) {
            if (room.timers[2] != 0x77) {
                return;
            }
        }
    }
    svc_BridgeSfx(0xB);
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void feather6_FewMoreBlows(void)
{
    room.busy = 0;
    svc_StopAnim(0);
    feather6_Say(N_Capt_Kirk, T_FEA6_001);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void feather6_StalactiteLoose(void)
{
    svc_Anim(0, N_s5r6kd, -1, -1, 2);
    svc_Anim(9, N_s5r6tf, -1, -1, 0);
    svc_Music(0, STALE_ARG);
    return;
}

/* Rocks and the knife elsewhere. The two USE IT_ROCK, ANY scripts share a
 * key, so the second never runs.
 * [MSG_USE 43 23 00] */
static void feather6_RockAtStalagmites(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_019);
    return;
}

/* [MSG_USE 43 20 00] */
static void feather6_RockAtEntrance(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_019);
    return;
}

/* [MSG_USE 43 FF 00] */
static void feather6_UseRock(void)
{
    feather6_Say(S_0941, T_FEA6N009);
    return;
}

/* [MSG_USE 43 FF 00] */
static void feather6_UseRockNever(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_021);
    return;
}

/* [MSG_USE 79 23 00] */
static void feather6_KnifeOnStalagmites(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_028);
    return;
}

/* [MSG_USE 79 FF 00] */
static void feather6_UseKnife(void)
{
    feather6_Say(S_0941, T_FEA6N010);
    return;
}

/* [MSG_USE 79 02 00] */
static void feather6_KnifeOnMcCoy(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_039);
    return;
}

/* [MSG_USE 79 01 00] */
static void feather6_KnifeOnSpock(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_039);
    return;
}

/* [MSG_USE 79 03 00] */
static void feather6_KnifeOnStragey(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_039);
    return;
}

/* [MSG_USE 78 FF 00] */
static void feather6_UseCrystal(void)
{
    feather6_Say(S_0941, T_FEA6N015);
    return;
}

/* [MSG_USE 76 FF 00] */
static void feather6_UseSnake(void)
{
    feather6_Say(S_0941, T_FEA6N001);
    return;
}

/* [MSG_USE 7F FF 00] */
static void feather6_UseMedkit(void)
{
    feather6_Say(N_Dr_McCoy, T_FEA6_011);
    return;
}

/* [MSG_USE 40 FF 00] */
static void feather6_StunAnything(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_023);
    return;
}

/* [MSG_USE 41 FF 00] */
static void feather6_KillAnything(void)
{
    feather6_Say(N_Mr_Spock, T_FEA6_023);
    return;
}

/* Looking and talking.
 * [MSG_LOOK 20 00 00] */
static void feather6_LookAtEntrance(void)
{
    feather6_Say(0, T_FEA6N016);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void feather6_LookAtScene(void)
{
    feather6_Say(0, T_FEA6N003);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void feather6_LookAtRocks(void)
{
    feather6_Say(0, T_FEA6N014);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void feather6_LookAtStalagmites(void)
{
    feather6_Say(0, T_FEA6N013);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void feather6_LookAtCrystal(void)
{
    feather6_Say(0, T_FEA6N005);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void feather6_LookAtKirk(void)
{
    feather6_Say(0, T_FEA6N007);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void feather6_LookAtSpock(void)
{
    feather6_Say(0, T_FEA6N011);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void feather6_LookAtMcCoy(void)
{
    feather6_Say(0, T_FEA6N002);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void feather6_LookAtRedshirt(void)
{
    feather6_Say(0, T_FEA6N008);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void feather6_LookAtStalactites(void)
{
    uint16_t ax = 0;
    feather6_Say(S_0941, T_FEA6N012);
    feather6_Say(N_Mr_Spock, T_FEA6_034);
    feather6_Say(N_Dr_McCoy, T_FEA6_017);
    feather6_Say(N_Mr_Spock, T_FEA6_033);
    ax = (uint16_t)feather6_Talk(KirkOnArguing);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L0597;
        }
        goto L05A1;
    }
    feather6_Say(N_Mr_Spock, T_FEA6_032);
    return;
L0597:;
    feather6_Say(N_Mr_Spock, T_FEA6_036);
    return;
L05A1:;
    feather6_Say(N_Mr_Spock, T_FEA6_037);
    return;
}

/* [MSG_TALK 02 00 00] */
static void feather6_TalkToMcCoy(void)
{
    feather6_Say(N_Dr_McCoy, T_FEA6_013);
    return;
}

/* [MSG_TALK 03 00 00] */
static void feather6_TalkToRedshirt(void)
{
    feather6_Say(N_Lt_Stragey, T_FEA6_042);
    feather6_Say(N_Capt_Kirk, T_FEA6_002);
    feather6_Say(N_Lt_Stragey, T_FEA6_041);
    feather6_Say(N_Dr_McCoy, T_FEA6_016);
    feather6_Say(N_Lt_Stragey, T_FEA6_043);
    return;
}

/* [MSG_TALK 01 00 00] */
static void feather6_TalkToSpock(void)
{
    uint16_t ax = 0;
    if (room.redshirt_dead == 1) {
        feather6_Say(N_Mr_Spock, T_FEA6_038);
        ax = (uint16_t)feather6_Talk(KirkOnQuetzecoatl);
        if (ax == 0) {
        } else {
            if (ax == 1) {
                goto L0627;
            }
            goto L0631;
        }
        feather6_Say(N_Mr_Spock, T_FEA6_029);
        return;
L0627:;
        feather6_Say(N_Mr_Spock, T_FEA6_030);
        return;
L0631:;
        feather6_Say(N_Mr_Spock, T_FEA6_031);
        return;
    }
    feather6_Say(N_Mr_Spock, T_FEA6_035);
    return;
}

/* Tricorders.
 * [MSG_USE 45 20 00] */
static void feather6_McCoyScansEntrance(void)
{
    svc_Anim(2, N_mscane, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Dr_McCoy, T_FEA6_012);
    return;
}

/* [MSG_USE 45 21 00] */
static void feather6_McCoyScansRocks(void)
{
    feather6_Say(N_Dr_McCoy, T_FEA6_015);
    return;
}

/* [MSG_USE 44 20 00] */
static void feather6_SpockScansEntrance(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_024);
    return;
}

/* [MSG_USE 44 FF 00] */
static void feather6_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_022);
    return;
}

/* [MSG_USE 44 21 00] */
static void feather6_SpockScansRocks(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_018);
    return;
}

/* [MSG_USE 44 08 00] */
static void feather6_SpockScansCrystal(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_027);
    return;
}

/* [MSG_USE 44 22 00] */
static void feather6_SpockScansStalactites(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_025);
    return;
}

/* [MSG_USE 44 23 00] */
static void feather6_SpockScansStalagmites(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    feather6_Say(N_Mr_Spock, T_FEA6_026);
    return;
}

/* The crystal: pried loose with the knife (msgs 2, 3), +1; timer 4 takes
 * it off the wall.
 * [MSG_GET 08 00 00] */
static void feather6_GetCrystal(void)
{
    feather6_Say(S_0941, T_FEA6N020);
    return;
}

/* [MSG_USE 79 08 00] */
static void feather6_KnifeOnCrystal(void)
{
    room.busy = 1;
    svc_Walk(0, 0x9A, 0x97, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void feather6_KirkPries(void)
{
    svc_Anim(0, N_s5r6kp, -1, -1, 3);
    room.timers[4] = 0x7A;
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void feather6_TookCrystal(void)
{
    room.busy = 0;
    svc_StopAnim(0);
    rv_score += 1;
    svc_GiveItem(IT_CRYSTAL);
    feather6_Say(S_0941, T_FEA6N017);
    return;
}

/* [MSG_TIMER 04 00 00] */
static void feather6_CrystalGone(void)
{
    svc_StopAnim(8);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t feather6_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void feather6_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1CF7) = bx;
    feather6_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript feather6_scripts[] = {
    { 0x012C, feather6_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0186, feather6_BackToRiver },    /* MSG_REGION 00 00 00 */
    { 0x0196, feather6_SpockOnCrystal },    /* MSG_USE 01 08 00 */
    { 0x01A6, feather6_McCoyOnCrystal },    /* MSG_USE 02 08 00 */
    { 0x01B6, feather6_StrageyOnCrystal },    /* MSG_USE 03 08 00 */
    { 0x01C6, feather6_E_01C6 },    /* MSG_USE 43 09 00 */
    { 0x01CE, feather6_RockAtStalactites },    /* MSG_USE 43 22 00 */
    { 0x01ED, feather6_KirkThrows },    /* MSG_WALK_DONE 01 00 00 */
    { 0x021F, feather6_StalactiteFalls },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0268, feather6_KirkKilled },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x029B, feather6_RockOnCrystal },    /* MSG_USE 43 08 00 */
    { 0x02BA, feather6_KirkHammers },    /* MSG_WALK_DONE 04 00 00 */
    { 0x0313, feather6_Rumbles },    /* MSG_TICK FF FF FF */
    { 0x0339, feather6_FewMoreBlows },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x0359, feather6_StalactiteLoose },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x0396, feather6_RockAtStalagmites },    /* MSG_USE 43 23 00 */
    { 0x03A6, feather6_RockAtEntrance },    /* MSG_USE 43 20 00 */
    { 0x03B6, feather6_UseRock },    /* MSG_USE 43 FF 00 */
    { 0x03C6, feather6_UseRockNever },    /* MSG_USE 43 FF 00 */
    { 0x03D6, feather6_KnifeOnStalagmites },    /* MSG_USE 79 23 00 */
    { 0x03E6, feather6_UseKnife },    /* MSG_USE 79 FF 00 */
    { 0x03F6, feather6_KnifeOnMcCoy },    /* MSG_USE 79 02 00 */
    { 0x0406, feather6_KnifeOnSpock },    /* MSG_USE 79 01 00 */
    { 0x0416, feather6_KnifeOnStragey },    /* MSG_USE 79 03 00 */
    { 0x0426, feather6_UseCrystal },    /* MSG_USE 78 FF 00 */
    { 0x0436, feather6_UseSnake },    /* MSG_USE 76 FF 00 */
    { 0x0446, feather6_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0x04A9, feather6_StunAnything },    /* MSG_USE 40 FF 00 */
    { 0x04B9, feather6_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x04C9, feather6_LookAtEntrance },    /* MSG_LOOK 20 00 00 */
    { 0x04D8, feather6_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x04E7, feather6_LookAtRocks },    /* MSG_LOOK 21 00 00 */
    { 0x04F6, feather6_LookAtStalagmites },    /* MSG_LOOK 23 00 00 */
    { 0x0505, feather6_LookAtCrystal },    /* MSG_LOOK 08 00 00 */
    { 0x0514, feather6_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0523, feather6_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0532, feather6_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0541, feather6_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0550, feather6_LookAtStalactites },    /* MSG_LOOK 22 00 00 */
    { 0x05B1, feather6_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x05C1, feather6_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x05F5, feather6_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x064B, feather6_McCoyScansEntrance },    /* MSG_USE 45 20 00 */
    { 0x067C, feather6_McCoyScansRocks },    /* MSG_USE 45 21 00 */
    { 0x068C, feather6_SpockScansEntrance },    /* MSG_USE 44 20 00 */
    { 0x06BD, feather6_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x06EE, feather6_SpockScansRocks },    /* MSG_USE 44 21 00 */
    { 0x071F, feather6_SpockScansCrystal },    /* MSG_USE 44 08 00 */
    { 0x0750, feather6_SpockScansStalactites },    /* MSG_USE 44 22 00 */
    { 0x0781, feather6_SpockScansStalagmites },    /* MSG_USE 44 23 00 */
    { 0x07B2, feather6_GetCrystal },    /* MSG_GET 08 00 00 */
    { 0x07C2, feather6_KnifeOnCrystal },    /* MSG_USE 79 08 00 */
    { 0x07E1, feather6_KirkPries },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0802, feather6_TookCrystal },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x0832, feather6_CrystalGone },    /* MSG_TIMER 04 00 00 */
    { 0, 0 }
};
