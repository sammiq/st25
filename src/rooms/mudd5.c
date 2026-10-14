/* MUDD5.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/mudd5.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * MUDD5.RDF - "Another Fine Mess", the alien ship's engine pod, where
 * Mudd's salvage hatch joins his own ship and his cheap "Sav-a-Ship" life
 * support generator runs. On the party's second visit Mudd comes asking
 * for his doover, and the generator fails (the air starts running out in
 * every room); Kirk repairs it with the doover.
 *
 * Actors: 8 Mudd, 9 the generator, 10 (0Ah) the door.
 * Objects: 9 the generator, 20h the hatch, 21h the engine, 22h the door,
 * 23h the crane. Exit 0: the cargo bay (room 0).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - Firing the alien device here (DeviceFired) sets used_device without the
 * +5 the other rooms give, so the points are lost if it happens here first.
 * - The script chain runs on into code at 0A04: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s4epdo = 0x00CC,
    N_s4eplo = 0x00D3,
    N_s4epls = 0x00DA,
    N_kuseln = 0x00E1,
    N_s4ephh = 0x00E8,
    N_s4ephc = 0x00EF,
    N_kgetdn = 0x00F6,
    N_sgetde = 0x00FD,
    N_mgetds = 0x0104,
    N_rgetdw = 0x010B,
    N_sscans = 0x0112,
    N_sscanw = 0x0119,
    N_sscane = 0x0120,
    N_s4cbxp = 0x0127,
    N_MUD5LOOP = 0x0136,
    T_MUD5_003 = 0x0261,
    N_Lt_Uhura = 0x0286,
    T_STATICU1 = 0x0290,
    N_EXPLO3 = 0x0593,
    N_SMADOOR3 = 0x0633,
    S_071F = 0x071F,
    S_0742 = 0x0742,
    S_0765 = 0x0765,
    T_MUD5N105 = 0x0788,
    T_MUD5N007 = 0x084A,
    T_MUD5_012 = 0x09B1,
    N_Capt_Kirk = 0x0AB0,
    N_Dr_McCoy = 0x0ABB,
    N_Mr_Spock = 0x0AC5,
    N_Lt_Buchert = 0x0ACF,
    N_Harry_Mudd = 0x0ADB,
    S_0AE6 = 0x0AE6,
    T_MUD5_022 = 0x0AE7,
    T_MUD5_011 = 0x0B95,
    T_MUD5N011 = 0x0C0C,
    T_MUD5_031 = 0x0C78,
    T_MUD5_009 = 0x0CF6,
    T_MUD5_024 = 0x0D1F,
    T_MUD5_001 = 0x0D9C,
    T_MUD5_025 = 0x0DDF,
    T_MUD5_015 = 0x0EBE,
    T_MUD5_030 = 0x0F1D,
    T_MUD5_039 = 0x0F58,
    T_MUD5_027 = 0x0FEC,
    T_MUD5N008 = 0x1038,
    T_MUD5N012 = 0x108A,
    T_MUD5_026 = 0x111E,
    T_MUD5_014 = 0x117D,
    T_MUD5_019 = 0x1204,
    T_MUD5_021 = 0x12A8,
    T_MUD5_007 = 0x13AD,
    T_MUD5_028 = 0x1411,
    T_MUD5_034 = 0x15FD,
    T_MUD5_037 = 0x16FC,
    T_MUD5_035 = 0x1775,
    T_MUD5_005 = 0x179D,
    T_MUD5_036 = 0x17BE,
    T_MUD2_002 = 0x17FE,
    T_MUD5N009 = 0x187E,
    T_MUD5N001 = 0x18CF,
    T_MUD5N002 = 0x1925,
    T_MUD5N005 = 0x199D,
    T_MUD5N004 = 0x19EE,
    T_MUD5N003 = 0x1A3E,
    T_MUD5_002 = 0x1A99,
    T_MUD5_032 = 0x1B3B,
    T_MUD5_033 = 0x1B6E,
    T_MUD5_016 = 0x1BC8,
    T_MUD5_017 = 0x1C1A,
    T_MUD5_004 = 0x1C91,
    T_MUD5_040 = 0x1CE7,
    T_MUD5_018 = 0x1D64,
    T_MUD5N006 = 0x1E24,
    T_MUD5N000 = 0x1EA2,
    T_MUD5_023 = 0x1EE2,
    T_MUD5N010 = 0x1F47,
    KirkOnDoover = 0x1F82,
    talk_buf = 0x1F8A,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_collapsed CS_B(0xCB)
#define fv_going_door CS_B(0xCA)
#define fv_talk_buf CS_W(0x1F8A)
#define rv_air_countdown ROOM_W(0x52)
#define rv_air_failing ROOM_B(0x46)
#define rv_air_left ROOM_W(0x50)
#define rv_generator_fixed ROOM_B(0x59)
#define rv_mudd_incident ROOM_B(0x3F)
#define rv_score ROOM_W(0x5A)
#define rv_used_device ROOM_B(0x3C)
#define rv_visits ROOM_B(0x47)

/* functions */
static void mudd5_StartRoom(void);
static void mudd5_MuddWantsDoover(void);
static void mudd5_LifeSupportFails(void);
static void mudd5_CallEnterprise(void);
static void mudd5_SpockScans(void);
static void mudd5_SpockScansEngine(void);
static void mudd5_SpockScansCrane(void);
static void mudd5_SpockScansHatch(void);
static void mudd5_SpockScansGenerator(void);
static void mudd5_StunHatch(void);
static void mudd5_KillHatch(void);
static void mudd5_UseOnGenerator(void);
static void mudd5_DooverOnGenerator(void);
static void mudd5_KirkAtGenerator(void);
static void mudd5_GeneratorFixed(void);
static void mudd5_Repaired(void);
static void mudd5_MuddReturns(void);
static void mudd5_CleanLenses(void);
static void mudd5_FireDevice(void);
static void mudd5_DeviceFired(void);
static void mudd5_UseDegrimer(void);
static void mudd5_LookAtHatch(void);
static void mudd5_LookAtGenerator(void);
static void mudd5_ReachDoor(void);
static void mudd5_WalkToDoor(void);
static void mudd5_AirRunsOut(void);
static void mudd5_KirkCollapses(void);
static void mudd5_SpockCollapses(void);
static void mudd5_part_0833(void);
static void mudd5_LookAtKirk(void);
static void mudd5_LookAtSpock(void);
static void mudd5_LookAtMcCoy(void);
static void mudd5_LookAtRedshirt(void);
static void mudd5_LookAtDoor(void);
static void mudd5_LookAtCrane(void);
static void mudd5_LookAtEngine(void);
static void mudd5_TalkToKirk(void);
static void mudd5_TalkToSpock(void);
static void mudd5_TalkToMcCoy(void);
static void mudd5_TalkToRedshirt(void);
static void mudd5_UseMedkit(void);
static void mudd5_Talk(uint16_t ax);
static void mudd5_Say(uint16_t ax, uint16_t bx);

/* The room starts: the generator, failing or not. On the second visit,
 * with no other trouble with Mudd, Mudd comes in (timer 1).
 * [MSG_TICK 01 00 00] */
static void mudd5_StartRoom(void)
{
    svc_Sfx(N_MUD5LOOP);
    room.fall[3] = 2;
    if (rv_air_failing == 1) {
        svc_Music(0x13, STALE_ARG);
        svc_Anim0(9, N_s4epls, 0x54, 0xB0, 0);
        return;
    }
    svc_Anim0(9, N_s4eplo, 0x54, 0xB0, 0);
    rv_visits++;
    if (rv_visits == 2) {
        if (rv_mudd_incident == 0) {
            if (rv_generator_fixed == 0) {
                rv_mudd_incident = 1;
                rv_visits = 1;
                room.busy = 2;
                svc_Music(3, STALE_ARG);
                svc_Anim0(8, N_s4ephh, 0xE, 0xA7, 0);
                room.timers[1] = 0x70;
                room.fall[0] = 3;
            }
        }
    }
    return;
}

/* Timer 1: Mudd asks for his doover and leaves...
 * [MSG_TIMER 01 00 00] */
static void mudd5_MuddWantsDoover(void)
{
    mudd5_Say(N_Harry_Mudd, T_MUD5_034);
    mudd5_Talk(KirkOnDoover);
    mudd5_Say(N_Harry_Mudd, T_MUD5_037);
    svc_Anim(8, N_s4ephc, -1, -1, 0);
    room.timers[2] = 0x8C;
    return;
}

/* ...and timer 2: the life support generator fails (air_failing).
 * [MSG_TIMER 02 00 00] */
static void mudd5_LifeSupportFails(void)
{
    room.busy = 0;
    rv_air_failing = 1;
    svc_Music(0x13, STALE_ARG);
    svc_Anim0(9, N_s4epls, 0x54, 0xB0, 0);
    return;
}

/* [MSG_USE 47 FF 00] */
static void mudd5_CallEnterprise(void)
{
    mudd5_Say(N_Capt_Kirk, T_MUD5_003);
    mudd5_Say(N_Lt_Uhura, T_STATICU1);
    return;
}

/* [MSG_USE 44 FF 00] */
static void mudd5_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd5_Say(N_Mr_Spock, T_MUD5_022);
    return;
}

/* Tricorders and phasers.
 * [MSG_USE 44 21 00] */
static void mudd5_SpockScansEngine(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd5_Say(N_Mr_Spock, T_MUD5_011);
    return;
}

/* [MSG_USE 44 23 00] */
static void mudd5_SpockScansCrane(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd5_Say(N_Mr_Spock, T_MUD5_023);
    return;
}

/* [MSG_USE 44 20 00] */
static void mudd5_SpockScansHatch(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd5_Say(N_Mr_Spock, T_MUD5_024);
    mudd5_Say(N_Capt_Kirk, T_MUD5_001);
    return;
}

/* [MSG_USE 44 09 00] */
static void mudd5_SpockScansGenerator(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_air_failing != 0) {
        mudd5_Say(N_Mr_Spock, T_MUD5_019);
        return;
    }
    mudd5_Say(N_Mr_Spock, T_MUD5_021);
    return;
}

/* [MSG_USE 40 20 00] */
static void mudd5_StunHatch(void)
{
    mudd5_Say(N_Mr_Spock, T_MUD5_025);
    mudd5_Say(N_Dr_McCoy, T_MUD5_015);
    mudd5_Say(N_Mr_Spock, T_MUD5_030);
    return;
}

/* [MSG_USE 41 20 00] */
static void mudd5_KillHatch(void)
{
    mudd5_Say(N_Lt_Buchert, T_MUD5_039);
    mudd5_Say(N_Mr_Spock, T_MUD5_027);
    return;
}

/* The doover on the failing generator: Kirk walks there (msg 2), works on
 * it (anim msg 2): the air is safe again; timer 4: fixed, +2. Timer 3:
 * Mudd looks in.
 * [MSG_USE FF 09 00] */
static void mudd5_UseOnGenerator(void)
{
    mudd5_Say(S_0AE6, T_MUD5_014);
    return;
}

/* [MSG_USE 7C 09 00] */
static void mudd5_DooverOnGenerator(void)
{
    if (rv_air_failing == 1) {
        room.busy = 1;
        room.fall[0] = 0;
        svc_Walk(0, 0x57, 0xB4, 2);
        return;
    }
    mudd5_Say(S_0AE6, T_MUD5_014);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void mudd5_KirkAtGenerator(void)
{
    svc_Anim(0, N_kuseln, -1, -1, 2);
    room.timers[4] = 0x14;
    return;
}

/* [MSG_TIMER 04 00 00] */
static void mudd5_GeneratorFixed(void)
{
    svc_Music(-1, STALE_ARG);
    svc_Anim(9, N_s4eplo, -1, -1, 0);
    rv_generator_fixed = 1;
    rv_score += 2;
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void mudd5_Repaired(void)
{
    mudd5_Say(N_Capt_Kirk, T_MUD5_007);
    mudd5_Say(N_Mr_Spock, T_MUD5_028);
    rv_air_failing = 0;
    rv_mudd_incident = 0;
    svc_Music(3, STALE_ARG);
    svc_Anim0(8, N_s4ephh, 0xE, 0xA7, 0);
    room.busy = 1;
    room.fall[0] = 3;
    svc_StopAnim(0);
    room.timers[3] = 0x6E;
    return;
}

/* [MSG_TIMER 03 00 00] */
static void mudd5_MuddReturns(void)
{
    mudd5_Say(N_Harry_Mudd, T_MUD5_035);
    mudd5_Say(N_Capt_Kirk, T_MUD5_005);
    mudd5_Say(N_Harry_Mudd, T_MUD5_036);
    svc_Anim(8, N_s4ephc, -1, -1, 0);
    room.busy = 0;
    return;
}

/* Common to all of the mission's rooms: the de-grimer on the lenses makes
 * the alien device (+1); firing the device sets off an explosion at Kirk
 * (anim msg 9), +5 the first time anywhere; the de-grimer on anything else
 * just cleans it.
 * [MSG_USE 5F 5E 00] */
static void mudd5_CleanLenses(void)
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
L053E:;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 2;
    di = di - 1;
    if (di != 0)
        goto L053E;
    rv_score += 1;
    mudd5_Say(S_0AE6, T_MUD5N009);
    return;
}

/* [MSG_USE 7D FF 00] */
static void mudd5_FireDevice(void)
{
    uint16_t ax = 0;
    room.busy = 1;
    room.fall[0] = 1;
    svc_StopAnim(0);
    ax = (uint16_t)svc_ActorPos(0);
    svc_Anim0(0xB, N_s4cbxp, ax, 0xA, 9);
    svc_Sfx(N_EXPLO3);
    return;
}

/* The alien device has gone off here (anim msg 9): this one sets used_device
 * without the +5.
 * [MSG_ANIM_DONE 09 00 00] */
static void mudd5_DeviceFired(void)
{
    room.busy = 0;
    if (rv_used_device == 0) {
        rv_used_device = 1;
        mudd5_Say(N_Capt_Kirk, T_MUD2_002);
    }
    return;
}

/* [MSG_USE 5E FF 00] */
static void mudd5_UseDegrimer(void)
{
    mudd5_Say(S_0AE6, T_MUD5N001);
    return;
}

/* Looking, and the door back to the cargo bay (room 0).
 * [MSG_LOOK 20 00 00] */
static void mudd5_LookAtHatch(void)
{
    mudd5_Say(S_0AE6, T_MUD5N011);
    mudd5_Say(N_Mr_Spock, T_MUD5_031);
    mudd5_Say(N_Capt_Kirk, T_MUD5_009);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void mudd5_LookAtGenerator(void)
{
    if (rv_air_failing != 0) {
        mudd5_Say(S_0AE6, T_MUD5N008);
        return;
    }
    mudd5_Say(S_0AE6, T_MUD5N012);
    mudd5_Say(N_Mr_Spock, T_MUD5_026);
    return;
}

/* [MSG_REGION 00 00 00] */
static void mudd5_ReachDoor(void)
{
    if (fv_going_door == 1) {
        svc_Sfx(N_SMADOOR3);
        svc_Anim0(0xA, N_s4epdo, 0x85, 0x74, 0);
    }
    return;
}

/* [MSG_WALK 22 00 00] */
static void mudd5_WalkToDoor(void)
{
    fv_going_door = 1;
    room.busy = 1;
    svc_Walk(0, 0x85, 0x74, 0);
    return;
}

/* Every tick, common to all of the mission's rooms: while life support is
 * failing, the air runs down from 6978h, with warnings at 4F1Ah, 34BCh
 * and 1A5Eh; at 1 the crew collapse (walk msgs 9-12) and the game is
 * over.
 * [MSG_TICK FF FF FF] */
static void mudd5_AirRunsOut(void)
{
    if (rv_air_failing != 0) {
        if (rv_air_countdown != 1) {
            rv_air_countdown = 1;
            rv_air_left = 0x6978;
        }
        rv_air_left--;
        if (rv_air_left == 0x1A5E)
            goto L0715;
        if (rv_air_left == 0x34BC)
            goto L070B;
        if (rv_air_left == 0x4F1A)
            goto L0701;
        if (rv_air_left == 1) {
            room.busy = 1;
            svc_Walk(0, 0x8B, 0xAC, 9);
            svc_Walk(1, 0x6F, 0x99, 0xA);
            mudd5_Say(S_0AE6, T_MUD5N105);
            svc_GameOver();
        }
    }
    return;
L0701:;
    mudd5_Say(N_Mr_Spock, S_071F);
    return;
L070B:;
    mudd5_Say(N_Mr_Spock, S_0742);
    return;
L0715:;
    mudd5_Say(N_Mr_Spock, S_0765);
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void mudd5_KirkCollapses(void)
{
    svc_Anim(0, N_kgetdn, -1, -1, 0);
    svc_Anim(2, N_mgetds, -1, -1, 0);
    svc_Anim(3, N_rgetdw, -1, -1, 0);
    { mudd5_part_0833(); return; }
}

/* [MSG_WALK_DONE 0A 00 00] */
static void mudd5_SpockCollapses(void)
{
    svc_Anim(1, N_sgetde, -1, -1, 0);
    { mudd5_part_0833(); return; }
}

static void mudd5_part_0833(void)
{
    fv_collapsed++;
    if (fv_collapsed == 2) {
        mudd5_Say(S_0AE6, T_MUD5N007);
    }
    return;
}

/* [MSG_LOOK 00 00 00] */
static void mudd5_LookAtKirk(void)
{
    mudd5_Say(0, T_MUD5N002);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void mudd5_LookAtSpock(void)
{
    mudd5_Say(0, T_MUD5N005);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void mudd5_LookAtMcCoy(void)
{
    mudd5_Say(0, T_MUD5N004);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void mudd5_LookAtRedshirt(void)
{
    mudd5_Say(0, T_MUD5N003);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void mudd5_LookAtDoor(void)
{
    mudd5_Say(0, T_MUD5N010);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void mudd5_LookAtCrane(void)
{
    mudd5_Say(0, T_MUD5N000);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void mudd5_LookAtEngine(void)
{
    mudd5_Say(0, T_MUD5N006);
    return;
}

/* [MSG_TALK 00 00 00] */
static void mudd5_TalkToKirk(void)
{
    mudd5_Say(N_Capt_Kirk, T_MUD5_002);
    mudd5_Say(N_Mr_Spock, T_MUD5_032);
    return;
}

/* [MSG_TALK 01 00 00] */
static void mudd5_TalkToSpock(void)
{
    mudd5_Say(N_Mr_Spock, T_MUD5_033);
    mudd5_Say(N_Dr_McCoy, T_MUD5_016);
    return;
}

/* [MSG_TALK 02 00 00] */
static void mudd5_TalkToMcCoy(void)
{
    mudd5_Say(N_Dr_McCoy, T_MUD5_017);
    mudd5_Say(N_Capt_Kirk, T_MUD5_004);
    return;
}

/* [MSG_TALK 03 00 00] */
static void mudd5_TalkToRedshirt(void)
{
    mudd5_Say(N_Lt_Buchert, T_MUD5_040);
    mudd5_Say(N_Dr_McCoy, T_MUD5_018);
    return;
}

/* [MSG_USE 7F FF 00] */
static void mudd5_UseMedkit(void)
{
    mudd5_Say(N_Dr_McCoy, T_MUD5_012);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void mudd5_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void mudd5_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1F8C) = bx;
    mudd5_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript mudd5_scripts[] = {
    { 0x0134, mudd5_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x01E4, mudd5_MuddWantsDoover },    /* MSG_TIMER 01 00 00 */
    { 0x021D, mudd5_LifeSupportFails },    /* MSG_TIMER 02 00 00 */
    { 0x024E, mudd5_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0x02BC, mudd5_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x02ED, mudd5_SpockScansEngine },    /* MSG_USE 44 21 00 */
    { 0x031E, mudd5_SpockScansCrane },    /* MSG_USE 44 23 00 */
    { 0x034F, mudd5_SpockScansHatch },    /* MSG_USE 44 20 00 */
    { 0x0389, mudd5_SpockScansGenerator },    /* MSG_USE 44 09 00 */
    { 0x03CA, mudd5_StunHatch },    /* MSG_USE 40 20 00 */
    { 0x03EC, mudd5_KillHatch },    /* MSG_USE 41 20 00 */
    { 0x0405, mudd5_UseOnGenerator },    /* MSG_USE FF 09 00 */
    { 0x0415, mudd5_DooverOnGenerator },    /* MSG_USE 7C 09 00 */
    { 0x0447, mudd5_KirkAtGenerator },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0468, mudd5_GeneratorFixed },    /* MSG_TIMER 04 00 00 */
    { 0x0498, mudd5_Repaired },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x04F4, mudd5_MuddReturns },    /* MSG_TIMER 03 00 00 */
    { 0x052F, mudd5_CleanLenses },    /* MSG_USE 5F 5E 00 */
    { 0x055D, mudd5_FireDevice },    /* MSG_USE 7D FF 00 */
    { 0x05B0, mudd5_DeviceFired },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x05CE, mudd5_UseDegrimer },    /* MSG_USE 5E FF 00 */
    { 0x05DE, mudd5_LookAtHatch },    /* MSG_LOOK 20 00 00 */
    { 0x0600, mudd5_LookAtGenerator },    /* MSG_LOOK 09 00 00 */
    { 0x0629, mudd5_ReachDoor },    /* MSG_REGION 00 00 00 */
    { 0x0668, mudd5_WalkToDoor },    /* MSG_WALK 22 00 00 */
    { 0x068C, mudd5_AirRunsOut },    /* MSG_TICK FF FF FF */
    { 0x07D5, mudd5_KirkCollapses },    /* MSG_WALK_DONE 09 00 00 */
    { 0x081C, mudd5_SpockCollapses },    /* MSG_WALK_DONE 0A 00 00 */
    { 0x08DA, mudd5_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x08E9, mudd5_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x08F8, mudd5_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0907, mudd5_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0916, mudd5_LookAtDoor },    /* MSG_LOOK 22 00 00 */
    { 0x0925, mudd5_LookAtCrane },    /* MSG_LOOK 23 00 00 */
    { 0x0934, mudd5_LookAtEngine },    /* MSG_LOOK 21 00 00 */
    { 0x0943, mudd5_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x095C, mudd5_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0975, mudd5_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x098E, mudd5_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x09A7, mudd5_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0, 0 }
};
