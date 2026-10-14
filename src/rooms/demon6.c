/* DEMON6.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/demon6.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * DEMON6.RDF - "Demon World", Brother Stephen's study. His Ardak-4
 * synthesizer turns the Laraxian berries into the Hypo-Dytoxin for Brother
 * Chub; Spock repairs the android's hand at the workbench; the computer
 * runs a simulation of an eclipse. Stephen's display case holds a
 * meteorite, fossil shells, minerals, a skull and a corroded piece of metal
 * (the key the Nauian needs, room 4): talk him round and he unlocks it.
 *
 * Actors: 8 Brother Stephen (here once the berries have been found, or
 * after the Klingons until Chub has been examined), 9 the computer screen,
 * 10 (0Ah) the door.
 * Objects: 8 Brother Stephen, 9/21h the computer, 0Ah/26h the door, 20h
 * the case, 22h the workbench, 23h the synthesizer, 24h the skylight,
 * 25h the glassware; the case's items by their item numbers.
 * Region 0: the doorstep. Exit 3: back to the colony (room 0).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 0F69: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50 (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_oldman = 0x00D2,
    N_scrnan = 0x00D9,
    N_spokop = 0x00E0,
    N_s0r6d1 = 0x00E7,
    N_s0r6d2 = 0x00EE,
    N_musemn = 0x00F5,
    N_sscans = 0x00FC,
    N_mscans = 0x0103,
    N_DEM6LOOP = 0x0112,
    T_DEM6_028 = 0x0265,
    T_DEM6_017 = 0x02CB,
    T_DEM6N024 = 0x040C,
    T_DEM6N021 = 0x0473,
    T_DEM6_027 = 0x052A,
    T_DEM6_026 = 0x05CD,
    N_LD1REPLI = 0x065F,
    T_DEM6N023 = 0x0697,
    T_DEM6_018 = 0x06E5,
    T_DEM6_005 = 0x0787,
    T_DEM6_004 = 0x097B,
    T_DEM6_015 = 0x09F5,
    T_DEM6_052 = 0x0A5F,
    T_DEM6_016 = 0x0B7E,
    T_DEM6_009 = 0x0BF1,
    T_DEM6_007 = 0x0CA7,
    T_DEM6_008 = 0x0E54,
    T_DEM6N022 = 0x0EE4,
    case_items = 0x0F64,
    N_Capt_Kirk = 0x1036,
    N_Dr_McCoy = 0x1041,
    N_Mr_Spock = 0x104B,
    N_Ensign_Everts = 0x1055,
    N_Brother_Stephen = 0x1063,
    S_1077 = 0x1077,
    T_DEM6_042 = 0x1078,
    T_DEM6N006 = 0x1192,
    T_DEM6N003 = 0x11F0,
    T_DEM6N001 = 0x1253,
    T_DEM6N016 = 0x1307,
    T_DEM6N011 = 0x132D,
    T_DEM6N017 = 0x1360,
    T_DEM6N005 = 0x13C2,
    T_DEM6N000 = 0x13F1,
    T_DEM6N002 = 0x1436,
    T_DEM6N004 = 0x1484,
    T_DEM6N007 = 0x14CB,
    T_DEM6N008 = 0x1530,
    T_DEM6N015 = 0x157D,
    T_DEM6N009 = 0x15DA,
    T_DEM6N010 = 0x161E,
    T_DEM6N020 = 0x1676,
    T_DEM6_024 = 0x16B4,
    T_DEM6N018 = 0x1776,
    T_DEM6N012 = 0x17BA,
    T_DEM6_019 = 0x17D7,
    T_DEM6_044 = 0x1845,
    T_DEM6_030 = 0x191F,
    T_DEM6_031 = 0x197C,
    T_DEM6_034 = 0x19D5,
    T_DEM6_051 = 0x1A0B,
    T_DEM6_039 = 0x1B12,
    T_DEM6_040 = 0x1BD2,
    T_DEM6_036 = 0x1C2A,
    T_DEM6_038 = 0x1C93,
    T_DEM6_041 = 0x1D8E,
    T_DEM6_033 = 0x1E4E,
    S_2016 = 0x2016,
    T_DEM6_014 = 0x2034,
    T_DEM6N019 = 0x20B4,
    T_DEM6_029 = 0x20FB,
    T_DEM6_043 = 0x212D,
    T_DEM6_037 = 0x2251,
    T_DEM6_020 = 0x230D,
    T_DEM6_025 = 0x23E4,
    T_DEM6_002 = 0x249D,
    T_DEM6_053 = 0x24C5,
    T_DEM6_035 = 0x2503,
    T_DEM6_013 = 0x2581,
    T_DEM6_021 = 0x25F6,
    T_DEM6_012 = 0x2654,
    T_DEM6_011 = 0x2738,
    T_DEM6_010 = 0x2799,
    T_DEM6_023 = 0x27D5,
    talk_buf = 0x2890,
    KirkOnCollection = 0x2896,
    KirkOnFinds = 0x28A0,
    KirkAsksAbout = 0x28A8,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_case_open CS_B(0xCD)
#define fv_door_steps CS_B(0xCE)
#define fv_going_out CS_B(0xCF)
#define fv_stephen_here CS_B(0xCB)
#define fv_stephen_offended CS_B(0xCA)
#define fv_talk_buf CS_W(0x2890)
#define rv_examined_chub ROOM_B(0x3E)
#define rv_got_berries ROOM_B(0x50)
#define rv_hand_repaired ROOM_B(0x3B)
#define rv_hypo_scored ROOM_B(0x54)
#define rv_insulted_stephen ROOM_W(0x2B)
#define rv_made_hypo ROOM_B(0x51)
#define rv_need_berries ROOM_B(0x3F)
#define rv_saw_klingons ROOM_B(0x37)
#define rv_saw_simulation ROOM_B(0x55)
#define rv_score ROOM_W(0x58)
#define rv_stephen_welcomed ROOM_B(0x34)
#define rv_taken_items ROOM_B(0x4F)

/* functions */
static void demon6_StartRoom(void);
static void demon6_StephenWelcomes(void);
static void demon6_SpockAtComputer(void);
static void demon6_E_01C8(void);
static void demon6_WalkToDoor(void);
static void demon6_ReachDoor(void);
static void demon6_E_0227(void);
static void demon6_GoOut(void);
static void demon6_E_0252(void);
static void demon6_PhaserStephen(void);
static void demon6_E_02B8(void);
static void demon6_PhaserCase(void);
static void demon6_LookAtWorkbench(void);
static void demon6_LookAtCase(void);
static void demon6_LookAtGlassware(void);
static void demon6_LookAtMinerals(void);
static void demon6_LookAtShells(void);
static void demon6_LookAtSkull(void);
static void demon6_LookAtMetal(void);
static void demon6_LookAtMeteorite(void);
static void demon6_LookAtSkylight(void);
static void demon6_LookAtSynthesizer(void);
static void demon6_LookAtKirk(void);
static void demon6_LookAtSpock(void);
static void demon6_LookAtMcCoy(void);
static void demon6_LookAtRedshirt(void);
static void demon6_E_03DB(void);
static void demon6_LookAtComputer(void);
static void demon6_LookAtScene(void);
static void demon6_LookAtStephen(void);
static void demon6_TalkToMcCoy(void);
static void demon6_TalkToSpock(void);
static void demon6_TalkToKirk(void);
static void demon6_TalkToRedshirt(void);
static void demon6_TalkToStephen(void);
static void demon6_ShowBerriesToStephen(void);
static void demon6_UseBerriesOnSynthesizer(void);
static void demon6_McCoyAtSynthesizer(void);
static void demon6_HypoMade(void);
static void demon6_UseHandOnWorkbench(void);
static void demon6_SpockAtWorkbench(void);
static void demon6_HandRepaired(void);
static void demon6_UseWorkbench(void);
static void demon6_E_0837(void);
static void demon6_McCoyAtCase(void);
static void demon6_SpockAtCase(void);
static void demon6_AskAboutCase(void);
static void demon6_E_0968(void);
static void demon6_KirkAtComputer(void);
static void demon6_E_09E2(void);
static void demon6_McCoyAtComputer(void);
static void demon6_E_0A4C(void);
static void demon6_EvertsAtComputer(void);
static void demon6_E_0AAF(void);
static void demon6_SpockUsesComputer(void);
static void demon6_ReturnMinerals(void);
static void demon6_ReturnShells(void);
static void demon6_ReturnMeteorite(void);
static void demon6_ReturnSkull(void);
static void demon6_ReturnMetal(void);
static void demon6_part_0B0F(uint16_t s0);
static void demon6_ShowHandToStephen(void);
static void demon6_McCoyScansStephen(void);
static void demon6_SpockScansGlassware(void);
static void demon6_E_0C73(void);
static void demon6_SpockScansComputer(void);
static void demon6_SpockScansMinerals(void);
static void demon6_SpockScansShells(void);
static void demon6_SpockScansSkull(void);
static void demon6_SpockScansMetal(void);
static void demon6_SpockScansMeteorite(void);
static void demon6_SpockScansCase(void);
static void demon6_SpockScansSynthesizer(void);
static void demon6_GetFromCase(void);
static void demon6_AtCase(uint16_t ax);
static uint16_t demon6_Talk(uint16_t ax);
static void demon6_Say(uint16_t ax, uint16_t bx);

/* The room starts: Brother Stephen if he is here, the door, the computer.
 * [MSG_TICK 01 00 00] */
static void demon6_StartRoom(void)
{
    svc_Sfx(N_DEM6LOOP);
    if (rv_got_berries != 1) {
        if (rv_examined_chub == 1)
            goto L0158;
        if (rv_saw_klingons != 1)
            goto L0158;
    }
    svc_Anim0(8, N_oldman, 0x5F, 0xB1, 0);
    fv_stephen_here = 1;
L0158:;
    svc_Anim0(0xA, N_s0r6d2, 0xA0, 0x92, 0);
    svc_Anim0(9, N_scrnan, 0, 0xC7, 0);
    return;
}

/* Tick 30: Stephen says hello, once.
 * [MSG_TICK 1E 00 00] */
static void demon6_StephenWelcomes(void)
{
    if (rv_got_berries == 1) {
        if (rv_examined_chub != 1) {
            if (rv_saw_klingons == 1) {
                if (rv_stephen_welcomed != 1) {
                    demon6_Say(N_Brother_Stephen, T_DEM6_042);
                    rv_stephen_welcomed = 1;
                }
            }
        }
    }
    return;
}

/* Spock has reached the computer (walk msg 3): the eclipse simulation.
 * [MSG_WALK_DONE 03 00 00] */
static void demon6_SpockAtComputer(void)
{
    demon6_Say(N_Mr_Spock, T_DEM6_014);
    return;
}

/* Leaving: walk to the door (msg 4); at the doorstep it opens (anim
 * msg 2), and when both are done, back to the colony.
 * [MSG_WALK 26 00 00] */
static void demon6_E_01C8(void)
{
    { demon6_WalkToDoor(); return; }
}

/* [MSG_WALK 0A 00 00] */
static void demon6_WalkToDoor(void)
{
    room.busy = 1;
    fv_going_out = 1;
    svc_Walk(0, 0xA0, 0x94, 4);
    return;
}

/* [MSG_REGION 00 00 00] */
static void demon6_ReachDoor(void)
{
    if (fv_going_out == 1) {
        svc_Anim0(0xA, N_s0r6d1, 0xA0, 0x92, 2);
        svc_BridgeSfx(5);
    }
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void demon6_E_0227(void)
{
    { demon6_GoOut(); return; }
}

/* [MSG_ANIM_DONE 02 00 00] */
static void demon6_GoOut(void)
{
    fv_door_steps++;
    if (fv_door_steps != 2) {
        return;
    }
    svc_GotoRoom(0, 2);
    return;
}

/* Phasers.
 * [MSG_USE 41 08 00] */
static void demon6_E_0252(void)
{
    { demon6_PhaserStephen(); return; }
}

/* [MSG_USE 40 08 00] */
static void demon6_PhaserStephen(void)
{
    demon6_Say(N_Brother_Stephen, T_DEM6_028);
    return;
}

/* [MSG_USE 41 20 00] */
static void demon6_E_02B8(void)
{
    { demon6_PhaserCase(); return; }
}

/* [MSG_USE 40 20 00] */
static void demon6_PhaserCase(void)
{
    demon6_Say(N_Dr_McCoy, T_DEM6_017);
    return;
}

/* Looking at things. The computer's simulation is worth a point, once.
 * [MSG_LOOK 22 00 00] */
static void demon6_LookAtWorkbench(void)
{
    demon6_Say(0, T_DEM6N003);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void demon6_LookAtCase(void)
{
    demon6_Say(0, T_DEM6N001);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void demon6_LookAtGlassware(void)
{
    demon6_Say(0, T_DEM6N007);
    return;
}

/* [MSG_LOOK 5B 00 00] */
static void demon6_LookAtMinerals(void)
{
    demon6_Say(0, T_DEM6N016);
    return;
}

/* [MSG_LOOK 5D 00 00] */
static void demon6_LookAtShells(void)
{
    demon6_Say(0, T_DEM6N011);
    return;
}

/* [MSG_LOOK 5A 00 00] */
static void demon6_LookAtSkull(void)
{
    demon6_Say(0, T_DEM6N017);
    return;
}

/* [MSG_LOOK 59 00 00] */
static void demon6_LookAtMetal(void)
{
    demon6_Say(0, T_DEM6N005);
    return;
}

/* [MSG_LOOK 5C 00 00] */
static void demon6_LookAtMeteorite(void)
{
    demon6_Say(0, T_DEM6N000);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void demon6_LookAtSkylight(void)
{
    demon6_Say(0, T_DEM6N002);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void demon6_LookAtSynthesizer(void)
{
    demon6_Say(0, T_DEM6N004);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void demon6_LookAtKirk(void)
{
    demon6_Say(0, T_DEM6N008);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void demon6_LookAtSpock(void)
{
    demon6_Say(0, T_DEM6N015);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void demon6_LookAtMcCoy(void)
{
    demon6_Say(0, T_DEM6N009);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void demon6_LookAtRedshirt(void)
{
    demon6_Say(0, T_DEM6N010);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void demon6_E_03DB(void)
{
    { demon6_LookAtComputer(); return; }
}

/* [MSG_LOOK 09 00 00] */
static void demon6_LookAtComputer(void)
{
    demon6_Say(S_1077, T_DEM6N006);
    if (rv_saw_simulation != 1) {
        rv_saw_simulation = 1;
        rv_score += 1;
    }
    return;
}

/* [MSG_LOOK FF 00 00] */
static void demon6_LookAtScene(void)
{
    demon6_Say(0, T_DEM6N024);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void demon6_LookAtStephen(void)
{
    demon6_Say(0, T_DEM6N021);
    return;
}

/* Talking.
 * [MSG_TALK 02 00 00] */
static void demon6_TalkToMcCoy(void)
{
    demon6_Say(N_Dr_McCoy, T_DEM6_020);
    return;
}

/* [MSG_TALK 01 00 00] */
static void demon6_TalkToSpock(void)
{
    demon6_Say(N_Mr_Spock, T_DEM6_025);
    return;
}

/* [MSG_TALK 00 00 00] */
static void demon6_TalkToKirk(void)
{
    demon6_Say(N_Capt_Kirk, T_DEM6_002);
    return;
}

/* [MSG_TALK 03 00 00] */
static void demon6_TalkToRedshirt(void)
{
    demon6_Say(N_Ensign_Everts, T_DEM6_053);
    return;
}

/* [MSG_TALK 08 00 00] */
static void demon6_TalkToStephen(void)
{
    demon6_Say(N_Brother_Stephen, T_DEM6_035);
    return;
}

/* [MSG_USE 7B 08 00] */
static void demon6_ShowBerriesToStephen(void)
{
    if (rv_need_berries != 1) {
    } else {
        demon6_Say(N_Brother_Stephen, T_DEM6_027);
        return;
    }
    demon6_Say(N_Brother_Stephen, T_DEM6_026);
    return;
}

/* McCoy makes the Hypo-Dytoxin from the berries, once he knows they are
 * the cure (msgs 1): +1 score, once.
 * [MSG_USE 7B 23 00] */
static void demon6_UseBerriesOnSynthesizer(void)
{
    if (rv_need_berries != 1) {
        return;
    }
    svc_Walk(2, 0x104, 0xA5, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void demon6_McCoyAtSynthesizer(void)
{
    svc_Anim(2, N_musemn, -1, -1, 1);
    svc_Sfx(N_LD1REPLI);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void demon6_HypoMade(void)
{
    svc_Music(-1, -1);
    demon6_Say(0, T_DEM6N023);
    demon6_Say(N_Dr_McCoy, T_DEM6_018);
    svc_GiveItem(IT_DETOXIN);
    rv_made_hypo = 1;
    svc_TakeItem(IT_BERRY);
    if (rv_hypo_scored != 1) {
        rv_hypo_scored = 1;
        rv_score += 1;
    }
    return;
}

/* Spock repairs the hand at the workbench (msgs 2, 3): +2 score.
 * [MSG_USE 42 22 00] */
static void demon6_UseHandOnWorkbench(void)
{
    if (rv_hand_repaired == 1) {
        demon6_Say(N_Mr_Spock, T_DEM6N018);
        return;
    }
    room.busy = 1;
    demon6_Say(N_Capt_Kirk, T_DEM6_005);
    svc_Walk(1, 0xB3, 0xBB, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void demon6_SpockAtWorkbench(void)
{
    svc_Anim0(1, N_spokop, 0xB3, 0xBB, 3);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void demon6_HandRepaired(void)
{
    demon6_Say(N_Mr_Spock, T_DEM6_024);
    rv_hand_repaired = 1;
    rv_score += 2;
    svc_StopAnim(1);
    room.busy = 0;
    return;
}

/* [MSG_USE FF 22 00] */
static void demon6_UseWorkbench(void)
{
    demon6_Say(S_1077, T_DEM6N020);
    return;
}

/* Asking about the case (with Stephen here): calling it junk offends him
 * (insulted_stephen); hearing him out about every item gets it unlocked.
 * [MSG_USE 00 20 00] */
static void demon6_E_0837(void)
{
    { demon6_McCoyAtCase(); return; }
}

/* [MSG_USE 02 20 00] */
static void demon6_McCoyAtCase(void)
{
    { demon6_SpockAtCase(); return; }
}

/* [MSG_USE 01 20 00] */
static void demon6_SpockAtCase(void)
{
    { demon6_AskAboutCase(); return; }
}

/* [MSG_USE 03 20 00] */
static void demon6_AskAboutCase(void)
{
    uint16_t ax = 0;
    if (fv_stephen_here != 1) {
        demon6_Say(S_1077, T_DEM6N012);
        demon6_Say(N_Dr_McCoy, T_DEM6_019);
        return;
    }
    if (fv_stephen_offended == 1) {
        return;
    }
    demon6_Say(N_Brother_Stephen, T_DEM6_044);
    ax = (uint16_t)demon6_Talk(KirkOnCollection);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L08AE;
        }
        if (ax == 2) {
            goto L08B9;
        }
        goto L0911;
    }
L0899:;
    demon6_Say(N_Brother_Stephen, T_DEM6_030);
    fv_stephen_offended = 1;
    rv_insulted_stephen = 1;
    return;
L08AE:;
    demon6_Say(N_Brother_Stephen, T_DEM6_034);
    goto L08D8;
L08B9:;
    demon6_Say(N_Brother_Stephen, T_DEM6_031);
    ax = (uint16_t)demon6_Talk(KirkOnFinds);
    if (ax == 0) {
        goto L0899;
    }
    if (ax == 1) {
        goto L08AE;
    }
    goto L0911;
L08D8:;
    demon6_Say(N_Brother_Stephen, T_DEM6_051);
    ax = (uint16_t)demon6_Talk(KirkAsksAbout);
    if (ax == 0) {
        goto L091B;
    }
    if (ax == 1) {
        goto L0926;
    }
    if (ax == 2) {
        goto L0931;
    }
    if (ax == 3) {
        goto L093C;
    }
    if (ax == 4) {
        goto L0947;
    }
    if (ax == 5) {
        goto L0952;
    }
L0911:;
    demon6_Say(S_1077, S_2016);
    return;
L091B:;
    demon6_Say(N_Brother_Stephen, T_DEM6_040);
    goto L08D8;
L0926:;
    demon6_Say(N_Brother_Stephen, T_DEM6_033);
    goto L08D8;
L0931:;
    demon6_Say(N_Brother_Stephen, T_DEM6_036);
    goto L08D8;
L093C:;
    demon6_Say(N_Brother_Stephen, T_DEM6_038);
    goto L08D8;
L0947:;
    demon6_Say(N_Brother_Stephen, T_DEM6_039);
    goto L08D8;
L0952:;
    demon6_Say(N_Brother_Stephen, T_DEM6_041);
    fv_case_open = 1;
    return;
}

/* The crew at the computer: only Spock will (walk msg 3).
 * [MSG_USE 00 21 00] */
static void demon6_E_0968(void)
{
    { demon6_KirkAtComputer(); return; }
}

/* [MSG_USE 00 09 00] */
static void demon6_KirkAtComputer(void)
{
    demon6_Say(N_Capt_Kirk, T_DEM6_004);
    { demon6_SpockUsesComputer(); return; }
}

/* [MSG_USE 02 21 00] */
static void demon6_E_09E2(void)
{
    { demon6_McCoyAtComputer(); return; }
}

/* [MSG_USE 02 09 00] */
static void demon6_McCoyAtComputer(void)
{
    demon6_Say(N_Dr_McCoy, T_DEM6_015);
    return;
}

/* [MSG_USE 03 21 00] */
static void demon6_E_0A4C(void)
{
    { demon6_EvertsAtComputer(); return; }
}

/* [MSG_USE 03 09 00] */
static void demon6_EvertsAtComputer(void)
{
    demon6_Say(N_Ensign_Everts, T_DEM6_052);
    return;
}

/* [MSG_USE 01 21 00] */
static void demon6_E_0AAF(void)
{
    { demon6_SpockUsesComputer(); return; }
}

/* [MSG_USE 01 09 00] */
static void demon6_SpockUsesComputer(void)
{
    svc_Walk(1, 0x3D, 0xC5, 3);
    return;
}

/* Giving the case's items back to Stephen clears their bits.
 * [MSG_USE 5B 08 00] */
static void demon6_ReturnMinerals(void)
{
    uint16_t s0;
    rv_taken_items &= 0xF7;
    s0 = 0x5B;
    { demon6_part_0B0F(s0); return; }
}

/* [MSG_USE 5D 08 00] */
static void demon6_ReturnShells(void)
{
    uint16_t s0;
    rv_taken_items &= 0xFD;
    s0 = 0x5D;
    { demon6_part_0B0F(s0); return; }
}

/* [MSG_USE 5C 08 00] */
static void demon6_ReturnMeteorite(void)
{
    uint16_t s0;
    rv_taken_items &= 0xFB;
    s0 = 0x5C;
    { demon6_part_0B0F(s0); return; }
}

/* [MSG_USE 5A 08 00] */
static void demon6_ReturnSkull(void)
{
    uint16_t s0;
    rv_taken_items &= 0xEF;
    s0 = 0x5A;
    { demon6_part_0B0F(s0); return; }
}

/* [MSG_USE 59 08 00] */
static void demon6_ReturnMetal(void)
{
    uint16_t s0;
    rv_taken_items &= 0xFE;
    s0 = 0x59;
    { demon6_part_0B0F(s0); return; }
}

static void demon6_part_0B0F(uint16_t s0)
{
    svc_TakeItem(s0);
    demon6_Say(S_1077, T_DEM6N019);
    demon6_Say(N_Brother_Stephen, T_DEM6_029);
    return;
}

/* [MSG_USE 42 08 00] */
static void demon6_ShowHandToStephen(void)
{
    if (rv_hand_repaired == 1) {
        demon6_Say(N_Brother_Stephen, T_DEM6_037);
        return;
    }
    demon6_Say(N_Brother_Stephen, T_DEM6_043);
    return;
}

/* Tricorders.
 * [MSG_USE 45 08 00] */
static void demon6_McCoyScansStephen(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Dr_McCoy, T_DEM6_016);
    return;
}

/* [MSG_USE 44 25 00] */
static void demon6_SpockScansGlassware(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_009);
    return;
}

/* [MSG_USE 44 09 00] */
static void demon6_E_0C73(void)
{
    { demon6_SpockScansComputer(); return; }
}

/* [MSG_USE 44 21 00] */
static void demon6_SpockScansComputer(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_007);
    return;
}

/* [MSG_USE 44 5B 00] */
static void demon6_SpockScansMinerals(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_013);
    return;
}

/* [MSG_USE 44 5D 00] */
static void demon6_SpockScansShells(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_021);
    return;
}

/* [MSG_USE 44 5A 00] */
static void demon6_SpockScansSkull(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_012);
    return;
}

/* [MSG_USE 44 59 00] */
static void demon6_SpockScansMetal(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_011);
    return;
}

/* [MSG_USE 44 5C 00] */
static void demon6_SpockScansMeteorite(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_010);
    return;
}

/* [MSG_USE 44 20 00] */
static void demon6_SpockScansCase(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_023);
    return;
}

/* [MSG_USE 44 23 00] */
static void demon6_SpockScansSynthesizer(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    demon6_Say(N_Mr_Spock, T_DEM6_008);
    return;
}

/* Taking from the case, once it is unlocked: walk there (msg 5), choose
 * with the item case, and take what was chosen (taken_items, case_items).
 * [MSG_GET 20 00 00] */
static void demon6_GetFromCase(void)
{
    if (fv_case_open != 1) {
        demon6_Say(0, T_DEM6N022);
        return;
    }
    svc_Walk(0, 0xFF, 0xBA, 5);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void demon6_AtCase(uint16_t ax)
{
    uint16_t di = 0;
    uint16_t t0;
    uint16_t t1;
    SETLO(ax, rv_taken_items);
    SETHI(ax, 0);
    ax = ax ^ 0x1F;
    t0 = ax;
    ax = (uint16_t)svc_ItemCase(ax);
    ax = ax & 0x1F;
    t1 = ax;
    ax = ax ^ 0x1F;
    rv_taken_items = LO(ax);
    di = case_items;
L0F48:;
    if ((LO(ax) & 1) != 0) {
        ax = 0;
        SETLO(ax, CS_B(0 + di));
        svc_GiveItem(ax);
        ax = ax;
    }
    di = di + 1;
    ax = ax >> 1;
    if (ax != 0)
        goto L0F48;
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t demon6_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void demon6_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x2892) = bx;
    demon6_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript demon6_scripts[] = {
    { 0x0110, demon6_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x018C, demon6_StephenWelcomes },    /* MSG_TICK 1E 00 00 */
    { 0x01B8, demon6_SpockAtComputer },    /* MSG_WALK_DONE 03 00 00 */
    { 0x01C8, demon6_E_01C8 },    /* MSG_WALK 26 00 00 */
    { 0x01D0, demon6_WalkToDoor },    /* MSG_WALK 0A 00 00 */
    { 0x01F5, demon6_ReachDoor },    /* MSG_REGION 00 00 00 */
    { 0x0227, demon6_E_0227 },    /* MSG_WALK_DONE 04 00 00 */
    { 0x022F, demon6_GoOut },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x0252, demon6_E_0252 },    /* MSG_USE 41 08 00 */
    { 0x025A, demon6_PhaserStephen },    /* MSG_USE 40 08 00 */
    { 0x02B8, demon6_E_02B8 },    /* MSG_USE 41 20 00 */
    { 0x02C0, demon6_PhaserCase },    /* MSG_USE 40 20 00 */
    { 0x0309, demon6_LookAtWorkbench },    /* MSG_LOOK 22 00 00 */
    { 0x0318, demon6_LookAtCase },    /* MSG_LOOK 20 00 00 */
    { 0x0327, demon6_LookAtGlassware },    /* MSG_LOOK 25 00 00 */
    { 0x0336, demon6_LookAtMinerals },    /* MSG_LOOK 5B 00 00 */
    { 0x0345, demon6_LookAtShells },    /* MSG_LOOK 5D 00 00 */
    { 0x0354, demon6_LookAtSkull },    /* MSG_LOOK 5A 00 00 */
    { 0x0363, demon6_LookAtMetal },    /* MSG_LOOK 59 00 00 */
    { 0x0372, demon6_LookAtMeteorite },    /* MSG_LOOK 5C 00 00 */
    { 0x0381, demon6_LookAtSkylight },    /* MSG_LOOK 24 00 00 */
    { 0x0390, demon6_LookAtSynthesizer },    /* MSG_LOOK 23 00 00 */
    { 0x039F, demon6_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x03AE, demon6_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x03BD, demon6_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x03CC, demon6_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x03DB, demon6_E_03DB },    /* MSG_LOOK 21 00 00 */
    { 0x03E3, demon6_LookAtComputer },    /* MSG_LOOK 09 00 00 */
    { 0x0401, demon6_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0468, demon6_LookAtStephen },    /* MSG_LOOK 08 00 00 */
    { 0x04C5, demon6_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x04D5, demon6_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x04E5, demon6_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x04F5, demon6_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0505, demon6_TalkToStephen },    /* MSG_TALK 08 00 00 */
    { 0x0515, demon6_ShowBerriesToStephen },    /* MSG_USE 7B 08 00 */
    { 0x0626, demon6_UseBerriesOnSynthesizer },    /* MSG_USE 7B 23 00 */
    { 0x0648, demon6_McCoyAtSynthesizer },    /* MSG_WALK_DONE 01 00 00 */
    { 0x067E, demon6_HypoMade },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0768, demon6_UseHandOnWorkbench },    /* MSG_USE 42 22 00 */
    { 0x07E1, demon6_SpockAtWorkbench },    /* MSG_WALK_DONE 02 00 00 */
    { 0x07FF, demon6_HandRepaired },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x0827, demon6_UseWorkbench },    /* MSG_USE FF 22 00 */
    { 0x0837, demon6_E_0837 },    /* MSG_USE 00 20 00 */
    { 0x083F, demon6_McCoyAtCase },    /* MSG_USE 02 20 00 */
    { 0x0847, demon6_SpockAtCase },    /* MSG_USE 01 20 00 */
    { 0x084F, demon6_AskAboutCase },    /* MSG_USE 03 20 00 */
    { 0x0968, demon6_E_0968 },    /* MSG_USE 00 21 00 */
    { 0x0970, demon6_KirkAtComputer },    /* MSG_USE 00 09 00 */
    { 0x09E2, demon6_E_09E2 },    /* MSG_USE 02 21 00 */
    { 0x09EA, demon6_McCoyAtComputer },    /* MSG_USE 02 09 00 */
    { 0x0A4C, demon6_E_0A4C },    /* MSG_USE 03 21 00 */
    { 0x0A54, demon6_EvertsAtComputer },    /* MSG_USE 03 09 00 */
    { 0x0AAF, demon6_E_0AAF },    /* MSG_USE 01 21 00 */
    { 0x0AB7, demon6_SpockUsesComputer },    /* MSG_USE 01 09 00 */
    { 0x0AD1, demon6_ReturnMinerals },    /* MSG_USE 5B 08 00 */
    { 0x0ADF, demon6_ReturnShells },    /* MSG_USE 5D 08 00 */
    { 0x0AED, demon6_ReturnMeteorite },    /* MSG_USE 5C 08 00 */
    { 0x0AFB, demon6_ReturnSkull },    /* MSG_USE 5A 08 00 */
    { 0x0B09, demon6_ReturnMetal },    /* MSG_USE 59 08 00 */
    { 0x0B32, demon6_ShowHandToStephen },    /* MSG_USE 42 08 00 */
    { 0x0B52, demon6_McCoyScansStephen },    /* MSG_USE 45 08 00 */
    { 0x0BC5, demon6_SpockScansGlassware },    /* MSG_USE 44 25 00 */
    { 0x0C73, demon6_E_0C73 },    /* MSG_USE 44 09 00 */
    { 0x0C7B, demon6_SpockScansComputer },    /* MSG_USE 44 21 00 */
    { 0x0D02, demon6_SpockScansMinerals },    /* MSG_USE 44 5B 00 */
    { 0x0D33, demon6_SpockScansShells },    /* MSG_USE 44 5D 00 */
    { 0x0D64, demon6_SpockScansSkull },    /* MSG_USE 44 5A 00 */
    { 0x0D95, demon6_SpockScansMetal },    /* MSG_USE 44 59 00 */
    { 0x0DC6, demon6_SpockScansMeteorite },    /* MSG_USE 44 5C 00 */
    { 0x0DF7, demon6_SpockScansCase },    /* MSG_USE 44 20 00 */
    { 0x0E28, demon6_SpockScansSynthesizer },    /* MSG_USE 44 23 00 */
    { 0x0ED1, demon6_GetFromCase },    /* MSG_GET 20 00 00 */
    /* 0F29: a helper, not a script */
    { 0, 0 }
};
