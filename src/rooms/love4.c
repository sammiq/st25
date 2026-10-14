/* LOVE4.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/love4.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * LOVE4.RDF - "Love's Labor Jeopardized", the station's crew quarters,
 * on the deck the Romulans took. The virus has floored them (or the
 * Romulan laughing gas has); McCoy can cure all four with the cure from
 * room 2, and water helps their dehydration.
 *
 * Actors: 8 the door, 9-12 (0Ch) the Romulans.
 * Objects: 8/20h the door (room 5), 9-0Ch the Romulans, 21h the ladder up
 * to the lab (room 1). Region 0: the doorstep.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The communicator's four cases (CallEnterprise) all play the same
 * placeholder lines, recorded for other missions.
 * - Watering the Romulans gives its point every time: gave_water is set to
 * 2 but tested against 1.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s3r5d1 = 0x00D1,
    N_s3r5d1a = 0x00D8,
    N_s3r5r1 = 0x00E0,
    N_s3r5r2 = 0x00E7,
    N_s3r5r3 = 0x00EE,
    N_s3r5r4 = 0x00F5,
    N_s3r5r1a = 0x00FC,
    N_s3r5r2a = 0x0104,
    N_s3r5r3a = 0x010C,
    N_s3r5r4a = 0x0114,
    N_s3r5r1b = 0x011C,
    N_s3r5r2b = 0x0124,
    N_s3r5r3b = 0x012C,
    N_s3r5r4b = 0x0134,
    N_musele = 0x013C,
    N_museme = 0x0143,
    N_sscans = 0x014A,
    N_mscans = 0x0151,
    N_LOV4LOOP = 0x0160,
    T_LOV4_005 = 0x0447,
    T_LOV4_024 = 0x048C,
    T_LOV4_002 = 0x04BB,
    T_LOV4_022 = 0x0515,
    S_05C1 = 0x05C1,
    T_LOV4_018 = 0x0641,
    T_LOV4_015 = 0x06C5,
    T_LOV4_019 = 0x073F,
    T_LOV4_013 = 0x081E,
    T_LOV4N005 = 0x0985,
    T_LOV4_023 = 0x09F1,
    T_LOV4N013 = 0x0A84,
    T_LOV4_026 = 0x0AD4,
    T_LOV4_001 = 0x0B53,
    T_LOV4_012 = 0x0DD1,
    N_musemn = 0x0E65,
    N_sgetdn = 0x0E6C,
    N_get_name_error = 0x0ECB,
    T_LOV5_019 = 0x113C,
    T_LOV5_015 = 0x118A,
    T_LOVA_F55 = 0x11CD,
    T_LOV5_015_2 = 0x123D,
    T_LOV5_030 = 0x12DD,
    T_LOV5_038 = 0x1310,
    T_LOV5_027 = 0x133D,
    S_1372 = 0x1372,
    T_spokcoff = 0x13B5,
    T_LOVA_F08 = 0x13EC,
    T_spokcoff_2 = 0x1452,
    T_LOVA_F54 = 0x1488,
    T_GENER004 = 0x151D,
    T_LOVA_F01 = 0x1564,
    T_LOVA_F02 = 0x15AC,
    T_LOVA_F04 = 0x161E,
    T_LOVA_F03 = 0x1673,
    T_LOVA_F08_2 = 0x16B7,
    T_TUG2_010 = 0x1718,
    T_LOVA_F10 = 0x175E,
    T_LOVA_100 = 0x1792,
    T_LOVA_F07 = 0x17DF,
    T_LOV2N005 = 0x181A,
    T_LOV2N005_2 = 0x186A,
    T_MUD2_040 = 0x18A3,
    T_LOV2N005_3 = 0x18FC,
    N_Lt_Uhura = 0x1947,
    T_MUD4_018 = 0x1974,
    T_BRIDU146 = 0x19B9,
    T_VENA_F41 = 0x19EF,
    T_MUD4_018_2 = 0x1A18,
    T_BRIDU146_2 = 0x1A5D,
    T_VENA_F41_2 = 0x1A93,
    T_MUD4_018_3 = 0x1AC1,
    T_BRIDU146_3 = 0x1B06,
    T_VENA_F41_3 = 0x1B3C,
    T_MUD4_018_4 = 0x1B65,
    T_BRIDU146_4 = 0x1BAA,
    T_VENA_F41_4 = 0x1BE0,
    N_Capt_Kirk = 0x1C62,
    N_Dr_McCoy = 0x1C6D,
    N_Mr_Spock = 0x1C77,
    N_Lt_Ferris = 0x1C81,
    S_1C8C = 0x1C8C,
    T_LOV4N006 = 0x1C8D,
    T_LOV4N009 = 0x1CC2,
    T_LOV4N011 = 0x1D11,
    T_LOV4N010 = 0x1D47,
    T_LOV4N002 = 0x1D82,
    T_LOV4_025 = 0x1DBA,
    T_LOV4N003 = 0x1E0A,
    T_LOV4N001 = 0x1E3C,
    T_LOV4N008 = 0x1EA3,
    T_LOV4_014 = 0x1EE4,
    T_LOV4_007 = 0x1F29,
    T_LOV4_017 = 0x1FC2,
    T_LOV4_010 = 0x2005,
    T_LOV4_028 = 0x2063,
    T_LOV4_027 = 0x20A8,
    T_LOV4_003 = 0x20EC,
    T_LOV4_020 = 0x2129,
    T_LOV4_021 = 0x21AA,
    T_LOV4_006 = 0x21ED,
    T_LOV0_011 = 0x2243,
    T_LOV0_020 = 0x2265,
    T_LOV0_016 = 0x22A1,
    T_LOV0_012 = 0x22C2,
    T_LOV0_014 = 0x22FE,
    T_LOV0_022 = 0x232E,
    T_LOV0_015 = 0x2366,
    T_LOV0_021 = 0x239C,
    T_LOV0_013 = 0x23EA,
    T_LOV0_019 = 0x2420,
    T_LOV0_017 = 0x2470,
    T_LOV0_039 = 0x24AD,
    T_LOV0_048 = 0x24CF,
    T_LOV0_045 = 0x250B,
    T_LOV0_040 = 0x252C,
    T_LOV0_042 = 0x2568,
    T_LOV0_050 = 0x2598,
    T_LOV0_043 = 0x25D0,
    T_LOV0_049 = 0x2606,
    T_LOV0_041 = 0x2654,
    T_LOV0_047 = 0x268A,
    T_LOV0_046 = 0x26DA,
    T_LOV0_029 = 0x2717,
    T_LOV0_033 = 0x2753,
    T_LOV0_026 = 0x278F,
    T_LOV0_124 = 0x27DC,
    T_LOV0_025 = 0x280E,
    T_LOV0_101 = 0x285F,
    T_LOV0_102 = 0x289F,
    T_LOV0_103 = 0x28E5,
    T_LOV0_104 = 0x2917,
    T_LOV0_105 = 0x2950,
    T_LOV0_106 = 0x29C0,
    T_LOV0_107 = 0x2A04,
    talk_buf = 0x2A4E,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_cure_steps CS_B(0xE73)
#define fv_cure_x CS_B(0xE74)
#define fv_cure_y CS_B(0xE75)
#define fv_door_steps CS_B(0xCB)
#define fv_going_door CS_B(0xCC)
#define fv_laugher CS_W(0xCD)
#define fv_roll CS_W(0xCF)
#define fv_talk_buf CS_W(0x2A4E)
#define fv_watered CS_B(0xCA)
#define rv_been_below ROOM_B(0x3F)
#define rv_called_ship ROOM_B(0x47)
#define rv_called_ship_cured ROOM_B(0x48)
#define rv_gave_water ROOM_B(0x51)
#define rv_have_research ROOM_B(0x2A)
#define rv_made_cure ROOM_B(0x4F)
#define rv_n2o_released ROOM_B(0x2C)
#define rv_rlg_released ROOM_B(0x2D)
#define rv_romulans_cured ROOM_B(0x40)
#define rv_romulans_out ROOM_B(0x2B)
#define rv_score ROOM_W(0x52)
#define rv_spock_cured ROOM_B(0x46)
#define rv_virus ROOM_B(0x45)
#define rv_virus_struck ROOM_B(0x41)

/* functions */
static void love4_StartRoom(void);
static void love4_FirstVisit(void);
static void love4_E_02B5(void);
static void love4_WalkToDoor(void);
static void love4_ReachDoor(void);
static void love4_E_0318(void);
static void love4_ThroughDoor(void);
static void love4_ClimbLadder(void);
static void love4_AtLadder(void);
static void love4_E_0373(void);
static void love4_PhaserRomulan2(void);
static void love4_PhaserRomulan3(void);
static void love4_PhaserRomulan(void);
static void love4_LookAtScene(void);
static void love4_LookAtLadder(void);
static void love4_LookAtDoor(void);
static void love4_LookAtKirk(void);
static void love4_LookAtMcCoy(void);
static void love4_LookAtSpock(void);
static void love4_LookAtRedshirt(void);
static void love4_E_0404(void);
static void love4_LookAtRomulan2(void);
static void love4_LookAtRomulan3(void);
static void love4_LookAtRomulan(void);
static void love4_TalkToKirk(void);
static void love4_TalkToMcCoy(void);
static void love4_TalkToRedshirt(void);
static void love4_TalkToSpock(void);
static void love4_E_0598(void);
static void love4_TalkToRomulan2(void);
static void love4_TalkToRomulan3(void);
static void love4_TalkToRomulan(void);
static void love4_E_05F4(void);
static void love4_McCoyScansRomulan2(void);
static void love4_McCoyScansRomulan3(void);
static void love4_McCoyScansRomulan(void);
static void love4_McCoyScans(void);
static void love4_SpockScans(void);
static void love4_E_07D4(void);
static void love4_CureRomulan2(void);
static void love4_CureRomulan3(void);
static void love4_CureRomulans(void);
static void love4_McCoyAtRomulan4(void);
static void love4_Romulan4Cured(void);
static void love4_McCoyAtRomulan3(void);
static void love4_Romulan3Cured(void);
static void love4_McCoyAtRomulan2(void);
static void love4_Romulan2Cured(void);
static void love4_McCoyAtRomulan1(void);
static void love4_RomulansCured(void);
static void love4_E_0A52(void);
static void love4_WaterRomulan2(void);
static void love4_WaterRomulan3(void);
static void love4_WaterRomulans(void);
static void love4_E_0DAE(void);
static void love4_MedkitOnRomulan2(void);
static void love4_MedkitOnRomulan3(void);
static void love4_MedkitOnRomulan(void);
static void love4_E_0E1B(void);
static void love4_SampleOnRomulan2(void);
static void love4_SampleOnRomulan3(void);
static void love4_SampleOnRomulan(void);
static void love4_E_0E43(void);
static void love4_UseOnRomulan2(void);
static void love4_UseOnRomulan3(void);
static void love4_UseOnRomulan(void);
static void love4_CrewLaugh(void);
static void love4_SpockLaughs(void);
static void love4_MedkitOnSpock(void);
static void love4_SampleOnSpock(uint16_t ax);
static void love4_CureSpock(uint16_t ax);
static void love4_part_1266(uint16_t ax);
static void love4_AtSpock(void);
static void love4_SpockCured(void);
static void love4_VirusProgresses(void);
static void love4_McCoyScansSpock(void);
static void love4_E_17B8(void);
static void love4_McCoyScansMcCoy(void);
static void love4_McCoyScansCrew(void);
static void love4_ReleaseRLG(void);
static void love4_ReleaseN2O(void);
static void love4_ReleaseNH3(void);
static void love4_CallEnterprise(void);
static void love4_Talk(uint16_t ax);
static void love4_Say(uint16_t ax, uint16_t bx);

/* The room starts: the four Romulans, sick or cured.
 * [MSG_TICK 01 00 00] */
static void love4_StartRoom(void)
{
    uint16_t ax = 0;
    svc_Sfx(N_LOV4LOOP);
    if (rv_romulans_cured == 1) {
        svc_Anim(9, N_s3r5r1b, 0x36, 0xB3, 0);
        svc_Anim(0xA, N_s3r5r2b, 0xB9, 0xBB, 0);
        svc_Anim(0xB, N_s3r5r3b, 0xEF, 0xC4, 0);
        svc_Anim(0xC, N_s3r5r4b, 0x12A, 0xAA, 0);
    } else {
        svc_Anim(9, N_s3r5r1a, 0x36, 0xB3, 0);
        svc_Anim(0xA, N_s3r5r2a, 0xB9, 0xBB, 0);
        svc_Anim(0xB, N_s3r5r3a, 0xEF, 0xC4, 0);
        svc_Anim(0xC, N_s3r5r4a, 0x12A, 0xAA, 0);
    }
    if (rv_n2o_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[0] = ax;
    }
    if (rv_rlg_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[1] = ax;
    }
    room.timers[2] = 0xC8;
    svc_Anim0(8, N_s3r5d1a, 0x90, 0x99, 0);
    fv_cure_x = 0x6B;
    fv_cure_y = 0xB2;
    return;
}

/* Tick 10: the music, the first time.
 * [MSG_TICK 0A 00 00] */
static void love4_FirstVisit(void)
{
    if (rv_been_below != 1) {
        svc_Music(1, -1);
        rv_been_below = 1;
    }
    return;
}

/* The door to room 5 (msg 1, anim msg 1).
 * [MSG_WALK 08 00 00] */
static void love4_E_02B5(void)
{
    { love4_WalkToDoor(); return; }
}

/* [MSG_WALK 20 00 00] */
static void love4_WalkToDoor(void)
{
    room.busy = 1;
    fv_going_door = 1;
    svc_Walk(0, 0x91, 0x9C, 1);
    room.fall[0] = 0;
    return;
}

/* [MSG_REGION 00 00 00] */
static void love4_ReachDoor(void)
{
    if (fv_going_door == 1) {
        svc_Anim(8, N_s3r5d1, 0x90, 0x99, 1);
        svc_BridgeSfx(5);
    }
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void love4_E_0318(void)
{
    { love4_ThroughDoor(); return; }
}

/* [MSG_ANIM_DONE 01 00 00] */
static void love4_ThroughDoor(void)
{
    fv_door_steps++;
    if (fv_door_steps != 2) {
        return;
    }
    svc_GotoRoom(5, 1);
    return;
}

/* The ladder back up to the lab (msg 2).
 * [MSG_USE 00 21 00] */
static void love4_ClimbLadder(void)
{
    svc_Walk(0, 0xF6, 0xAC, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void love4_AtLadder(void)
{
    svc_GotoRoom(1, 2);
    return;
}

/* Phasers on the Romulans.
 * [MSG_USE 40 09 00] */
static void love4_E_0373(void)
{
    { love4_PhaserRomulan2(); return; }
}

/* [MSG_USE 40 0A 00] */
static void love4_PhaserRomulan2(void)
{
    { love4_PhaserRomulan3(); return; }
}

/* [MSG_USE 40 0B 00] */
static void love4_PhaserRomulan3(void)
{
    { love4_PhaserRomulan(); return; }
}

/* [MSG_USE 40 0C 00] */
static void love4_PhaserRomulan(void)
{
    love4_Say(N_Dr_McCoy, T_LOV4_007);
    return;
}

/* Looking and talking.
 * [MSG_LOOK FF 00 00] */
static void love4_LookAtScene(void)
{
    love4_Say(0, T_LOV4N009);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void love4_LookAtLadder(void)
{
    love4_Say(0, T_LOV4N010);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void love4_LookAtDoor(void)
{
    love4_Say(0, T_LOV4N008);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void love4_LookAtKirk(void)
{
    love4_Say(0, T_LOV4N002);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void love4_LookAtMcCoy(void)
{
    love4_Say(0, T_LOV4_025);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void love4_LookAtSpock(void)
{
    love4_Say(0, T_LOV4N003);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void love4_LookAtRedshirt(void)
{
    love4_Say(0, T_LOV4N001);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void love4_E_0404(void)
{
    { love4_LookAtRomulan2(); return; }
}

/* [MSG_LOOK 0A 00 00] */
static void love4_LookAtRomulan2(void)
{
    { love4_LookAtRomulan3(); return; }
}

/* [MSG_LOOK 0B 00 00] */
static void love4_LookAtRomulan3(void)
{
    { love4_LookAtRomulan(); return; }
}

/* [MSG_LOOK 0C 00 00] */
static void love4_LookAtRomulan(void)
{
    if (rv_romulans_cured == 0) {
        love4_Say(S_1C8C, T_LOV4N006);
        return;
    }
    love4_Say(S_1C8C, T_LOV4N011);
    return;
}

/* [MSG_TALK 00 00 00] */
static void love4_TalkToKirk(void)
{
    love4_Say(N_Capt_Kirk, T_LOV4_005);
    love4_Say(N_Dr_McCoy, T_LOV4_024);
    love4_Say(N_Capt_Kirk, T_LOV4_002);
    return;
}

/* [MSG_TALK 02 00 00] */
static void love4_TalkToMcCoy(void)
{
    if (rv_romulans_cured == 1) {
        love4_Say(N_Dr_McCoy, T_LOV4_022);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOV4_010);
    return;
}

/* [MSG_TALK 03 00 00] */
static void love4_TalkToRedshirt(void)
{
    love4_Say(N_Lt_Ferris, T_LOV4_028);
    return;
}

/* [MSG_TALK 01 00 00] */
static void love4_TalkToSpock(void)
{
    love4_Say(N_Mr_Spock, T_LOV4_027);
    love4_Say(N_Capt_Kirk, T_LOV4_003);
    if (room.redshirt_dead != 1) {
    }
    return;
}

/* [MSG_TALK 09 00 00] */
static void love4_E_0598(void)
{
    { love4_TalkToRomulan2(); return; }
}

/* [MSG_TALK 0A 00 00] */
static void love4_TalkToRomulan2(void)
{
    { love4_TalkToRomulan3(); return; }
}

/* [MSG_TALK 0B 00 00] */
static void love4_TalkToRomulan3(void)
{
    { love4_TalkToRomulan(); return; }
}

/* [MSG_TALK 0C 00 00] */
static void love4_TalkToRomulan(void)
{
    if (rv_romulans_cured == 1) {
        love4_Say(0, S_05C1);
        goto L05ED;
    }
L05ED:;
    return;
}

/* Tricorders.
 * [MSG_USE 45 09 00] */
static void love4_E_05F4(void)
{
    { love4_McCoyScansRomulan2(); return; }
}

/* [MSG_USE 45 0A 00] */
static void love4_McCoyScansRomulan2(void)
{
    { love4_McCoyScansRomulan3(); return; }
}

/* [MSG_USE 45 0B 00] */
static void love4_McCoyScansRomulan3(void)
{
    { love4_McCoyScansRomulan(); return; }
}

/* [MSG_USE 45 0C 00] */
static void love4_McCoyScansRomulan(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_romulans_cured != 1) {
    } else {
        love4_Say(N_Dr_McCoy, T_LOV4_018);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOV4_015);
    return;
}

/* [MSG_USE 45 FF 00] */
static void love4_McCoyScans(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_romulans_cured == 1) {
        love4_Say(N_Dr_McCoy, T_LOV4_019);
        return;
    }
    if (rv_romulans_out == 1) {
        love4_Say(N_Dr_McCoy, T_LOV4_020);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOV4_021);
    return;
}

/* [MSG_USE 44 FF 00] */
static void love4_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    love4_Say(N_Mr_Spock, T_LOV4_006);
    return;
}

/* The cure on the Romulans (+2, once): McCoy injects all four in turn
 * (walk msgs 3-6, anim msgs 2-5).
 * [MSG_USE 4D 09 00] */
static void love4_E_07D4(void)
{
    { love4_CureRomulan2(); return; }
}

/* [MSG_USE 4D 0A 00] */
static void love4_CureRomulan2(void)
{
    { love4_CureRomulan3(); return; }
}

/* [MSG_USE 4D 0B 00] */
static void love4_CureRomulan3(void)
{
    { love4_CureRomulans(); return; }
}

/* [MSG_USE 4D 0C 00] */
static void love4_CureRomulans(void)
{
    if (rv_romulans_cured != 1) {
        rv_score += 2;
        rv_romulans_cured = 1;
        room.busy = 1;
        svc_Walk(2, 0x10E, 0xB1, 3);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOV4_013);
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void love4_McCoyAtRomulan4(void)
{
    svc_Anim(2, N_museme, -1, -1, 2);
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void love4_Romulan4Cured(void)
{
    svc_Anim(0xC, N_s3r5r4, 0x12A, 0xAA, 0);
    svc_Walk(2, 0xEB, 0xC4, 4);
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void love4_McCoyAtRomulan3(void)
{
    svc_Anim(2, N_musele, -1, -1, 3);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void love4_Romulan3Cured(void)
{
    svc_Anim(0xB, N_s3r5r3, 0xEF, 0xC4, 0);
    svc_Walk(2, 0xB9, 0xC3, 5);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void love4_McCoyAtRomulan2(void)
{
    svc_Anim(2, N_musemn, -1, -1, 4);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void love4_Romulan2Cured(void)
{
    svc_Anim(0xA, N_s3r5r2, 0xB9, 0xBB, 0);
    svc_Walk(2, 0x36, 0xBA, 6);
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void love4_McCoyAtRomulan1(void)
{
    svc_Anim(2, N_musemn, -1, -1, 5);
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void love4_RomulansCured(void)
{
    svc_Anim(9, N_s3r5r1, 0x36, 0xB3, 0);
    love4_Say(0, T_LOV4N005);
    if (fv_watered != 1) {
        love4_Say(N_Dr_McCoy, T_LOV4_023);
        goto L0A47;
    }
L0A47:;
    room.busy = 0;
    return;
}

/* Water for the Romulans: +1 for the cured ones, every time (gave_water
 * is set to 2 but tested against 1).
 * [MSG_USE 67 09 00] */
static void love4_E_0A52(void)
{
    { love4_WaterRomulan2(); return; }
}

/* [MSG_USE 67 0A 00] */
static void love4_WaterRomulan2(void)
{
    { love4_WaterRomulan3(); return; }
}

/* [MSG_USE 67 0B 00] */
static void love4_WaterRomulan3(void)
{
    { love4_WaterRomulans(); return; }
}

/* [MSG_USE 67 0C 00] */
static void love4_WaterRomulans(void)
{
    fv_watered = 1;
    if (rv_romulans_cured != 1) {
    } else {
        love4_Say(0, T_LOV4N013);
        love4_Say(N_Dr_McCoy, T_LOV4_026);
        love4_Say(N_Capt_Kirk, T_LOV4_001);
        if (rv_gave_water != 1) {
            rv_score += 1;
            rv_gave_water = 2;
        }
    }
    svc_TakeItem(IT_H2O);
    return;
}

/* Other things on the Romulans.
 * [MSG_USE 7F 09 00] */
static void love4_E_0DAE(void)
{
    { love4_MedkitOnRomulan2(); return; }
}

/* [MSG_USE 7F 0A 00] */
static void love4_MedkitOnRomulan2(void)
{
    { love4_MedkitOnRomulan3(); return; }
}

/* [MSG_USE 7F 0B 00] */
static void love4_MedkitOnRomulan3(void)
{
    { love4_MedkitOnRomulan(); return; }
}

/* [MSG_USE 7F 0C 00] */
static void love4_MedkitOnRomulan(void)
{
    love4_Say(N_Dr_McCoy, T_LOV4_012);
    return;
}

/* [MSG_USE 4C 09 00] */
static void love4_E_0E1B(void)
{
    { love4_SampleOnRomulan2(); return; }
}

/* [MSG_USE 4C 0A 00] */
static void love4_SampleOnRomulan2(void)
{
    { love4_SampleOnRomulan3(); return; }
}

/* [MSG_USE 4C 0B 00] */
static void love4_SampleOnRomulan3(void)
{
    { love4_SampleOnRomulan(); return; }
}

/* [MSG_USE 4C 0C 00] */
static void love4_SampleOnRomulan(void)
{
    love4_Say(N_Dr_McCoy, T_LOV4_014);
    return;
}

/* [MSG_USE FF 09 00] */
static void love4_E_0E43(void)
{
    { love4_UseOnRomulan2(); return; }
}

/* [MSG_USE FF 0A 00] */
static void love4_UseOnRomulan2(void)
{
    { love4_UseOnRomulan3(); return; }
}

/* [MSG_USE FF 0B 00] */
static void love4_UseOnRomulan3(void)
{
    { love4_UseOnRomulan(); return; }
}

/* [MSG_USE FF 0C 00] */
static void love4_UseOnRomulan(void)
{
    love4_Say(S_1C8C, T_LOV4_017);
    return;
}

/* The gases, common to all of the mission's rooms. Timer 0 (after
 * laughing gas): a random crew member giggles, and again in 200-400 ticks.
 * [MSG_TIMER 00 00 00] */
static void love4_CrewLaugh(void)
{
    uint16_t ax = 0, bx = 0;
    ax = (uint16_t)svc_Random(0, 2);
    fv_roll = ax;
    if (fv_roll == 0) {
        fv_laugher = N_Dr_McCoy;
    } else {
        if (fv_roll == 1) {
            fv_laugher = N_Lt_Ferris;
            if (room.redshirt_dead == 1) {
                fv_laugher = N_Dr_McCoy;
                goto L0F87;
            }
        } else {
            love4_Say(0, N_get_name_error);
            return;
        }
    }
    fv_laugher = N_Dr_McCoy;
    ax = (uint16_t)svc_Random(0, 0xA);
    fv_roll = ax;
    if (fv_roll == 0) {
        bx = T_LOV0_011;
        goto L102E;
    }
    if (fv_roll == 1) {
        bx = T_LOV0_020;
        goto L102E;
    }
    if (fv_roll == 2) {
        bx = T_LOV0_016;
        goto L102E;
    }
    if (fv_roll == 3) {
        bx = T_LOV0_012;
        goto L102E;
    }
    if (fv_roll == 4) {
        bx = T_LOV0_014;
        goto L102E;
    }
    if (fv_roll == 5) {
        bx = T_LOV0_022;
        goto L102E;
    }
    if (fv_roll == 6) {
        bx = T_LOV0_015;
        goto L102E;
    }
    if (fv_roll == 7) {
        bx = T_LOV0_021;
        goto L102E;
    }
    if (fv_roll == 8) {
        bx = T_LOV0_013;
        goto L102E;
    }
    if (fv_roll == 9) {
        bx = T_LOV0_019;
        goto L102E;
    }
    bx = T_LOV0_017;
    goto L102E;
L0F87:;
    if (room.redshirt_dead == 1) {
        return;
    }
    fv_laugher = N_Lt_Ferris;
    ax = (uint16_t)svc_Random(0, 0xA);
    fv_roll = ax;
    if (fv_roll == 0) {
        bx = T_LOV0_039;
    } else if (fv_roll == 1) {
        bx = T_LOV0_048;
    } else if (fv_roll == 2) {
        bx = T_LOV0_045;
    } else if (fv_roll == 3) {
        bx = T_LOV0_040;
    } else if (fv_roll == 4) {
        bx = T_LOV0_042;
    } else if (fv_roll == 5) {
        bx = T_LOV0_050;
    } else if (fv_roll == 6) {
        bx = T_LOV0_043;
    } else if (fv_roll == 7) {
        bx = T_LOV0_049;
    } else if (fv_roll == 8) {
        bx = T_LOV0_041;
    } else if (fv_roll == 9) {
        bx = T_LOV0_047;
    } else {
        bx = T_LOV0_046;
        goto L102E;
    }
L102E:;
    ax = fv_laugher;
    love4_Say(ax, bx);
    if (rv_rlg_released == 1) {
        return;
    }
    ax = (uint16_t)svc_Random(0, 3);
    fv_roll = ax;
    ax = N_Mr_Spock;
    if (fv_roll == 0) {
        bx = T_LOV0_029;
    } else if (fv_roll == 1) {
        bx = T_LOV0_033;
    } else if (fv_roll == 2) {
        bx = T_LOV0_026;
    } else {
        bx = T_LOV0_124;
        goto L107D;
    }
L107D:;
    love4_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[0] = ax;
    return;
}

/* Timer 1 (after Romulan laughing gas): Spock laughs.
 * [MSG_TIMER 01 00 00] */
static void love4_SpockLaughs(void)
{
    uint16_t ax = 0, bx = 0;
    ax = (uint16_t)svc_Random(0, 7);
    fv_roll = ax;
    ax = N_Mr_Spock;
    if (fv_roll == 0) {
        bx = T_LOV0_025;
    } else if (fv_roll == 1) {
        bx = T_LOV0_101;
    } else if (fv_roll == 2) {
        bx = T_LOV0_102;
    } else if (fv_roll == 3) {
        bx = T_LOV0_103;
    } else if (fv_roll == 4) {
        bx = T_LOV0_104;
    } else if (fv_roll == 5) {
        bx = T_LOV0_105;
    } else if (fv_roll == 6) {
        bx = T_LOV0_106;
    } else {
        bx = T_LOV0_107;
        goto L110E;
    }
L110E:;
    love4_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[1] = ax;
    return;
}

/* Curing Spock, common to all of the mission's rooms: with the cure,
 * Spock and McCoy walk to the cure spot (msg 99), and McCoy injects him.
 * [MSG_USE 7F 01 00] */
static void love4_MedkitOnSpock(void)
{
    if (rv_spock_cured != 1) {
        love4_Say(N_Dr_McCoy, T_LOV5_019);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOV5_015);
    return;
}

/* [MSG_USE 4C 01 00] */
static void love4_SampleOnSpock(uint16_t ax)
{
    if (rv_spock_cured == 1) {
        { love4_part_1266(ax); return; }
    }
    love4_Say(N_Dr_McCoy, T_LOVA_F55);
    return;
}

/* [MSG_USE 4D 01 00] */
static void love4_CureSpock(uint16_t ax)
{
    if (rv_spock_cured != 1)
        { love4_part_1266(ax); return; }
    love4_Say(N_Dr_McCoy, T_LOV5_015_2);
    return;
}

static void love4_part_1266(uint16_t ax)
{
    uint16_t t0;
    SETHI(ax, 0);
    SETLO(ax, fv_cure_y);
    SETHI(ax, 0);
    SETLO(ax, fv_cure_x);
    ax = (uint16_t)svc_Walk(1, ax, ax, 0x63);
    SETHI(ax, 0);
    SETLO(ax, fv_cure_y);
    ax = ax + 0xA;
    t0 = ax;
    SETHI(ax, 0);
    SETLO(ax, fv_cure_x);
    svc_Walk(2, ax, ax, 0x63);
    return;
}

/* [MSG_WALK_DONE 63 00 00] */
static void love4_AtSpock(void)
{
    fv_cure_steps++;
    if (fv_cure_steps != 2) {
        return;
    }
    svc_Anim(2, N_musemn, -1, -1, 0x63);
    return;
}

/* [MSG_ANIM_DONE 63 00 00] */
static void love4_SpockCured(void)
{
    love4_Say(N_Dr_McCoy, T_LOV5_030);
    love4_Say(N_Mr_Spock, T_LOV5_038);
    love4_Say(N_Dr_McCoy, T_LOV5_027);
    love4_Say(N_Mr_Spock, S_1372);
    rv_spock_cured = 1;
    return;
}

/* Timer 2, every 200 ticks until Spock is cured: the virus advances,
 * with a line at 0Fh, 1Eh, 2Dh; at 3Ch Spock collapses, at 64h he dies.
 * [MSG_TIMER 02 00 00] */
static void love4_VirusProgresses(void)
{
    if (rv_spock_cured == 1) {
        return;
    }
    rv_virus++;
    if ((int8_t)rv_virus >= (int8_t)0x64) {
    } else {
        if (rv_virus == 0xF) {
            love4_Say(N_Mr_Spock, T_spokcoff);
            rv_virus++;
        } else {
            if (rv_virus == 0x1E) {
                love4_Say(N_Dr_McCoy, T_LOVA_F08);
                rv_virus++;
            } else {
                if (rv_virus == 0x2D) {
                    love4_Say(N_Mr_Spock, T_spokcoff_2);
                    rv_virus++;
                } else {
                    if (rv_virus == 0x3C) {
                        love4_Say(N_Mr_Spock, T_LOVA_F54);
                        rv_virus_struck = 1;
                        rv_virus++;
                        goto L14EB;
                    }
                }
            }
        }
L14EB:;
        room.timers[2] = 0xC8;
        return;
    }
    svc_Anim(1, N_sgetdn, -1, -1, 0);
    svc_Music(2, STALE_ARG);
    love4_Say(0, T_GENER004);
    svc_GameOver();
    return;
}

/* McCoy's tricorder on Spock follows the virus; on the others, normal.
 * [MSG_USE 45 01 00] */
static void love4_McCoyScansSpock(void)
{
    svc_BridgeSfx(4);
    if (rv_spock_cured == 1) {
        love4_Say(N_Dr_McCoy, T_LOVA_F01);
        return;
    }
    if (rv_have_research != 1) {
        love4_Say(N_Dr_McCoy, T_LOVA_F02);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0xA) {
        love4_Say(N_Dr_McCoy, T_LOVA_F04);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x1E) {
        love4_Say(N_Dr_McCoy, T_LOVA_F03);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x32) {
        love4_Say(N_Dr_McCoy, T_LOVA_F08_2);
        return;
    }
    if (rv_virus == 0x46) {
        love4_Say(N_Dr_McCoy, T_TUG2_010);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x64) {
        love4_Say(N_Dr_McCoy, T_LOVA_F10);
        return;
    }
    love4_Say(N_Dr_McCoy, T_LOVA_100);
    return;
}

/* [MSG_USE 45 00 00] */
static void love4_E_17B8(void)
{
    { love4_McCoyScansMcCoy(); return; }
}

/* [MSG_USE 45 02 00] */
static void love4_McCoyScansMcCoy(void)
{
    { love4_McCoyScansCrew(); return; }
}

/* [MSG_USE 45 03 00] */
static void love4_McCoyScansCrew(void)
{
    svc_BridgeSfx(4);
    love4_Say(N_Dr_McCoy, T_LOVA_F07);
    return;
}

/* Opening a gas canister: Romulan laughing gas sets Spock laughing,
 * nitrous oxide the crew, ammonia nothing.
 * [MSG_USE 49 FF 00] */
static void love4_ReleaseRLG(void)
{
    love4_Say(S_1C8C, T_LOV2N005);
    rv_rlg_released = 1;
    svc_TakeItem(IT_RLG);
    return;
}

/* [MSG_USE 65 FF 00] */
static void love4_ReleaseN2O(void)
{
    love4_Say(S_1C8C, T_LOV2N005_2);
    love4_Say(N_Mr_Spock, T_MUD2_040);
    rv_n2o_released = 1;
    svc_TakeItem(IT_N2O);
    svc_Music(3, STALE_ARG);
    return;
}

/* [MSG_USE 66 FF 00] */
static void love4_ReleaseNH3(void)
{
    love4_Say(S_1C8C, T_LOV2N005_3);
    if (room.redshirt_dead != 1) {
    }
    svc_TakeItem(IT_NH3);
    return;
}

/* The communicator: four cases (first call, later calls, the same after
 * the cure), all with the same three lines borrowed from other missions'
 * speech files.
 * [MSG_USE 47 FF 00] */
static void love4_CallEnterprise(void)
{
    if (rv_made_cure == 1) {
    } else {
        if (rv_called_ship == 1) {
        } else {
            love4_Say(N_Capt_Kirk, T_MUD4_018);
            love4_Say(N_Lt_Uhura, T_BRIDU146);
            love4_Say(N_Capt_Kirk, T_VENA_F41);
            rv_called_ship = 1;
            return;
        }
        love4_Say(N_Capt_Kirk, T_MUD4_018_2);
        love4_Say(N_Lt_Uhura, T_BRIDU146_2);
        love4_Say(N_Capt_Kirk, T_VENA_F41_2);
        return;
    }
    if (rv_called_ship_cured == 1) {
    } else {
        love4_Say(N_Capt_Kirk, T_MUD4_018_3);
        love4_Say(N_Lt_Uhura, T_BRIDU146_3);
        love4_Say(N_Capt_Kirk, T_VENA_F41_3);
        rv_called_ship_cured = 1;
        return;
    }
    love4_Say(N_Capt_Kirk, T_MUD4_018_4);
    love4_Say(N_Lt_Uhura, T_BRIDU146_4);
    love4_Say(N_Capt_Kirk, T_VENA_F41_4);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void love4_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void love4_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x2A50) = bx;
    love4_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript love4_scripts[] = {
    { 0x015E, love4_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0296, love4_FirstVisit },    /* MSG_TICK 0A 00 00 */
    { 0x02B5, love4_E_02B5 },    /* MSG_WALK 08 00 00 */
    { 0x02BD, love4_WalkToDoor },    /* MSG_WALK 20 00 00 */
    { 0x02E6, love4_ReachDoor },    /* MSG_REGION 00 00 00 */
    { 0x0318, love4_E_0318 },    /* MSG_WALK_DONE 01 00 00 */
    { 0x0320, love4_ThroughDoor },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0343, love4_ClimbLadder },    /* MSG_USE 00 21 00 */
    { 0x035E, love4_AtLadder },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0373, love4_E_0373 },    /* MSG_USE 40 09 00 */
    { 0x037B, love4_PhaserRomulan2 },    /* MSG_USE 40 0A 00 */
    { 0x0383, love4_PhaserRomulan3 },    /* MSG_USE 40 0B 00 */
    { 0x038B, love4_PhaserRomulan },    /* MSG_USE 40 0C 00 */
    { 0x039B, love4_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x03AA, love4_LookAtLadder },    /* MSG_LOOK 21 00 00 */
    { 0x03B9, love4_LookAtDoor },    /* MSG_LOOK 08 00 00 */
    { 0x03C8, love4_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x03D7, love4_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x03E6, love4_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x03F5, love4_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0404, love4_E_0404 },    /* MSG_LOOK 09 00 00 */
    { 0x040C, love4_LookAtRomulan2 },    /* MSG_LOOK 0A 00 00 */
    { 0x0414, love4_LookAtRomulan3 },    /* MSG_LOOK 0B 00 00 */
    { 0x041C, love4_LookAtRomulan },    /* MSG_LOOK 0C 00 00 */
    { 0x043C, love4_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0504, love4_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0569, love4_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0579, love4_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0598, love4_E_0598 },    /* MSG_TALK 09 00 00 */
    { 0x05A0, love4_TalkToRomulan2 },    /* MSG_TALK 0A 00 00 */
    { 0x05A8, love4_TalkToRomulan3 },    /* MSG_TALK 0B 00 00 */
    { 0x05B0, love4_TalkToRomulan },    /* MSG_TALK 0C 00 00 */
    { 0x05F4, love4_E_05F4 },    /* MSG_USE 45 09 00 */
    { 0x05FC, love4_McCoyScansRomulan2 },    /* MSG_USE 45 0A 00 */
    { 0x0604, love4_McCoyScansRomulan3 },    /* MSG_USE 45 0B 00 */
    { 0x060C, love4_McCoyScansRomulan },    /* MSG_USE 45 0C 00 */
    { 0x070D, love4_McCoyScans },    /* MSG_USE 45 FF 00 */
    { 0x07A3, love4_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x07D4, love4_E_07D4 },    /* MSG_USE 4D 09 00 */
    { 0x07DC, love4_CureRomulan2 },    /* MSG_USE 4D 0A 00 */
    { 0x07E4, love4_CureRomulan3 },    /* MSG_USE 4D 0B 00 */
    { 0x07EC, love4_CureRomulans },    /* MSG_USE 4D 0C 00 */
    { 0x085F, love4_McCoyAtRomulan4 },    /* MSG_WALK_DONE 03 00 00 */
    { 0x087B, love4_Romulan4Cured },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x08AD, love4_McCoyAtRomulan3 },    /* MSG_WALK_DONE 04 00 00 */
    { 0x08C9, love4_Romulan3Cured },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x08FB, love4_McCoyAtRomulan2 },    /* MSG_WALK_DONE 05 00 00 */
    { 0x0917, love4_Romulan2Cured },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0948, love4_McCoyAtRomulan1 },    /* MSG_WALK_DONE 06 00 00 */
    { 0x0964, love4_RomulansCured },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x0A52, love4_E_0A52 },    /* MSG_USE 67 09 00 */
    { 0x0A5A, love4_WaterRomulan2 },    /* MSG_USE 67 0A 00 */
    { 0x0A62, love4_WaterRomulan3 },    /* MSG_USE 67 0B 00 */
    { 0x0A6A, love4_WaterRomulans },    /* MSG_USE 67 0C 00 */
    { 0x0DAE, love4_E_0DAE },    /* MSG_USE 7F 09 00 */
    { 0x0DB6, love4_MedkitOnRomulan2 },    /* MSG_USE 7F 0A 00 */
    { 0x0DBE, love4_MedkitOnRomulan3 },    /* MSG_USE 7F 0B 00 */
    { 0x0DC6, love4_MedkitOnRomulan },    /* MSG_USE 7F 0C 00 */
    { 0x0E1B, love4_E_0E1B },    /* MSG_USE 4C 09 00 */
    { 0x0E23, love4_SampleOnRomulan2 },    /* MSG_USE 4C 0A 00 */
    { 0x0E2B, love4_SampleOnRomulan3 },    /* MSG_USE 4C 0B 00 */
    { 0x0E33, love4_SampleOnRomulan },    /* MSG_USE 4C 0C 00 */
    { 0x0E43, love4_E_0E43 },    /* MSG_USE FF 09 00 */
    { 0x0E4B, love4_UseOnRomulan2 },    /* MSG_USE FF 0A 00 */
    { 0x0E53, love4_UseOnRomulan3 },    /* MSG_USE FF 0B 00 */
    { 0x0E5B, love4_UseOnRomulan },    /* MSG_USE FF 0C 00 */
    { 0x0E7C, love4_CrewLaugh },    /* MSG_TIMER 00 00 00 */
    { 0x1099, love4_SpockLaughs },    /* MSG_TIMER 01 00 00 */
    { 0x112B, love4_MedkitOnSpock },    /* MSG_USE 7F 01 00 */
    /* 11B9: a helper, not a script */
    /* 122C: a helper, not a script */
    { 0x12A8, love4_AtSpock },    /* MSG_WALK_DONE 63 00 00 */
    { 0x12D2, love4_SpockCured },    /* MSG_ANIM_DONE 63 00 00 */
    { 0x1391, love4_VirusProgresses },    /* MSG_TIMER 02 00 00 */
    { 0x1547, love4_McCoyScansSpock },    /* MSG_USE 45 01 00 */
    { 0x17B8, love4_E_17B8 },    /* MSG_USE 45 00 00 */
    { 0x17C0, love4_McCoyScansMcCoy },    /* MSG_USE 45 02 00 */
    { 0x17C8, love4_McCoyScansCrew },    /* MSG_USE 45 03 00 */
    { 0x180F, love4_ReleaseRLG },    /* MSG_USE 49 FF 00 */
    { 0x185F, love4_ReleaseN2O },    /* MSG_USE 65 FF 00 */
    { 0x18F1, love4_ReleaseNH3 },    /* MSG_USE 66 FF 00 */
    { 0x1957, love4_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0, 0 }
};
