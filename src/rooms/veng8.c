/* VENG8.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/veng8.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * VENG8.RDF - "Vengeance", the Republic's transporter room, the only way
 * into the torpedo bay. Powered by impulse power or the fusion pack, it
 * sends Kije to unjam the torpedo loader.
 *
 * Actors: 3 Kije beaming, 9/0Ah (10) the fusion pack and its cable,
 * 0Bh (11) the door, 0Ch (12) the console, 0Dh/0Eh (13/14) the sparks,
 * 0Fh/10h (15/16) the pads. Objects: 8 the transporter chief, 9 the
 * fusion pack, 0Bh/23h the door, 0Fh/21h the coordinate controls, 10h/20h
 * the sliders, 22h the conduits, 24h the chamber. Exit 2: the corridor
 * (room 7).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - KijeOnPad also answers walk message 4, and counts arrivals in on_pad
 * as if two crewmen were to walk to the pad; only Kije's walk (msg 1)
 * is ever made, so the count is always 1.
 * - The script chain runs on into code at 1D91: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50 (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_VEN8LOOP = 0x00D3,
    N_s7r8pp = 0x00F6,
    N_s7r8pl = 0x0116,
    N_s7r8t2 = 0x0136,
    N_s7r8q1 = 0x0155,
    N_s7r8q2 = 0x0174,
    N_s7r8d1c = 0x0193,
    N_s7r8d1 = 0x0203,
    N_s7r8s1 = 0x024D,
    N_s7r8s2 = 0x027D,
    T_VEN8N004 = 0x02B7,
    T_VEN8N009 = 0x0325,
    T_VEN8N006 = 0x037C,
    T_VEN8N007 = 0x03DE,
    T_VEN8N003 = 0x0437,
    T_VEN8N008 = 0x0495,
    T_VEN8N001 = 0x04E6,
    T_VEN5N002 = 0x0522,
    T_VEN5N004 = 0x0559,
    T_VEN8N000 = 0x05A9,
    T_VEN8N010 = 0x05EF,
    T_VEN8N005 = 0x0655,
    T_VEN8_002 = 0x06B9,
    T_VEN8_005 = 0x0711,
    T_VEN8_006 = 0x0754,
    T_VEN8_032 = 0x07C2,
    T_VEN8_029 = 0x0802,
    T_VEN8_028 = 0x0852,
    T_VEN8_020 = 0x08A7,
    T_VEN8_001 = 0x08F0,
    T_VEN8_021 = 0x0957,
    T_VEN8_018 = 0x098B,
    T_VEN8_034 = 0x0A10,
    T_VEN8_023 = 0x0A44,
    T_VEN8_036 = 0x0A7A,
    T_VEN8_041 = 0x0AC5,
    T_VEN8_040 = 0x0AFC,
    T_VEN8_003 = 0x0B4C,
    N_susemn = 0x0BBD,
    T_VEN8_026 = 0x0BFE,
    T_VEN8_014 = 0x0C61,
    T_VEN8_030 = 0x0CBF,
    T_VEN8_031 = 0x0D65,
    T_VEN8_011 = 0x0E00,
    T_VEN8_033 = 0x0E85,
    T_VEN8_009 = 0x0F05,
    N_susemn_2 = 0x0F47,
    N_MTHTRNSP = 0x0F6C,
    N_s7r8pp_2 = 0x0F86,
    N_s7r8pl_2 = 0x0FA6,
    N_s7r8t1 = 0x0FC6,
    N_s7r8q1_2 = 0x1020,
    N_s7r8q2_2 = 0x103F,
    T_VEN8_004 = 0x1076,
    T_VEN8_027 = 0x1107,
    T_VEN8_015 = 0x116A,
    T_VEN8_013 = 0x11E0,
    T_VEN8_042 = 0x123C,
    T_VEN8_010 = 0x12BB,
    T_VEN8_035 = 0x1378,
    T_VEN8_009_2 = 0x13E4,
    N_susehn = 0x1403,
    N_rteled = 0x1438,
    T_VEN8_037 = 0x148A,
    T_VEN8_012 = 0x14FB,
    T_VEN8_039 = 0x154F,
    T_VEN8_008 = 0x1623,
    T_VEN8_038 = 0x168A,
    T_VEN8_007 = 0x16DB,
    N_susehn_2 = 0x1702,
    N_rtele = 0x173B,
    N_sscans = 0x17B4,
    T_VEN8_025 = 0x17EB,
    T_VEN2_098 = 0x1881,
    T_VENA_F40 = 0x18E0,
    T_VENA_F32 = 0x1967,
    T_VENA_F34 = 0x1A75,
    T_VEN4N010 = 0x1B0F,
    T_VEN6N007 = 0x1B50,
    T_MUD4_018 = 0x1BA5,
    T_BRIDU146 = 0x1BEA,
    T_MUD4_018_2 = 0x1C2A,
    T_VEN2U093 = 0x1C6E,
    T_VEN2_028 = 0x1CD9,
    T_VEN2_066 = 0x1D27,
    N_Capt_Kirk = 0x1E40,
    N_Dr_McCoy = 0x1E4B,
    N_Mr_Spock = 0x1E55,
    N_Ensign_Kije = 0x1E63,
    N_Elasi_Captain = 0x1E6F,
    N_Lt_Uhura = 0x1E7D,
    talk_buf = 0x1F57,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_on_pad CS_B(0xCA)
#define fv_talk_buf CS_W(0x1F57)
#define rv_attack_timer ROOM_W(0x51)
#define rv_board_timer ROOM_W(0x49)
#define rv_complied ROOM_B(0x45)
#define rv_comply_timer ROOM_W(0x4D)
#define rv_coords_set ROOM_B(0x65)
#define rv_elasi_here ROOM_B(0x43)
#define rv_enterprise_away ROOM_B(0x36)
#define rv_fusion_wired ROOM_B(0x67)
#define rv_hypo_oiled ROOM_B(0x3A)
#define rv_know_jam ROOM_B(0x66)
#define rv_power_on ROOM_B(0x34)
#define rv_power_to ROOM_B(0x3C)
#define rv_rescued ROOM_B(0x42)
#define rv_stall_timer ROOM_W(0x4B)
#define rv_stalled ROOM_B(0x44)
#define rv_torpedo_loaded ROOM_B(0x41)
#define rv_torpedo_timer ROOM_W(0x4F)

/* functions */
static void veng8_StartRoom(void);
static void veng8_E_01C2(void);
static void veng8_WalkToDoor(void);
static void veng8_AtDoor(void);
static void veng8_DoorOpened(void);
static void veng8_Sparks1(void);
static void veng8_Sparks1Done(void);
static void veng8_Sparks2(void);
static void veng8_Sparks2Done(void);
static void veng8_LookAtFusion(void);
static void veng8_E_0312(void);
static void veng8_LookAtDoor(void);
static void veng8_LookAtConduits(void);
static void veng8_E_03CB(void);
static void veng8_LookAtControls(void);
static void veng8_LookAtChief(void);
static void veng8_E_0482(void);
static void veng8_LookAtSliders(void);
static void veng8_LookAtKirk(void);
static void veng8_LookAtSpock(void);
static void veng8_LookAtMcCoy(void);
static void veng8_LookAtRedshirt(void);
static void veng8_LookAtPads(void);
static void veng8_LookAtTransporterRoom(void);
static void veng8_TalkToKirk(void);
static void veng8_TalkToSpock(void);
static void veng8_TalkToMcCoy(void);
static void veng8_TalkToRedshirt(void);
static void veng8_E_0B39(void);
static void veng8_KirkOnControls(void);
static void veng8_E_0B8C(void);
static void veng8_SpockOnControls(void);
static void veng8_SpockAtControls(void);
static void veng8_SetCoordinates(void);
static void veng8_FusionOnConduits(void);
static void veng8_SpockAtConduits(void);
static void veng8_FusionWired(void);
static void veng8_PadsLit(void);
static void veng8_E_1063(void);
static void veng8_KirkOnSliders(void);
static void veng8_E_10B6(void);
static void veng8_SpockOnSliders(void);
static void veng8_Energise(void);
static void veng8_E_13C4(void);
static void veng8_KijeOnPad(void);
static void veng8_KijeBeamed(void);
static void veng8_KijeInTorpedoBay(void);
static void veng8_KijeBeamedBack(void);
static void veng8_KijeBack(void);
static void veng8_E_179A(void);
static void veng8_SpockScansSliders(void);
static void veng8_SpockScansControls(void);
static void veng8_SpockScansSliders2(void);
static void veng8_ElasiDeadlines(void);
static void veng8_E_1A61(void);
static void veng8_KillAnything(void);
static void veng8_LookAtHypo(void);
static void veng8_CallEnterprise(void);
static void veng8_Talk(uint16_t ax);
static void veng8_Say(uint16_t ax, uint16_t bx);

/* The room starts: the fusion pack if wired in, the console, the door, and
 * the sparks (timers 0, 1).
 * [MSG_TICK 01 00 00] */
static void veng8_StartRoom(void)
{
    svc_Sfx(N_VEN8LOOP);
    if (rv_fusion_wired != 1) {
    } else {
        svc_Anim(9, N_s7r8pp, 0x10D, 0xA2, 0);
        svc_Anim(0xA, N_s7r8pl, 0x101, 0x97, 0);
        svc_Anim(0xC, N_s7r8t2, 0x3C, 0x9F, 0);
        svc_Anim(0xF, N_s7r8q1, 0x35, 0xA0, 0);
        svc_Anim(0x10, N_s7r8q2, 0x44, 0xA0, 0);
    }
    svc_Anim(0xB, N_s7r8d1c, 0x12C, 0xAC, 0);
    room.timers[0] = 0x2F;
    room.timers[1] = 0xE;
    return;
}

/* Walking out to the corridor (msg 8, anim msg 7).
 * [MSG_WALK 0B 00 00] */
static void veng8_E_01C2(void)
{
    { veng8_WalkToDoor(); return; }
}

/* [MSG_WALK 23 00 00] */
static void veng8_WalkToDoor(void)
{
    svc_Walk(0, 0x117, 0xB6, 8);
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void veng8_AtDoor(void)
{
    room.busy = 1;
    svc_BridgeSfx(5);
    svc_Anim(0xB, N_s7r8d1, 0x12C, 0xAC, 7);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void veng8_DoorOpened(void)
{
    svc_Walk(0, 0x134, 0xB6, 0);
    return;
}

/* The sparks loop: each animation restarts the other's timer.
 * [MSG_TIMER 00 00 00] */
static void veng8_Sparks1(void)
{
    svc_Anim(0xD, N_s7r8s1, 0, 0, 0xB);
    return;
}

/* [MSG_ANIM_DONE 0B 00 00] */
static void veng8_Sparks1Done(void)
{
    room.timers[0] = 0x80;
    return;
}

/* [MSG_TIMER 01 00 00] */
static void veng8_Sparks2(void)
{
    svc_Anim(0xE, N_s7r8s2, 0, 0, 0xC);
    return;
}

/* [MSG_ANIM_DONE 0C 00 00] */
static void veng8_Sparks2Done(void)
{
    room.timers[1] = 0x20;
    return;
}

/* Looking.
 * [MSG_LOOK 09 00 00] */
static void veng8_LookAtFusion(void)
{
    veng8_Say(0, T_VEN8N004);
    return;
}

/* [MSG_LOOK 0B 00 00] */
static void veng8_E_0312(void)
{
    { veng8_LookAtDoor(); return; }
}

/* [MSG_LOOK 23 00 00] */
static void veng8_LookAtDoor(void)
{
    veng8_Say(0, T_VEN8N009);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void veng8_LookAtConduits(void)
{
    veng8_Say(0, T_VEN8N006);
    return;
}

/* [MSG_LOOK 0F 00 00] */
static void veng8_E_03CB(void)
{
    { veng8_LookAtControls(); return; }
}

/* [MSG_LOOK 21 00 00] */
static void veng8_LookAtControls(void)
{
    veng8_Say(0, T_VEN8N007);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void veng8_LookAtChief(void)
{
    veng8_Say(0, T_VEN8N003);
    return;
}

/* [MSG_LOOK 10 00 00] */
static void veng8_E_0482(void)
{
    { veng8_LookAtSliders(); return; }
}

/* [MSG_LOOK 20 00 00] */
static void veng8_LookAtSliders(void)
{
    veng8_Say(0, T_VEN8N008);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void veng8_LookAtKirk(void)
{
    veng8_Say(0, T_VEN8N001);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void veng8_LookAtSpock(void)
{
    veng8_Say(0, T_VEN5N002);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void veng8_LookAtMcCoy(void)
{
    veng8_Say(0, T_VEN5N004);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void veng8_LookAtRedshirt(void)
{
    veng8_Say(0, T_VEN8N000);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void veng8_LookAtPads(void)
{
    veng8_Say(0, T_VEN8N010);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void veng8_LookAtTransporterRoom(void)
{
    veng8_Say(0, T_VEN8N005);
    return;
}

/* Talking: before the jam is known, after stalling, after the loading.
 * [MSG_TALK 00 00 00] */
static void veng8_TalkToKirk(void)
{
    if (rv_torpedo_loaded == 1) {
        veng8_Say(N_Capt_Kirk, T_VEN8_002);
        return;
    }
    if (rv_stalled == 1) {
        veng8_Say(N_Capt_Kirk, T_VEN8_005);
        return;
    }
    veng8_Say(N_Capt_Kirk, T_VEN8_006);
    return;
}

/* [MSG_TALK 01 00 00] */
static void veng8_TalkToSpock(void)
{
    if (rv_torpedo_loaded == 1) {
        veng8_Say(N_Mr_Spock, T_VEN8_032);
        return;
    }
    if (rv_stalled == 1) {
        veng8_Say(N_Mr_Spock, T_VEN8_029);
        return;
    }
    veng8_Say(N_Mr_Spock, T_VEN8_028);
    return;
}

/* [MSG_TALK 02 00 00] */
static void veng8_TalkToMcCoy(void)
{
    if (rv_torpedo_loaded != 1) {
    } else {
        veng8_Say(N_Dr_McCoy, T_VEN8_020);
        veng8_Say(N_Capt_Kirk, T_VEN8_001);
        return;
    }
    if (rv_stalled == 1) {
        veng8_Say(N_Dr_McCoy, T_VEN8_021);
        return;
    }
    veng8_Say(N_Dr_McCoy, T_VEN8_018);
    return;
}

/* [MSG_TALK 03 00 00] */
static void veng8_TalkToRedshirt(void)
{
    if (rv_torpedo_loaded != 1) {
    } else {
        veng8_Say(N_Ensign_Kije, T_VEN8_034);
        veng8_Say(N_Dr_McCoy, T_VEN8_023);
        veng8_Say(N_Ensign_Kije, T_VEN8_036);
        return;
    }
    if (rv_stalled == 1) {
        veng8_Say(N_Ensign_Kije, T_VEN8_041);
        return;
    }
    veng8_Say(N_Ensign_Kije, T_VEN8_040);
    return;
}

/* The coordinate controls (Spock, msgs 0Dh, 0Eh): with power from the
 * fusion pack or to the transporter, and the jam known, Spock sets them for
 * the torpedo bay.
 * [MSG_USE 00 0F 00] */
static void veng8_E_0B39(void)
{
    { veng8_KirkOnControls(); return; }
}

/* [MSG_USE 00 21 00] */
static void veng8_KirkOnControls(void)
{
    veng8_Say(N_Capt_Kirk, T_VEN8_003);
    { veng8_SpockOnControls(); return; }
}

/* [MSG_USE 01 0F 00] */
static void veng8_E_0B8C(void)
{
    { veng8_SpockOnControls(); return; }
}

/* [MSG_USE 01 21 00] */
static void veng8_SpockOnControls(void)
{
    room.busy = 1;
    svc_Walk(1, 0x37, 0xC4, 0xD);
    return;
}

/* [MSG_WALK_DONE 0D 00 00] */
static void veng8_SpockAtControls(void)
{
    svc_Anim(1, N_susemn, -1, -1, 0xE);
    return;
}

/* [MSG_ANIM_DONE 0E 00 00] */
static void veng8_SetCoordinates(void)
{
    room.busy = 0;
    if (rv_fusion_wired == 1) {
    } else {
        if (rv_power_on != 1) {
            veng8_Say(N_Mr_Spock, T_VEN8_026);
            return;
        }
        if (rv_power_to != 3) {
            veng8_Say(N_Mr_Spock, T_VEN8_014);
            return;
        }
    }
    if (rv_know_jam == 1) {
    } else {
        veng8_Say(N_Mr_Spock, T_VEN8_030);
        return;
    }
    veng8_Say(N_Mr_Spock, T_VEN8_031);
    rv_coords_set = 1;
    return;
}

/* The fusion pack on the conduits (msgs 9, 0Ah, anim 0Fh): enough power
 * for one person.
 * [MSG_USE 83 22 00] */
static void veng8_FusionOnConduits(void)
{
    if (rv_fusion_wired == 1) {
        return;
    }
    room.busy = 1;
    veng8_Say(N_Capt_Kirk, T_VEN8_011);
    veng8_Say(N_Mr_Spock, T_VEN8_033);
    veng8_Say(N_Capt_Kirk, T_VEN8_009);
    svc_Walk(1, 0xFA, 0xA4, 9);
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void veng8_SpockAtConduits(void)
{
    svc_Anim(1, N_susemn_2, -1, -1, 0xA);
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void veng8_FusionWired(void)
{
    svc_Sfx(N_MTHTRNSP);
    svc_Anim(9, N_s7r8pp_2, 0x10D, 0xA2, 0);
    svc_Anim(0xA, N_s7r8pl_2, 0x101, 0x97, 0);
    svc_Anim(0xC, N_s7r8t1, 0x3C, 0x9F, 0xF);
    svc_Walk(1, 0xD4, 0xB2, 0);
    rv_fusion_wired = 1;
    svc_TakeItem(IT_FUSION);
    room.busy = 0;
    return;
}

/* [MSG_ANIM_DONE 0F 00 00] */
static void veng8_PadsLit(void)
{
    svc_Anim(0xF, N_s7r8q1_2, 0x35, 0xA0, 0);
    svc_Anim(0x10, N_s7r8q2_2, 0x44, 0xA0, 0);
    return;
}

/* The sliders (Spock, msg 10h): with power and coordinates, Kije beams
 * to the torpedo bay (msg 1, anims 5, 2), forces the loading door, and
 * comes back (anims 6, 3): the torpedo is loaded.
 * [MSG_USE 00 10 00] */
static void veng8_E_1063(void)
{
    { veng8_KirkOnSliders(); return; }
}

/* [MSG_USE 00 20 00] */
static void veng8_KirkOnSliders(void)
{
    veng8_Say(N_Capt_Kirk, T_VEN8_004);
    { veng8_SpockOnSliders(); return; }
}

/* [MSG_USE 01 10 00] */
static void veng8_E_10B6(void)
{
    { veng8_SpockOnSliders(); return; }
}

/* [MSG_USE 01 20 00] */
static void veng8_SpockOnSliders(void)
{
    room.busy = 1;
    svc_Walk(1, 0x37, 0xC4, 0x10);
    room.fall[1] = 0;
    return;
}

/* [MSG_WALK_DONE 10 00 00] */
static void veng8_Energise(void)
{
    room.busy = 0;
    if (rv_fusion_wired == 1) {
    } else {
        if (rv_power_on != 1) {
            veng8_Say(N_Mr_Spock, T_VEN8_027);
            return;
        }
        if (rv_power_to != 3) {
            veng8_Say(N_Mr_Spock, T_VEN8_015);
            return;
        }
    }
    if (rv_coords_set != 1) {
        svc_Walk(1, 0x98, 0xB6, 0);
        veng8_Say(N_Mr_Spock, T_VEN8_013);
        return;
    }
    if (rv_torpedo_loaded == 1) {
        veng8_Say(N_Ensign_Kije, T_VEN8_042);
        return;
    }
    room.busy = 1;
    veng8_Say(N_Capt_Kirk, T_VEN8_010);
    veng8_Say(N_Ensign_Kije, T_VEN8_035);
    svc_Walk(3, 0xC8, 0x7F, 1);
    room.fall[3] = 1;
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void veng8_E_13C4(void)
{
    { veng8_KijeOnPad(); return; }
}

/* [MSG_WALK_DONE 04 00 00] */
static void veng8_KijeOnPad(void)
{
    fv_on_pad++;
    if (fv_on_pad == 1) {
        veng8_Say(N_Capt_Kirk, T_VEN8_009_2);
        svc_Anim(1, N_susehn, -1, -1, 5);
        svc_BridgeSfx(0xA);
    }
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void veng8_KijeBeamed(void)
{
    svc_Anim(3, N_rteled, -1, -1, 2);
    svc_BridgeSfx(8);
    room.redshirt_dead = 1;
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void veng8_KijeInTorpedoBay(void)
{
    svc_StopAnim(3);
    veng8_Say(N_Ensign_Kije, T_VEN8_037);
    veng8_Say(N_Capt_Kirk, T_VEN8_012);
    veng8_Say(N_Ensign_Kije, T_VEN8_039);
    veng8_Say(N_Capt_Kirk, T_VEN8_008);
    veng8_Say(N_Ensign_Kije, T_VEN8_038);
    veng8_Say(N_Capt_Kirk, T_VEN8_007);
    svc_Anim(1, N_susehn_2, -1, -1, 6);
    svc_BridgeSfx(0xA);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void veng8_KijeBeamedBack(void)
{
    room.redshirt_dead = 0;
    svc_Anim(3, N_rtele, 0xC8, 0x81, 3);
    svc_BridgeSfx(9);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void veng8_KijeBack(void)
{
    svc_Walk(3, 0xBA, 0xB2, 0);
    room.busy = 0;
    rv_torpedo_loaded = 1;
    return;
}

/* Spock's tricorder on the transporter.
 * [MSG_USE 44 0F 00] */
static void veng8_E_179A(void)
{
    { veng8_SpockScansSliders(); return; }
}

/* [MSG_USE 44 10 00] */
static void veng8_SpockScansSliders(void)
{
    { veng8_SpockScansControls(); return; }
}

/* [MSG_USE 44 21 00] */
static void veng8_SpockScansControls(void)
{
    { veng8_SpockScansSliders2(); return; }
}

/* [MSG_USE 44 20 00] */
static void veng8_SpockScansSliders2(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    veng8_Say(N_Mr_Spock, T_VEN8_025);
    return;
}

/* Every tick, common to the mission's rooms: the Elasi deadlines set on
 * the bridge (room 2). Until the Enterprise returns: boarding unless the
 * shields are up, the thirty minutes, the wait after Kirk complies (then
 * a warning and the torpedoes), the chance after they lower their
 * shields. When one runs out, the game is over.
 * [MSG_TICK FF FF FF] */
static void veng8_ElasiDeadlines(void)
{
    if (rv_board_timer != 0) {
        rv_board_timer--;
        if (rv_board_timer == 0) {
            if (rv_power_to != 2) {
                goto L1951;
            }
        }
    }
    if (rv_stall_timer != 0) {
        rv_stall_timer--;
        if (rv_stall_timer == 0) {
            if (rv_complied != 1) {
                goto L1951;
            }
        }
    }
    if (rv_comply_timer != 0) {
        rv_comply_timer--;
        if (rv_comply_timer == 0) {
            if (rv_rescued != 1) {
                veng8_Say(N_Elasi_Captain, T_VEN2_098);
                rv_torpedo_timer = 0xA8C;
            }
        }
    }
    if (rv_torpedo_timer != 0) {
        rv_torpedo_timer--;
        if (rv_torpedo_timer == 0) {
            if (rv_rescued != 1) {
                veng8_Say(0, T_VENA_F40);
                svc_GameOver();
            }
        }
    }
    if (rv_attack_timer != 0) {
        rv_attack_timer--;
        if (rv_attack_timer == 0) {
            if (rv_rescued != 1) {
                goto L1951;
            }
        }
    }
    return;
L1951:;
    veng8_Say(0, T_VENA_F32);
    svc_GameOver();
    return;
}

/* [MSG_USE 40 FF 00] */
static void veng8_E_1A61(void)
{
    { veng8_KillAnything(); return; }
}

/* [MSG_USE 41 FF 00] */
static void veng8_KillAnything(void)
{
    veng8_Say(N_Mr_Spock, T_VENA_F34);
    return;
}

/* The hypo, empty or full of oil.
 * [MSG_LOOK 82 00 00] */
static void veng8_LookAtHypo(void)
{
    if (rv_hypo_oiled != 1) {
        veng8_Say(0, T_VEN4N010);
        return;
    }
    veng8_Say(0, T_VEN6N007);
    return;
}

/* The communicator: before the Enterprise leaves, Uhura; while the Elasi
 * jam it, Spock; otherwise Uhura, tracking the distress call.
 * [MSG_USE 47 00 00] */
static void veng8_CallEnterprise(void)
{
    if (rv_enterprise_away != 1) {
        veng8_Say(N_Capt_Kirk, T_MUD4_018);
        veng8_Say(N_Lt_Uhura, T_BRIDU146);
        return;
    }
    if (rv_elasi_here == 1) {
    } else {
        veng8_Say(N_Capt_Kirk, T_MUD4_018_2);
        veng8_Say(N_Lt_Uhura, T_VEN2U093);
        veng8_Say(N_Capt_Kirk, T_VEN2_028);
        return;
    }
    veng8_Say(N_Mr_Spock, T_VEN2_066);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void veng8_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void veng8_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1F59) = bx;
    veng8_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript veng8_scripts[] = {
    { 0x00D1, veng8_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x01C2, veng8_E_01C2 },    /* MSG_WALK 0B 00 00 */
    { 0x01CA, veng8_WalkToDoor },    /* MSG_WALK 23 00 00 */
    { 0x01ED, veng8_AtDoor },    /* MSG_WALK_DONE 08 00 00 */
    { 0x0228, veng8_DoorOpened },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x024B, veng8_Sparks1 },    /* MSG_TIMER 00 00 00 */
    { 0x0270, veng8_Sparks1Done },    /* MSG_ANIM_DONE 0B 00 00 */
    { 0x027B, veng8_Sparks2 },    /* MSG_TIMER 01 00 00 */
    { 0x02A0, veng8_Sparks2Done },    /* MSG_ANIM_DONE 0C 00 00 */
    { 0x02AC, veng8_LookAtFusion },    /* MSG_LOOK 09 00 00 */
    { 0x0312, veng8_E_0312 },    /* MSG_LOOK 0B 00 00 */
    { 0x031A, veng8_LookAtDoor },    /* MSG_LOOK 23 00 00 */
    { 0x0371, veng8_LookAtConduits },    /* MSG_LOOK 22 00 00 */
    { 0x03CB, veng8_E_03CB },    /* MSG_LOOK 0F 00 00 */
    { 0x03D3, veng8_LookAtControls },    /* MSG_LOOK 21 00 00 */
    { 0x042C, veng8_LookAtChief },    /* MSG_LOOK 08 00 00 */
    { 0x0482, veng8_E_0482 },    /* MSG_LOOK 10 00 00 */
    { 0x048A, veng8_LookAtSliders },    /* MSG_LOOK 20 00 00 */
    { 0x04DB, veng8_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0517, veng8_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x054E, veng8_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x059E, veng8_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x05E4, veng8_LookAtPads },    /* MSG_LOOK 24 00 00 */
    { 0x064A, veng8_LookAtTransporterRoom },    /* MSG_LOOK FF 00 00 */
    { 0x06A8, veng8_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x07B1, veng8_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0893, veng8_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x09FC, veng8_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0B39, veng8_E_0B39 },    /* MSG_USE 00 0F 00 */
    { 0x0B41, veng8_KirkOnControls },    /* MSG_USE 00 21 00 */
    { 0x0B8C, veng8_E_0B8C },    /* MSG_USE 01 0F 00 */
    { 0x0B94, veng8_SpockOnControls },    /* MSG_USE 01 21 00 */
    { 0x0BBB, veng8_SpockAtControls },    /* MSG_WALK_DONE 0D 00 00 */
    { 0x0BE0, veng8_SetCoordinates },    /* MSG_ANIM_DONE 0E 00 00 */
    { 0x0DEA, veng8_FusionOnConduits },    /* MSG_USE 83 22 00 */
    { 0x0F45, veng8_SpockAtConduits },    /* MSG_WALK_DONE 09 00 00 */
    { 0x0F6A, veng8_FusionWired },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x101E, veng8_PadsLit },    /* MSG_ANIM_DONE 0F 00 00 */
    { 0x1063, veng8_E_1063 },    /* MSG_USE 00 10 00 */
    { 0x106B, veng8_KirkOnSliders },    /* MSG_USE 00 20 00 */
    { 0x10B6, veng8_E_10B6 },    /* MSG_USE 01 10 00 */
    { 0x10BE, veng8_SpockOnSliders },    /* MSG_USE 01 20 00 */
    { 0x10E9, veng8_Energise },    /* MSG_WALK_DONE 10 00 00 */
    { 0x13C4, veng8_E_13C4 },    /* MSG_WALK_DONE 01 00 00 */
    { 0x13CC, veng8_KijeOnPad },    /* MSG_WALK_DONE 04 00 00 */
    { 0x1436, veng8_KijeBeamed },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x146F, veng8_KijeInTorpedoBay },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x1735, veng8_KijeBeamedBack },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x176F, veng8_KijeBack },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x179A, veng8_E_179A },    /* MSG_USE 44 0F 00 */
    { 0x17A2, veng8_SpockScansSliders },    /* MSG_USE 44 10 00 */
    { 0x17AA, veng8_SpockScansControls },    /* MSG_USE 44 21 00 */
    { 0x17B2, veng8_SpockScansSliders2 },    /* MSG_USE 44 20 00 */
    { 0x183D, veng8_ElasiDeadlines },    /* MSG_TICK FF FF FF */
    { 0x1A61, veng8_E_1A61 },    /* MSG_USE 40 FF 00 */
    { 0x1A69, veng8_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x1AFE, veng8_LookAtHypo },    /* MSG_LOOK 82 00 00 */
    { 0x1B94, veng8_CallEnterprise },    /* MSG_USE 47 00 00 */
    { 0, 0 }
};
