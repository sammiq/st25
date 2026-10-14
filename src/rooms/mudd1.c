/* MUDD1.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/mudd1.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * MUDD1.RDF - "Another Fine Mess", the alien ship's weapons room. A claw
 * loads cylinders, which Spock and Buchert recognise as powerboosters,
 * into a weapons-delivery system; the buttons work it. Once Spock has
 * studied it, Kirk decides whether the Enterprise should take it.
 *
 * Actors: 8 the claw, 9 the cartridge, 10 (0Ah) the loading, 11, 12
 * (0Bh, 0Ch) the doors.
 * Objects: 8 the claw, 9/20h/21h the cylinders, 22h the weapon, 23h the
 * blue buttons, 24h the yellow, 25h the red.
 * Exits: 0 room 3, 1 room 0, 3 room 2. Regions 0-2: the doorsteps.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 0AE3: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s4wbhs = 0x00CC,
    N_s4wbcs = 0x00D3,
    N_s4wbcr = 0x00DA,
    N_s4wbcn = 0x00E1,
    N_s4wbsl = 0x00E8,
    N_s4wbhb = 0x00EF,
    N_s4wbcb = 0x00F6,
    N_s4wbsb = 0x00FD,
    N_susemn = 0x0104,
    N_s4wbd1 = 0x010B,
    N_s4wbd2 = 0x0112,
    N_s4cbxp = 0x0119,
    N_kgetdn = 0x0120,
    N_sgetde = 0x0127,
    N_mgetds = 0x012E,
    N_rgetdw = 0x0135,
    N_sscanw = 0x013C,
    N_MUD1LOOP = 0x0161,
    T_MUD1_001 = 0x01D7,
    N_crn2can = 0x025B,
    N_crn2rack = 0x033F,
    N_EXPLO3 = 0x04DB,
    T_MUD1_011 = 0x060A,
    N_SMADOOR3 = 0x07B9,
    N_SMADOOR3_2 = 0x0806,
    N_SMADOOR3_3 = 0x0868,
    S_0930 = 0x0930,
    S_0953 = 0x0953,
    S_0976 = 0x0976,
    T_MUD1N007 = 0x0999,
    T_MUD1N010 = 0x0A59,
    N_Capt_Kirk = 0x0C2F,
    N_Dr_McCoy = 0x0C3A,
    N_Mr_Spock = 0x0C44,
    N_Lt_Buchert = 0x0C4E,
    N_Lt_Uhura = 0x0C5A,
    S_0C64 = 0x0C64,
    T_MUD1N011 = 0x0C65,
    T_MUD1N002 = 0x0CDB,
    T_MUD1N012 = 0x0D14,
    T_MUD1N003 = 0x0D53,
    T_MUD1_026 = 0x0D91,
    T_MUD1_009 = 0x0DE9,
    T_MUD1N014 = 0x0E78,
    T_MUD1_010 = 0x0EA7,
    T_MUD1_025 = 0x0F42,
    T_MUD1_027 = 0x0FA5,
    T_MUD1_020 = 0x0FFC,
    T_MUD1_028 = 0x10D5,
    T_MUD1_024 = 0x117E,
    T_MUD1_022 = 0x11E6,
    T_MUD1_021 = 0x124E,
    T_MUD1_023 = 0x12FC,
    S_1388 = 0x1388,
    T_MUD1_007 = 0x146F,
    T_STATICU1 = 0x14C6,
    T_MUD1N005 = 0x14ED,
    T_MUD1N009 = 0x153A,
    T_MUD1N008 = 0x15C9,
    T_MUD1N006 = 0x1622,
    T_MUD1_006 = 0x1670,
    T_MUD1_013 = 0x16B7,
    T_MUD1_005 = 0x16FD,
    T_MUD1_019 = 0x171C,
    T_MUD1_008 = 0x1753,
    T_MUD1_012 = 0x17C2,
    T_MUD1_017 = 0x185C,
    T_MUD1_016 = 0x1895,
    T_MUD1N000 = 0x18CB,
    T_MUD1N001 = 0x190D,
    T_MUD1N016 = 0x1935,
    T_MUD1N015 = 0x1963,
    T_MUD1_014 = 0x1991,
    T_MUD1_015 = 0x19F7,
    T_MUD1_002 = 0x1A3A,
    talk_buf = 0x1ABA,
    T_MUD1N013 = 0x1AC0,
    T_MUD1N004 = 0x1B11,
    KirkDecides = 0x1B67,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_collapsed CS_B(0xCB)
#define fv_going_door CS_B(0xCA)
#define fv_talk_buf CS_W(0x1ABA)
#define rv_air_countdown ROOM_W(0x52)
#define rv_air_failing ROOM_B(0x46)
#define rv_air_left ROOM_W(0x50)
#define rv_loaded ROOM_B(0x33)
#define rv_music_started ROOM_B(0x58)
#define rv_pressed_blue ROOM_B(0x55)
#define rv_pressed_red ROOM_B(0x56)
#define rv_score ROOM_W(0x5A)
#define rv_take_weapon ROOM_W(0x3D)
#define rv_used_device ROOM_B(0x3C)
#define rv_weapon_examined ROOM_B(0x34)

/* functions */
static void mudd1_StartRoom(void);
static void mudd1_Timer1Sound(void);
static void mudd1_CallEnterprise(void);
static void mudd1_SpockPressesBlue(void);
static void mudd1_SpockAtButtons(void);
static void mudd1_ClawLoads(void);
static void mudd1_ClawDone(void);
static void mudd1_SpockPressesYellow(void);
static void mudd1_SpockAtButtons2(void);
static void mudd1_ClawUnloads(void);
static void mudd1_SpockPressesRed(void);
static void mudd1_SpockAtButtons3(void);
static void mudd1_SpockOnWeapon(void);
static void mudd1_UseDegrimer(void);
static void mudd1_CleanLenses(void);
static void mudd1_FireDevice(void);
static void mudd1_DeviceFired(void);
static void mudd1_E_051A(void);
static void mudd1_GetCylinders(void);
static void mudd1_GetCylinders2(void);
static void mudd1_E_053A(void);
static void mudd1_SpockScansCylinders(void);
static void mudd1_SpockScansCylinders2(void);
static void mudd1_SpockScansWeapon(void);
static void mudd1_E_05D0(void);
static void mudd1_SpockScansBlue(void);
static void mudd1_SpockScansButtons(void);
static void mudd1_SpockScansClaw(void);
static void mudd1_UseMedkit(void);
static void mudd1_LookAtScene(void);
static void mudd1_LookAtCylinders(void);
static void mudd1_LookAtCylinders2(void);
static void mudd1_LookAtFallenCylinders(void);
static void mudd1_LookAtWeapon(void);
static void mudd1_LookAtKirk(void);
static void mudd1_LookAtSpock(void);
static void mudd1_LookAtMcCoy(void);
static void mudd1_LookAtRedshirt(void);
static void mudd1_LookAtClaw(void);
static void mudd1_LookAtRedButton(void);
static void mudd1_LookAtBlueButtons(void);
static void mudd1_LookAtYellowButtons(void);
static void mudd1_TalkToKirk(void);
static void mudd1_TalkToSpock(void);
static void mudd1_TalkToMcCoy(void);
static void mudd1_TalkToRedshirt(void);
static void mudd1_WalkToLowerDoor(void);
static void mudd1_ReachLowerDoor(void);
static void mudd1_WalkToRightDoor(void);
static void mudd1_ReachRightDoor(void);
static void mudd1_WalkToLeftDoor(void);
static void mudd1_ReachLeftDoor(void);
static void mudd1_AirRunsOut(void);
static void mudd1_KirkCollapses(void);
static void mudd1_SpockCollapses(void);
static void mudd1_part_0A42(void);
static uint16_t mudd1_Talk(uint16_t ax);
static void mudd1_Say(uint16_t ax, uint16_t bx);

/* The room starts: the claw, and the cartridge if loaded.
 * [MSG_TICK 01 00 00] */
static void mudd1_StartRoom(void)
{
    if (rv_music_started != 1) {
        svc_Music(0, STALE_ARG);
        rv_music_started = 1;
        svc_Sfx(N_MUD1LOOP);
    }
    svc_Anim0(8, N_s4wbhs, 0x9B, 0x31, 0);
    if (rv_loaded == 0) {
        svc_Anim0(9, N_s4wbcs, 0x32, 0x65, 0);
    }
    return;
}

/* Timer 1: a sound.
 * [MSG_TIMER 01 00 00] */
static void mudd1_Timer1Sound(void)
{
    svc_BridgeSfx(7);
    return;
}

/* [MSG_USE 47 FF 00] */
static void mudd1_CallEnterprise(void)
{
    mudd1_Say(N_Capt_Kirk, T_MUD1_001);
    mudd1_Say(N_Lt_Uhura, T_STATICU1);
    return;
}

/* The buttons, Spock only: blue loads a cartridge (msgs 1, 1, 2, +2 once),
 * yellow unloads it (msgs 2, 3), red (loaded) has Spock work out the
 * weapon (msgs 3, 4, +3 once), and Kirk decides what to do with it.
 * [MSG_USE 01 23 00] */
static void mudd1_SpockPressesBlue(void)
{
    if (rv_loaded == 0) {
        svc_Walk(1, 0x2F, 0x9F, 1);
        room.busy = 1;
        if (rv_pressed_blue != 1) {
            rv_pressed_blue = 1;
            rv_score += 2;
        }
    }
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void mudd1_SpockAtButtons(void)
{
    room.fall[1] = 2;
    svc_Anim(1, N_susemn, -1, -1, 1);
    room.timers[1] = 5;
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void mudd1_ClawLoads(void)
{
    svc_Sfx(N_crn2can);
    svc_Anim0(8, N_s4wbcr, -1, -1, 0);
    svc_Anim0(9, N_s4wbcn, -1, -1, 0);
    svc_Anim0(0xA, N_s4wbsl, 0x77, 0x83, 2);
    rv_loaded = 1;
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void mudd1_ClawDone(void)
{
    room.busy = 0;
    svc_Walk(1, 0x43, 0x9F, 0);
    if (rv_loaded != 1) {
        mudd1_Say(N_Mr_Spock, T_MUD1_016);
        return;
    }
    mudd1_Say(N_Mr_Spock, T_MUD1_017);
    return;
}

/* [MSG_USE 01 24 00] */
static void mudd1_SpockPressesYellow(void)
{
    if (rv_loaded == 1) {
        room.fall[1] = 2;
        svc_Walk(1, 0x2F, 0x9F, 2);
        room.busy = 1;
    }
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void mudd1_SpockAtButtons2(void)
{
    svc_Anim(1, N_susemn, -1, -1, 3);
    room.timers[1] = 5;
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void mudd1_ClawUnloads(void)
{
    svc_Sfx(N_crn2rack);
    svc_Anim(8, N_s4wbhb, -1, -1, 0);
    svc_Anim(9, N_s4wbcb, 0x32, 0x65, 0);
    svc_Anim(0xA, N_s4wbsb, -1, -1, 2);
    rv_loaded = 0;
    return;
}

/* [MSG_USE 01 25 00] */
static void mudd1_SpockPressesRed(void)
{
    if (rv_loaded == 1) {
        svc_Walk(1, 0x2F, 0x9F, 3);
        room.busy = 1;
        if (rv_pressed_red != 1) {
            rv_pressed_red = 1;
            rv_score += 3;
        }
    }
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void mudd1_SpockAtButtons3(void)
{
    svc_Anim(1, N_susemn, -1, -1, 4);
    room.timers[1] = 5;
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void mudd1_SpockOnWeapon(void)
{
    uint16_t ax = 0;
    svc_Walk(1, 0x43, 0x9F, 0);
    mudd1_Say(N_Mr_Spock, T_MUD1_022);
    mudd1_Say(N_Mr_Spock, T_MUD1_021);
    mudd1_Say(N_Mr_Spock, T_MUD1_023);
    room.busy = 0;
    rv_weapon_examined = 1;
    ax = (uint16_t)mudd1_Talk(KirkDecides);
    if (ax == 0) {
        rv_take_weapon = 0;
        mudd1_Say(N_Mr_Spock, S_1388);
        return;
    }
    if (ax == 1) {
        mudd1_Say(N_Capt_Kirk, T_MUD1_007);
    }
    rv_take_weapon = 1;
    mudd1_Say(N_Lt_Uhura, T_STATICU1);
    return;
}

/* [MSG_USE 5E FF 00] */
static void mudd1_UseDegrimer(void)
{
    mudd1_Say(S_0C64, T_MUD1N004);
    return;
}

/* Common to all of the mission's rooms: the de-grimer on the lenses makes
 * the alien device (+1); firing the device sets off an explosion at Kirk
 * (anim msg 9), +5 the first time anywhere; the de-grimer on anything else
 * just cleans it.
 * [MSG_USE 5F 5E 00] */
static void mudd1_CleanLenses(void)
{
    uint16_t di = 0;
    uint16_t stk[32 + 8] = { 0 };       /* the top 8 are what a service call reads past the stack */
    int sp_ = 32;
    stk[--sp_] = 0x5F;
    stk[--sp_] = 7;
    stk[--sp_] = 0x5E;
    stk[--sp_] = 7;
    stk[--sp_] = 0x7D;
    stk[--sp_] = 4;
    di = 3;
L0486:;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 2;
    di = di - 1;
    if (di != 0)
        goto L0486;
    rv_score += 1;
    mudd1_Say(S_0C64, T_MUD1N013);
    return;
}

/* [MSG_USE 7D FF 00] */
static void mudd1_FireDevice(void)
{
    uint16_t ax = 0;
    room.busy = 1;
    room.fall[0] = 1;
    svc_StopAnim(0);
    ax = (uint16_t)svc_ActorPos(0);
    svc_Anim0(0xD, N_s4cbxp, ax, 0xA, 9);
    svc_Sfx(N_EXPLO3);
    return;
}

/* The alien device has gone off here (anim msg 9).
 * [MSG_ANIM_DONE 09 00 00] */
static void mudd1_DeviceFired(void)
{
    room.busy = 0;
    if (rv_used_device == 0) {
        rv_used_device = 1;
        rv_score += 5;
        mudd1_Say(N_Capt_Kirk, T_MUD1_002);
    }
    return;
}

/* Getting the cylinders: too heavy.
 * [MSG_GET 09 00 00] */
static void mudd1_E_051A(void)
{
    { mudd1_GetCylinders(); return; }
}

/* [MSG_GET 21 00 00] */
static void mudd1_GetCylinders(void)
{
    { mudd1_GetCylinders2(); return; }
}

/* [MSG_GET 20 00 00] */
static void mudd1_GetCylinders2(void)
{
    mudd1_Say(S_0C64, T_MUD1N014);
    return;
}

/* Tricorders: the cylinders are powerboosters.
 * [MSG_USE 44 21 00] */
static void mudd1_E_053A(void)
{
    { mudd1_SpockScansCylinders(); return; }
}

/* [MSG_USE 44 09 00] */
static void mudd1_SpockScansCylinders(void)
{
    { mudd1_SpockScansCylinders2(); return; }
}

/* [MSG_USE 44 20 00] */
static void mudd1_SpockScansCylinders2(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd1_Say(N_Mr_Spock, T_MUD1_025);
    mudd1_Say(N_Lt_Buchert, T_MUD1_027);
    mudd1_Say(N_Mr_Spock, T_MUD1_020);
    mudd1_Say(N_Lt_Buchert, T_MUD1_028);
    mudd1_Say(N_Mr_Spock, T_MUD1_024);
    return;
}

/* [MSG_USE 44 22 00] */
static void mudd1_SpockScansWeapon(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd1_Say(N_Mr_Spock, T_MUD1_010);
    return;
}

/* [MSG_USE 44 25 00] */
static void mudd1_E_05D0(void)
{
    { mudd1_SpockScansBlue(); return; }
}

/* [MSG_USE 44 23 00] */
static void mudd1_SpockScansBlue(void)
{
    { mudd1_SpockScansButtons(); return; }
}

/* [MSG_USE 44 24 00] */
static void mudd1_SpockScansButtons(void)
{
    mudd1_Say(N_Mr_Spock, T_MUD1_015);
    return;
}

/* [MSG_USE 44 08 00] */
static void mudd1_SpockScansClaw(void)
{
    mudd1_Say(N_Mr_Spock, T_MUD1_014);
    return;
}

/* [MSG_USE 7F FF 00] */
static void mudd1_UseMedkit(void)
{
    mudd1_Say(N_Dr_McCoy, T_MUD1_011);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void mudd1_LookAtScene(void)
{
    mudd1_Say(0, T_MUD1N011);
    return;
}

/* Looking.
 * [MSG_LOOK 20 00 00] */
static void mudd1_LookAtCylinders(void)
{
    mudd1_Say(0, T_MUD1N002);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void mudd1_LookAtCylinders2(void)
{
    mudd1_Say(0, T_MUD1N002);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void mudd1_LookAtFallenCylinders(void)
{
    mudd1_Say(0, T_MUD1N012);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void mudd1_LookAtWeapon(void)
{
    mudd1_Say(0, T_MUD1N003);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void mudd1_LookAtKirk(void)
{
    mudd1_Say(0, T_MUD1N005);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void mudd1_LookAtSpock(void)
{
    mudd1_Say(0, T_MUD1N009);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void mudd1_LookAtMcCoy(void)
{
    mudd1_Say(0, T_MUD1N008);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void mudd1_LookAtRedshirt(void)
{
    mudd1_Say(0, T_MUD1N006);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void mudd1_LookAtClaw(void)
{
    mudd1_Say(0, T_MUD1N000);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void mudd1_LookAtRedButton(void)
{
    mudd1_Say(0, T_MUD1N001);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void mudd1_LookAtBlueButtons(void)
{
    mudd1_Say(0, T_MUD1N015);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void mudd1_LookAtYellowButtons(void)
{
    mudd1_Say(0, T_MUD1N016);
    return;
}

/* [MSG_TALK 00 00 00] */
static void mudd1_TalkToKirk(void)
{
    mudd1_Say(N_Capt_Kirk, T_MUD1_006);
    mudd1_Say(N_Dr_McCoy, T_MUD1_013);
    mudd1_Say(N_Capt_Kirk, T_MUD1_005);
    return;
}

/* [MSG_TALK 01 00 00] */
static void mudd1_TalkToSpock(void)
{
    mudd1_Say(N_Mr_Spock, T_MUD1_019);
    mudd1_Say(N_Capt_Kirk, T_MUD1_008);
    return;
}

/* [MSG_TALK 02 00 00] */
static void mudd1_TalkToMcCoy(void)
{
    mudd1_Say(N_Dr_McCoy, T_MUD1_012);
    return;
}

/* [MSG_TALK 03 00 00] */
static void mudd1_TalkToRedshirt(void)
{
    mudd1_Say(N_Lt_Buchert, T_MUD1_026);
    mudd1_Say(N_Capt_Kirk, T_MUD1_009);
    return;
}

/* The doors: walk there; at the step the door slides (regions 0-2).
 * [MSG_WALK 28 00 00] */
static void mudd1_WalkToLowerDoor(void)
{
    fv_going_door = 1;
    room.busy = 1;
    svc_Walk(0, 0xA0, 0xC7, 0);
    return;
}

/* [MSG_REGION 02 00 00] */
static void mudd1_ReachLowerDoor(void)
{
    if (fv_going_door == 1) {
        svc_Sfx(N_SMADOOR3);
    }
    return;
}

/* [MSG_WALK 27 00 00] */
static void mudd1_WalkToRightDoor(void)
{
    fv_going_door = 1;
    room.busy = 1;
    svc_Walk(0, 0xAB, 0x71, 0);
    return;
}

/* [MSG_REGION 01 00 00] */
static void mudd1_ReachRightDoor(void)
{
    if (fv_going_door == 1) {
        svc_Sfx(N_SMADOOR3_2);
        svc_Anim0(0xB, N_s4wbd1, 0xAB, 0x73, 0);
    }
    return;
}

/* [MSG_WALK 26 00 00] */
static void mudd1_WalkToLeftDoor(void)
{
    fv_going_door = 2;
    room.busy = 1;
    svc_Walk(0, 0x35, 0x71, 0);
    return;
}

/* [MSG_REGION 00 00 00] */
static void mudd1_ReachLeftDoor(void)
{
    if (fv_going_door == 2) {
        svc_Sfx(N_SMADOOR3_3);
        svc_Anim0(0xC, N_s4wbd2, 0x37, 0x73, 0);
    }
    return;
}

/* Every tick, common to all of the mission's rooms: while life support is
 * failing, the air runs down from 6978h, with warnings at 4F1Ah, 34BCh
 * and 1A5Eh; at 1 the crew collapse (walk msgs 9-12) and the game is
 * over.
 * [MSG_TICK FF FF FF] */
static void mudd1_AirRunsOut(void)
{
    if (rv_air_failing != 0) {
        if (rv_air_countdown != 1) {
            rv_air_countdown = 1;
            rv_air_left = 0x6978;
        }
        rv_air_left--;
        if (rv_air_left == 0x1A5E)
            goto L0926;
        if (rv_air_left == 0x34BC)
            goto L091C;
        if (rv_air_left == 0x4F1A)
            goto L0912;
        if (rv_air_left == 1) {
            room.busy = 1;
            svc_Walk(0, 0xAA, 0xA5, 9);
            svc_Walk(1, 0x83, 0xAC, 0xA);
            mudd1_Say(S_0C64, T_MUD1N007);
            svc_GameOver();
        }
    }
    return;
L0912:;
    mudd1_Say(N_Mr_Spock, S_0930);
    return;
L091C:;
    mudd1_Say(N_Mr_Spock, S_0953);
    return;
L0926:;
    mudd1_Say(N_Mr_Spock, S_0976);
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void mudd1_KirkCollapses(void)
{
    svc_Anim(0, N_kgetdn, -1, -1, 0);
    svc_Anim(2, N_mgetds, -1, -1, 0);
    svc_Anim(3, N_rgetdw, -1, -1, 0);
    { mudd1_part_0A42(); return; }
}

/* [MSG_WALK_DONE 0A 00 00] */
static void mudd1_SpockCollapses(void)
{
    svc_Anim(1, N_sgetde, -1, -1, 0);
    { mudd1_part_0A42(); return; }
}

static void mudd1_part_0A42(void)
{
    fv_collapsed++;
    if (fv_collapsed == 2) {
        mudd1_Say(S_0C64, T_MUD1N010);
    }
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t mudd1_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void mudd1_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1ABC) = bx;
    mudd1_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript mudd1_scripts[] = {
    { 0x0149, mudd1_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x01B1, mudd1_Timer1Sound },    /* MSG_TIMER 01 00 00 */
    { 0x01C4, mudd1_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0x0202, mudd1_SpockPressesBlue },    /* MSG_USE 01 23 00 */
    { 0x0234, mudd1_SpockAtButtons },    /* MSG_WALK_DONE 01 00 00 */
    { 0x0259, mudd1_ClawLoads },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x02BD, mudd1_ClawDone },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x02F4, mudd1_SpockPressesYellow },    /* MSG_USE 01 24 00 */
    { 0x031C, mudd1_SpockAtButtons2 },    /* MSG_WALK_DONE 02 00 00 */
    { 0x033D, mudd1_ClawUnloads },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x03A1, mudd1_SpockPressesRed },    /* MSG_USE 01 25 00 */
    { 0x03D3, mudd1_SpockAtButtons3 },    /* MSG_WALK_DONE 03 00 00 */
    { 0x03F4, mudd1_SpockOnWeapon },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0467, mudd1_UseDegrimer },    /* MSG_USE 5E FF 00 */
    { 0x0477, mudd1_CleanLenses },    /* MSG_USE 5F 5E 00 */
    { 0x04A5, mudd1_FireDevice },    /* MSG_USE 7D FF 00 */
    { 0x04F8, mudd1_DeviceFired },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x051A, mudd1_E_051A },    /* MSG_GET 09 00 00 */
    { 0x0522, mudd1_GetCylinders },    /* MSG_GET 21 00 00 */
    { 0x052A, mudd1_GetCylinders2 },    /* MSG_GET 20 00 00 */
    { 0x053A, mudd1_E_053A },    /* MSG_USE 44 21 00 */
    { 0x0542, mudd1_SpockScansCylinders },    /* MSG_USE 44 09 00 */
    { 0x054A, mudd1_SpockScansCylinders2 },    /* MSG_USE 44 20 00 */
    { 0x059F, mudd1_SpockScansWeapon },    /* MSG_USE 44 22 00 */
    { 0x05D0, mudd1_E_05D0 },    /* MSG_USE 44 25 00 */
    { 0x05D8, mudd1_SpockScansBlue },    /* MSG_USE 44 23 00 */
    { 0x05E0, mudd1_SpockScansButtons },    /* MSG_USE 44 24 00 */
    { 0x05F0, mudd1_SpockScansClaw },    /* MSG_USE 44 08 00 */
    { 0x0600, mudd1_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0x0663, mudd1_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0672, mudd1_LookAtCylinders },    /* MSG_LOOK 20 00 00 */
    { 0x0681, mudd1_LookAtCylinders2 },    /* MSG_LOOK 09 00 00 */
    { 0x0690, mudd1_LookAtFallenCylinders },    /* MSG_LOOK 21 00 00 */
    { 0x069F, mudd1_LookAtWeapon },    /* MSG_LOOK 22 00 00 */
    { 0x06AE, mudd1_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x06BD, mudd1_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x06CC, mudd1_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x06DB, mudd1_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x06EA, mudd1_LookAtClaw },    /* MSG_LOOK 08 00 00 */
    { 0x06F9, mudd1_LookAtRedButton },    /* MSG_LOOK 25 00 00 */
    { 0x0708, mudd1_LookAtBlueButtons },    /* MSG_LOOK 23 00 00 */
    { 0x0717, mudd1_LookAtYellowButtons },    /* MSG_LOOK 24 00 00 */
    { 0x0726, mudd1_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0748, mudd1_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0761, mudd1_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0771, mudd1_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x078A, mudd1_WalkToLowerDoor },    /* MSG_WALK 28 00 00 */
    { 0x07AF, mudd1_ReachLowerDoor },    /* MSG_REGION 02 00 00 */
    { 0x07D8, mudd1_WalkToRightDoor },    /* MSG_WALK 27 00 00 */
    { 0x07FC, mudd1_ReachRightDoor },    /* MSG_REGION 01 00 00 */
    { 0x083B, mudd1_WalkToLeftDoor },    /* MSG_WALK 26 00 00 */
    { 0x085E, mudd1_ReachLeftDoor },    /* MSG_REGION 00 00 00 */
    { 0x089C, mudd1_AirRunsOut },    /* MSG_TICK FF FF FF */
    { 0x09E6, mudd1_KirkCollapses },    /* MSG_WALK_DONE 09 00 00 */
    { 0x0A2D, mudd1_SpockCollapses },    /* MSG_WALK_DONE 0A 00 00 */
    { 0, 0 }
};
