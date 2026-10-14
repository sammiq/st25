/* LOVE2.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/love2.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * LOVE2.RDF - "Love's Labor Jeopardized", the science lab, with the
 * synthesizer. Two gas tanks feed it (oxygen and hydrogen at first; the
 * nitrogen tank is elsewhere), through a valve opened with the wrench.
 * It combines the two gases (laughing gas, ammonia, water), turns
 * polyberylcarbonate into TLTDH, the Romulan laughing gas, and the cure
 * sample from room 1 into the cure. The cabinet holds an antigrav unit
 * for moving the tanks.
 *
 * Actors: 8 the cabinet, 9 the chamber door, 10 (0Ah) the door, 11 (0Bh)
 * the valve, 12/13 (0Ch/0Dh) the tanks, 14 (0Eh) the antigrav unit,
 * 15-19 (0Fh-13h) what comes out of the chamber.
 * Objects: 8 the cabinet, 9 the chamber, 0Ah/22h the door (room 1), 0Bh
 * the valve, 0Ch/0Dh the tanks, 0Eh the antigrav unit, 0Fh-13h the
 * chamber's contents (gas, PBC, culture, sample, cure), 20h the
 * synthesizer, 21h the Ardak 1000, 23h the feed lines.
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
    N_cure = 0x00DE,
    N_pbcanm = 0x00E3,
    N_dishes = 0x00EA,
    N_dishes_2 = 0x00F1,
    N_btle4 = 0x00F8,
    N_btle1 = 0x00FE,
    N_btle3 = 0x0104,
    N_btle2 = 0x010A,
    N_antigr = 0x0110,
    N_o2can = 0x0117,
    N_n2can = 0x011D,
    N_h2can = 0x0123,
    N_s3r3d1 = 0x0129,
    N_s3r3d1o = 0x0130,
    N_s3r3d1c = 0x0138,
    N_s3r3d4 = 0x0140,
    N_s3r3d2 = 0x0147,
    N_s3r3d2o = 0x014E,
    N_s3r3d2c = 0x0156,
    N_s3r3d5 = 0x015E,
    N_s3r3d3 = 0x0165,
    N_s3r3d3a = 0x016C,
    N_s3r3v1 = 0x0174,
    N_s3r3v2 = 0x017B,
    N_s3r3vc = 0x0182,
    N_s3r3vo = 0x0189,
    N_kusehn = 0x0190,
    N_susehn = 0x0197,
    N_kusemn = 0x01AC,
    N_susemn = 0x01B3,
    N_rusemn = 0x01BA,
    N_kusehw = 0x01CF,
    N_susehw = 0x01D6,
    N_musehw = 0x01DD,
    N_rusehw = 0x01E4,
    N_sscans = 0x01EB,
    N_mscans = 0x01F9,
    N_LOV2LOOP = 0x0208,
    T_LOV2N009 = 0x05BB,
    T_LOV2_001 = 0x06C6,
    T_LOV2_012 = 0x077C,
    N_MUR3E1S = 0x0965,
    T_LOV2_026 = 0x0A4F,
    N_SE6FOOD = 0x0AD9,
    T_LOV2N034 = 0x0B1B,
    T_LOV2N024 = 0x0B8B,
    T_LOV2N035 = 0x0C09,
    T_LOV2_025 = 0x0C9A,
    T_LOV2N030 = 0x0D1C,
    T_LOV2_025_2 = 0x0DD4,
    N_SE3PLBAT = 0x0E46,
    T_LOV2N031 = 0x0E70,
    T_LOV2_006 = 0x0FB9,
    T_LOV2_005 = 0x100E,
    T_LOV2N012 = 0x10EE,
    N_EFX24 = 0x111A,
    T_LOV2N005 = 0x113A,
    T_LOV2_024 = 0x1174,
    N_EFX24_2 = 0x120D,
    T_LOV2N005_2 = 0x122D,
    T_LOV2_010 = 0x1266,
    N_LD1SCAN = 0x13CC,
    T_LOV2N017 = 0x141B,
    T_LOV2_032 = 0x1468,
    T_LOV2_015 = 0x1501,
    T_LOV2_036 = 0x1547,
    T_LOV2N016 = 0x15B3,
    T_LOV2_043 = 0x1602,
    T_LOV2_003 = 0x165A,
    T_LOV2N018 = 0x16B4,
    T_LOV2_004 = 0x16FD,
    T_LOV2N019 = 0x177D,
    T_LOV2_031 = 0x17C8,
    T_LOV2_018 = 0x1868,
    T_LOV2_030 = 0x18A8,
    T_LOV2_022 = 0x1902,
    T_LOV2_038 = 0x1948,
    T_LOV2N051 = 0x19D4,
    T_LOV2_002 = 0x1A22,
    T_LOV2N050 = 0x1A7A,
    T_LOV2_021 = 0x1AC8,
    T_LOV2_028 = 0x1B18,
    T_LOV2N049 = 0x1B79,
    S_1BC7 = 0x1BC7,
    T_LOV2N048 = 0x1C29,
    S_1C78 = 0x1C78,
    T_LOV2_023 = 0x1CCB,
    T_LOV2N047 = 0x1DC0,
    T_LOV2_017 = 0x1E13,
    T_LOV2N046 = 0x1F45,
    T_LOV2_020 = 0x1F93,
    T_LOV2N025 = 0x2027,
    T_LOV2N025_2 = 0x2106,
    S_21E5 = 0x21E5,
    T_LOV2_014 = 0x2290,
    T_LOV2_007 = 0x22E5,
    T_LOV2_040 = 0x2362,
    T_LOV2_011 = 0x23A9,
    T_LOV2N042 = 0x2452,
    T_LOV2N044 = 0x24D6,
    T_LOV2N036 = 0x2592,
    T_LOV2N037 = 0x264B,
    T_LOV2N040 = 0x2702,
    T_LOV2N039 = 0x2758,
    T_LOV2N041 = 0x27A8,
    T_LOV2N038 = 0x27F4,
    T_LOV2N043 = 0x28F4,
    N_musemn = 0x2957,
    N_sgetdn = 0x295E,
    N_get_name_error = 0x29BD,
    T_LOV5_019 = 0x2C2E,
    T_LOV5_015 = 0x2C7C,
    T_LOVA_F55 = 0x2CBF,
    T_LOV5_015_2 = 0x2D2F,
    T_LOV5_030 = 0x2DCF,
    T_LOV5_038 = 0x2E02,
    T_LOV5_027 = 0x2E2F,
    S_2E64 = 0x2E64,
    T_spokcoff = 0x2EA7,
    T_LOVA_F08 = 0x2EDE,
    T_spokcoff_2 = 0x2F44,
    T_LOVA_F54 = 0x2F7A,
    T_GENER004 = 0x300F,
    T_LOVA_F01 = 0x3056,
    T_LOVA_F02 = 0x309E,
    T_LOVA_F04 = 0x3110,
    T_LOVA_F03 = 0x3165,
    T_LOVA_F08_2 = 0x31A9,
    T_TUG2_010 = 0x320A,
    T_LOVA_F10 = 0x3250,
    T_LOVA_100 = 0x3284,
    T_LOVA_F07 = 0x32D1,
    T_LOV2N005_3 = 0x330C,
    T_LOV2N005_4 = 0x335C,
    T_MUD2_040 = 0x3395,
    T_LOV2N005_5 = 0x33EE,
    N_Lt_Uhura = 0x3439,
    T_MUD4_018 = 0x3466,
    T_BRIDU146 = 0x34AB,
    T_VENA_F41 = 0x34E1,
    T_MUD4_018_2 = 0x350A,
    T_BRIDU146_2 = 0x354F,
    T_VENA_F41_2 = 0x3585,
    T_MUD4_018_3 = 0x35B3,
    T_BRIDU146_3 = 0x35F8,
    T_VENA_F41_3 = 0x362E,
    T_MUD4_018_4 = 0x3657,
    T_BRIDU146_4 = 0x369C,
    T_VENA_F41_4 = 0x36D2,
    N_Capt_Kirk = 0x37A4,
    N_Dr_McCoy = 0x37AF,
    N_Mr_Spock = 0x37B9,
    N_Lt_Ferris = 0x37C3,
    S_37CE = 0x37CE,
    T_LOV2N006 = 0x37DC,
    T_LOV2N028 = 0x3859,
    T_LOV2N004 = 0x38E9,
    T_LOV2N002 = 0x3910,
    T_LOV2N003 = 0x3937,
    T_LOV2N011 = 0x395E,
    T_LOV2N014 = 0x3988,
    T_LOV2N010 = 0x39BC,
    T_LOV2N026 = 0x39F8,
    T_LOV2N027 = 0x3A1E,
    T_LOV2N029 = 0x3A48,
    T_LOV2_035 = 0x3A82,
    T_LOV2_019 = 0x3B9B,
    T_LOV2_033 = 0x3BD9,
    T_LOV2_016 = 0x3C1A,
    T_LOV2N045 = 0x3C39,
    T_LOV2_037 = 0x3CC9,
    T_LOV2N001 = 0x3D7D,
    T_LOV2_009 = 0x3D94,
    T_LOV2_029 = 0x3DD8,
    T_LOV2_041 = 0x3E11,
    T_LOV2_008 = 0x3E7E,
    T_LOV2_027 = 0x3ED1,
    T_LOV2N008 = 0x3F3A,
    T_LOV2N007 = 0x3F59,
    T_LOV0_011 = 0x4057,
    T_LOV0_020 = 0x4079,
    T_LOV0_016 = 0x40B5,
    T_LOV0_012 = 0x40D6,
    T_LOV0_014 = 0x4112,
    T_LOV0_022 = 0x4142,
    T_LOV0_015 = 0x417A,
    T_LOV0_021 = 0x41B0,
    T_LOV0_013 = 0x41FE,
    T_LOV0_019 = 0x4234,
    T_LOV0_017 = 0x4284,
    T_LOV0_039 = 0x42C1,
    T_LOV0_048 = 0x42E3,
    T_LOV0_045 = 0x431F,
    T_LOV0_040 = 0x4340,
    T_LOV0_042 = 0x437C,
    T_LOV0_050 = 0x43AC,
    T_LOV0_043 = 0x43E4,
    T_LOV0_049 = 0x441A,
    T_LOV0_041 = 0x4468,
    T_LOV0_047 = 0x449E,
    T_LOV0_046 = 0x44EE,
    T_LOV0_029 = 0x452B,
    T_LOV0_033 = 0x4567,
    T_LOV0_026 = 0x45A3,
    T_LOV0_124 = 0x45F0,
    T_LOV0_025 = 0x4622,
    T_LOV0_101 = 0x4673,
    T_LOV0_102 = 0x46B3,
    T_LOV0_103 = 0x46F9,
    T_LOV0_104 = 0x472B,
    T_LOV0_105 = 0x4764,
    T_LOV0_106 = 0x47D4,
    T_LOV0_107 = 0x4818,
    talk_buf = 0x484D,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_chamber_anim CS_W(0xD6)
#define fv_crew CS_W(0xD2)
#define fv_crew_anim CS_W(0xD4)
#define fv_cure_steps CS_B(0x2965)
#define fv_cure_x CS_B(0x2966)
#define fv_cure_y CS_B(0x2967)
#define fv_door_steps CS_B(0xCD)
#define fv_dry_runs CS_B(0xCB)
#define fv_gas_anim CS_W(0xD8)
#define fv_going_door CS_B(0xCC)
#define fv_laugher CS_W(0xDA)
#define fv_roll CS_W(0xDC)
#define fv_talk_buf CS_W(0x484D)
#define fv_tank_anim CS_W(0xD0)
#define fv_tank_gas CS_B(0xCA)
#define fv_tank_item CS_W(0xCE)
#define rv_cabinet_open ROOM_B(0x32)
#define rv_called_ship ROOM_B(0x47)
#define rv_called_ship_cured ROOM_B(0x48)
#define rv_chamber ROOM_B(0x35)
#define rv_feed_on ROOM_B(0x33)
#define rv_gas_made ROOM_B(0x34)
#define rv_have_research ROOM_B(0x2A)
#define rv_made_cure ROOM_B(0x4F)
#define rv_n2o_released ROOM_B(0x2C)
#define rv_rlg_released ROOM_B(0x2D)
#define rv_score ROOM_W(0x52)
#define rv_spock_cured ROOM_B(0x46)
#define rv_tank1 ROOM_B(0x36)
#define rv_tank2 ROOM_B(0x37)
#define rv_took_gas ROOM_B(0x4C)
#define rv_virus ROOM_B(0x45)
#define rv_virus_struck ROOM_B(0x41)

/* functions */
static void love2_StartRoom(void);
static void love2_ExitToBridge(void);
static void love2_GoToBridge(void);
static void love2_E_04AE(void);
static void love2_WalkToDoor(void);
static void love2_ReachDoor(void);
static void love2_E_050D(void);
static void love2_ThroughDoor(void);
static void love2_LookAtCabinet(void);
static void love2_LookAtDoor(void);
static void love2_LookAtSynthesizer(void);
static void love2_LookAtScene(void);
static void love2_LookAtAntigrav(void);
static void love2_LookAtMcCoy(void);
static void love2_LookAtSpock(void);
static void love2_LookAtRedshirt(void);
static void love2_LookAtKirk(void);
static void love2_LookAtArdak(void);
static void love2_E_05FF(void);
static void love2_LookAtChamber2(void);
static void love2_LookAtChamber3(void);
static void love2_LookAtChamber4(void);
static void love2_LookAtChamber5(void);
static void love2_LookAtChamber(void);
static void love2_LookAtTank1(void);
static void love2_LookAtTank2(void);
static void love2_TalkToKirk(void);
static void love2_TalkToSpock(void);
static void love2_TalkToMcCoy(void);
static void love2_TalkToRedshirt(void);
static void love2_McCoyScans(void);
static void love2_SpockScans(void);
static void love2_KirkOpensCabinet(void);
static void love2_SpockOpensCabinet(void);
static void love2_McCoyOpensCabinet(void);
static void love2_FerrisOpensCabinet(void);
static void love2_part_0878(void);
static void love2_AtCabinet(void);
static void love2_CabinetToggled(void);
static void love2_CabinetOpened(void);
static void love2_WrenchOnValve(void);
static void love2_KirkAtValve(void);
static void love2_ValveTurned(void);
static void love2_O2InSlot(void);
static void love2_H2InSlot(void);
static void love2_N2InSlot(void);
static void love2_part_0A23(void);
static void love2_KirkAtSlots(void);
static void love2_Tank1Fitted(void);
static void love2_Tank2Fitted(void);
static void love2_AntigravOnTank1(void);
static void love2_KirkAtTank1(void);
static void love2_Tank1Freed(void);
static void love2_AntigravOnTank2(void);
static void love2_KirkAtTank2(void);
static void love2_Tank2Freed(void);
static void love2_KirkUsesSynthesizer(void);
static void love2_SpockUsesSynthesizer(void);
static void love2_McCoyUsesSynthesizer(void);
static void love2_FerrisUsesSynthesizer(void);
static void love2_part_0F4C(void);
static void love2_AtSynthesizer(void);
static void love2_Synthesize(void);
static void love2_SpockAtValve(void);
static void love2_SpockTurnsValve(void);
static void love2_part_110F(void);
static void love2_part_13A0(void);
static void love2_ChamberOpensGas(void);
static void love2_GasMade(void);
static void love2_ChamberResult(void);
static void love2_ChamberOpensCure(void);
static void love2_CureAppears(void);
static void love2_CureMade(void);
static void love2_PBCInChamber(void);
static void love2_KirkAtChamber(void);
static void love2_ChamberOpen(void);
static void love2_PBCIn(void);
static void love2_DishesInChamber(void);
static void love2_KirkAtChamber2(void);
static void love2_ChamberOpen2(void);
static void love2_DishesIn(void);
static void love2_SampleInChamber(void);
static void love2_KirkAtChamber3(void);
static void love2_ChamberOpen3(void);
static void love2_SampleIn(void);
static void love2_UseChamber(void);
static void love2_UseSynthesizer(void);
static void love2_E_2349(void);
static void love2_GetTank(void);
static void love2_GetAntigrav(void);
static void love2_KirkAtAntigrav(void);
static void love2_TookAntigrav(void);
static void love2_GetPBC(void);
static void love2_KirkAtPBC(void);
static void love2_TookPBC(void);
static void love2_GetDishes(void);
static void love2_KirkAtDishes(void);
static void love2_TookDishes(void);
static void love2_GetSample(void);
static void love2_KirkAtSample(void);
static void love2_TookSample(void);
static void love2_GetGas(void);
static void love2_KirkAtGas(void);
static void love2_TookGas(void);
static void love2_GetCure(void);
static void love2_KirkAtCure(void);
static void love2_TookCure(void);
static void love2_CrewLaugh(void);
static void love2_SpockLaughs(void);
static void love2_MedkitOnSpock(void);
static void love2_SampleOnSpock(uint16_t ax);
static void love2_CureSpock(uint16_t ax);
static void love2_part_2D58(uint16_t ax);
static void love2_AtSpock(void);
static void love2_SpockCured(void);
static void love2_VirusProgresses(void);
static void love2_McCoyScansSpock(void);
static void love2_E_32AA(void);
static void love2_McCoyScansMcCoy(void);
static void love2_McCoyScansCrew(void);
static void love2_ReleaseRLG(void);
static void love2_ReleaseN2O(void);
static void love2_ReleaseNH3(void);
static void love2_CallEnterprise(void);
static void love2_Talk(uint16_t ax);
static void love2_Say(uint16_t ax, uint16_t bx);

/* The room starts: the two gas tanks, the cabinet, the chamber.
 * [MSG_TICK 01 00 00] */
static void love2_StartRoom(void)
{
    uint16_t ax = 0;
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    svc_Sfx(N_LOV2LOOP);
    if (rv_tank1 == 1) {
        fv_tank_anim = N_o2can;
    } else if (rv_tank1 == 2) {
        fv_tank_anim = N_h2can;
    } else if (rv_tank1 == 3) {
        fv_tank_anim = N_n2can;
    } else {
        goto L0268;
    }
    svc_Anim0(0xC, fv_tank_anim, 0xA7, 0xAE, 0);
L0268:;
    if (rv_tank2 == 1) {
        fv_tank_anim = N_o2can;
    } else if (rv_tank2 == 2) {
        fv_tank_anim = N_h2can;
    } else if (rv_tank2 == 3) {
        fv_tank_anim = N_n2can;
    } else {
        goto L02B0;
    }
    svc_Anim0(0xD, fv_tank_anim, 0xB1, 0xAF, 0);
L02B0:;
    s0 = 0;
    s1 = 0x75;
    s2 = 0xAC;
    s3 = N_s3r3vc;
    if (rv_feed_on == 1) {
        s3 = N_s3r3vo;
    }
    svc_Anim0(0xB, s3, s2, s1, s0);
    s0 = 0;
    s1 = 0xAF;
    s2 = 0x1E;
    s3 = N_s3r3d1c;
    if (rv_cabinet_open == 1) {
        s3 = N_s3r3d1o;
    }
    svc_Anim0(8, s3, s2, s1, s0);
    if (rv_cabinet_open == 1) {
        ax = (uint16_t)svc_HaveItem(IT_ANTIGRA);
        if (ax == 0) {
            svc_Anim0(0xE, N_antigr, 0x1F, 0xA7, 0);
        }
    }
    s0 = 0;
    s1 = 0x8D;
    s2 = 0x8A;
    s3 = N_s3r3d2c;
    if (rv_chamber != 0) {
        s3 = N_s3r3d2o;
    }
    svc_Anim0(9, s3, s2, s1, s0);
    if (rv_chamber == 1) {
        fv_chamber_anim = N_pbcanm;
        fv_crew = 0x10;
    } else {
        if (rv_chamber == 2) {
            fv_chamber_anim = N_dishes;
            fv_crew = 0x11;
        } else {
            if (rv_chamber == 3) {
                fv_chamber_anim = N_dishes_2;
                fv_crew = 0x12;
            } else {
                if (rv_chamber != 9) {
                    goto L0419;
                }
                if (rv_gas_made == 1) {
                    fv_gas_anim = N_btle1;
                    fv_crew = 0xF;
                } else if (rv_gas_made == 2) {
                    fv_gas_anim = N_btle2;
                    fv_crew = 0xF;
                } else if (rv_gas_made == 3) {
                    fv_gas_anim = N_btle3;
                    fv_crew = 0xF;
                } else if (rv_gas_made == 4) {
                    fv_gas_anim = N_btle4;
                    fv_crew = 0xF;
                } else {
                    fv_gas_anim = N_cure;
                    fv_crew = 0x13;
                }
            }
        }
    }
    s0 = 0;
    s1 = 0x8B;
    s2 = 0x8A;
    s3 = fv_chamber_anim;
    if (rv_chamber == 9) {
        s3 = fv_gas_anim;
    }
    svc_Anim(fv_crew, s3, s2, s1, s0);
L0419:;
    if (rv_n2o_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[0] = ax;
    }
    if (rv_rlg_released == 1) {
        ax = (uint16_t)svc_Random(0xC8, 0x190);
        room.timers[1] = ax;
    }
    room.timers[2] = 0xC8;
    svc_Anim0(0xA, N_s3r3d3a, 0x12A, 0xB5, 0);
    fv_cure_x = 0xC9;
    fv_cure_y = 0xB7;
    return;
}

/* Exit 1 (timer 3): to the bridge, room 0.
 * [MSG_EXIT 01 00 00] */
static void love2_ExitToBridge(void)
{
    room.busy = 1;
    svc_BridgeSfx(5);
    room.timers[3] = 0xA;
    return;
}

/* [MSG_TIMER 03 00 00] */
static void love2_GoToBridge(void)
{
    svc_GotoRoom(0, 0);
    room.busy = 0;
    return;
}

/* The door to the lab (room 1): walk (msg 14), it opens (anim msg 1).
 * [MSG_WALK 0A 00 00] */
static void love2_E_04AE(void)
{
    { love2_WalkToDoor(); return; }
}

/* [MSG_WALK 22 00 00] */
static void love2_WalkToDoor(void)
{
    room.busy = 1;
    fv_going_door = 1;
    svc_Walk(0, 0x127, 0xBA, 0xE);
    return;
}

/* [MSG_REGION 00 00 00] */
static void love2_ReachDoor(void)
{
    if (fv_going_door == 1) {
        svc_Anim0(0xA, N_s3r3d3, 0x12A, 0xB5, 1);
        svc_BridgeSfx(5);
    }
    return;
}

/* [MSG_WALK_DONE 0E 00 00] */
static void love2_E_050D(void)
{
    { love2_ThroughDoor(); return; }
}

/* [MSG_ANIM_DONE 01 00 00] */
static void love2_ThroughDoor(void)
{
    fv_door_steps++;
    if (fv_door_steps != 2) {
        return;
    }
    svc_GotoRoom(1, 1);
    return;
}

/* Looking and talking.
 * [MSG_LOOK 08 00 00] */
static void love2_LookAtCabinet(void)
{
    love2_Say(0, T_LOV2N027);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void love2_LookAtDoor(void)
{
    love2_Say(0, T_LOV2N001);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void love2_LookAtSynthesizer(void)
{
    love2_Say(0, T_LOV2N028);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void love2_LookAtScene(void)
{
    love2_Say(0, T_LOV2N026);
    return;
}

/* [MSG_LOOK 0E 00 00] */
static void love2_LookAtAntigrav(void)
{
    love2_Say(0, T_LOV2N006);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void love2_LookAtMcCoy(void)
{
    love2_Say(0, T_LOV2N011);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void love2_LookAtSpock(void)
{
    love2_Say(0, T_LOV2N014);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void love2_LookAtRedshirt(void)
{
    love2_Say(0, T_LOV2N010);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void love2_LookAtKirk(void)
{
    love2_Say(0, T_LOV2N009);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void love2_LookAtArdak(void)
{
    love2_Say(S_37CE, T_LOV2N045);
    love2_Say(N_Mr_Spock, T_LOV2_037);
    return;
}

/* [MSG_LOOK 0F 00 00] */
static void love2_E_05FF(void)
{
    { love2_LookAtChamber2(); return; }
}

/* [MSG_LOOK 10 00 00] */
static void love2_LookAtChamber2(void)
{
    { love2_LookAtChamber3(); return; }
}

/* [MSG_LOOK 12 00 00] */
static void love2_LookAtChamber3(void)
{
    { love2_LookAtChamber4(); return; }
}

/* [MSG_LOOK 13 00 00] */
static void love2_LookAtChamber4(void)
{
    { love2_LookAtChamber5(); return; }
}

/* [MSG_LOOK 11 00 00] */
static void love2_LookAtChamber5(void)
{
    { love2_LookAtChamber(); return; }
}

/* [MSG_LOOK 09 00 00] */
static void love2_LookAtChamber(void)
{
    love2_Say(S_37CE, T_LOV2N029);
    love2_Say(N_Mr_Spock, T_LOV2_035);
    love2_Say(N_Dr_McCoy, T_LOV2_019);
    love2_Say(N_Mr_Spock, T_LOV2_033);
    love2_Say(N_Dr_McCoy, T_LOV2_016);
    return;
}

/* [MSG_LOOK 0C 00 00] */
static void love2_LookAtTank1(void)
{
    if (rv_tank1 == 1) {
        love2_Say(S_37CE, T_LOV2N004);
        return;
    }
    if (rv_tank1 == 2) {
        love2_Say(S_37CE, T_LOV2N002);
        return;
    }
    love2_Say(S_37CE, T_LOV2N003);
    return;
}

/* [MSG_LOOK 0D 00 00] */
static void love2_LookAtTank2(void)
{
    if (rv_tank2 == 1) {
        love2_Say(S_37CE, T_LOV2N004);
        return;
    }
    if (rv_tank2 == 2) {
        love2_Say(S_37CE, T_LOV2N002);
        return;
    }
    love2_Say(S_37CE, T_LOV2N003);
    return;
}

/* [MSG_TALK 00 00 00] */
static void love2_TalkToKirk(void)
{
    love2_Say(N_Capt_Kirk, T_LOV2_001);
    return;
}

/* [MSG_TALK 01 00 00] */
static void love2_TalkToSpock(void)
{
    love2_Say(N_Mr_Spock, T_LOV2_029);
    return;
}

/* [MSG_TALK 02 00 00] */
static void love2_TalkToMcCoy(void)
{
    love2_Say(N_Dr_McCoy, T_LOV2_009);
    return;
}

/* [MSG_TALK 03 00 00] */
static void love2_TalkToRedshirt(void)
{
    love2_Say(N_Lt_Ferris, T_LOV2_041);
    return;
}

/* Tricorders.
 * [MSG_USE 45 FF 00] */
static void love2_McCoyScans(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_have_research != 1) {
        love2_Say(N_Dr_McCoy, T_LOV2_012);
        return;
    }
    love2_Say(N_Dr_McCoy, T_LOV2_008);
    return;
}

/* [MSG_USE 44 FF 00] */
static void love2_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    love2_Say(N_Mr_Spock, T_LOV2_027);
    return;
}

/* The cabinet, by anyone (msgs 12, 11, 2): open, and the antigrav unit.
 * [MSG_USE 00 08 00] */
static void love2_KirkOpensCabinet(void)
{
    fv_crew = 0;
    fv_crew_anim = N_kusehw;
    { love2_part_0878(); return; }
}

/* [MSG_USE 01 08 00] */
static void love2_SpockOpensCabinet(void)
{
    fv_crew = 1;
    fv_crew_anim = N_susehw;
    { love2_part_0878(); return; }
}

/* [MSG_USE 02 08 00] */
static void love2_McCoyOpensCabinet(void)
{
    fv_crew = 2;
    fv_crew_anim = N_musehw;
    { love2_part_0878(); return; }
}

/* [MSG_USE 03 08 00] */
static void love2_FerrisOpensCabinet(void)
{
    fv_crew = 3;
    fv_crew_anim = N_rusehw;
    { love2_part_0878(); return; }
}

static void love2_part_0878(void)
{
    svc_Walk(fv_crew, 0x2D, 0xBB, 0xC);
    return;
}

/* [MSG_WALK_DONE 0C 00 00] */
static void love2_AtCabinet(void)
{
    svc_Anim(fv_crew, fv_crew_anim, -1, -1, 0xB);
    return;
}

/* [MSG_ANIM_DONE 0B 00 00] */
static void love2_CabinetToggled(void)
{
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;
    uint16_t s3;
    s0 = 2;
    s1 = 0xAF;
    s2 = 0x1E;
    s3 = N_s3r3d1;
    if (rv_cabinet_open == 1) {
        s3 = N_s3r3d4;
    }
    svc_Anim(8, s3, s2, s1, s0);
    svc_BridgeSfx(5);
    if (rv_cabinet_open == 1) {
        svc_StopAnim(0xE);
    }
    rv_cabinet_open ^= 1;
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void love2_CabinetOpened(void)
{
    uint16_t ax = 0;
    if (rv_cabinet_open != 0) {
        ax = (uint16_t)svc_HaveItem(IT_ANTIGRA);
        if (ax != 1) {
            svc_Anim0(0xE, N_antigr, 0x1F, 0xA7, 0);
        }
    }
    return;
}

/* The wrench on the valve (msgs 1, 12): the gas feed on or off.
 * [MSG_USE 4A 0B 00] */
static void love2_WrenchOnValve(void)
{
    svc_Walk(0, 0xA8, 0xB7, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void love2_KirkAtValve(void)
{
    svc_Anim(0, N_kusehn, -1, -1, 0xC);
    svc_Sfx(N_MUR3E1S);
    return;
}

/* [MSG_ANIM_DONE 0C 00 00] */
static void love2_ValveTurned(void)
{
    if (rv_feed_on != 1) {
        rv_feed_on = 1;
        svc_Anim(0xB, N_s3r3v1, 0xAC, 0x75, 0);
        love2_Say(S_37CE, T_LOV2N008);
        return;
    }
    rv_feed_on = 0;
    svc_Anim(0xB, N_s3r3v2, 0xAC, 0x75, 0);
    love2_Say(S_37CE, T_LOV2N007);
    return;
}

/* Fitting a gas tank to the feed lines (msgs 3, 13, 14), only with the
 * feed off and a slot free.
 * [MSG_USE 63 23 00] */
static void love2_O2InSlot(void)
{
    fv_tank_gas = 1;
    fv_tank_anim = N_o2can;
    fv_tank_item = 0x63;
    { love2_part_0A23(); return; }
}

/* [MSG_USE 64 23 00] */
static void love2_H2InSlot(void)
{
    fv_tank_gas = 2;
    fv_tank_anim = N_h2can;
    fv_tank_item = 0x64;
    { love2_part_0A23(); return; }
}

/* [MSG_USE 62 23 00] */
static void love2_N2InSlot(void)
{
    fv_tank_gas = 3;
    fv_tank_anim = N_n2can;
    fv_tank_item = 0x62;
    { love2_part_0A23(); return; }
}

static void love2_part_0A23(void)
{
    svc_Walk(0, 0xA8, 0xB7, 3);
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void love2_KirkAtSlots(void)
{
    if (rv_feed_on == 1) {
        love2_Say(N_Mr_Spock, T_LOV2_026);
        return;
    }
    if (rv_tank1 != 0) {
    } else {
        svc_Anim(0, N_kusehn, -1, -1, 0xD);
        return;
    }
    if (rv_tank2 != 0) {
        love2_Say(0, T_LOV2N024);
        return;
    }
    svc_Anim(0, N_kusehn, -1, -1, 0xE);
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void love2_Tank1Fitted(void)
{
    uint16_t ax = 0;
    svc_Sfx(N_SE6FOOD);
    ax = (uint16_t)svc_Anim0(0xC, fv_tank_anim, 0xA7, 0xAE, 0);
    SETLO(ax, fv_tank_gas);
    rv_tank1 = LO(ax);
    love2_Say(0, T_LOV2N034);
    if (room.redshirt_dead != 1) {
    }
    svc_TakeItem(fv_tank_item);
    return;
}

/* [MSG_ANIM_DONE 0E 00 00] */
static void love2_Tank2Fitted(void)
{
    uint16_t ax = 0;
    ax = (uint16_t)svc_Anim0(0xD, fv_tank_anim, 0xB1, 0xAF, 0);
    SETLO(ax, fv_tank_gas);
    rv_tank2 = LO(ax);
    love2_Say(0, T_LOV2N035);
    if (room.redshirt_dead != 1) {
    }
    svc_TakeItem(fv_tank_item);
    return;
}

/* Moving a tank off its slot with the antigrav unit (msgs 4, 15, 5,
 * 16), only with the feed off.
 * [MSG_USE 61 0C 00] */
static void love2_AntigravOnTank1(void)
{
    svc_Walk(0, 0xA8, 0xB7, 4);
    return;
}

/* [MSG_WALK_DONE 04 00 00] */
static void love2_KirkAtTank1(void)
{
    if (rv_feed_on == 1) {
        love2_Say(N_Mr_Spock, T_LOV2_025);
        return;
    }
    svc_Anim(0, N_kusehn, -1, -1, 0xF);
    return;
}

/* [MSG_ANIM_DONE 0F 00 00] */
static void love2_Tank1Freed(void)
{
    uint16_t s0;
    love2_Say(0, T_LOV2N030);
    svc_StopAnim(0xC);
    if (rv_tank1 == 1) {
        s0 = 0x63;
    } else if (rv_tank1 == 2) {
        s0 = 0x64;
    } else {
        s0 = 0x62;
    }
    svc_GiveItem(s0);
    rv_tank1 = 0;
    return;
}

/* [MSG_USE 61 0D 00] */
static void love2_AntigravOnTank2(void)
{
    svc_Walk(0, 0xA8, 0xB7, 5);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void love2_KirkAtTank2(void)
{
    if (rv_feed_on == 1) {
        love2_Say(N_Mr_Spock, T_LOV2_025_2);
        return;
    }
    svc_Anim(0, N_kusehn, -1, -1, 0x10);
    svc_Sfx(N_SE3PLBAT);
    return;
}

/* [MSG_ANIM_DONE 10 00 00] */
static void love2_Tank2Freed(void)
{
    uint16_t s0;
    love2_Say(0, T_LOV2N031);
    svc_StopAnim(0xD);
    if (rv_tank2 == 1) {
        s0 = 0x63;
    } else if (rv_tank2 == 2) {
        s0 = 0x64;
    } else {
        s0 = 0x62;
    }
    svc_GiveItem(s0);
    rv_tank2 = 0;
    return;
}

/* Running the synthesizer (msgs 13, 17): with the feed on and two tanks,
 * it makes a gas from the pair (O2+N2 N2O, H2+N2 ammonia, O2+H2 water)
 * or, with something in the chamber, works on that: TLTDH from
 * polyberylcarbonate, the cure from the cure sample, goo from the rest.
 * With the feed off, after three tries Spock turns it on.
 * [MSG_USE 00 20 00] */
static void love2_KirkUsesSynthesizer(void)
{
    fv_crew = 0;
    fv_crew_anim = N_kusemn;
    { love2_part_0F4C(); return; }
}

/* [MSG_USE 01 20 00] */
static void love2_SpockUsesSynthesizer(void)
{
    fv_crew = 1;
    fv_crew_anim = N_susemn;
    { love2_part_0F4C(); return; }
}

/* [MSG_USE 02 20 00] */
static void love2_McCoyUsesSynthesizer(void)
{
    fv_crew = 2;
    fv_crew_anim = N_musemn;
    { love2_part_0F4C(); return; }
}

/* [MSG_USE 03 20 00] */
static void love2_FerrisUsesSynthesizer(void)
{
    fv_crew = 3;
    fv_crew_anim = N_rusemn;
    { love2_part_0F4C(); return; }
}

static void love2_part_0F4C(void)
{
    svc_Walk(fv_crew, 0x46, 0xAE, 0xD);
    return;
}

/* [MSG_WALK_DONE 0D 00 00] */
static void love2_AtSynthesizer(void)
{
    svc_Anim(fv_crew, fv_crew_anim, -1, -1, 0x11);
    return;
}

/* [MSG_ANIM_DONE 11 00 00] */
static void love2_Synthesize(void)
{
    uint16_t ax = 0;
    uint16_t t0;
    uint16_t t1;
    if (rv_feed_on == 1) {
        { love2_part_110F(); return; }
    }
    if (fv_dry_runs != 3) {
    } else {
        ax = (uint16_t)svc_HaveItem(IT_WRENCH);
        ax = ax | ax;
        if (ax == 0) {
            t0 = ax;
            love2_Say(N_Mr_Spock, T_LOV2_006);
            return;
        }
        t1 = ax;
        love2_Say(N_Mr_Spock, T_LOV2_005);
        svc_Walk(1, 0xA8, 0xB7, 0x10);
        return;
    }
    fv_dry_runs++;
    love2_Say(0, T_LOV2N012);
    return;
}

/* [MSG_WALK_DONE 10 00 00] */
static void love2_SpockAtValve(void)
{
    svc_Anim(1, N_susehn, -1, -1, 0x12);
    return;
}

/* [MSG_ANIM_DONE 12 00 00] */
static void love2_SpockTurnsValve(void)
{
    rv_feed_on = 1;
    svc_Anim(0xB, N_s3r3v1, 0xAC, 0x75, 0);
    love2_Say(S_37CE, T_LOV2N008);
    svc_Walk(1, 0xC6, 0xBB, 0);
    { love2_part_110F(); return; }
}

static void love2_part_110F(void)
{
    if (rv_tank1 != 0) {
    } else {
        svc_Sfx(N_EFX24);
        love2_Say(0, T_LOV2N005);
        love2_Say(N_Mr_Spock, T_LOV2_024);
        return;
    }
    if (rv_tank2 != 0) {
    } else {
        svc_Sfx(N_EFX24_2);
        love2_Say(0, T_LOV2N005_2);
        love2_Say(N_Dr_McCoy, T_LOV2_010);
        return;
    }
    if (rv_tank1 == 1) {
        if (rv_tank2 == 2) {
            goto L130F;
        }
        goto L1371;
    }
    if (rv_tank1 == 2) {
        if (rv_tank2 == 1) {
            goto L130F;
        }
        goto L1340;
    }
    if (rv_tank2 == 1) {
        goto L1371;
    }
    goto L1340;
L130F:;
    if (rv_chamber == 1) {
        goto L1987;
    }
    if (rv_chamber == 2) {
        goto L19C9;
    }
    if (rv_chamber == 3) {
        goto L1A6F;
    }
    if (rv_chamber == 9) {
        goto L1B0D;
    }
    rv_gas_made = 3;
    fv_gas_anim = N_btle3;
    { love2_part_13A0(); return; }
L1340:;
    if (rv_chamber == 1) {
        goto L1B6E;
    }
    if (rv_chamber == 2) {
        goto L1C1E;
    }
    if (rv_chamber == 3) {
        goto L1EB8;
    }
    if (rv_chamber == 9) {
        goto L1B0D;
    }
    rv_gas_made = 2;
    fv_gas_anim = N_btle2;
    { love2_part_13A0(); return; }
L1371:;
    if (rv_chamber == 1) {
        goto L1B6E;
    }
    if (rv_chamber == 2) {
        goto L1DB5;
    }
    if (rv_chamber == 3) {
        goto L1A6F;
    }
    if (rv_chamber == 9) {
        goto L1B0D;
    }
    rv_gas_made = 1;
    fv_gas_anim = N_btle1;
    { love2_part_13A0(); return; }
L1987:;
    svc_Anim(9, N_s3r3d5, -1, -1, 0x1B);
    svc_BridgeSfx(5);
    svc_StopAnim(0x10);
    return;
L19C9:;
    love2_Say(0, T_LOV2N051);
    love2_Say(N_Capt_Kirk, T_LOV2_002);
    svc_StopAnim(0x11);
    goto L1E90;
L1A6F:;
    love2_Say(0, T_LOV2N050);
    love2_Say(N_Dr_McCoy, T_LOV2_021);
    svc_StopAnim(0x12);
    goto L1E90;
L1B0D:;
    love2_Say(N_Mr_Spock, T_LOV2_028);
    return;
L1B6E:;
    love2_Say(0, T_LOV2N049);
    love2_Say(N_Mr_Spock, S_1BC7);
    svc_StopAnim(0x10);
    goto L1E90;
L1C1E:;
    love2_Say(0, T_LOV2N048);
    love2_Say(N_Mr_Spock, S_1C78);
    love2_Say(N_Dr_McCoy, T_LOV2_023);
    svc_StopAnim(0x11);
    goto L1E90;
L1DB5:;
    love2_Say(0, T_LOV2N047);
    love2_Say(N_Dr_McCoy, T_LOV2_017);
    svc_StopAnim(0x11);
L1E90:;
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    rv_chamber = 0;
    return;
L1EB8:;
    svc_StopAnim(0x12);
    svc_Anim(9, N_s3r3d5, 0x8A, 0x8D, 8);
    svc_BridgeSfx(5);
    return;
}

static void love2_part_13A0(void)
{
    svc_Anim0(9, N_s3r3d2, 0x8A, 0x8D, 3);
    svc_BridgeSfx(5);
    return;
}

/* The results come out of the chamber (anim msgs 3, 4, 27, 8-10).
 * [MSG_ANIM_DONE 03 00 00] */
static void love2_ChamberOpensGas(void)
{
    svc_Sfx(N_LD1SCAN);
    svc_Anim0(0xF, fv_gas_anim, 0x8A, 0x8D, 4);
    rv_chamber = 9;
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void love2_GasMade(void)
{
    if (rv_gas_made != 1) {
    } else {
        love2_Say(0, T_LOV2N017);
        love2_Say(N_Mr_Spock, T_LOV2_032);
        love2_Say(N_Dr_McCoy, T_LOV2_015);
        love2_Say(N_Mr_Spock, T_LOV2_036);
        return;
    }
    if (rv_gas_made != 2) {
    } else {
        love2_Say(0, T_LOV2N016);
        if (room.redshirt_dead == 1) {
        } else {
            love2_Say(N_Lt_Ferris, T_LOV2_043);
            love2_Say(N_Capt_Kirk, T_LOV2_003);
            goto L169F;
        }
L169F:;
        return;
    }
    if (rv_gas_made != 3) {
    } else {
        love2_Say(0, T_LOV2N018);
        love2_Say(N_Mr_Spock, T_LOV2_004);
        return;
    }
    love2_Say(0, T_LOV2N019);
    love2_Say(N_Mr_Spock, T_LOV2_031);
    love2_Say(N_Dr_McCoy, T_LOV2_018);
    love2_Say(N_Mr_Spock, T_LOV2_030);
    love2_Say(N_Dr_McCoy, T_LOV2_022);
    love2_Say(N_Mr_Spock, T_LOV2_038);
    return;
}

/* [MSG_ANIM_DONE 1B 00 00] */
static void love2_ChamberResult(void)
{
    rv_gas_made = 4;
    fv_gas_anim = N_btle4;
    { love2_part_13A0(); return; }
}

/* [MSG_ANIM_DONE 08 00 00] */
static void love2_ChamberOpensCure(void)
{
    svc_Anim(9, N_s3r3d2, 0x8A, 0x8D, 9);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void love2_CureAppears(void)
{
    svc_Anim0(0x13, N_cure, 0x8A, 0x8B, 0xA);
    rv_chamber = 9;
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void love2_CureMade(void)
{
    love2_Say(0, T_LOV2N046);
    love2_Say(N_Dr_McCoy, T_LOV2_020);
    return;
}

/* Putting things in the chamber (msgs 6-8, 5-7, 19-21).
 * [MSG_USE 48 09 00] */
static void love2_PBCInChamber(void)
{
    svc_Walk(0, 0x85, 0xAD, 6);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void love2_KirkAtChamber(void)
{
    if (rv_chamber != 0) {
        love2_Say(0, T_LOV2N025);
        return;
    }
    svc_Anim0(9, N_s3r3d2, 0x8A, 0x8D, 5);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void love2_ChamberOpen(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x13);
    return;
}

/* [MSG_ANIM_DONE 13 00 00] */
static void love2_PBCIn(void)
{
    svc_Anim0(0x10, N_pbcanm, 0x8A, 0x8B, 0);
    svc_TakeItem(IT_PBC);
    rv_chamber = 1;
    return;
}

/* [MSG_USE 4E 09 00] */
static void love2_DishesInChamber(void)
{
    svc_Walk(0, 0x85, 0xAD, 7);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 07 00 00] */
static void love2_KirkAtChamber2(void)
{
    if (rv_chamber != 0) {
        love2_Say(0, T_LOV2N025_2);
        return;
    }
    svc_Anim0(9, N_s3r3d2, 0x8A, 0x8D, 6);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void love2_ChamberOpen2(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x14);
    return;
}

/* [MSG_ANIM_DONE 14 00 00] */
static void love2_DishesIn(void)
{
    svc_Anim0(0x11, N_dishes, 0x8A, 0x8B, 0);
    svc_TakeItem(IT_DISHES);
    rv_chamber = 2;
    return;
}

/* [MSG_USE 4C 09 00] */
static void love2_SampleInChamber(void)
{
    svc_Walk(0, 0x85, 0xAD, 8);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void love2_KirkAtChamber3(void)
{
    if (rv_chamber != 0) {
        love2_Say(0, S_21E5);
        return;
    }
    svc_Anim0(9, N_s3r3d2, 0x8A, 0x8D, 7);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void love2_ChamberOpen3(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x15);
    return;
}

/* [MSG_ANIM_DONE 15 00 00] */
static void love2_SampleIn(void)
{
    svc_Anim0(0x12, N_dishes_2, 0x8A, 0x8B, 0);
    svc_TakeItem(IT_SAMPLE);
    rv_chamber = 3;
    return;
}

/* Other things.
 * [MSG_USE FF 09 00] */
static void love2_UseChamber(void)
{
    love2_Say(N_Dr_McCoy, T_LOV2_014);
    return;
}

/* [MSG_USE FF 20 00] */
static void love2_UseSynthesizer(void)
{
    love2_Say(N_Mr_Spock, T_LOV2_007);
    return;
}

/* Getting things: the tanks are too heavy; the antigrav unit; whatever
 * is in the chamber (the first gas +4, the cure +5, once).
 * [MSG_GET 0D 00 00] */
static void love2_E_2349(void)
{
    { love2_GetTank(); return; }
}

/* [MSG_GET 0C 00 00] */
static void love2_GetTank(void)
{
    if (room.redshirt_dead != 1) {
        love2_Say(N_Lt_Ferris, T_LOV2_040);
        return;
    }
    love2_Say(N_Dr_McCoy, T_LOV2_011);
    return;
}

/* [MSG_GET 0E 00 00] */
static void love2_GetAntigrav(void)
{
    svc_Walk(0, 0x37, 0xB4, 9);
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void love2_KirkAtAntigrav(void)
{
    svc_Anim(0, N_kusehw, -1, -1, 0x16);
    room.fall[0] = 3;
    return;
}

/* [MSG_ANIM_DONE 16 00 00] */
static void love2_TookAntigrav(void)
{
    svc_StopAnim(0xE);
    svc_GiveItem(IT_ANTIGRA);
    love2_Say(0, T_LOV2N042);
    return;
}

/* [MSG_GET 10 00 00] */
static void love2_GetPBC(void)
{
    svc_Walk(0, 0x85, 0xAD, 0x12);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 12 00 00] */
static void love2_KirkAtPBC(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x1A);
    return;
}

/* [MSG_ANIM_DONE 1A 00 00] */
static void love2_TookPBC(void)
{
    svc_StopAnim(0x10);
    love2_Say(0, T_LOV2N044);
    svc_GiveItem(IT_PBC);
    rv_chamber = 0;
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_GET 11 00 00] */
static void love2_GetDishes(void)
{
    svc_Walk(0, 0x85, 0xAD, 0x11);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 11 00 00] */
static void love2_KirkAtDishes(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x19);
    return;
}

/* [MSG_ANIM_DONE 19 00 00] */
static void love2_TookDishes(void)
{
    svc_StopAnim(0x11);
    love2_Say(0, T_LOV2N036);
    svc_GiveItem(IT_DISHES);
    rv_chamber = 0;
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_GET 12 00 00] */
static void love2_GetSample(void)
{
    svc_Walk(0, 0x85, 0xAD, 0x13);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 13 00 00] */
static void love2_KirkAtSample(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x1C);
    return;
}

/* [MSG_ANIM_DONE 1C 00 00] */
static void love2_TookSample(void)
{
    svc_StopAnim(0x12);
    love2_Say(0, T_LOV2N037);
    svc_GiveItem(IT_SAMPLE);
    rv_chamber = 0;
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    return;
}

/* [MSG_GET 0F 00 00] */
static void love2_GetGas(void)
{
    svc_Walk(0, 0x85, 0xAD, 0xA);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 0A 00 00] */
static void love2_KirkAtGas(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x17);
    return;
}

/* [MSG_ANIM_DONE 17 00 00] */
static void love2_TookGas(void)
{
    uint16_t s0;
    if (rv_gas_made == 1) {
        love2_Say(0, T_LOV2N040);
        s0 = 0x65;
    } else {
        if (rv_gas_made == 2) {
            love2_Say(0, T_LOV2N039);
            s0 = 0x66;
        } else {
            if (rv_gas_made == 3) {
                love2_Say(0, T_LOV2N041);
                s0 = 0x67;
            } else {
                love2_Say(0, T_LOV2N038);
                if (rv_took_gas != 1) {
                    rv_took_gas = 1;
                    rv_score += 4;
                }
                s0 = 0x49;
            }
        }
    }
    svc_GiveItem(s0);
    svc_StopAnim(0xF);
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    rv_chamber = 0;
    rv_gas_made = 0;
    return;
}

/* [MSG_GET 13 00 00] */
static void love2_GetCure(void)
{
    svc_Walk(0, 0x85, 0xAD, 0xB);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 0B 00 00] */
static void love2_KirkAtCure(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x18);
    return;
}

/* [MSG_ANIM_DONE 18 00 00] */
static void love2_TookCure(void)
{
    if (rv_made_cure != 1) {
        rv_made_cure = 1;
        rv_score += 5;
    }
    svc_GiveItem(IT_CURE);
    svc_StopAnim(0x13);
    love2_Say(0, T_LOV2N043);
    svc_Anim0(9, N_s3r3d5, 0x8A, 0x8D, 0);
    svc_BridgeSfx(5);
    rv_chamber = 0;
    return;
}

/* The gases, common to all of the mission's rooms. Timer 0 (after
 * laughing gas): a random crew member giggles, and again in 200-400 ticks.
 * [MSG_TIMER 00 00 00] */
static void love2_CrewLaugh(void)
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
                goto L2A79;
            }
        } else {
            love2_Say(0, N_get_name_error);
            return;
        }
    }
    fv_laugher = N_Dr_McCoy;
    ax = (uint16_t)svc_Random(0, 0xA);
    fv_roll = ax;
    if (fv_roll == 0) {
        bx = T_LOV0_011;
        goto L2B20;
    }
    if (fv_roll == 1) {
        bx = T_LOV0_020;
        goto L2B20;
    }
    if (fv_roll == 2) {
        bx = T_LOV0_016;
        goto L2B20;
    }
    if (fv_roll == 3) {
        bx = T_LOV0_012;
        goto L2B20;
    }
    if (fv_roll == 4) {
        bx = T_LOV0_014;
        goto L2B20;
    }
    if (fv_roll == 5) {
        bx = T_LOV0_022;
        goto L2B20;
    }
    if (fv_roll == 6) {
        bx = T_LOV0_015;
        goto L2B20;
    }
    if (fv_roll == 7) {
        bx = T_LOV0_021;
        goto L2B20;
    }
    if (fv_roll == 8) {
        bx = T_LOV0_013;
        goto L2B20;
    }
    if (fv_roll == 9) {
        bx = T_LOV0_019;
        goto L2B20;
    }
    bx = T_LOV0_017;
    goto L2B20;
L2A79:;
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
        goto L2B20;
    }
L2B20:;
    ax = fv_laugher;
    love2_Say(ax, bx);
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
        goto L2B6F;
    }
L2B6F:;
    love2_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[0] = ax;
    return;
}

/* Timer 1 (after Romulan laughing gas): Spock laughs.
 * [MSG_TIMER 01 00 00] */
static void love2_SpockLaughs(void)
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
        goto L2C00;
    }
L2C00:;
    love2_Say(ax, bx);
    ax = (uint16_t)svc_Random(0xC8, 0x190);
    room.timers[1] = ax;
    return;
}

/* Curing Spock, common to all of the mission's rooms: with the cure,
 * Spock and McCoy walk to the cure spot (msg 99), and McCoy injects him.
 * [MSG_USE 7F 01 00] */
static void love2_MedkitOnSpock(void)
{
    if (rv_spock_cured != 1) {
        love2_Say(N_Dr_McCoy, T_LOV5_019);
        return;
    }
    love2_Say(N_Dr_McCoy, T_LOV5_015);
    return;
}

/* [MSG_USE 4C 01 00] */
static void love2_SampleOnSpock(uint16_t ax)
{
    if (rv_spock_cured == 1) {
        { love2_part_2D58(ax); return; }
    }
    love2_Say(N_Dr_McCoy, T_LOVA_F55);
    return;
}

/* [MSG_USE 4D 01 00] */
static void love2_CureSpock(uint16_t ax)
{
    if (rv_spock_cured != 1)
        { love2_part_2D58(ax); return; }
    love2_Say(N_Dr_McCoy, T_LOV5_015_2);
    return;
}

static void love2_part_2D58(uint16_t ax)
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
static void love2_AtSpock(void)
{
    fv_cure_steps++;
    if (fv_cure_steps != 2) {
        return;
    }
    svc_Anim(2, N_musemn, -1, -1, 0x63);
    return;
}

/* [MSG_ANIM_DONE 63 00 00] */
static void love2_SpockCured(void)
{
    love2_Say(N_Dr_McCoy, T_LOV5_030);
    love2_Say(N_Mr_Spock, T_LOV5_038);
    love2_Say(N_Dr_McCoy, T_LOV5_027);
    love2_Say(N_Mr_Spock, S_2E64);
    rv_spock_cured = 1;
    return;
}

/* Timer 2, every 200 ticks until Spock is cured: the virus advances,
 * with a line at 0Fh, 1Eh, 2Dh; at 3Ch Spock collapses, at 64h he dies.
 * [MSG_TIMER 02 00 00] */
static void love2_VirusProgresses(void)
{
    if (rv_spock_cured == 1) {
        return;
    }
    rv_virus++;
    if ((int8_t)rv_virus >= (int8_t)0x64) {
    } else {
        if (rv_virus == 0xF) {
            love2_Say(N_Mr_Spock, T_spokcoff);
            rv_virus++;
        } else {
            if (rv_virus == 0x1E) {
                love2_Say(N_Dr_McCoy, T_LOVA_F08);
                rv_virus++;
            } else {
                if (rv_virus == 0x2D) {
                    love2_Say(N_Mr_Spock, T_spokcoff_2);
                    rv_virus++;
                } else {
                    if (rv_virus == 0x3C) {
                        love2_Say(N_Mr_Spock, T_LOVA_F54);
                        rv_virus_struck = 1;
                        rv_virus++;
                        goto L2FDD;
                    }
                }
            }
        }
L2FDD:;
        room.timers[2] = 0xC8;
        return;
    }
    svc_Anim(1, N_sgetdn, -1, -1, 0);
    svc_Music(2, STALE_ARG);
    love2_Say(0, T_GENER004);
    svc_GameOver();
    return;
}

/* McCoy's tricorder on Spock follows the virus; on the others, normal.
 * [MSG_USE 45 01 00] */
static void love2_McCoyScansSpock(void)
{
    svc_BridgeSfx(4);
    if (rv_spock_cured == 1) {
        love2_Say(N_Dr_McCoy, T_LOVA_F01);
        return;
    }
    if (rv_have_research != 1) {
        love2_Say(N_Dr_McCoy, T_LOVA_F02);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0xA) {
        love2_Say(N_Dr_McCoy, T_LOVA_F04);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x1E) {
        love2_Say(N_Dr_McCoy, T_LOVA_F03);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x32) {
        love2_Say(N_Dr_McCoy, T_LOVA_F08_2);
        return;
    }
    if (rv_virus == 0x46) {
        love2_Say(N_Dr_McCoy, T_TUG2_010);
        return;
    }
    if ((int8_t)rv_virus < (int8_t)0x64) {
        love2_Say(N_Dr_McCoy, T_LOVA_F10);
        return;
    }
    love2_Say(N_Dr_McCoy, T_LOVA_100);
    return;
}

/* [MSG_USE 45 00 00] */
static void love2_E_32AA(void)
{
    { love2_McCoyScansMcCoy(); return; }
}

/* [MSG_USE 45 02 00] */
static void love2_McCoyScansMcCoy(void)
{
    { love2_McCoyScansCrew(); return; }
}

/* [MSG_USE 45 03 00] */
static void love2_McCoyScansCrew(void)
{
    svc_BridgeSfx(4);
    love2_Say(N_Dr_McCoy, T_LOVA_F07);
    return;
}

/* Opening a gas canister: Romulan laughing gas sets Spock laughing,
 * nitrous oxide the crew, ammonia nothing.
 * [MSG_USE 49 FF 00] */
static void love2_ReleaseRLG(void)
{
    love2_Say(S_37CE, T_LOV2N005_3);
    rv_rlg_released = 1;
    svc_TakeItem(IT_RLG);
    return;
}

/* [MSG_USE 65 FF 00] */
static void love2_ReleaseN2O(void)
{
    love2_Say(S_37CE, T_LOV2N005_4);
    love2_Say(N_Mr_Spock, T_MUD2_040);
    rv_n2o_released = 1;
    svc_TakeItem(IT_N2O);
    svc_Music(3, STALE_ARG);
    return;
}

/* [MSG_USE 66 FF 00] */
static void love2_ReleaseNH3(void)
{
    love2_Say(S_37CE, T_LOV2N005_5);
    if (room.redshirt_dead != 1) {
    }
    svc_TakeItem(IT_NH3);
    return;
}

/* The communicator: four cases (first call, later calls, the same after
 * the cure), all with the same three lines borrowed from other missions'
 * speech files.
 * [MSG_USE 47 FF 00] */
static void love2_CallEnterprise(void)
{
    if (rv_made_cure == 1) {
    } else {
        if (rv_called_ship == 1) {
        } else {
            love2_Say(N_Capt_Kirk, T_MUD4_018);
            love2_Say(N_Lt_Uhura, T_BRIDU146);
            love2_Say(N_Capt_Kirk, T_VENA_F41);
            rv_called_ship = 1;
            return;
        }
        love2_Say(N_Capt_Kirk, T_MUD4_018_2);
        love2_Say(N_Lt_Uhura, T_BRIDU146_2);
        love2_Say(N_Capt_Kirk, T_VENA_F41_2);
        return;
    }
    if (rv_called_ship_cured == 1) {
    } else {
        love2_Say(N_Capt_Kirk, T_MUD4_018_3);
        love2_Say(N_Lt_Uhura, T_BRIDU146_3);
        love2_Say(N_Capt_Kirk, T_VENA_F41_3);
        rv_called_ship_cured = 1;
        return;
    }
    love2_Say(N_Capt_Kirk, T_MUD4_018_4);
    love2_Say(N_Lt_Uhura, T_BRIDU146_4);
    love2_Say(N_Capt_Kirk, T_VENA_F41_4);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void love2_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void love2_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x484F) = bx;
    love2_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript love2_scripts[] = {
    { 0x0206, love2_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0479, love2_ExitToBridge },    /* MSG_EXIT 01 00 00 */
    { 0x0495, love2_GoToBridge },    /* MSG_TIMER 03 00 00 */
    { 0x04AE, love2_E_04AE },    /* MSG_WALK 0A 00 00 */
    { 0x04B6, love2_WalkToDoor },    /* MSG_WALK 22 00 00 */
    { 0x04DB, love2_ReachDoor },    /* MSG_REGION 00 00 00 */
    { 0x050D, love2_E_050D },    /* MSG_WALK_DONE 0E 00 00 */
    { 0x0515, love2_ThroughDoor },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0538, love2_LookAtCabinet },    /* MSG_LOOK 08 00 00 */
    { 0x0547, love2_LookAtDoor },    /* MSG_LOOK 0A 00 00 */
    { 0x0556, love2_LookAtSynthesizer },    /* MSG_LOOK 20 00 00 */
    { 0x0565, love2_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0574, love2_LookAtAntigrav },    /* MSG_LOOK 0E 00 00 */
    { 0x0583, love2_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0592, love2_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x05A1, love2_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x05B0, love2_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x05E6, love2_LookAtArdak },    /* MSG_LOOK 21 00 00 */
    { 0x05FF, love2_E_05FF },    /* MSG_LOOK 0F 00 00 */
    { 0x0607, love2_LookAtChamber2 },    /* MSG_LOOK 10 00 00 */
    { 0x060F, love2_LookAtChamber3 },    /* MSG_LOOK 12 00 00 */
    { 0x0617, love2_LookAtChamber4 },    /* MSG_LOOK 13 00 00 */
    { 0x061F, love2_LookAtChamber5 },    /* MSG_LOOK 11 00 00 */
    { 0x0627, love2_LookAtChamber },    /* MSG_LOOK 09 00 00 */
    { 0x065B, love2_LookAtTank1 },    /* MSG_LOOK 0C 00 00 */
    { 0x068B, love2_LookAtTank2 },    /* MSG_LOOK 0D 00 00 */
    { 0x06BB, love2_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x071A, love2_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x072A, love2_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x073A, love2_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x074A, love2_McCoyScans },    /* MSG_USE 45 FF 00 */
    { 0x07F7, love2_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x0828, love2_KirkOpensCabinet },    /* MSG_USE 00 08 00 */
    { 0x083E, love2_SpockOpensCabinet },    /* MSG_USE 01 08 00 */
    { 0x0854, love2_McCoyOpensCabinet },    /* MSG_USE 02 08 00 */
    { 0x086A, love2_FerrisOpensCabinet },    /* MSG_USE 03 08 00 */
    { 0x0895, love2_AtCabinet },    /* MSG_WALK_DONE 0C 00 00 */
    { 0x08B6, love2_CabinetToggled },    /* MSG_ANIM_DONE 0B 00 00 */
    { 0x08FF, love2_CabinetOpened },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x0933, love2_WrenchOnValve },    /* MSG_USE 4A 0B 00 */
    { 0x094E, love2_KirkAtValve },    /* MSG_WALK_DONE 01 00 00 */
    { 0x0983, love2_ValveTurned },    /* MSG_ANIM_DONE 0C 00 00 */
    { 0x09D7, love2_O2InSlot },    /* MSG_USE 63 23 00 */
    { 0x09F3, love2_H2InSlot },    /* MSG_USE 64 23 00 */
    { 0x0A0F, love2_N2InSlot },    /* MSG_USE 62 23 00 */
    { 0x0A3E, love2_KirkAtSlots },    /* MSG_WALK_DONE 03 00 00 */
    { 0x0AD7, love2_Tank1Fitted },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x0BDE, love2_Tank2Fitted },    /* MSG_ANIM_DONE 0E 00 00 */
    { 0x0C6E, love2_AntigravOnTank1 },    /* MSG_USE 61 0C 00 */
    { 0x0C89, love2_KirkAtTank1 },    /* MSG_WALK_DONE 04 00 00 */
    { 0x0D11, love2_Tank1Freed },    /* MSG_ANIM_DONE 0F 00 00 */
    { 0x0DA8, love2_AntigravOnTank2 },    /* MSG_USE 61 0D 00 */
    { 0x0DC3, love2_KirkAtTank2 },    /* MSG_WALK_DONE 05 00 00 */
    { 0x0E65, love2_Tank2Freed },    /* MSG_ANIM_DONE 10 00 00 */
    { 0x0EFC, love2_KirkUsesSynthesizer },    /* MSG_USE 00 20 00 */
    { 0x0F12, love2_SpockUsesSynthesizer },    /* MSG_USE 01 20 00 */
    { 0x0F28, love2_McCoyUsesSynthesizer },    /* MSG_USE 02 20 00 */
    { 0x0F3E, love2_FerrisUsesSynthesizer },    /* MSG_USE 03 20 00 */
    { 0x0F69, love2_AtSynthesizer },    /* MSG_WALK_DONE 0D 00 00 */
    { 0x0F8A, love2_Synthesize },    /* MSG_ANIM_DONE 11 00 00 */
    { 0x1089, love2_SpockAtValve },    /* MSG_WALK_DONE 10 00 00 */
    { 0x10A5, love2_SpockTurnsValve },    /* MSG_ANIM_DONE 12 00 00 */
    { 0x13CA, love2_ChamberOpensGas },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x1407, love2_GasMade },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x19BB, love2_ChamberResult },    /* MSG_ANIM_DONE 1B 00 00 */
    { 0x1EEE, love2_ChamberOpensCure },    /* MSG_ANIM_DONE 08 00 00 */
    { 0x1F18, love2_CureAppears },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x1F3A, love2_CureMade },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x1FF7, love2_PBCInChamber },    /* MSG_USE 48 09 00 */
    { 0x2016, love2_KirkAtChamber },    /* MSG_WALK_DONE 06 00 00 */
    { 0x208C, love2_ChamberOpen },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x20A8, love2_PBCIn },    /* MSG_ANIM_DONE 13 00 00 */
    { 0x20D6, love2_DishesInChamber },    /* MSG_USE 4E 09 00 */
    { 0x20F5, love2_KirkAtChamber2 },    /* MSG_WALK_DONE 07 00 00 */
    { 0x216B, love2_ChamberOpen2 },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x2187, love2_DishesIn },    /* MSG_ANIM_DONE 14 00 00 */
    { 0x21B5, love2_SampleInChamber },    /* MSG_USE 4C 09 00 */
    { 0x21D4, love2_KirkAtChamber3 },    /* MSG_WALK_DONE 08 00 00 */
    { 0x223B, love2_ChamberOpen3 },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x2257, love2_SampleIn },    /* MSG_ANIM_DONE 15 00 00 */
    { 0x2285, love2_UseChamber },    /* MSG_USE FF 09 00 */
    { 0x22DA, love2_UseSynthesizer },    /* MSG_USE FF 20 00 */
    { 0x2349, love2_E_2349 },    /* MSG_GET 0D 00 00 */
    { 0x2351, love2_GetTank },    /* MSG_GET 0C 00 00 */
    { 0x23F5, love2_GetAntigrav },    /* MSG_GET 0E 00 00 */
    { 0x240F, love2_KirkAtAntigrav },    /* MSG_WALK_DONE 09 00 00 */
    { 0x242F, love2_TookAntigrav },    /* MSG_ANIM_DONE 16 00 00 */
    { 0x2484, love2_GetPBC },    /* MSG_GET 10 00 00 */
    { 0x24A3, love2_KirkAtPBC },    /* MSG_WALK_DONE 12 00 00 */
    { 0x24BF, love2_TookPBC },    /* MSG_ANIM_DONE 1A 00 00 */
    { 0x2540, love2_GetDishes },    /* MSG_GET 11 00 00 */
    { 0x255F, love2_KirkAtDishes },    /* MSG_WALK_DONE 11 00 00 */
    { 0x257B, love2_TookDishes },    /* MSG_ANIM_DONE 19 00 00 */
    { 0x25F9, love2_GetSample },    /* MSG_GET 12 00 00 */
    { 0x2618, love2_KirkAtSample },    /* MSG_WALK_DONE 13 00 00 */
    { 0x2634, love2_TookSample },    /* MSG_ANIM_DONE 1C 00 00 */
    { 0x26B6, love2_GetGas },    /* MSG_GET 0F 00 00 */
    { 0x26D5, love2_KirkAtGas },    /* MSG_WALK_DONE 0A 00 00 */
    { 0x26F1, love2_TookGas },    /* MSG_ANIM_DONE 17 00 00 */
    { 0x2888, love2_GetCure },    /* MSG_GET 13 00 00 */
    { 0x28A7, love2_KirkAtCure },    /* MSG_WALK_DONE 0B 00 00 */
    { 0x28C3, love2_TookCure },    /* MSG_ANIM_DONE 18 00 00 */
    { 0x296E, love2_CrewLaugh },    /* MSG_TIMER 00 00 00 */
    { 0x2B8B, love2_SpockLaughs },    /* MSG_TIMER 01 00 00 */
    { 0x2C1D, love2_MedkitOnSpock },    /* MSG_USE 7F 01 00 */
    /* 2CAB: a helper, not a script */
    /* 2D1E: a helper, not a script */
    { 0x2D9A, love2_AtSpock },    /* MSG_WALK_DONE 63 00 00 */
    { 0x2DC4, love2_SpockCured },    /* MSG_ANIM_DONE 63 00 00 */
    { 0x2E83, love2_VirusProgresses },    /* MSG_TIMER 02 00 00 */
    { 0x3039, love2_McCoyScansSpock },    /* MSG_USE 45 01 00 */
    { 0x32AA, love2_E_32AA },    /* MSG_USE 45 00 00 */
    { 0x32B2, love2_McCoyScansMcCoy },    /* MSG_USE 45 02 00 */
    { 0x32BA, love2_McCoyScansCrew },    /* MSG_USE 45 03 00 */
    { 0x3301, love2_ReleaseRLG },    /* MSG_USE 49 FF 00 */
    { 0x3351, love2_ReleaseN2O },    /* MSG_USE 65 FF 00 */
    { 0x33E3, love2_ReleaseNH3 },    /* MSG_USE 66 FF 00 */
    { 0x3449, love2_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0, 0 }
};
