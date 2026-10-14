/* VENG5.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/veng5.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * VENG5.RDF - "Vengeance", the corridor outside the Republic's engineering
 * section (room 6). Phasering a pile of debris uncovers a portable fusion
 * power pack, very nearly setting it off.
 *
 * Actors: 8 the door, 9 the debris, 0Ah (10) the fusion pack, 0Bh (11) a
 * body. Objects: those, 20h the door, 21h the ladder. Exit 1: the
 * turbolift.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 16D8: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50 (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_VEN5LOOP = 0x00D2,
    N_s7r5de = 0x00F2,
    N_s7r5fu = 0x011A,
    N_s7r5c1 = 0x013A,
    T_VEN5_005 = 0x0174,
    N_s7r5d1 = 0x0212,
    T_VEN5N007 = 0x029C,
    T_VEN5N008 = 0x02E3,
    T_VEN5N000 = 0x0381,
    T_VEN5N009 = 0x03E1,
    T_VEN5N001 = 0x04B6,
    T_VEN5N003 = 0x04F6,
    T_VEN5N002 = 0x053B,
    T_VEN5N004 = 0x0572,
    T_VEN5N005 = 0x05C2,
    T_VEN5N006 = 0x0624,
    T_VEN5_007 = 0x0688,
    T_VEN5_013 = 0x06F4,
    N_kfiren = 0x0763,
    N_s7r5p1 = 0x0788,
    T_VEN5_011 = 0x07E8,
    T_VEN5_018 = 0x082F,
    T_VEN5_010 = 0x087A,
    T_VEN5_019 = 0x08FB,
    T_VEN5_003 = 0x098E,
    N_mscanw = 0x09DA,
    T_VEN5_008 = 0x0A11,
    N_sscann = 0x0A78,
    T_VEN5_012 = 0x0AAF,
    T_VEN5_009 = 0x0B35,
    N_sscann_2 = 0x0B96,
    T_VEN5_004 = 0x0BCD,
    T_VEN5_001 = 0x0C36,
    T_VEN5_006 = 0x0C80,
    T_VEN5_017 = 0x0CD0,
    T_VEN5_R20 = 0x0D52,
    T_VEN5_R19 = 0x0DB7,
    T_VEN5_015 = 0x0DF6,
    T_VEN5_002 = 0x0E6F,
    T_VEN5_016 = 0x0EB4,
    N_kuseln = 0x0F55,
    T_VEN5N010 = 0x0F99,
    T_VEN2_098 = 0x1033,
    T_VENA_F40 = 0x1092,
    T_VENA_F32 = 0x1119,
    T_VENA_F34 = 0x1227,
    T_VEN4N010 = 0x12C1,
    T_VEN6N007 = 0x1302,
    T_MUD4_018 = 0x1357,
    T_BRIDU146 = 0x139C,
    T_MUD4_018_2 = 0x13DC,
    T_VEN2U093 = 0x1420,
    T_VEN2_028 = 0x148B,
    T_VEN2_066 = 0x14D9,
    T_DEM3_019 = 0x1580,
    T_VEN4_016 = 0x15B3,
    T_G_014 = 0x15FB,
    T_G_014_2 = 0x1634,
    T_VEN0_016 = 0x166D,
    T_VEN0_016_2 = 0x16A8,
    N_Capt_Kirk = 0x174B,
    N_Dr_McCoy = 0x1756,
    N_Mr_Spock = 0x1760,
    N_Ensign_Kije = 0x176A,
    N_Elasi_Captain = 0x1776,
    N_Lt_Uhura = 0x1784,
    talk_buf = 0x1792,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_mccoy_line CS_W(0x1543)
#define fv_talk_buf CS_W(0x1792)
#define rv_asked_fire ROOM_B(0x5C)
#define rv_attack_timer ROOM_W(0x51)
#define rv_board_timer ROOM_W(0x49)
#define rv_complied ROOM_B(0x45)
#define rv_comply_timer ROOM_W(0x4D)
#define rv_debris_blasted ROOM_B(0x59)
#define rv_elasi_here ROOM_B(0x43)
#define rv_enterprise_away ROOM_B(0x36)
#define rv_found_fusion ROOM_B(0x5A)
#define rv_hypo_oiled ROOM_B(0x3A)
#define rv_power_to ROOM_B(0x3C)
#define rv_rescued ROOM_B(0x42)
#define rv_sensed_fusion ROOM_B(0x5B)
#define rv_stall_timer ROOM_W(0x4B)
#define rv_torpedo_timer ROOM_W(0x4F)

/* functions */
static void veng5_StartRoom(void);
static void veng5_SpockSenses(void);
static void veng5_WalkToDoor(void);
static void veng5_AtDoor(void);
static void veng5_ToEngineering(void);
static void veng5_ToTurbolift(void);
static void veng5_E_0289(void);
static void veng5_LookAtDoor(void);
static void veng5_LookAtLadder(void);
static void veng5_LookAtBody(void);
static void veng5_LookAtFusion(void);
static void veng5_LookAtDebris(void);
static void veng5_LookAtKirk(void);
static void veng5_LookAtSpock(void);
static void veng5_LookAtMcCoy(void);
static void veng5_LookAtRedshirt(void);
static void veng5_LookAtCorridor(void);
static void veng5_E_0675(void);
static void veng5_PhaserFusion(void);
static void veng5_StunDebris(void);
static void veng5_PhaserDebris(void);
static void veng5_KirkAtDebris(void);
static void veng5_Fired(void);
static void veng5_DebrisGone(void);
static void veng5_McCoyScansDebris(void);
static void veng5_SpockScansDebris(void);
static void veng5_McCoyScansFusion(void);
static void veng5_SpockScansFusion(void);
static void veng5_TalkToKirk(void);
static void veng5_TalkToMcCoy(void);
static void veng5_TalkToSpock(void);
static void veng5_TalkToRedshirt(void);
static void veng5_NearFire(void);
static void veng5_GetFusion(void);
static void veng5_KirkAtFusion(void);
static void veng5_TookFusion(void);
static void veng5_ElasiDeadlines(void);
static void veng5_E_1213(void);
static void veng5_KillAnything(void);
static void veng5_LookAtHypo(void);
static void veng5_CallEnterprise(void);
static void veng5_E_154B(void);
static void veng5_MTricorderOnBody(void);
static void veng5_McCoyOnBody(void);
static void veng5_Talk(uint16_t ax);
static void veng5_Say(uint16_t ax, uint16_t bx);

/* The room starts: the debris or the fusion pack.
 * [MSG_TICK 01 00 00] */
static void veng5_StartRoom(void)
{
    svc_Sfx(N_VEN5LOOP);
    if (rv_debris_blasted != 1) {
        svc_Anim(9, N_s7r5de, 0xB1, 0x89, 0);
    } else {
        if (rv_found_fusion != 1) {
            svc_Anim(0xA, N_s7r5fu, 0xB1, 0x89, 0);
        }
    }
    svc_Anim(0xB, N_s7r5c1, 0xE2, 0x91, 0);
    return;
}

/* Tick 45, the first time: Spock senses a power source.
 * [MSG_TICK 2D 00 00] */
static void veng5_SpockSenses(void)
{
    if (rv_sensed_fusion != 1) {
        rv_sensed_fusion = 1;
        veng5_Say(N_Mr_Spock, T_VEN5_005);
        goto L01D2;
    }
L01D2:;
    return;
}

/* Walking to the engineering door (msg 2, anim msg 1): on into room 6.
 * [MSG_WALK 20 00 00] */
static void veng5_WalkToDoor(void)
{
    svc_Walk(0, 0x78, 0x8E, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void veng5_AtDoor(void)
{
    room.busy = 1;
    svc_BridgeSfx(5);
    svc_Anim(8, N_s7r5d1, 0x6B, 0x8C, 1);
    svc_Walk(0, 0x64, 0x8E, 0);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void veng5_ToEngineering(void)
{
    svc_GotoRoom(6, 2);
    return;
}

/* Exit 1: the turbolift.
 * [MSG_EXIT 01 00 00] */
static void veng5_ToTurbolift(void)
{
    svc_BridgeSfx(5);
    svc_Turbolift(5, 1);
    return;
}

/* Looking.
 * [MSG_LOOK 20 00 00] */
static void veng5_E_0289(void)
{
    { veng5_LookAtDoor(); return; }
}

/* [MSG_LOOK 08 00 00] */
static void veng5_LookAtDoor(void)
{
    veng5_Say(0, T_VEN5N007);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void veng5_LookAtLadder(void)
{
    veng5_Say(0, T_VEN5N008);
    return;
}

/* [MSG_LOOK 0B 00 00] */
static void veng5_LookAtBody(void)
{
    veng5_Say(0, T_VEN5N000);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void veng5_LookAtFusion(void)
{
    veng5_Say(0, T_VEN5N009);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void veng5_LookAtDebris(void)
{
    veng5_Say(0, T_VEN5N001);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void veng5_LookAtKirk(void)
{
    veng5_Say(0, T_VEN5N003);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void veng5_LookAtSpock(void)
{
    veng5_Say(0, T_VEN5N002);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void veng5_LookAtMcCoy(void)
{
    veng5_Say(0, T_VEN5N004);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void veng5_LookAtRedshirt(void)
{
    veng5_Say(0, T_VEN5N005);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void veng5_LookAtCorridor(void)
{
    veng5_Say(0, T_VEN5N006);
    return;
}

/* Phasers: a near miss on the fusion pack; the debris (msgs 4, 5, anim
 * 3) uncovers it.
 * [MSG_USE 40 0A 00] */
static void veng5_E_0675(void)
{
    { veng5_PhaserFusion(); return; }
}

/* [MSG_USE 41 0A 00] */
static void veng5_PhaserFusion(void)
{
    veng5_Say(N_Dr_McCoy, T_VEN5_007);
    return;
}

/* [MSG_USE 40 09 00] */
static void veng5_StunDebris(void)
{
    veng5_Say(N_Mr_Spock, T_VEN5_013);
    return;
}

/* [MSG_USE 41 09 00] */
static void veng5_PhaserDebris(void)
{
    svc_Walk(0, 0xAA, 0xB9, 4);
    room.busy = 1;
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void veng5_KirkAtDebris(void)
{
    svc_Anim(0, N_kfiren, -1, -1, 5);
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void veng5_Fired(void)
{
    svc_Anim(0xA, N_s7r5p1, 0xB1, 0x89, 3);
    svc_BridgeSfx(6);
    svc_StopAnim(9);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void veng5_DebrisGone(void)
{
    svc_StopAnim(0);
    veng5_Say(N_Dr_McCoy, T_VEN5_011);
    veng5_Say(N_Mr_Spock, T_VEN5_018);
    veng5_Say(N_Dr_McCoy, T_VEN5_010);
    veng5_Say(N_Mr_Spock, T_VEN5_019);
    veng5_Say(N_Capt_Kirk, T_VEN5_003);
    rv_debris_blasted = 1;
    room.busy = 0;
    return;
}

/* The tricorders.
 * [MSG_USE 45 09 00] */
static void veng5_McCoyScansDebris(void)
{
    svc_Anim(2, N_mscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    veng5_Say(N_Dr_McCoy, T_VEN5_008);
    return;
}

/* [MSG_USE 44 09 00] */
static void veng5_SpockScansDebris(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    veng5_Say(N_Mr_Spock, T_VEN5_012);
    return;
}

/* [MSG_USE 45 0A 00] */
static void veng5_McCoyScansFusion(void)
{
    veng5_Say(N_Dr_McCoy, T_VEN5_009);
    return;
}

/* [MSG_USE 44 0A 00] */
static void veng5_SpockScansFusion(void)
{
    svc_Anim(1, N_sscann_2, -1, -1, 0);
    svc_BridgeSfx(4);
    veng5_Say(N_Mr_Spock, T_VEN5_004);
    return;
}

/* [MSG_TALK 00 00 00] */
static void veng5_TalkToKirk(void)
{
    veng5_Say(N_Capt_Kirk, T_VEN5_001);
    return;
}

/* [MSG_TALK 02 00 00] */
static void veng5_TalkToMcCoy(void)
{
    veng5_Say(N_Dr_McCoy, T_VEN5_006);
    return;
}

/* [MSG_TALK 01 00 00] */
static void veng5_TalkToSpock(void)
{
    veng5_Say(N_Mr_Spock, T_VEN5_017);
    veng5_Say(N_Ensign_Kije, T_VEN5_R20);
    return;
}

/* [MSG_TALK 03 00 00] */
static void veng5_TalkToRedshirt(void)
{
    veng5_Say(N_Ensign_Kije, T_VEN5_R19);
    return;
}

/* Region 0, the fire: Spock warns Kirk off.
 * [MSG_REGION 00 00 00] */
static void veng5_NearFire(void)
{
    veng5_Say(N_Mr_Spock, T_VEN5_015);
    if (rv_asked_fire == 1) {
    } else {
        rv_asked_fire = 1;
        veng5_Say(N_Capt_Kirk, T_VEN5_002);
        veng5_Say(N_Mr_Spock, T_VEN5_016);
        goto L0F29;
    }
L0F29:;
    return;
}

/* Taking the fusion pack (msgs 6, 7).
 * [MSG_GET 0A 00 00] */
static void veng5_GetFusion(void)
{
    svc_Walk(0, 0xB1, 0x8C, 6);
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void veng5_KirkAtFusion(void)
{
    svc_Anim(0, N_kuseln, -1, -1, 7);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void veng5_TookFusion(void)
{
    if (rv_found_fusion != 1) {
        svc_StopAnim(0xA);
        veng5_Say(0, T_VEN5N010);
        rv_found_fusion = 1;
        svc_GiveItem(IT_FUSION);
    }
    return;
}

/* Every tick, common to the mission's rooms: the Elasi deadlines set on
 * the bridge (room 2). Until the Enterprise returns: boarding unless the
 * shields are up, the thirty minutes, the wait after Kirk complies (then
 * a warning and the torpedoes), the chance after they lower their
 * shields. When one runs out, the game is over.
 * [MSG_TICK FF FF FF] */
static void veng5_ElasiDeadlines(void)
{
    if (rv_board_timer != 0) {
        rv_board_timer--;
        if (rv_board_timer == 0) {
            if (rv_power_to != 2) {
                goto L1103;
            }
        }
    }
    if (rv_stall_timer != 0) {
        rv_stall_timer--;
        if (rv_stall_timer == 0) {
            if (rv_complied != 1) {
                goto L1103;
            }
        }
    }
    if (rv_comply_timer != 0) {
        rv_comply_timer--;
        if (rv_comply_timer == 0) {
            if (rv_rescued != 1) {
                veng5_Say(N_Elasi_Captain, T_VEN2_098);
                rv_torpedo_timer = 0xA8C;
            }
        }
    }
    if (rv_torpedo_timer != 0) {
        rv_torpedo_timer--;
        if (rv_torpedo_timer == 0) {
            if (rv_rescued != 1) {
                veng5_Say(0, T_VENA_F40);
                svc_GameOver();
            }
        }
    }
    if (rv_attack_timer != 0) {
        rv_attack_timer--;
        if (rv_attack_timer == 0) {
            if (rv_rescued != 1) {
                goto L1103;
            }
        }
    }
    return;
L1103:;
    veng5_Say(0, T_VENA_F32);
    svc_GameOver();
    return;
}

/* [MSG_USE 40 FF 00] */
static void veng5_E_1213(void)
{
    { veng5_KillAnything(); return; }
}

/* [MSG_USE 41 FF 00] */
static void veng5_KillAnything(void)
{
    veng5_Say(N_Mr_Spock, T_VENA_F34);
    return;
}

/* The hypo, empty or full of oil.
 * [MSG_LOOK 82 00 00] */
static void veng5_LookAtHypo(void)
{
    if (rv_hypo_oiled != 1) {
        veng5_Say(0, T_VEN4N010);
        return;
    }
    veng5_Say(0, T_VEN6N007);
    return;
}

/* The communicator: before the Enterprise leaves, Uhura; while the Elasi
 * jam it, Spock; otherwise Uhura, tracking the distress call.
 * [MSG_USE 47 00 00] */
static void veng5_CallEnterprise(void)
{
    if (rv_enterprise_away != 1) {
        veng5_Say(N_Capt_Kirk, T_MUD4_018);
        veng5_Say(N_Lt_Uhura, T_BRIDU146);
        return;
    }
    if (rv_elasi_here == 1) {
    } else {
        veng5_Say(N_Capt_Kirk, T_MUD4_018_2);
        veng5_Say(N_Lt_Uhura, T_VEN2U093);
        veng5_Say(N_Capt_Kirk, T_VEN2_028);
        return;
    }
    veng5_Say(N_Mr_Spock, T_VEN2_066);
    return;
}

/* McCoy and the body.
 * [MSG_USE 7F 0B 00] */
static void veng5_E_154B(void)
{
    { veng5_MTricorderOnBody(); return; }
}

/* [MSG_USE 45 0B 00] */
static void veng5_MTricorderOnBody(void)
{
    { veng5_McCoyOnBody(); return; }
}

/* [MSG_USE 02 0B 00] */
static void veng5_McCoyOnBody(void)
{
    uint16_t ax = 0;
    ax = (uint16_t)svc_Random(0, 5);
    fv_mccoy_line = ax;
    if (fv_mccoy_line == 0) {
        veng5_Say(N_Dr_McCoy, T_DEM3_019);
        return;
    }
    if (fv_mccoy_line == 1) {
        veng5_Say(N_Dr_McCoy, T_VEN4_016);
        return;
    }
    if (fv_mccoy_line == 2) {
        veng5_Say(N_Dr_McCoy, T_G_014);
        return;
    }
    if (fv_mccoy_line == 3) {
        veng5_Say(N_Dr_McCoy, T_G_014_2);
        return;
    }
    if (fv_mccoy_line == 4) {
        veng5_Say(N_Dr_McCoy, T_VEN0_016);
        return;
    }
    veng5_Say(N_Dr_McCoy, T_VEN0_016_2);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void veng5_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void veng5_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1794) = bx;
    veng5_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript veng5_scripts[] = {
    { 0x00D0, veng5_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x015F, veng5_SpockSenses },    /* MSG_TICK 2D 00 00 */
    { 0x01D9, veng5_WalkToDoor },    /* MSG_WALK 20 00 00 */
    { 0x01FC, veng5_AtDoor },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0252, veng5_ToEngineering },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0264, veng5_ToTurbolift },    /* MSG_EXIT 01 00 00 */
    { 0x0289, veng5_E_0289 },    /* MSG_LOOK 20 00 00 */
    { 0x0291, veng5_LookAtDoor },    /* MSG_LOOK 08 00 00 */
    { 0x02D7, veng5_LookAtLadder },    /* MSG_LOOK 21 00 00 */
    { 0x0376, veng5_LookAtBody },    /* MSG_LOOK 0B 00 00 */
    { 0x03D5, veng5_LookAtFusion },    /* MSG_LOOK 0A 00 00 */
    { 0x04AB, veng5_LookAtDebris },    /* MSG_LOOK 09 00 00 */
    { 0x04EB, veng5_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0530, veng5_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0567, veng5_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x05B7, veng5_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0619, veng5_LookAtCorridor },    /* MSG_LOOK FF 00 00 */
    { 0x0675, veng5_E_0675 },    /* MSG_USE 40 0A 00 */
    { 0x067D, veng5_PhaserFusion },    /* MSG_USE 41 0A 00 */
    { 0x06E9, veng5_StunDebris },    /* MSG_USE 40 09 00 */
    { 0x073A, veng5_PhaserDebris },    /* MSG_USE 41 09 00 */
    { 0x0761, veng5_KirkAtDebris },    /* MSG_WALK_DONE 04 00 00 */
    { 0x0786, veng5_Fired },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x07CD, veng5_DebrisGone },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x09D8, veng5_McCoyScansDebris },    /* MSG_USE 45 09 00 */
    { 0x0A76, veng5_SpockScansDebris },    /* MSG_USE 44 09 00 */
    { 0x0B2A, veng5_McCoyScansFusion },    /* MSG_USE 45 0A 00 */
    { 0x0B94, veng5_SpockScansFusion },    /* MSG_USE 44 0A 00 */
    { 0x0C2B, veng5_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0C75, veng5_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0CC5, veng5_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0DAC, veng5_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0DEB, veng5_NearFire },    /* MSG_REGION 00 00 00 */
    { 0x0F30, veng5_GetFusion },    /* MSG_GET 0A 00 00 */
    { 0x0F53, veng5_KirkAtFusion },    /* MSG_WALK_DONE 06 00 00 */
    { 0x0F78, veng5_TookFusion },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0FEF, veng5_ElasiDeadlines },    /* MSG_TICK FF FF FF */
    { 0x1213, veng5_E_1213 },    /* MSG_USE 40 FF 00 */
    { 0x121B, veng5_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x12B0, veng5_LookAtHypo },    /* MSG_LOOK 82 00 00 */
    { 0x1346, veng5_CallEnterprise },    /* MSG_USE 47 00 00 */
    { 0x154B, veng5_E_154B },    /* MSG_USE 7F 0B 00 */
    { 0x1553, veng5_MTricorderOnBody },    /* MSG_USE 45 0B 00 */
    { 0x155B, veng5_McCoyOnBody },    /* MSG_USE 02 0B 00 */
    { 0, 0 }
};
