/* LOVE3.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/love3.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * LOVE3.RDF - "Love's Labor Jeopardized", the station's engineering
 * center. The air vent here leads to the Romulan-held deck below: Romulan
 * laughing gas poured down it puts the Romulans out. The nitrogen tank the
 * synthesizer needs (room 2) is here, bolted to its feed lines; the access
 * panel holds the insulation for polyberylcarbonate (room 1); the wrench
 * lies on the floor.
 *
 * Actors: 8 the service panel, 9 the access panel, 10 (0Ah) the wrench,
 * 11 (0Bh) the vent cover, 12 (0Ch) the nitrogen tank.
 * Objects: 8 the service panel, 9/20h the access panel, 0Ah the wrench,
 * 0Bh the vent cover, 0Ch the tank, 21h the readouts, 22h the vent, 23h
 * the reactor. Exit 1: the lab (room 1).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The communicator's four cases (CallEnterprise) all play the same
 * placeholder lines, recorded for other missions.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_wrench = 0x00D2,
    N_r4n2 = 0x00D9,
    N_s3r4p1 = 0x00DE,
    N_s3r4p2 = 0x00E5,
    N_s3r4d1 = 0x00EC,
    N_s3r4d1a = 0x00F3,
    N_s3r4d2 = 0x00FB,
    N_s3r4d1b = 0x0102,
    N_s3r4g1 = 0x010A,
    N_s3r4g3 = 0x0111,
    N_s3r4g2 = 0x0118,
    N_s3r4g2a = 0x011F,
    N_s5r1kg = 0x0127,
    N_kusehe = 0x0135,
    N_kuselw = 0x013C,
    N_kusemn = 0x0143,
    N_kuseme = 0x014A,
    N_susehe = 0x0151,
    N_musehe = 0x0158,
    N_rusehe = 0x015F,
    N_sscans = 0x0166,
    N_sscann = 0x016D,
    N_sscanw = 0x0174,
    N_mscans = 0x017B,
    N_LOV3LOOP = 0x018A,
    T_LOV3_010 = 0x02AC,
    T_LOV3_020 = 0x0353,
    T_LOV3NA23 = 0x0441,
    T_LOV3NA21 = 0x04A3,
    T_LOV3N019 = 0x04F8,
    T_LOV3N017 = 0x055E,
    T_LOV3N006 = 0x05DF,
    T_LOV3_001 = 0x063E,
    T_LOV3_017 = 0x06A0,
    T_LOV3_008 = 0x0722,
    T_LOV3_024 = 0x07A8,
    T_LOV3_006 = 0x0823,
    N_SMADOOR1 = 0x09E7,
    T_LOV3N020 = 0x0A14,
    T_LOV3N023 = 0x0A47,
    T_LOV3N022 = 0x0A73,
    N_MUR3E1S = 0x0B1D,
    T_LOV3N012 = 0x0B4F,
    T_LOV3_022 = 0x0B89,
    T_LOV3N015 = 0x0BF4,
    T_LOV3_016 = 0x0C54,
    N_MUR3E1S_2 = 0x0D02,
    T_LOV3N013 = 0x0D2B,
    N_MUR3E1S_3 = 0x0DC9,
    N_MADR4E4A = 0x0DEF,
    T_LOV3_021 = 0x0E82,
    T_LOV3N030 = 0x0F4A,
    N_MUR3E1S_4 = 0x0FD1,
    T_LOV3N032 = 0x105C,
    T_LOV3N011 = 0x10B5,
    T_LOV3N009 = 0x1146,
    T_LOV3N018 = 0x11D4,
    T_LOV3_012 = 0x120B,
    T_LOV3N001 = 0x12F4,
    T_LOV3_023 = 0x1340,
    T_LOV3_011 = 0x13D7,
    N_EFX24 = 0x1460,
    N_ROMULANL = 0x149D,
    T_LOV3N007 = 0x1564,
    T_LOV3N014 = 0x15AE,
    N_SE3PLBAT = 0x1641,
    T_LOV3NJ32 = 0x1687,
    T_LOV3_025 = 0x16ED,
    T_LOV3N010 = 0x173B,
    T_LOV3_015 = 0x179E,
    N_musemn = 0x187F,
    N_sgetdn = 0x1886,
    N_get_name_error = 0x18E5,
    T_LOV5_019 = 0x1B56,
    T_LOV5_015 = 0x1BA4,
    T_LOVA_F55 = 0x1BE7,
    T_LOV5_015_2 = 0x1C57,
    T_LOV5_030 = 0x1CF7,
    T_LOV5_038 = 0x1D2A,
    T_LOV5_027 = 0x1D57,
    S_1D8C = 0x1D8C,
    T_spokcoff = 0x1DCF,
    T_LOVA_F08 = 0x1E06,
    T_spokcoff_2 = 0x1E6C,
    T_LOVA_F54 = 0x1EA2,
    T_GENER004 = 0x1F37,
    T_LOVA_F01 = 0x1F7E,
    T_LOVA_F02 = 0x1FC6,
    T_LOVA_F04 = 0x2038,
    T_LOVA_F03 = 0x208D,
    T_LOVA_F08_2 = 0x20D1,
    T_TUG2_010 = 0x2132,
    T_LOVA_F10 = 0x2178,
    T_LOVA_100 = 0x21AC,
    T_LOVA_F07 = 0x21F9,
    T_LOV2N005 = 0x2234,
    T_LOV2N005_2 = 0x2284,
    T_MUD2_040 = 0x22BD,
    T_LOV2N005_3 = 0x2316,
    N_Lt_Uhura = 0x2361,
    T_MUD4_018 = 0x238E,
    T_BRIDU146 = 0x23D3,
    T_VENA_F41 = 0x2409,
    T_MUD4_018_2 = 0x2432,
    T_BRIDU146_2 = 0x2477,
    T_VENA_F41_2 = 0x24AD,
    T_MUD4_018_3 = 0x24DB,
    T_BRIDU146_3 = 0x2520,
    T_VENA_F41_3 = 0x2556,
    T_MUD4_018_4 = 0x257F,
    T_BRIDU146_4 = 0x25C4,
    T_VENA_F41_4 = 0x25FA,
    N_Capt_Kirk = 0x26E7,
    N_Dr_McCoy = 0x26F2,
    N_Mr_Spock = 0x26FC,
    N_Lt_Ferris = 0x2706,
    S_2711 = 0x2711,
    T_LOV3NA22 = 0x2712,
    T_LOV3NA20 = 0x278B,
    T_LOV3_ = 0x27EF,
    T_LOV3N002 = 0x2814,
    T_LOV3N003 = 0x2854,
    T_LOV3N005 = 0x288D,
    T_LOV3N008 = 0x28B7,
    T_LOV3N004 = 0x28EB,
    T_LOV3N026 = 0x2968,
    T_LOV3_009 = 0x29D8,
    T_LOV3_019 = 0x2A2A,
    T_LOV3_026 = 0x2ACA,
    T_LOV3_003 = 0x2AFF,
    T_LOV3_004 = 0x2B80,
    T_LOV3_005 = 0x2BF3,
    T_LOV3_013 = 0x2C46,
    T_LOV3_014 = 0x2C72,
    T_LOV3_002 = 0x2CEE,
    T_LOV3N031 = 0x2D2B,
    T_LOV3N029 = 0x2D82,
    T_LOV3N021 = 0x2DE4,
    T_LOV3_018 = 0x2E73,
    T_LOV3N016 = 0x2F0D,
    T_LOV3N025 = 0x2F4E,
    T_LOV3NA09 = 0x2F7C,
    T_LOV3NA08 = 0x3047,
    T_LOV0_011 = 0x30B5,
    T_LOV0_020 = 0x30D7,
    T_LOV0_016 = 0x3113,
    T_LOV0_012 = 0x3134,
    T_LOV0_014 = 0x3170,
    T_LOV0_022 = 0x31A0,
    T_LOV0_015 = 0x31D8,
    T_LOV0_021 = 0x320E,
    T_LOV0_013 = 0x325C,
    T_LOV0_019 = 0x3292,
    T_LOV0_017 = 0x32E2,
    T_LOV0_039 = 0x331F,
    T_LOV0_048 = 0x3341,
    T_LOV0_045 = 0x337D,
    T_LOV0_040 = 0x339E,
    T_LOV0_042 = 0x33DA,
    T_LOV0_050 = 0x340A,
    T_LOV0_043 = 0x3442,
    T_LOV0_049 = 0x3478,
    T_LOV0_041 = 0x34C6,
    T_LOV0_047 = 0x34FC,
    T_LOV0_046 = 0x354C,
    T_LOV0_029 = 0x3589,
    T_LOV0_033 = 0x35C5,
    T_LOV0_026 = 0x3601,
    T_LOV0_124 = 0x364E,
    T_LOV0_025 = 0x3680,
    T_LOV0_101 = 0x36D1,
    T_LOV0_102 = 0x3711,
    T_LOV0_103 = 0x3757,
    T_LOV0_104 = 0x3789,
    T_LOV0_105 = 0x37C2,
    T_LOV0_106 = 0x3832,
    T_LOV0_107 = 0x3876,
    talk_buf = 0x38CD,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_crew CS_W(0xCE)
#define fv_crew_anim CS_W(0xD0)
#define fv_cure_steps CS_B(0x188D)
#define fv_cure_x CS_B(0x188E)
#define fv_cure_y CS_B(0x188F)
#define fv_laugher CS_W(0xCA)
#define fv_roll CS_W(0xCC)
#define fv_talk_buf CS_W(0x38CD)
#define rv_access_panel_open ROOM_B(0x3E)
#define rv_been_below ROOM_B(0x3F)
#define rv_called_ship ROOM_B(0x47)
#define rv_called_ship_cured ROOM_B(0x48)
#define rv_gassed ROOM_B(0x4E)
#define rv_have_research ROOM_B(0x2A)
#define rv_made_cure ROOM_B(0x4F)
#define rv_n2o_released ROOM_B(0x2C)
#define rv_never_set ROOM_B(0x3C)
#define rv_opened_vent ROOM_B(0x4D)
#define rv_rlg_released ROOM_B(0x2D)
#define rv_romulans_out ROOM_B(0x2B)
#define rv_score ROOM_W(0x52)
#define rv_service_panel_open ROOM_B(0x38)
#define rv_spock_cured ROOM_B(0x46)
#define rv_tank_loose ROOM_B(0x39)
#define rv_took_n2_tank ROOM_B(0x3B)
#define rv_took_wrench ROOM_B(0x3A)
#define rv_vent_open ROOM_B(0x3D)
#define rv_virus ROOM_B(0x45)
#define rv_virus_struck ROOM_B(0x41)

/* functions */
static void love3_StartRoom(void);
static void love3_McCoyJokes(void);
static void love3_ExitToLab(void);
static void love3_GoToLab(void);
static void love3_LookAtServicePanel(void);
static void love3_LookAtReadouts(void);
static void love3_LookAtWrench(void);
static void love3_LookAtKirk(void);
static void love3_LookAtMcCoy(void);
static void love3_LookAtSpock(void);
static void love3_LookAtRedshirt(void);
static void love3_LookAtScene(void);
static void love3_LookAtVent(void);
static void love3_LookAtVentCover(void);
static void love3_E_0545(void);
static void love3_LookAtAccessPanel(void);
static void love3_LookAtTank(void);
static void love3_LookAtReactor(void);
static void love3_TalkToKirk(void);
static void love3_TalkToSpock(void);
static void love3_TalkToMcCoy(void);
static void love3_TalkToRedshirt(void);
static void love3_McCoyScans(void);
static void love3_SpockScansReactor(void);
static void love3_SpockScansInsulation(void);
static void love3_SpockScans(void);
static void love3_KirkOpensServicePanel(void);
static void love3_SpockOpensServicePanel(void);
static void love3_McCoyOpensServicePanel(void);
static void love3_FerrisOpensServicePanel(void);
static void love3_part_0987(void);
static void love3_AtServicePanel(void);
static void love3_ServicePanelToggled(void);
static void love3_WrenchOnTank(void);
static void love3_KirkAtTank(void);
static void love3_TankWrenched(void);
static void love3_E_0C41(void);
static void love3_WrenchOnOtherTank(void);
static void love3_WrenchOnN2Tank(void);
static void love3_KirkAtTankSlot(void);
static void love3_N2TankBack(void);
static void love3_WrenchOnAccessPanel(void);
static void love3_KirkAtAccessPanel(void);
static void love3_AccessPanelOpen(void);
static void love3_WrenchOnReadouts(void);
static void love3_WaterOnReadouts(void);
static void love3_KirkAtReadouts(void);
static void love3_ReadoutsWet(void);
static void love3_E_0F07(void);
static void love3_SpockOnAccessPanel(void);
static void love3_McCoyOnAccessPanel(void);
static void love3_FerrisOnAccessPanel(void);
static void love3_KirkOnVentCover(void);
static void love3_SpockOnVentCover(void);
static void love3_McCoyOnVentCover(void);
static void love3_FerrisOnVentCover(void);
static void love3_WrenchOnVentCover(void);
static void love3_KirkAtVent(void);
static void love3_VentToggled(void);
static void love3_E_1092(void);
static void love3_SpockInVent(void);
static void love3_McCoyInVent(void);
static void love3_FerrisInVent(void);
static void love3_WaterInVent(void);
static void love3_KirkAtVent2(void);
static void love3_VentWet(void);
static void love3_N2OInVent(void);
static void love3_KirkAtVent3(void);
static void love3_N2OVented(void);
static void love3_NH3InVent(void);
static void love3_KirkAtVent4(void);
static void love3_NH3Vented(void);
static void love3_RLGInVent(void);
static void love3_KirkAtVent5(void);
static void love3_RLGVented(void);
static void love3_GetWrench(void);
static void love3_KirkAtWrench(void);
static void love3_TookWrench(void);
static void love3_GetTank(void);
static void love3_KirkAtTank2(void);
static void love3_AntigravOnTank(void);
static void love3_KirkAtTank3(void);
static void love3_TankLifted(void);
static void love3_GetInsulation(void);
static void love3_KirkAtInsulation(void);
static void love3_TookInsulation(void);
static void love3_part_187E(void);
static void love3_CrewLaugh(void);
static void love3_SpockLaughs(void);
static void love3_MedkitOnSpock(void);
static void love3_SampleOnSpock(uint16_t ax);
static void love3_CureSpock(uint16_t ax);
static void love3_part_1C80(uint16_t ax);
static void love3_AtSpock(void);
static void love3_SpockCured(void);
static void love3_VirusProgresses(void);
static void love3_McCoyScansSpock(void);
static void love3_E_21D2(void);
static void love3_McCoyScansMcCoy(void);
static void love3_McCoyScansCrew(void);
static void love3_ReleaseRLG(void);
static void love3_ReleaseN2O(void);
static void love3_ReleaseNH3(void);
static void love3_CallEnterprise(void);
static void love3_Talk(uint16_t ax);
static void love3_Say(uint16_t ax, uint16_t bx);

/* The room starts: the wrench, the vent, the panels, the tank.
 * [MSG_TICK 01 00 00] */
static void love3_StartRoom(void)
{
    uint16_t ax = 0;
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    svc_Sfx(N_LOV3LOOP);
    if (rv_took_wrench != 1) {
        svc_Anim0(0xA, N_wrench, 0x10A, 0xB6, 0);
    }
    s0 = 0;
    if (rv_vent_open == 1) {
        s1 = 0;
        s2 = 0;
        s3 = N_s3r4g3;
    } else {
        s1 = 0x8F;
        s2 = 0x116;
        s3 = N_s3r4g1;
    }
    svc_Anim0(0xB, s3, s2, s1, s0);
    if (rv_access_panel_open == 1) {
        svc_Anim0(9, N_s3r4p2, 0, 0, 0);
    }
    if (rv_took_n2_tank != 1) {
        svc_Anim0(0xC, N_r4n2, 0, 0, 0);
    }
    s0 = 0;
    s1 = 0;
    s2 = 1;
    s3 = N_s3r4d2;
    if (rv_service_panel_open == 1) {
        s3 = N_s3r4d1a;
    }
    svc_Anim0(8, s3, s2, s1, s0);
    if (rv_n2o_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[0] = ax;
    }
    if (rv_rlg_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[1] = ax;
    }
    room.timers[2] = 0xC8;
    fv_cure_x = 0xB4;
    fv_cure_y = 0xB7;
    return;
}

/* Tick 80, with the Romulan laughing gas and not yet below: McCoy jokes.
 * [MSG_TICK 50 00 00] */
static void love3_McCoyJokes(void)
{
    uint16_t ax = 0;
    if (rv_been_below == 1) {
    } else {
        ax = (uint16_t)svc_HaveItem(IT_RLG);
        if (ax == 0) {
        } else {
            love3_Say(N_Dr_McCoy, T_LOV3_010);
            love3_Say(N_Mr_Spock, T_LOV3_020);
            goto L0391;
        }
    }
L0391:;
    return;
}

/* Exit 1 (timer 3): to the lab, room 1.
 * [MSG_EXIT 01 00 00] */
static void love3_ExitToLab(void)
{
    room.busy = 1;
    svc_BridgeSfx(5);
    room.timers[3] = 0xA;
    return;
}

/* [MSG_TIMER 03 00 00] */
static void love3_GoToLab(void)
{
    svc_GotoRoom(1, 0);
    room.busy = 0;
    return;
}

/* Looking and talking.
 * [MSG_LOOK 08 00 00] */
static void love3_LookAtServicePanel(void)
{
    love3_Say(0, T_LOV3NA20);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void love3_LookAtReadouts(void)
{
    love3_Say(0, T_LOV3_);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void love3_LookAtWrench(void)
{
    love3_Say(0, T_LOV3N002);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void love3_LookAtKirk(void)
{
    love3_Say(0, T_LOV3N003);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void love3_LookAtMcCoy(void)
{
    love3_Say(0, T_LOV3N005);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void love3_LookAtSpock(void)
{
    love3_Say(0, T_LOV3N008);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void love3_LookAtRedshirt(void)
{
    love3_Say(0, T_LOV3N004);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void love3_LookAtScene(void)
{
    love3_Say(0, T_LOV3NA23);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void love3_LookAtVent(void)
{
    if (rv_vent_open == 1) {
        love3_Say(0, T_LOV3NA21);
        goto L04E6;
    }
L04E6:;
    return;
}

/* [MSG_LOOK 0B 00 00] */
static void love3_LookAtVentCover(void)
{
    love3_Say(0, T_LOV3N019);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void love3_E_0545(void)
{
    { love3_LookAtAccessPanel(); return; }
}

/* [MSG_LOOK 20 00 00] */
static void love3_LookAtAccessPanel(void)
{
    if (rv_access_panel_open == 1) {
        love3_Say(0, T_LOV3N017);
        return;
    }
    love3_Say(S_2711, T_LOV3NA22);
    return;
}

/* [MSG_LOOK 0C 00 00] */
static void love3_LookAtTank(void)
{
    love3_Say(0, T_LOV3N006);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void love3_LookAtReactor(void)
{
    love3_Say(S_2711, T_LOV3N026);
    love3_Say(N_Dr_McCoy, T_LOV3_009);
    love3_Say(N_Mr_Spock, T_LOV3_019);
    if (room.redshirt_dead != 1) {
        love3_Say(N_Lt_Ferris, T_LOV3_026);
    }
    return;
}

/* [MSG_TALK 00 00 00] */
static void love3_TalkToKirk(void)
{
    love3_Say(N_Capt_Kirk, T_LOV3_001);
    return;
}

/* [MSG_TALK 01 00 00] */
static void love3_TalkToSpock(void)
{
    uint16_t ax = 0;
    if (rv_been_below != 1) {
        if (rv_romulans_out == 1) {
            love3_Say(N_Mr_Spock, T_LOV3_017);
            return;
        }
        ax = (uint16_t)svc_HaveItem(IT_RLG);
        if (ax == 1) {
            love3_Say(N_Mr_Spock, T_LOV3_004);
            return;
        }
    }
    love3_Say(N_Mr_Spock, T_LOV3_003);
    return;
}

/* [MSG_TALK 02 00 00] */
static void love3_TalkToMcCoy(void)
{
    love3_Say(N_Dr_McCoy, T_LOV3_008);
    return;
}

/* [MSG_TALK 03 00 00] */
static void love3_TalkToRedshirt(void)
{
    love3_Say(N_Lt_Ferris, T_LOV3_024);
    return;
}

/* Tricorders.
 * [MSG_USE 45 FF 00] */
static void love3_McCoyScans(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_have_research != 1) {
        love3_Say(N_Dr_McCoy, T_LOV3_006);
        return;
    }
    love3_Say(N_Dr_McCoy, T_LOV3_005);
    return;
}

/* [MSG_USE 44 23 00] */
static void love3_SpockScansReactor(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    love3_Say(N_Mr_Spock, T_LOV3_014);
    return;
}

/* [MSG_USE 44 09 00] */
static void love3_SpockScansInsulation(void)
{
    if (rv_access_panel_open == 1) {
        svc_Anim(1, N_sscanw, -1, -1, 0);
        svc_BridgeSfx(4);
        love3_Say(N_Mr_Spock, T_LOV3_002);
    }
    return;
}

/* [MSG_USE 44 FF 00] */
static void love3_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    love3_Say(N_Mr_Spock, T_LOV3_013);
    return;
}

/* The service panel, by anyone (msg 1): open or shut.
 * [MSG_USE 00 08 00] */
static void love3_KirkOpensServicePanel(void)
{
    fv_crew = 0;
    fv_crew_anim = N_kusehe;
    { love3_part_0987(); return; }
}

/* [MSG_USE 01 08 00] */
static void love3_SpockOpensServicePanel(void)
{
    fv_crew = 1;
    fv_crew_anim = N_susehe;
    { love3_part_0987(); return; }
}

/* [MSG_USE 02 08 00] */
static void love3_McCoyOpensServicePanel(void)
{
    fv_crew = 2;
    fv_crew_anim = N_musehe;
    { love3_part_0987(); return; }
}

/* [MSG_USE 03 08 00] */
static void love3_FerrisOpensServicePanel(void)
{
    fv_crew = 3;
    fv_crew_anim = N_rusehe;
    { love3_part_0987(); return; }
}

static void love3_part_0987(void)
{
    svc_Walk(fv_crew, 0x104, 0xAE, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void love3_AtServicePanel(void)
{
    svc_Anim(fv_crew, fv_crew_anim, -1, -1, 1);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void love3_ServicePanelToggled(void)
{
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    s0 = 0;
    s1 = 0;
    s2 = 1;
    s3 = N_s3r4d1;
    if (rv_service_panel_open == 1) {
        s3 = N_s3r4d1b;
    }
    svc_Anim(8, s3, s2, s1, s0);
    svc_Sfx(N_SMADOOR1);
    rv_service_panel_open ^= 1;
    if (rv_service_panel_open != 1) {
        love3_Say(0, T_LOV3N020);
        return;
    }
    if (rv_took_n2_tank == 1) {
        love3_Say(0, T_LOV3N023);
        return;
    }
    love3_Say(0, T_LOV3N022);
    return;
}

/* The nitrogen tank: the wrench frees it from its feed lines or fixes it
 * back (msgs 2), the antigrav unit then lifts it; another gas won't do.
 * [MSG_USE 4A 0C 00] */
static void love3_WrenchOnTank(void)
{
    svc_Walk(0, 0x10E, 0xAE, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void love3_KirkAtTank(void)
{
    svc_Anim(0, N_kusehe, -1, -1, 2);
    svc_Sfx(N_MUR3E1S);
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void love3_TankWrenched(void)
{
    if (rv_tank_loose == 1) {
    } else {
        love3_Say(0, T_LOV3N012);
        if (room.redshirt_dead != 1) {
            love3_Say(N_Lt_Ferris, T_LOV3_022);
            goto L0BE4;
        }
L0BE4:;
        rv_tank_loose = 1;
        return;
    }
    love3_Say(0, T_LOV3N015);
    rv_tank_loose = 0;
    return;
}

/* [MSG_USE 4A 63 00] */
static void love3_E_0C41(void)
{
    { love3_WrenchOnOtherTank(); return; }
}

/* [MSG_USE 4A 64 00] */
static void love3_WrenchOnOtherTank(void)
{
    love3_Say(N_Mr_Spock, T_LOV3_016);
    return;
}

/* [MSG_USE 4A 62 00] */
static void love3_WrenchOnN2Tank(void)
{
    svc_Walk(0, 0x10E, 0xAE, 0xF);
    return;
}

/* [MSG_WALK_DONE 0F 00 00] */
static void love3_KirkAtTankSlot(void)
{
    svc_Anim(0, N_kusehe, -1, -1, 3);
    svc_Sfx(N_MUR3E1S_2);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void love3_N2TankBack(void)
{
    love3_Say(0, T_LOV3N013);
    rv_took_n2_tank = 0;
    svc_TakeItem(IT_N2GAS);
    svc_Anim0(0xC, N_r4n2, 0, 0, 0);
    return;
}

/* The access panel (msgs 3, 4): the insulation falls out.
 * [MSG_USE 4A 20 00] */
static void love3_WrenchOnAccessPanel(void)
{
    svc_Walk(0, 0x37, 0xB4, 3);
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void love3_KirkAtAccessPanel(void)
{
    svc_Anim(0, N_kuselw, -1, -1, 4);
    svc_Sfx(N_MUR3E1S_3);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void love3_AccessPanelOpen(void)
{
    if (rv_access_panel_open != 1) {
        svc_Sfx(N_MADR4E4A);
        svc_Anim0(9, N_s3r4p1, 0, 0, 0);
        rv_access_panel_open = 1;
    }
    love3_Say(S_2711, T_LOV3N021);
    return;
}

/* Other things on the readouts.
 * [MSG_USE 4A 21 00] */
static void love3_WrenchOnReadouts(void)
{
    love3_Say(N_Mr_Spock, T_LOV3_018);
    return;
}

/* [MSG_USE 67 21 00] */
static void love3_WaterOnReadouts(void)
{
    svc_Walk(0, 0xC8, 0xB2, 0xE);
    return;
}

/* [MSG_WALK_DONE 0E 00 00] */
static void love3_KirkAtReadouts(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 6);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void love3_ReadoutsWet(void)
{
    love3_Say(0, T_LOV3_021);
    svc_TakeItem(IT_H2O);
    return;
}

/* The crew can't open the panels by hand.
 * [MSG_USE 00 20 00] */
static void love3_E_0F07(void)
{
    { love3_SpockOnAccessPanel(); return; }
}

/* [MSG_USE 01 20 00] */
static void love3_SpockOnAccessPanel(void)
{
    { love3_McCoyOnAccessPanel(); return; }
}

/* [MSG_USE 02 20 00] */
static void love3_McCoyOnAccessPanel(void)
{
    { love3_FerrisOnAccessPanel(); return; }
}

/* [MSG_USE 03 20 00] */
static void love3_FerrisOnAccessPanel(void)
{
    { love3_KirkOnVentCover(); return; }
}

/* [MSG_USE 00 0B 00] */
static void love3_KirkOnVentCover(void)
{
    { love3_SpockOnVentCover(); return; }
}

/* [MSG_USE 01 0B 00] */
static void love3_SpockOnVentCover(void)
{
    { love3_McCoyOnVentCover(); return; }
}

/* [MSG_USE 02 0B 00] */
static void love3_McCoyOnVentCover(void)
{
    { love3_FerrisOnVentCover(); return; }
}

/* [MSG_USE 03 0B 00] */
static void love3_FerrisOnVentCover(void)
{
    love3_Say(0, T_LOV3N030);
    return;
}

/* The vent cover: the wrench opens or closes it (msgs 4, 7), +1 once.
 * [MSG_USE 4A 0B 00] */
static void love3_WrenchOnVentCover(void)
{
    svc_Walk(0, 0xF6, 0xAA, 4);
    room.busy = 1;
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void love3_KirkAtVent(void)
{
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    s0 = 7;
    s1 = 0xFFFF;
    s2 = 0xFFFF;
    s3 = N_s3r4g2;
    if (rv_vent_open == 1) {
        s3 = N_s3r4g2a;
    }
    svc_Anim(0, s3, s2, s1, s0);
    svc_StopAnim(0xB);
    svc_Sfx(N_MUR3E1S_4);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void love3_VentToggled(void)
{
    room.busy = 0;
    svc_StopAnim(0);
    if (rv_vent_open != 1) {
        rv_vent_open = 1;
        svc_Anim0(0xB, N_s3r4g3, 0, 0, 0);
        love3_Say(S_2711, T_LOV3N016);
        if (rv_opened_vent != 1) {
            rv_opened_vent = 1;
            rv_score += 1;
        }
        return;
    }
    svc_Anim0(0xB, N_s3r4g1, 0x116, 0x8F, 0);
    rv_vent_open = 0;
    love3_Say(0, T_LOV3N032);
    return;
}

/* Gases down the vent: water and laughing gas do nothing, ammonia
 * angers the Romulans, Romulan laughing gas puts them out (+6, once)
 * unless the virus already has.
 * [MSG_USE 00 22 00] */
static void love3_E_1092(void)
{
    { love3_SpockInVent(); return; }
}

/* [MSG_USE 01 22 00] */
static void love3_SpockInVent(void)
{
    { love3_McCoyInVent(); return; }
}

/* [MSG_USE 02 22 00] */
static void love3_McCoyInVent(void)
{
    { love3_FerrisInVent(); return; }
}

/* [MSG_USE 03 22 00] */
static void love3_FerrisInVent(void)
{
    love3_Say(0, T_LOV3N011);
    return;
}

/* [MSG_USE 67 22 00] */
static void love3_WaterInVent(void)
{
    svc_Walk(0, 0x100, 0xA6, 5);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void love3_KirkAtVent2(void)
{
    if (rv_vent_open != 1) {
        love3_Say(S_2711, T_LOV3N025);
        return;
    }
    svc_Anim(0, N_kuseme, -1, -1, 8);
    return;
}

/* [MSG_ANIM_DONE 08 00 00] */
static void love3_VentWet(void)
{
    love3_Say(0, T_LOV3N009);
    svc_TakeItem(IT_H2O);
    return;
}

/* [MSG_USE 65 22 00] */
static void love3_N2OInVent(void)
{
    svc_Walk(0, 0x100, 0xA6, 6);
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void love3_KirkAtVent3(void)
{
    if (rv_vent_open != 1) {
        love3_Say(S_2711, T_LOV3N025);
        return;
    }
    svc_Anim(0, N_kuseme, -1, -1, 9);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void love3_N2OVented(void)
{
    love3_Say(0, T_LOV3N018);
    love3_Say(N_Mr_Spock, T_LOV3_012);
    svc_TakeItem(IT_N2O);
    return;
}

/* [MSG_USE 66 22 00] */
static void love3_NH3InVent(void)
{
    svc_Walk(0, 0x100, 0xA6, 7);
    return;
}

/* [MSG_WALK_DONE 07 00 00] */
static void love3_KirkAtVent4(void)
{
    if (rv_vent_open != 1) {
        love3_Say(S_2711, T_LOV3N025);
        return;
    }
    svc_Anim(0, N_kuseme, -1, -1, 0xA);
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void love3_NH3Vented(void)
{
    if (rv_virus_struck == 1) {
    } else {
        love3_Say(0, T_LOV3N001);
        if (room.redshirt_dead != 1) {
            love3_Say(N_Lt_Ferris, T_LOV3_023);
            goto L13A5;
        }
    }
L13A5:;
    svc_TakeItem(IT_NH3);
    return;
}

/* [MSG_USE 49 22 00] */
static void love3_RLGInVent(void)
{
    if (rv_gassed != 1) {
        rv_gassed = 1;
        rv_score += 6;
    }
    if (rv_romulans_out == 1) {
        love3_Say(N_Mr_Spock, T_LOV3_011);
        return;
    }
    svc_Walk(0, 0xF6, 0xA4, 8);
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void love3_KirkAtVent5(void)
{
    if (rv_vent_open != 1) {
        love3_Say(S_2711, T_LOV3N025);
        return;
    }
    svc_Anim(0, N_kuseme, -1, -1, 0xB);
    svc_Sfx(N_EFX24);
    return;
}

/* [MSG_ANIM_DONE 0B 00 00] */
static void love3_RLGVented(void)
{
    svc_TakeItem(IT_RLG);
    rv_romulans_out = 1;
    if (rv_virus_struck != 1) {
        love3_Say(S_2711, T_LOV3NA09);
        svc_Sfx(N_ROMULANL);
        return;
    }
    love3_Say(S_2711, T_LOV3NA08);
    rv_romulans_out = 0;
    return;
}

/* Getting things.
 * [MSG_GET 0A 00 00] */
static void love3_GetWrench(void)
{
    svc_Walk(0, 0x104, 0xB6, 9);
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void love3_KirkAtWrench(void)
{
    svc_Anim(0, N_s5r1kg, -1, -1, 0xC);
    return;
}

/* [MSG_ANIM_DONE 0C 00 00] */
static void love3_TookWrench(void)
{
    svc_StopAnim(0);
    svc_GiveItem(IT_WRENCH);
    svc_StopAnim(0xA);
    rv_took_wrench = 1;
    love3_Say(0, T_LOV3N031);
    return;
}

/* [MSG_GET 0C 00 00] */
static void love3_GetTank(void)
{
    svc_Walk(0, 0x121, 0xB7, 0xA);
    return;
}

/* [MSG_WALK_DONE 0A 00 00] */
static void love3_KirkAtTank2(void)
{
    if (rv_tank_loose == 1) {
        love3_Say(0, T_LOV3N007);
        return;
    }
    love3_Say(0, T_LOV3N014);
    return;
}

/* [MSG_USE 61 0C 00] */
static void love3_AntigravOnTank(void)
{
    svc_Walk(0, 0x10E, 0xAE, 0xB);
    return;
}

/* [MSG_WALK_DONE 0B 00 00] */
static void love3_KirkAtTank3(void)
{
    if (rv_tank_loose != 1) {
    } else {
        svc_Anim(0, N_kusehe, -1, -1, 0xD);
        svc_Sfx(N_SE3PLBAT);
        return;
    }
    love3_Say(0, T_LOV3N010);
    love3_Say(N_Mr_Spock, T_LOV3_015);
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void love3_TankLifted(void)
{
    svc_GiveItem(IT_N2GAS);
    svc_StopAnim(0xC);
    rv_took_n2_tank = 1;
    love3_Say(0, T_LOV3NJ32);
    if (room.redshirt_dead != 1) {
        love3_Say(N_Lt_Ferris, T_LOV3_025);
        goto L172F;
    }
L172F:;
    return;
}

/* [MSG_GET 09 00 00] */
static void love3_GetInsulation(void)
{
    svc_Walk(0, 0x32, 0xBE, 0xC);
    return;
}

/* [MSG_WALK_DONE 0C 00 00] */
static void love3_KirkAtInsulation(void)
{
    if (rv_never_set == 1)
        { love3_part_187E(); return; }
    svc_Anim(0, N_s5r1kg, -1, -1, 0xE);
    return;
}

/* [MSG_ANIM_DONE 0E 00 00] */
static void love3_TookInsulation(void)
{
    svc_StopAnim(0);
    svc_GiveItem(IT_INSULAT);
    love3_Say(0, T_LOV3N029);
    { love3_part_187E(); return; }
}

static void love3_part_187E(void)
{
    return;
}

/* The gases, common to all of the mission's rooms. Timer 0 (after
 * laughing gas): a random crew member giggles, and again in 200-400 ticks.
 * [MSG_TIMER 00 00 00] */
static void love3_CrewLaugh(void)
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
                goto L19A1;
            }
        } else {
            love3_Say(0, N_get_name_error);
            return;
        }
    }
    fv_laugher = N_Dr_McCoy;
    ax = (uint16_t)svc_Random(0, 0xA);
    fv_roll = ax;
    if (fv_roll == 0) {
        bx = T_LOV0_011;
        goto L1A48;
    }
    if (fv_roll == 1) {
        bx = T_LOV0_020;
        goto L1A48;
    }
    if (fv_roll == 2) {
        bx = T_LOV0_016;
        goto L1A48;
    }
    if (fv_roll == 3) {
        bx = T_LOV0_012;
        goto L1A48;
    }
    if (fv_roll == 4) {
        bx = T_LOV0_014;
        goto L1A48;
    }
    if (fv_roll == 5) {
        bx = T_LOV0_022;
        goto L1A48;
    }
    if (fv_roll == 6) {
        bx = T_LOV0_015;
        goto L1A48;
    }
    if (fv_roll == 7) {
        bx = T_LOV0_021;
        goto L1A48;
    }
    if (fv_roll == 8) {
        bx = T_LOV0_013;
        goto L1A48;
    }
    if (fv_roll == 9) {
        bx = T_LOV0_019;
        goto L1A48;
    }
    bx = T_LOV0_017;
    goto L1A48;
L19A1:;
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
        goto L1A48;
    }
L1A48:;
    ax = fv_laugher;
    love3_Say(ax, bx);
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
        goto L1A97;
    }
L1A97:;
    love3_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[0] = ax;
    return;
}

/* Timer 1 (after Romulan laughing gas): Spock laughs.
 * [MSG_TIMER 01 00 00] */
static void love3_SpockLaughs(void)
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
        goto L1B28;
    }
L1B28:;
    love3_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[1] = ax;
    return;
}

/* Curing Spock, common to all of the mission's rooms: with the cure,
 * Spock and McCoy walk to the cure spot (msg 99), and McCoy injects him.
 * [MSG_USE 7F 01 00] */
static void love3_MedkitOnSpock(void)
{
    if (rv_spock_cured != 1) {
        love3_Say(N_Dr_McCoy, T_LOV5_019);
        return;
    }
    love3_Say(N_Dr_McCoy, T_LOV5_015);
    return;
}

/* [MSG_USE 4C 01 00] */
static void love3_SampleOnSpock(uint16_t ax)
{
    if (rv_spock_cured == 1) {
        { love3_part_1C80(ax); return; }
    }
    love3_Say(N_Dr_McCoy, T_LOVA_F55);
    return;
}

/* [MSG_USE 4D 01 00] */
static void love3_CureSpock(uint16_t ax)
{
    if (rv_spock_cured != 1)
        { love3_part_1C80(ax); return; }
    love3_Say(N_Dr_McCoy, T_LOV5_015_2);
    return;
}

static void love3_part_1C80(uint16_t ax)
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
static void love3_AtSpock(void)
{
    fv_cure_steps++;
    if (fv_cure_steps != 2) {
        return;
    }
    svc_Anim(2, N_musemn, -1, -1, 0x63);
    return;
}

/* [MSG_ANIM_DONE 63 00 00] */
static void love3_SpockCured(void)
{
    love3_Say(N_Dr_McCoy, T_LOV5_030);
    love3_Say(N_Mr_Spock, T_LOV5_038);
    love3_Say(N_Dr_McCoy, T_LOV5_027);
    love3_Say(N_Mr_Spock, S_1D8C);
    rv_spock_cured = 1;
    return;
}

/* Timer 2, every 200 ticks until Spock is cured: the virus advances,
 * with a line at 0Fh, 1Eh, 2Dh; at 3Ch Spock collapses, at 64h he dies.
 * [MSG_TIMER 02 00 00] */
static void love3_VirusProgresses(void)
{
    if (rv_spock_cured == 1) {
        return;
    }
    rv_virus++;
    if ((int8_t)rv_virus >= (int8_t)0x64) {
    } else {
        if (rv_virus == 0xF) {
            love3_Say(N_Mr_Spock, T_spokcoff);
            rv_virus++;
        } else {
            if (rv_virus == 0x1E) {
                love3_Say(N_Dr_McCoy, T_LOVA_F08);
                rv_virus++;
            } else {
                if (rv_virus == 0x2D) {
                    love3_Say(N_Mr_Spock, T_spokcoff_2);
                    rv_virus++;
                } else {
                    if (rv_virus == 0x3C) {
                        love3_Say(N_Mr_Spock, T_LOVA_F54);
                        rv_virus_struck = 1;
                        rv_virus++;
                        goto L1F05;
                    }
                }
            }
        }
L1F05:;
        room.timers[2] = 0xC8;
        return;
    }
    svc_Anim(1, N_sgetdn, -1, -1, 0);
    svc_Music(2, STALE_ARG);
    love3_Say(0, T_GENER004);
    svc_GameOver();
    return;
}

/* McCoy's tricorder on Spock follows the virus; on the others, normal.
 * [MSG_USE 45 01 00] */
static void love3_McCoyScansSpock(void)
{
    svc_BridgeSfx(4);
    if (rv_spock_cured == 1) {
        love3_Say(N_Dr_McCoy, T_LOVA_F01);
        return;
    }
    if (rv_have_research != 1) {
        love3_Say(N_Dr_McCoy, T_LOVA_F02);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0xA) {
        love3_Say(N_Dr_McCoy, T_LOVA_F04);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x1E) {
        love3_Say(N_Dr_McCoy, T_LOVA_F03);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x32) {
        love3_Say(N_Dr_McCoy, T_LOVA_F08_2);
        return;
    }
    if (rv_virus == 0x46) {
        love3_Say(N_Dr_McCoy, T_TUG2_010);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x64) {
        love3_Say(N_Dr_McCoy, T_LOVA_F10);
        return;
    }
    love3_Say(N_Dr_McCoy, T_LOVA_100);
    return;
}

/* [MSG_USE 45 00 00] */
static void love3_E_21D2(void)
{
    { love3_McCoyScansMcCoy(); return; }
}

/* [MSG_USE 45 02 00] */
static void love3_McCoyScansMcCoy(void)
{
    { love3_McCoyScansCrew(); return; }
}

/* [MSG_USE 45 03 00] */
static void love3_McCoyScansCrew(void)
{
    svc_BridgeSfx(4);
    love3_Say(N_Dr_McCoy, T_LOVA_F07);
    return;
}

/* Opening a gas canister: Romulan laughing gas sets Spock laughing,
 * nitrous oxide the crew, ammonia nothing.
 * [MSG_USE 49 FF 00] */
static void love3_ReleaseRLG(void)
{
    love3_Say(S_2711, T_LOV2N005);
    rv_rlg_released = 1;
    svc_TakeItem(IT_RLG);
    return;
}

/* [MSG_USE 65 FF 00] */
static void love3_ReleaseN2O(void)
{
    love3_Say(S_2711, T_LOV2N005_2);
    love3_Say(N_Mr_Spock, T_MUD2_040);
    rv_n2o_released = 1;
    svc_TakeItem(IT_N2O);
    svc_Music(3, STALE_ARG);
    return;
}

/* [MSG_USE 66 FF 00] */
static void love3_ReleaseNH3(void)
{
    love3_Say(S_2711, T_LOV2N005_3);
    if (room.redshirt_dead != 1) {
    }
    svc_TakeItem(IT_NH3);
    return;
}

/* The communicator: four cases (first call, later calls, the same after
 * the cure), all with the same three lines borrowed from other missions'
 * speech files.
 * [MSG_USE 47 FF 00] */
static void love3_CallEnterprise(void)
{
    if (rv_made_cure == 1) {
    } else {
        if (rv_called_ship == 1) {
        } else {
            love3_Say(N_Capt_Kirk, T_MUD4_018);
            love3_Say(N_Lt_Uhura, T_BRIDU146);
            love3_Say(N_Capt_Kirk, T_VENA_F41);
            rv_called_ship = 1;
            return;
        }
        love3_Say(N_Capt_Kirk, T_MUD4_018_2);
        love3_Say(N_Lt_Uhura, T_BRIDU146_2);
        love3_Say(N_Capt_Kirk, T_VENA_F41_2);
        return;
    }
    if (rv_called_ship_cured == 1) {
    } else {
        love3_Say(N_Capt_Kirk, T_MUD4_018_3);
        love3_Say(N_Lt_Uhura, T_BRIDU146_3);
        love3_Say(N_Capt_Kirk, T_VENA_F41_3);
        rv_called_ship_cured = 1;
        return;
    }
    love3_Say(N_Capt_Kirk, T_MUD4_018_4);
    love3_Say(N_Lt_Uhura, T_BRIDU146_4);
    love3_Say(N_Capt_Kirk, T_VENA_F41_4);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void love3_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void love3_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x38CF) = bx;
    love3_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript love3_scripts[] = {
    { 0x0188, love3_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0283, love3_McCoyJokes },    /* MSG_TICK 50 00 00 */
    { 0x0398, love3_ExitToLab },    /* MSG_EXIT 01 00 00 */
    { 0x03B4, love3_GoToLab },    /* MSG_TIMER 03 00 00 */
    { 0x03CD, love3_LookAtServicePanel },    /* MSG_LOOK 08 00 00 */
    { 0x03DC, love3_LookAtReadouts },    /* MSG_LOOK 21 00 00 */
    { 0x03EB, love3_LookAtWrench },    /* MSG_LOOK 0A 00 00 */
    { 0x03FA, love3_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0409, love3_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0418, love3_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0427, love3_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0436, love3_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0492, love3_LookAtVent },    /* MSG_LOOK 22 00 00 */
    { 0x04ED, love3_LookAtVentCover },    /* MSG_LOOK 0B 00 00 */
    { 0x0545, love3_E_0545 },    /* MSG_LOOK 09 00 00 */
    { 0x054D, love3_LookAtAccessPanel },    /* MSG_LOOK 20 00 00 */
    { 0x05D4, love3_LookAtTank },    /* MSG_LOOK 0C 00 00 */
    { 0x0602, love3_LookAtReactor },    /* MSG_LOOK 23 00 00 */
    { 0x0633, love3_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0689, love3_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0717, love3_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x079D, love3_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x07F1, love3_McCoyScans },    /* MSG_USE 45 FF 00 */
    { 0x089E, love3_SpockScansReactor },    /* MSG_USE 44 23 00 */
    { 0x08CF, love3_SpockScansInsulation },    /* MSG_USE 44 09 00 */
    { 0x0906, love3_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x0937, love3_KirkOpensServicePanel },    /* MSG_USE 00 08 00 */
    { 0x094D, love3_SpockOpensServicePanel },    /* MSG_USE 01 08 00 */
    { 0x0963, love3_McCoyOpensServicePanel },    /* MSG_USE 02 08 00 */
    { 0x0979, love3_FerrisOpensServicePanel },    /* MSG_USE 03 08 00 */
    { 0x09A5, love3_AtServicePanel },    /* MSG_WALK_DONE 01 00 00 */
    { 0x09C6, love3_ServicePanelToggled },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0AEB, love3_WrenchOnTank },    /* MSG_USE 4A 0C 00 */
    { 0x0B06, love3_KirkAtTank },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0B3B, love3_TankWrenched },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x0C41, love3_E_0C41 },    /* MSG_USE 4A 63 00 */
    { 0x0C49, love3_WrenchOnOtherTank },    /* MSG_USE 4A 64 00 */
    { 0x0CD0, love3_WrenchOnN2Tank },    /* MSG_USE 4A 62 00 */
    { 0x0CEB, love3_KirkAtTankSlot },    /* MSG_WALK_DONE 0F 00 00 */
    { 0x0D20, love3_N2TankBack },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x0D98, love3_WrenchOnAccessPanel },    /* MSG_USE 4A 20 00 */
    { 0x0DB2, love3_KirkAtAccessPanel },    /* MSG_WALK_DONE 03 00 00 */
    { 0x0DE7, love3_AccessPanelOpen },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0E30, love3_WrenchOnReadouts },    /* MSG_USE 4A 21 00 */
    { 0x0E40, love3_WaterOnReadouts },    /* MSG_USE 67 21 00 */
    { 0x0E5B, love3_KirkAtReadouts },    /* MSG_WALK_DONE 0E 00 00 */
    { 0x0E77, love3_ReadoutsWet },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x0F07, love3_E_0F07 },    /* MSG_USE 00 20 00 */
    { 0x0F0F, love3_SpockOnAccessPanel },    /* MSG_USE 01 20 00 */
    { 0x0F17, love3_McCoyOnAccessPanel },    /* MSG_USE 02 20 00 */
    { 0x0F1F, love3_FerrisOnAccessPanel },    /* MSG_USE 03 20 00 */
    { 0x0F27, love3_KirkOnVentCover },    /* MSG_USE 00 0B 00 */
    { 0x0F2F, love3_SpockOnVentCover },    /* MSG_USE 01 0B 00 */
    { 0x0F37, love3_McCoyOnVentCover },    /* MSG_USE 02 0B 00 */
    { 0x0F3F, love3_FerrisOnVentCover },    /* MSG_USE 03 0B 00 */
    { 0x0F85, love3_WrenchOnVentCover },    /* MSG_USE 4A 0B 00 */
    { 0x0FA4, love3_KirkAtVent },    /* MSG_WALK_DONE 04 00 00 */
    { 0x0FEF, love3_VentToggled },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x1092, love3_E_1092 },    /* MSG_USE 00 22 00 */
    { 0x109A, love3_SpockInVent },    /* MSG_USE 01 22 00 */
    { 0x10A2, love3_McCoyInVent },    /* MSG_USE 02 22 00 */
    { 0x10AA, love3_FerrisInVent },    /* MSG_USE 03 22 00 */
    { 0x10F4, love3_WaterInVent },    /* MSG_USE 67 22 00 */
    { 0x110F, love3_KirkAtVent2 },    /* MSG_WALK_DONE 05 00 00 */
    { 0x113B, love3_VentWet },    /* MSG_ANIM_DONE 08 00 00 */
    { 0x1182, love3_N2OInVent },    /* MSG_USE 65 22 00 */
    { 0x119D, love3_KirkAtVent3 },    /* MSG_WALK_DONE 06 00 00 */
    { 0x11C9, love3_N2OVented },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x1299, love3_NH3InVent },    /* MSG_USE 66 22 00 */
    { 0x12B4, love3_KirkAtVent4 },    /* MSG_WALK_DONE 07 00 00 */
    { 0x12E0, love3_NH3Vented },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x13B8, love3_RLGInVent },    /* MSG_USE 49 22 00 */
    { 0x1439, love3_KirkAtVent5 },    /* MSG_WALK_DONE 08 00 00 */
    { 0x147C, love3_RLGVented },    /* MSG_ANIM_DONE 0B 00 00 */
    { 0x14CA, love3_GetWrench },    /* MSG_GET 0A 00 00 */
    { 0x14E5, love3_KirkAtWrench },    /* MSG_WALK_DONE 09 00 00 */
    { 0x1501, love3_TookWrench },    /* MSG_ANIM_DONE 0C 00 00 */
    { 0x1538, love3_GetTank },    /* MSG_GET 0C 00 00 */
    { 0x1553, love3_KirkAtTank2 },    /* MSG_WALK_DONE 0A 00 00 */
    { 0x1606, love3_AntigravOnTank },    /* MSG_USE 61 0C 00 */
    { 0x1621, love3_KirkAtTank3 },    /* MSG_WALK_DONE 0B 00 00 */
    { 0x1660, love3_TankLifted },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x1822, love3_GetInsulation },    /* MSG_GET 09 00 00 */
    { 0x183C, love3_KirkAtInsulation },    /* MSG_WALK_DONE 0C 00 00 */
    { 0x185E, love3_TookInsulation },    /* MSG_ANIM_DONE 0E 00 00 */
    { 0x1896, love3_CrewLaugh },    /* MSG_TIMER 00 00 00 */
    { 0x1AB3, love3_SpockLaughs },    /* MSG_TIMER 01 00 00 */
    { 0x1B45, love3_MedkitOnSpock },    /* MSG_USE 7F 01 00 */
    /* 1BD3: a helper, not a script */
    /* 1C46: a helper, not a script */
    { 0x1CC2, love3_AtSpock },    /* MSG_WALK_DONE 63 00 00 */
    { 0x1CEC, love3_SpockCured },    /* MSG_ANIM_DONE 63 00 00 */
    { 0x1DAB, love3_VirusProgresses },    /* MSG_TIMER 02 00 00 */
    { 0x1F61, love3_McCoyScansSpock },    /* MSG_USE 45 01 00 */
    { 0x21D2, love3_E_21D2 },    /* MSG_USE 45 00 00 */
    { 0x21DA, love3_McCoyScansMcCoy },    /* MSG_USE 45 02 00 */
    { 0x21E2, love3_McCoyScansCrew },    /* MSG_USE 45 03 00 */
    { 0x2229, love3_ReleaseRLG },    /* MSG_USE 49 FF 00 */
    { 0x2279, love3_ReleaseN2O },    /* MSG_USE 65 FF 00 */
    { 0x230B, love3_ReleaseNH3 },    /* MSG_USE 66 FF 00 */
    { 0x2371, love3_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0, 0 }
};
