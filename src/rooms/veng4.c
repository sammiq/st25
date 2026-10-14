/* VENG4.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/veng4.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * VENG4.RDF - "Vengeance", the Republic's sickbay. Kirk's academy friend
 * Brittany Marata is dying here; treated, she says the attackers were the
 * Enterprise, then dies. An empty hypo and a medical drill lie about.
 *
 * Actors: 8 Marata, 9 the door, 0Ah (10) the hypo, 0Bh (11) the drill,
 * 0Ch (12) the monitor. Objects: those, 20h the door, 21h Marata's bed,
 * 22h a body, 23h the scanner, 24h a monitor. Exit 2: the corridor (room
 * 3).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 104D: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s9r1do = 0x00CC,
    N_s9r1bh = 0x00DA,
    N_museme = 0x00E1,
    N_mscane = 0x00E8,
    N_s9r1dc = 0x0107,
    N_s7r4m2 = 0x0127,
    N_s9r1bm = 0x014B,
    N_VEN4LOOP = 0x016A,
    N_VE42LOOP = 0x0186,
    N_s9r1bh_2 = 0x01A0,
    N_s7r4h1 = 0x01C5,
    N_s7r4dl = 0x01EB,
    N_s7r4bd = 0x0211,
    N_s7r4m1d = 0x0230,
    T_VEN4_008 = 0x02BF,
    N_s9r1bm_2 = 0x0454,
    N_s7r4m1 = 0x047C,
    N_s7r4bd_2 = 0x049A,
    N_VE42LOOP_2 = 0x04C9,
    T_VEN4_010 = 0x04ED,
    T_VEN4N005 = 0x05DA,
    T_VEN4N004 = 0x0668,
    T_VEN4N017 = 0x06E8,
    T_VEN4N012 = 0x0786,
    T_VEN4N010 = 0x07E3,
    T_VEN4N009 = 0x082A,
    T_VEN4N011 = 0x0889,
    N_kusemn = 0x0916,
    T_VEN4N015 = 0x0968,
    N_kusemn_2 = 0x09F2,
    T_VEN4N016 = 0x0A44,
    T_VEN2_098 = 0x0B3D,
    T_VENA_F40 = 0x0B9C,
    T_VENA_F32 = 0x0C23,
    T_VENA_F34 = 0x0D31,
    T_VEN4N010_2 = 0x0DCB,
    T_VEN6N007 = 0x0E0C,
    T_MUD4_018 = 0x0E61,
    T_BRIDU146 = 0x0EA6,
    T_MUD4_018_2 = 0x0EE6,
    T_VEN2U093 = 0x0F2A,
    T_VEN2_028 = 0x0F95,
    T_VEN2_066 = 0x0FE3,
    N_Capt_Kirk = 0x10FB,
    N_Dr_McCoy = 0x1106,
    N_Mr_Spock = 0x1110,
    N_Ensign_Kije = 0x111A,
    N_Brittany_Marata = 0x1126,
    N_Lt_Uhura = 0x1136,
    N_Elasi_Captain = 0x1140,
    S_114E = 0x114E,
    T_VEN4_007 = 0x114F,
    T_VEN4_011 = 0x1186,
    T_VEN4_001 = 0x11E7,
    T_VEN4N002 = 0x1235,
    T_VEN4N000 = 0x1275,
    T_VEN4N003 = 0x12AB,
    T_VEN4N001 = 0x12DF,
    T_VEN4_005 = 0x1312,
    T_VEN4_014 = 0x1391,
    T_VEN4N007 = 0x13D2,
    S_140D = 0x140D,
    T_VEN4N014 = 0x1448,
    T_VEN4N006 = 0x14A0,
    T_VEN4N013 = 0x14E8,
    T_VEN4N008 = 0x1529,
    T_VEN4_002 = 0x156D,
    T_VEN4_017 = 0x15C8,
    T_VEN4_015 = 0x1604,
    T_VEN4_022 = 0x1706,
    T_VEN4_019 = 0x174C,
    T_VEN4_004 = 0x1775,
    T_VEN4_020 = 0x179B,
    T_VEN4_006 = 0x17EC,
    T_VEN4_021 = 0x1816,
    T_VEN4_003 = 0x185E,
    T_VEN4_016 = 0x188F,
    T_VEN4_012 = 0x18BD,
    T_VEN4_009 = 0x18F3,
    T_VEN4_013 = 0x1952,
    talk_buf = 0x198D,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_door_opening CS_B(0xCA)
#define fv_talk_buf CS_W(0x198D)
#define fv_with_medkit CS_B(0xCB)
#define rv_attack_timer ROOM_W(0x51)
#define rv_board_timer ROOM_W(0x49)
#define rv_clue_marata ROOM_B(0x69)
#define rv_complied ROOM_B(0x45)
#define rv_comply_timer ROOM_W(0x4D)
#define rv_drill_taken ROOM_B(0x58)
#define rv_elasi_here ROOM_B(0x43)
#define rv_enterprise_away ROOM_B(0x36)
#define rv_hypo_oiled ROOM_B(0x3A)
#define rv_hypo_taken ROOM_B(0x57)
#define rv_knew_marata ROOM_B(0x2C)
#define rv_marata_dead ROOM_B(0x2B)
#define rv_marata_spoke ROOM_B(0x2A)
#define rv_marata_treated ROOM_B(0x29)
#define rv_power_to ROOM_B(0x3C)
#define rv_rescued ROOM_B(0x42)
#define rv_saw_marata ROOM_B(0x56)
#define rv_stall_timer ROOM_W(0x4B)
#define rv_torpedo_timer ROOM_W(0x4F)

/* functions */
static void veng4_StartRoom(void);
static void veng4_TalkToKirk(void);
static void veng4_TalkToMcCoy(void);
static void veng4_TalkToSpock(void);
static void veng4_TalkToRedshirt(void);
static void veng4_E_0294(void);
static void veng4_MTricorderOnBody(void);
static void veng4_McCoyOnBody(void);
static void veng4_HypoOnMarata(void);
static void veng4_MedkitOnMarata(void);
static void veng4_E_0306(void);
static void veng4_McCoyOnMarata(void);
static void veng4_part_0314(void);
static void veng4_part_032A(void);
static void veng4_McCoyAtMarata(void);
static void veng4_TreatedMarata(void);
static void veng4_ScannedMarata(void);
static void veng4_TalkToMarata(void);
static void veng4_LookAtBody(void);
static void veng4_LookAtMonitor(void);
static void veng4_LookAtDoor(void);
static void veng4_LookAtKirk(void);
static void veng4_LookAtSpock(void);
static void veng4_LookAtMcCoy(void);
static void veng4_LookAtRedshirt(void);
static void veng4_LookAtBed(void);
static void veng4_LookAtScanner(void);
static void veng4_LookAtMarata(void);
static void veng4_E_0773(void);
static void veng4_LookAtDrill(void);
static void veng4_E_07D0(void);
static void veng4_LookAtHypo2(void);
static void veng4_LookAtDoor2(void);
static void veng4_LookAtSickbay(void);
static void veng4_GetHypo(void);
static void veng4_KirkAtHypo(void);
static void veng4_TookHypo(void);
static void veng4_GetDrill(void);
static void veng4_KirkAtDrill(void);
static void veng4_TookDrill(void);
static void veng4_DoorOpens(void);
static void veng4_E_0ACC(void);
static void veng4_WalkToDoor(void);
static void veng4_ElasiDeadlines(void);
static void veng4_E_0D1D(void);
static void veng4_KillAnything(void);
static void veng4_LookAtHypo(void);
static void veng4_CallEnterprise(void);
static void veng4_Talk(uint16_t ax);
static void veng4_Say(uint16_t ax, uint16_t bx);

/* The room starts: Marata alive or dead, the hypo, the drill.
 * [MSG_TICK 01 00 00] */
static void veng4_StartRoom(void)
{
    svc_Music(0x1F, STALE_ARG);
    svc_Anim(9, N_s9r1dc, 0x13F, 0xAB, 0);
    svc_Anim(0xC, N_s7r4m2, 0x5C, 0x75, 0);
    if (rv_marata_treated != 1) {
        svc_Anim(8, N_s9r1bm, 0x5A, 0xA5, 0);
        svc_Sfx(N_VEN4LOOP);
    } else {
        svc_Sfx(N_VE42LOOP);
        svc_Anim(8, N_s9r1bh_2, 0x5A, 0xA5, 0);
    }
    if (rv_hypo_taken != 1) {
        svc_Anim(0xA, N_s7r4h1, 0xEA, 0x8B, 0);
    }
    if (rv_drill_taken != 1) {
        svc_Anim(0xB, N_s7r4dl, 0xEA, 0x87, 0);
    }
    if (rv_marata_dead == 1) {
        svc_Anim(8, N_s7r4bd, 0x5A, 0xA5, 0);
        svc_Anim(0xC, N_s7r4m1d, 0x5C, 0x75, 0);
    }
    return;
}

/* Talking.
 * [MSG_TALK 00 00 00] */
static void veng4_TalkToKirk(void)
{
    veng4_Say(N_Capt_Kirk, T_VEN4_002);
    return;
}

/* [MSG_TALK 02 00 00] */
static void veng4_TalkToMcCoy(void)
{
    veng4_Say(N_Dr_McCoy, T_VEN4_015);
    return;
}

/* [MSG_TALK 01 00 00] */
static void veng4_TalkToSpock(void)
{
    veng4_Say(N_Mr_Spock, T_VEN4_017);
    return;
}

/* [MSG_TALK 03 00 00] */
static void veng4_TalkToRedshirt(void)
{
    veng4_Say(N_Ensign_Kije, T_VEN4_022);
    return;
}

/* McCoy, the dead crewman and the hypo.
 * [MSG_USE 7F 22 00] */
static void veng4_E_0294(void)
{
    { veng4_MTricorderOnBody(); return; }
}

/* [MSG_USE 45 22 00] */
static void veng4_MTricorderOnBody(void)
{
    { veng4_McCoyOnBody(); return; }
}

/* [MSG_USE 02 22 00] */
static void veng4_McCoyOnBody(void)
{
    veng4_Say(N_Dr_McCoy, T_VEN4_007);
    return;
}

/* [MSG_USE 82 08 00] */
static void veng4_HypoOnMarata(void)
{
    veng4_Say(N_Dr_McCoy, T_VEN4_008);
    return;
}

/* McCoy on Marata (msg 5): the tricorder scans her (anim 7), the medkit
 * treats her (anim 6), which counts as evidence and is what lets her talk.
 * [MSG_USE 7F 08 00] */
static void veng4_MedkitOnMarata(void)
{
    fv_with_medkit = 1;
    { veng4_part_0314(); return; }
}

/* [MSG_USE 45 08 00] */
static void veng4_E_0306(void)
{
    { veng4_McCoyOnMarata(); return; }
}

/* [MSG_USE 02 08 00] */
static void veng4_McCoyOnMarata(void)
{
    fv_with_medkit = 0;
    { veng4_part_0314(); return; }
}

static void veng4_part_0314(void)
{
    if (rv_marata_dead != 1) {
        if (rv_marata_treated != 1)
            { veng4_part_032A(); return; }
        veng4_Say(N_Dr_McCoy, T_VEN4_009);
        return;
    }
    veng4_Say(N_Dr_McCoy, T_VEN4_013);
    return;
}

static void veng4_part_032A(void)
{
    if (rv_knew_marata != 1) {
        rv_knew_marata = 1;
        veng4_Say(N_Capt_Kirk, T_VEN4_005);
    }
    room.fall[2] = 2;
    svc_Walk(2, 0x3C, 0xA0, 5);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void veng4_McCoyAtMarata(void)
{
    if (fv_with_medkit != 1) {
        svc_Anim(2, N_mscane, -1, -1, 7);
        svc_BridgeSfx(4);
        return;
    }
    svc_Anim(2, N_museme, -1, -1, 6);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void veng4_TreatedMarata(void)
{
    svc_Anim(8, N_s9r1bh, -1, -1, 0);
    rv_clue_marata = 1;
    rv_marata_treated = 1;
    rv_saw_marata = 1;
    veng4_Say(N_Dr_McCoy, T_VEN4_009);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void veng4_ScannedMarata(void)
{
    veng4_Say(N_Dr_McCoy, T_VEN4_011);
    veng4_Say(N_Capt_Kirk, T_VEN4_001);
    fv_with_medkit = 1;
    { veng4_part_032A(); return; }
}

/* Talking to Marata, once treated: she thought it was the Enterprise, and
 * dies.
 * [MSG_TALK 08 00 00] */
static void veng4_TalkToMarata(void)
{
    if (rv_marata_spoke == 1) {
    } else {
        if (rv_marata_treated != 1) {
        } else {
            rv_marata_spoke = 1;
            veng4_Say(N_Brittany_Marata, T_VEN4_019);
            veng4_Say(N_Capt_Kirk, T_VEN4_004);
            veng4_Say(N_Brittany_Marata, T_VEN4_020);
            veng4_Say(N_Capt_Kirk, T_VEN4_006);
            veng4_Say(N_Brittany_Marata, T_VEN4_021);
            veng4_Say(N_Capt_Kirk, T_VEN4_003);
            svc_Anim(8, N_s9r1bm_2, 0x5A, 0xA5, 0);
            veng4_Say(N_Dr_McCoy, T_VEN4_016);
            svc_Anim(0xC, N_s7r4m1, 0x5C, 0x75, 0);
            svc_Anim(8, N_s7r4bd_2, 0x5A, 0xA5, 0);
            rv_marata_dead = 1;
            svc_DigiOff();
            svc_Sfx(N_VE42LOOP_2);
            return;
        }
        veng4_Say(N_Dr_McCoy, T_VEN4_010);
        return;
    }
    veng4_Say(N_Dr_McCoy, T_VEN4_012);
    return;
}

/* Looking.
 * [MSG_LOOK 22 00 00] */
static void veng4_LookAtBody(void)
{
    veng4_Say(0, T_VEN4N014);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void veng4_LookAtMonitor(void)
{
    veng4_Say(0, T_VEN4N013);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void veng4_LookAtDoor(void)
{
    veng4_Say(0, T_VEN4N008);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void veng4_LookAtKirk(void)
{
    veng4_Say(0, T_VEN4N002);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void veng4_LookAtSpock(void)
{
    veng4_Say(0, T_VEN4N003);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void veng4_LookAtMcCoy(void)
{
    veng4_Say(0, T_VEN4N000);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void veng4_LookAtRedshirt(void)
{
    veng4_Say(0, T_VEN4N001);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void veng4_LookAtBed(void)
{
    if (rv_marata_dead == 1) {
        veng4_Say(S_114E, S_140D);
        return;
    }
    veng4_Say(S_114E, T_VEN4N007);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void veng4_LookAtScanner(void)
{
    if (rv_marata_dead != 1) {
        veng4_Say(S_114E, T_VEN4N006);
        return;
    }
    veng4_Say(0, T_VEN4N005);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void veng4_LookAtMarata(void)
{
    if (rv_marata_dead == 1) {
    } else {
        if (rv_knew_marata != 1) {
            rv_knew_marata = 1;
            veng4_Say(N_Capt_Kirk, T_VEN4_005);
            veng4_Say(N_Dr_McCoy, T_VEN4_014);
            return;
        }
        if (rv_marata_spoke != 1) {
            veng4_Say(0, T_VEN4N004);
            return;
        }
    }
    veng4_Say(0, T_VEN4N017);
    return;
}

/* [MSG_USE 44 0B 00] */
static void veng4_E_0773(void)
{
    { veng4_LookAtDrill(); return; }
}

/* [MSG_LOOK 0B 00 00] */
static void veng4_LookAtDrill(void)
{
    veng4_Say(0, T_VEN4N012);
    return;
}

/* [MSG_USE 44 0A 00] */
static void veng4_E_07D0(void)
{
    { veng4_LookAtHypo2(); return; }
}

/* [MSG_LOOK 0A 00 00] */
static void veng4_LookAtHypo2(void)
{
    veng4_Say(0, T_VEN4N010);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void veng4_LookAtDoor2(void)
{
    veng4_Say(0, T_VEN4N009);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void veng4_LookAtSickbay(void)
{
    veng4_Say(0, T_VEN4N011);
    return;
}

/* Taking the hypo (msgs 1, 2) and the drill (msgs 3, 4).
 * [MSG_GET 0A 00 00] */
static void veng4_GetHypo(void)
{
    room.busy = 1;
    svc_Walk(0, 0xEA, 0xAE, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void veng4_KirkAtHypo(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 2);
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void veng4_TookHypo(void)
{
    svc_GiveItem(IT_HYPO);
    rv_hypo_taken = 1;
    svc_StopAnim(0xA);
    veng4_Say(0, T_VEN4N015);
    svc_Walk(0, 0xCD, 0xC2, 0);
    room.busy = 0;
    return;
}

/* [MSG_GET 0B 00 00] */
static void veng4_GetDrill(void)
{
    room.busy = 1;
    svc_Walk(0, 0xEA, 0xAE, 3);
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void veng4_KirkAtDrill(void)
{
    svc_Anim(0, N_kusemn_2, -1, -1, 4);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void veng4_TookDrill(void)
{
    svc_GiveItem(IT_DRILL);
    rv_drill_taken = 1;
    svc_StopAnim(0xB);
    veng4_Say(0, T_VEN4N016);
    svc_Walk(0, 0xCD, 0xC2, 0);
    room.busy = 0;
    return;
}

/* Leaving: the door opens as Kirk reaches it (region 0).
 * [MSG_REGION 00 00 00] */
static void veng4_DoorOpens(void)
{
    if (fv_door_opening == 1) {
        svc_BridgeSfx(5);
        svc_Anim0(9, N_s9r1do, 0x13F, 0xAB, 0);
    }
    return;
}

/* [MSG_WALK 09 00 00] */
static void veng4_E_0ACC(void)
{
    { veng4_WalkToDoor(); return; }
}

/* [MSG_WALK 20 00 00] */
static void veng4_WalkToDoor(void)
{
    fv_door_opening = 1;
    room.busy = 1;
    svc_Walk(0, 0x12E, 0xAB, 0);
    return;
}

/* Every tick, common to the mission's rooms: the Elasi deadlines set on
 * the bridge (room 2). Until the Enterprise returns: boarding unless the
 * shields are up, the thirty minutes, the wait after Kirk complies (then
 * a warning and the torpedoes), the chance after they lower their
 * shields. When one runs out, the game is over.
 * [MSG_TICK FF FF FF] */
static void veng4_ElasiDeadlines(void)
{
    if (rv_board_timer != 0) {
        rv_board_timer--;
        if (rv_board_timer == 0) {
            if (rv_power_to != 2) {
                goto L0C0D;
            }
        }
    }
    if (rv_stall_timer != 0) {
        rv_stall_timer--;
        if (rv_stall_timer == 0) {
            if (rv_complied != 1) {
                goto L0C0D;
            }
        }
    }
    if (rv_comply_timer != 0) {
        rv_comply_timer--;
        if (rv_comply_timer == 0) {
            if (rv_rescued != 1) {
                veng4_Say(N_Elasi_Captain, T_VEN2_098);
                rv_torpedo_timer = 0xA8C;
            }
        }
    }
    if (rv_torpedo_timer != 0) {
        rv_torpedo_timer--;
        if (rv_torpedo_timer == 0) {
            if (rv_rescued != 1) {
                veng4_Say(0, T_VENA_F40);
                svc_GameOver();
            }
        }
    }
    if (rv_attack_timer != 0) {
        rv_attack_timer--;
        if (rv_attack_timer == 0) {
            if (rv_rescued != 1) {
                goto L0C0D;
            }
        }
    }
    return;
L0C0D:;
    veng4_Say(0, T_VENA_F32);
    svc_GameOver();
    return;
}

/* [MSG_USE 40 FF 00] */
static void veng4_E_0D1D(void)
{
    { veng4_KillAnything(); return; }
}

/* [MSG_USE 41 FF 00] */
static void veng4_KillAnything(void)
{
    veng4_Say(N_Mr_Spock, T_VENA_F34);
    return;
}

/* The hypo, empty or full of oil.
 * [MSG_LOOK 82 00 00] */
static void veng4_LookAtHypo(void)
{
    if (rv_hypo_oiled != 1) {
        veng4_Say(0, T_VEN4N010_2);
        return;
    }
    veng4_Say(0, T_VEN6N007);
    return;
}

/* The communicator: before the Enterprise leaves, Uhura; while the Elasi
 * jam it, Spock; otherwise Uhura, tracking the distress call.
 * [MSG_USE 47 00 00] */
static void veng4_CallEnterprise(void)
{
    if (rv_enterprise_away != 1) {
        veng4_Say(N_Capt_Kirk, T_MUD4_018);
        veng4_Say(N_Lt_Uhura, T_BRIDU146);
        return;
    }
    if (rv_elasi_here == 1) {
    } else {
        veng4_Say(N_Capt_Kirk, T_MUD4_018_2);
        veng4_Say(N_Lt_Uhura, T_VEN2U093);
        veng4_Say(N_Capt_Kirk, T_VEN2_028);
        return;
    }
    veng4_Say(N_Mr_Spock, T_VEN2_066);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void veng4_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void veng4_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x198F) = bx;
    veng4_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript veng4_scripts[] = {
    { 0x00F5, veng4_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0254, veng4_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0264, veng4_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0274, veng4_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0284, veng4_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0294, veng4_E_0294 },    /* MSG_USE 7F 22 00 */
    { 0x029C, veng4_MTricorderOnBody },    /* MSG_USE 45 22 00 */
    { 0x02A4, veng4_McCoyOnBody },    /* MSG_USE 02 22 00 */
    { 0x02B4, veng4_HypoOnMarata },    /* MSG_USE 82 08 00 */
    { 0x02F8, veng4_MedkitOnMarata },    /* MSG_USE 7F 08 00 */
    { 0x0306, veng4_E_0306 },    /* MSG_USE 45 08 00 */
    { 0x030E, veng4_McCoyOnMarata },    /* MSG_USE 02 08 00 */
    { 0x036E, veng4_McCoyAtMarata },    /* MSG_WALK_DONE 05 00 00 */
    { 0x03B4, veng4_TreatedMarata },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x03E5, veng4_ScannedMarata },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0406, veng4_TalkToMarata },    /* MSG_TALK 08 00 00 */
    { 0x0536, veng4_LookAtBody },    /* MSG_LOOK 22 00 00 */
    { 0x0545, veng4_LookAtMonitor },    /* MSG_LOOK 24 00 00 */
    { 0x0554, veng4_LookAtDoor },    /* MSG_LOOK 20 00 00 */
    { 0x0563, veng4_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0572, veng4_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0581, veng4_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0590, veng4_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x059F, veng4_LookAtBed },    /* MSG_LOOK 21 00 00 */
    { 0x05BF, veng4_LookAtScanner },    /* MSG_LOOK 23 00 00 */
    { 0x0631, veng4_LookAtMarata },    /* MSG_LOOK 08 00 00 */
    { 0x0773, veng4_E_0773 },    /* MSG_USE 44 0B 00 */
    { 0x077B, veng4_LookAtDrill },    /* MSG_LOOK 0B 00 00 */
    { 0x07D0, veng4_E_07D0 },    /* MSG_USE 44 0A 00 */
    { 0x07D8, veng4_LookAtHypo2 },    /* MSG_LOOK 0A 00 00 */
    { 0x081F, veng4_LookAtDoor2 },    /* MSG_LOOK 09 00 00 */
    { 0x087E, veng4_LookAtSickbay },    /* MSG_LOOK FF 00 00 */
    { 0x08ED, veng4_GetHypo },    /* MSG_GET 0A 00 00 */
    { 0x0914, veng4_KirkAtHypo },    /* MSG_WALK_DONE 01 00 00 */
    { 0x0939, veng4_TookHypo },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x09C9, veng4_GetDrill },    /* MSG_GET 0B 00 00 */
    { 0x09F0, veng4_KirkAtDrill },    /* MSG_WALK_DONE 03 00 00 */
    { 0x0A15, veng4_TookDrill },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0A9A, veng4_DoorOpens },    /* MSG_REGION 00 00 00 */
    { 0x0ACC, veng4_E_0ACC },    /* MSG_WALK 09 00 00 */
    { 0x0AD4, veng4_WalkToDoor },    /* MSG_WALK 20 00 00 */
    { 0x0AF9, veng4_ElasiDeadlines },    /* MSG_TICK FF FF FF */
    { 0x0D1D, veng4_E_0D1D },    /* MSG_USE 40 FF 00 */
    { 0x0D25, veng4_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x0DBA, veng4_LookAtHypo },    /* MSG_LOOK 82 00 00 */
    { 0x0E50, veng4_CallEnterprise },    /* MSG_USE 47 00 00 */
    { 0, 0 }
};
