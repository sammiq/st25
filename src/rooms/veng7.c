/* VENG7.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/veng7.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * VENG7.RDF - "Vengeance", the corridor outside the Republic's
 * transporter room (room 8). A length of power cable lies here, under a
 * coolant exhaust port fallen through the ceiling.
 *
 * Actors: 8 the cable, 9 a body, 0Ah (10) the door. Objects: those, 20h
 * the door, 21h the exhaust port. Exit 1: the turbolift.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 0FA7: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50 (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_VEN7LOOP = 0x00D2,
    N_s7r7d1c = 0x00EC,
    N_s7r7ca = 0x0111,
    N_s7r7c1 = 0x0131,
    N_s7r7d1 = 0x0196,
    T_VEN7N001 = 0x020D,
    T_VEN0N016 = 0x02C4,
    T_VEN7N007 = 0x02FF,
    T_VEN7N008 = 0x0349,
    T_VEN7N003 = 0x03A0,
    T_VEN7N005 = 0x03D0,
    T_VEN7N004 = 0x03FF,
    T_VEN7N002 = 0x042F,
    T_VEN7N006 = 0x045D,
    T_VEN7_001 = 0x04AB,
    T_VEN7_005 = 0x0528,
    T_VEN1_004 = 0x05B4,
    T_VEN7_007 = 0x0628,
    N_sscans = 0x0672,
    T_VEN7_006 = 0x06AA,
    T_VEN7_003 = 0x076E,
    N_kusehn = 0x081B,
    N_MUR4E6 = 0x0839,
    T_VEN7N009 = 0x0871,
    T_VEN2_098 = 0x0902,
    T_VENA_F40 = 0x0961,
    T_VENA_F32 = 0x09E8,
    T_VENA_F34 = 0x0AF6,
    T_VEN4N010 = 0x0B90,
    T_VEN6N007 = 0x0BD1,
    T_MUD4_018 = 0x0C26,
    T_BRIDU146 = 0x0C6B,
    T_MUD4_018_2 = 0x0CAB,
    T_VEN2U093 = 0x0CEF,
    T_VEN2_028 = 0x0D5A,
    T_VEN2_066 = 0x0DA8,
    T_DEM3_019 = 0x0E4F,
    T_VEN4_016 = 0x0E82,
    T_G_014 = 0x0ECA,
    T_G_014_2 = 0x0F03,
    T_VEN0_016 = 0x0F3C,
    T_VEN0_016_2 = 0x0F77,
    N_Capt_Kirk = 0x102E,
    N_Dr_McCoy = 0x1039,
    N_Mr_Spock = 0x1043,
    N_Ensign_Kije = 0x104D,
    N_Elasi_Captain = 0x1059,
    N_Lt_Uhura = 0x1067,
    talk_buf = 0x1071,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_mccoy_line CS_W(0xE12)
#define fv_talk_buf CS_W(0x1071)
#define rv_attack_timer ROOM_W(0x51)
#define rv_board_timer ROOM_W(0x49)
#define rv_cable_taken ROOM_B(0x5E)
#define rv_complied ROOM_B(0x45)
#define rv_comply_timer ROOM_W(0x4D)
#define rv_elasi_here ROOM_B(0x43)
#define rv_enterprise_away ROOM_B(0x36)
#define rv_hypo_oiled ROOM_B(0x3A)
#define rv_power_to ROOM_B(0x3C)
#define rv_rescued ROOM_B(0x42)
#define rv_stall_timer ROOM_W(0x4B)
#define rv_torpedo_timer ROOM_W(0x4F)

/* functions */
static void veng7_StartRoom(void);
static void veng7_E_0155(void);
static void veng7_WalkToDoor(void);
static void veng7_AtDoor(void);
static void veng7_DoorOpened(void);
static void veng7_ToTurbolift(void);
static void veng7_LookAtExhaust(void);
static void veng7_LookAtBody(void);
static void veng7_E_02EC(void);
static void veng7_LookAtDoor(void);
static void veng7_LookAtCable(void);
static void veng7_LookAtKirk(void);
static void veng7_LookAtSpock(void);
static void veng7_LookAtMcCoy(void);
static void veng7_LookAtRedshirt(void);
static void veng7_LookAtCorridor(void);
static void veng7_TalkToKirk(void);
static void veng7_TalkToSpock(void);
static void veng7_TalkToMcCoy(void);
static void veng7_TalkToRedshirt(void);
static void veng7_SpockScansExhaust(void);
static void veng7_NearFloor(void);
static void veng7_GetCable(void);
static void veng7_KirkAtCable(void);
static void veng7_TookCable(void);
static void veng7_ElasiDeadlines(void);
static void veng7_E_0AE2(void);
static void veng7_KillAnything(void);
static void veng7_LookAtHypo(void);
static void veng7_CallEnterprise(void);
static void veng7_E_0E1A(void);
static void veng7_MTricorderOnBody(void);
static void veng7_McCoyOnBody(void);
static void veng7_Talk(uint16_t ax);
static void veng7_Say(uint16_t ax, uint16_t bx);

/* The room starts: the door, the cable.
 * [MSG_TICK 01 00 00] */
static void veng7_StartRoom(void)
{
    svc_Sfx(N_VEN7LOOP);
    svc_Anim(0xA, N_s7r7d1c, 0x59, 0x72, 0);
    if (rv_cable_taken != 1) {
        svc_Anim(8, N_s7r7ca, 0xAC, 0x8A, 0);
    }
    svc_Anim(9, N_s7r7c1, 0xA6, 0x49, 0);
    return;
}

/* Walking into the transporter room (msg 2, anim msg 1; room 8).
 * [MSG_WALK 0A 00 00] */
static void veng7_E_0155(void)
{
    { veng7_WalkToDoor(); return; }
}

/* [MSG_WALK 20 00 00] */
static void veng7_WalkToDoor(void)
{
    svc_Walk(0, 0x66, 0x76, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void veng7_AtDoor(void)
{
    room.busy = 1;
    svc_BridgeSfx(5);
    svc_Anim(0xA, N_s7r7d1, 0x59, 0x72, 1);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void veng7_DoorOpened(void)
{
    svc_Walk(0, 0x5B, 0x76, 0);
    return;
}

/* Exit 1: the turbolift.
 * [MSG_EXIT 01 00 00] */
static void veng7_ToTurbolift(void)
{
    svc_BridgeSfx(5);
    svc_Turbolift(7, 1);
    return;
}

/* Looking.
 * [MSG_LOOK 21 00 00] */
static void veng7_LookAtExhaust(void)
{
    veng7_Say(0, T_VEN7N001);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void veng7_LookAtBody(void)
{
    veng7_Say(0, T_VEN0N016);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void veng7_E_02EC(void)
{
    { veng7_LookAtDoor(); return; }
}

/* [MSG_LOOK 20 00 00] */
static void veng7_LookAtDoor(void)
{
    veng7_Say(0, T_VEN7N007);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void veng7_LookAtCable(void)
{
    veng7_Say(0, T_VEN7N008);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void veng7_LookAtKirk(void)
{
    veng7_Say(0, T_VEN7N003);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void veng7_LookAtSpock(void)
{
    veng7_Say(0, T_VEN7N005);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void veng7_LookAtMcCoy(void)
{
    veng7_Say(0, T_VEN7N004);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void veng7_LookAtRedshirt(void)
{
    veng7_Say(0, T_VEN7N002);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void veng7_LookAtCorridor(void)
{
    veng7_Say(0, T_VEN7N006);
    return;
}

/* [MSG_TALK 00 00 00] */
static void veng7_TalkToKirk(void)
{
    veng7_Say(N_Capt_Kirk, T_VEN7_001);
    return;
}

/* [MSG_TALK 01 00 00] */
static void veng7_TalkToSpock(void)
{
    veng7_Say(N_Mr_Spock, T_VEN7_005);
    return;
}

/* [MSG_TALK 02 00 00] */
static void veng7_TalkToMcCoy(void)
{
    veng7_Say(N_Dr_McCoy, T_VEN1_004);
    return;
}

/* [MSG_TALK 03 00 00] */
static void veng7_TalkToRedshirt(void)
{
    veng7_Say(N_Ensign_Kije, T_VEN7_007);
    return;
}

/* [MSG_USE 44 21 00] */
static void veng7_SpockScansExhaust(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    veng7_Say(N_Mr_Spock, T_VEN7_006);
    return;
}

/* Region 0, the fallen exhaust port: Spock warns Kirk off.
 * [MSG_REGION 00 00 00] */
static void veng7_NearFloor(void)
{
    veng7_Say(N_Mr_Spock, T_VEN7_003);
    return;
}

/* Taking the cable (msgs 3, 4).
 * [MSG_GET 08 00 00] */
static void veng7_GetCable(void)
{
    room.busy = 1;
    svc_Walk(0, 0xAC, 0x8B, 3);
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void veng7_KirkAtCable(void)
{
    svc_Anim(0, N_kusehn, -1, -1, 4);
    svc_Sfx(N_MUR4E6);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void veng7_TookCable(void)
{
    svc_StopAnim(8);
    veng7_Say(0, T_VEN7N009);
    svc_GiveItem(IT_CABLE1);
    rv_cable_taken = 1;
    room.busy = 0;
    return;
}

/* Every tick, common to the mission's rooms: the Elasi deadlines set on
 * the bridge (room 2). Until the Enterprise returns: boarding unless the
 * shields are up, the thirty minutes, the wait after Kirk complies (then
 * a warning and the torpedoes), the chance after they lower their
 * shields. When one runs out, the game is over.
 * [MSG_TICK FF FF FF] */
static void veng7_ElasiDeadlines(void)
{
    if (rv_board_timer != 0) {
        rv_board_timer--;
        if (rv_board_timer == 0) {
            if (rv_power_to != 2) {
                goto L09D2;
            }
        }
    }
    if (rv_stall_timer != 0) {
        rv_stall_timer--;
        if (rv_stall_timer == 0) {
            if (rv_complied != 1) {
                goto L09D2;
            }
        }
    }
    if (rv_comply_timer != 0) {
        rv_comply_timer--;
        if (rv_comply_timer == 0) {
            if (rv_rescued != 1) {
                veng7_Say(N_Elasi_Captain, T_VEN2_098);
                rv_torpedo_timer = 0xA8C;
            }
        }
    }
    if (rv_torpedo_timer != 0) {
        rv_torpedo_timer--;
        if (rv_torpedo_timer == 0) {
            if (rv_rescued != 1) {
                veng7_Say(0, T_VENA_F40);
                svc_GameOver();
            }
        }
    }
    if (rv_attack_timer != 0) {
        rv_attack_timer--;
        if (rv_attack_timer == 0) {
            if (rv_rescued != 1) {
                goto L09D2;
            }
        }
    }
    return;
L09D2:;
    veng7_Say(0, T_VENA_F32);
    svc_GameOver();
    return;
}

/* [MSG_USE 40 FF 00] */
static void veng7_E_0AE2(void)
{
    { veng7_KillAnything(); return; }
}

/* [MSG_USE 41 FF 00] */
static void veng7_KillAnything(void)
{
    veng7_Say(N_Mr_Spock, T_VENA_F34);
    return;
}

/* The hypo, empty or full of oil.
 * [MSG_LOOK 82 00 00] */
static void veng7_LookAtHypo(void)
{
    if (rv_hypo_oiled != 1) {
        veng7_Say(0, T_VEN4N010);
        return;
    }
    veng7_Say(0, T_VEN6N007);
    return;
}

/* The communicator: before the Enterprise leaves, Uhura; while the Elasi
 * jam it, Spock; otherwise Uhura, tracking the distress call.
 * [MSG_USE 47 00 00] */
static void veng7_CallEnterprise(void)
{
    if (rv_enterprise_away != 1) {
        veng7_Say(N_Capt_Kirk, T_MUD4_018);
        veng7_Say(N_Lt_Uhura, T_BRIDU146);
        return;
    }
    if (rv_elasi_here == 1) {
    } else {
        veng7_Say(N_Capt_Kirk, T_MUD4_018_2);
        veng7_Say(N_Lt_Uhura, T_VEN2U093);
        veng7_Say(N_Capt_Kirk, T_VEN2_028);
        return;
    }
    veng7_Say(N_Mr_Spock, T_VEN2_066);
    return;
}

/* McCoy and the body.
 * [MSG_USE 7F 09 00] */
static void veng7_E_0E1A(void)
{
    { veng7_MTricorderOnBody(); return; }
}

/* [MSG_USE 45 09 00] */
static void veng7_MTricorderOnBody(void)
{
    { veng7_McCoyOnBody(); return; }
}

/* [MSG_USE 02 09 00] */
static void veng7_McCoyOnBody(void)
{
    uint16_t ax = 0;
    ax = (uint16_t)svc_Random(0, 5);
    fv_mccoy_line = ax;
    if (fv_mccoy_line == 0) {
        veng7_Say(N_Dr_McCoy, T_DEM3_019);
        return;
    }
    if (fv_mccoy_line == 1) {
        veng7_Say(N_Dr_McCoy, T_VEN4_016);
        return;
    }
    if (fv_mccoy_line == 2) {
        veng7_Say(N_Dr_McCoy, T_G_014);
        return;
    }
    if (fv_mccoy_line == 3) {
        veng7_Say(N_Dr_McCoy, T_G_014_2);
        return;
    }
    if (fv_mccoy_line == 4) {
        veng7_Say(N_Dr_McCoy, T_VEN0_016);
        return;
    }
    veng7_Say(N_Dr_McCoy, T_VEN0_016_2);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void veng7_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void veng7_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1073) = bx;
    veng7_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript veng7_scripts[] = {
    { 0x00D0, veng7_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0155, veng7_E_0155 },    /* MSG_WALK 0A 00 00 */
    { 0x015D, veng7_WalkToDoor },    /* MSG_WALK 20 00 00 */
    { 0x0180, veng7_AtDoor },    /* MSG_WALK_DONE 02 00 00 */
    { 0x01B9, veng7_DoorOpened },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x01DC, veng7_ToTurbolift },    /* MSG_EXIT 01 00 00 */
    { 0x0201, veng7_LookAtExhaust },    /* MSG_LOOK 21 00 00 */
    { 0x02B9, veng7_LookAtBody },    /* MSG_LOOK 09 00 00 */
    { 0x02EC, veng7_E_02EC },    /* MSG_LOOK 0A 00 00 */
    { 0x02F4, veng7_LookAtDoor },    /* MSG_LOOK 20 00 00 */
    { 0x033E, veng7_LookAtCable },    /* MSG_LOOK 08 00 00 */
    { 0x0395, veng7_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x03C5, veng7_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x03F4, veng7_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0424, veng7_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0452, veng7_LookAtCorridor },    /* MSG_LOOK FF 00 00 */
    { 0x04A0, veng7_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x051D, veng7_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x05A9, veng7_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x061D, veng7_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0670, veng7_SpockScansExhaust },    /* MSG_USE 44 21 00 */
    { 0x0763, veng7_NearFloor },    /* MSG_REGION 00 00 00 */
    { 0x07F2, veng7_GetCable },    /* MSG_GET 08 00 00 */
    { 0x0819, veng7_KirkAtCable },    /* MSG_WALK_DONE 03 00 00 */
    { 0x0856, veng7_TookCable },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x08BE, veng7_ElasiDeadlines },    /* MSG_TICK FF FF FF */
    { 0x0AE2, veng7_E_0AE2 },    /* MSG_USE 40 FF 00 */
    { 0x0AEA, veng7_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x0B7F, veng7_LookAtHypo },    /* MSG_LOOK 82 00 00 */
    { 0x0C15, veng7_CallEnterprise },    /* MSG_USE 47 00 00 */
    { 0x0E1A, veng7_E_0E1A },    /* MSG_USE 7F 09 00 */
    { 0x0E22, veng7_MTricorderOnBody },    /* MSG_USE 45 09 00 */
    { 0x0E2A, veng7_McCoyOnBody },    /* MSG_USE 02 09 00 */
    { 0, 0 }
};
