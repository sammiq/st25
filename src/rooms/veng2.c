/* VENG2.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/veng2.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * VENG2.RDF - "Vengeance", the Republic's auxiliary control room. With
 * impulse power (room 6) and Spock's linked tricorders as a computer, the
 * power can go to weapons, shields or the transporter. The Elasi uncloak
 * and demand data; Kirk stalls, clears and loads a torpedo (room 8), lures
 * them into lowering their shields and fires. The Enterprise returns.
 *
 * Actors: 8 the record decks, 9/0Ah (10) the science/medical tricorders
 * wired in, 0Bh (11) the viewscreen, 0Ch (12) the shields, 0Dh (13) both
 * tricorders, 0Eh (14) the torpedo light, 0Fh-15h (15-21) the dead
 * consoles. Objects: those and 8 the decks, 0Bh/27h the viewscreen,
 * 0Ch/15h/28h the damage display, 0Dh/24h the computer, 0Eh/20h the
 * torpedo control, 0Fh/23h the power control, 10h/26h the helm, 12h/22h/25h
 * the deck slots, 21h the fire control. Exit 1: the corridor.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - Both conversation menus end in a debug line from a speaker "wes",
 * "kirk dialog error." and "power allocation error.", for a choice that
 * cannot happen.
 * - The script chain runs on into code at 4F85: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50, which is before rdf_scripts_end, so
 * the search carries on from there: 2E50 (key 54 2E EB 06), 020C (key FF
 * 1E 00 00), then next C483 ends it.
 * - The conversation at 00CB (comm_talk) is built at run time.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    comm_talk = 0x00CB,
    N_s7r2d1 = 0x0109,
    N_s7r2t1 = 0x013A,
    N_s7r2t2 = 0x0169,
    N_s7r2t3 = 0x018D,
    N_s7r2u2 = 0x01B1,
    N_s7r2sh2 = 0x01D6,
    N_s7r2pl = 0x01F6,
    N_s7r2pm = 0x021B,
    N_s7r2o1 = 0x0243,
    N_s7r2o2 = 0x0262,
    N_s7r2o3 = 0x0280,
    N_s7r2o4 = 0x029E,
    N_s7r2o5 = 0x02BD,
    N_s7r2o6 = 0x02DC,
    N_s7r2o7 = 0x02FC,
    N_VEN2LOFF = 0x031B,
    N_VEN2LON = 0x0337,
    N_UNCLOAK = 0x0393,
    N_s7r2u1 = 0x03AC,
    T_VEN2_093 = 0x03E3,
    T_VEN2_019 = 0x041B,
    T_VEN2_076 = 0x044C,
    T_VEN2_042 = 0x0473,
    T_VEN2_032 = 0x04AE,
    T_VEN2_073 = 0x0508,
    T_VEN2_016 = 0x0555,
    N_s7r2p1 = 0x057F,
    T_VEN2_109 = 0x05A7,
    T_VEN2_027 = 0x0621,
    T_VEN2_113 = 0x064D,
    T_VEN2_010 = 0x06D5,
    T_VEN2_103 = 0x0700,
    T_VEN2_006 = 0x073F,
    T_VEN2_111 = 0x079D,
    T_VEN2_020 = 0x081A,
    T_VEN2_112 = 0x0884,
    N_kirk_dialog_error = 0x092A,
    T_VEN2_099 = 0x0949,
    T_VEN2_008 = 0x099D,
    T_VEN2_105 = 0x09C9,
    T_VEN2_021 = 0x0A3B,
    T_VEN2_102 = 0x0AA6,
    T_VEN2_072 = 0x0B6F,
    T_VEN2_044 = 0x0BB5,
    T_VEN2_022 = 0x0BDF,
    N_s7r2u2_2 = 0x0C10,
    T_VEN2N022 = 0x0C71,
    T_VEN2N002 = 0x0CEF,
    T_VEN2N030 = 0x0D90,
    T_VEN2N035 = 0x0E62,
    T_VEN2N032 = 0x0EE9,
    T_VEN2N027 = 0x0F74,
    T_VEN2N024 = 0x0FD9,
    T_VEN2N033 = 0x1046,
    T_VEN2N033_2 = 0x109A,
    T_VEN2N001 = 0x10EE,
    T_VEN2N000 = 0x112B,
    T_VEN2N021 = 0x11C2,
    T_VEN2N020 = 0x1238,
    T_VEN2N034 = 0x12A3,
    T_VEN2N008 = 0x12F1,
    T_VEN2N009 = 0x1356,
    T_VEN2N009_2 = 0x13C4,
    T_VEN2N007 = 0x1432,
    T_VEN2N005 = 0x1494,
    T_VEN2N006 = 0x14EC,
    T_VEN2N003 = 0x1560,
    T_VEN2N023 = 0x15BF,
    T_VEN2N027_2 = 0x1622,
    T_VEN2N025 = 0x167F,
    T_VEN2N029 = 0x1703,
    T_VEN2N031 = 0x1758,
    T_VEN2N031_2 = 0x17B0,
    T_VEN2N010 = 0x1808,
    T_VEN2N019 = 0x186C,
    T_VEN2N012 = 0x18B4,
    T_VEN2N004 = 0x18FC,
    T_VEN2N035_2 = 0x1955,
    T_VEN2_005 = 0x19E2,
    T_VEN2_004 = 0x1A2F,
    T_VEN4_017 = 0x1AA3,
    T_VEN2_075 = 0x1AEB,
    T_VEN2_009 = 0x1B23,
    T_VEN2_039 = 0x1BAC,
    T_VEN2_041 = 0x1BEB,
    T_VEN2_007 = 0x1C43,
    T_VEN2_086 = 0x1C7F,
    T_VEN2_094 = 0x1CC8,
    T_VEN2_078 = 0x1D66,
    T_VEN2_046 = 0x1DCE,
    N_susemn = 0x1E61,
    T_VEN2_070 = 0x1E93,
    T_VEN2_043 = 0x1F0D,
    T_VEN2_017 = 0x1F60,
    T_VEN2_081 = 0x1FA1,
    N_kusemn = 0x2007,
    N_COMPUTER = 0x202C,
    T_VEN2_082 = 0x2060,
    T_VEN2_061 = 0x2186,
    T_VEN2_098 = 0x21D3,
    S_222F = 0x222F,
    T_VEN2U093 = 0x226E,
    T_VEN2_028 = 0x22D9,
    N_Hail_Elasi = 0x2325,
    N_Hail_Enterprise = 0x2340,
    N_cancel = 0x2360,
    T_VEN2_066 = 0x239D,
    T_VEN2_040 = 0x2426,
    N_s7r2p1_2 = 0x24B9,
    T_VEN2_108 = 0x24E1,
    T_VEN2_015 = 0x2554,
    T_VEN2_029 = 0x25AF,
    T_VEN2_100 = 0x2660,
    N_s7r2u2_3 = 0x26B0,
    T_VEN2_114 = 0x26DA,
    T_VEN2_025 = 0x276D,
    N_s7r2u2_4 = 0x279B,
    T_VEN2N028 = 0x27E1,
    T_VEN2_058 = 0x2818,
    T_VEN2_054 = 0x288F,
    N_kusemn_2 = 0x293E,
    N_DBLWHAM2 = 0x2963,
    N_s7r2ph = 0x297D,
    N_s7r2p1_3 = 0x29A2,
    T_VEN2_104 = 0x29CB,
    T_VEN2_F29 = 0x2A5B,
    T_VEN2_F28 = 0x2AA6,
    T_VEN2_101 = 0x2AFA,
    T_VEN2_014 = 0x2B49,
    T_VEN2_106 = 0x2B79,
    N_s7r2rc = 0x2BC6,
    T_VEN2U094 = 0x2BF4,
    T_VEN2_026 = 0x2C3D,
    T_VEN2_115 = 0x2C70,
    T_VEN2_013 = 0x2D07,
    T_VEN2_S09 = 0x2DBA,
    T_VEN2_F35 = 0x2ED6,
    N_sscann = 0x2F24,
    T_VEN2N030_2 = 0x2F5C,
    T_VEN2_001 = 0x3025,
    N_sscann_2 = 0x3073,
    T_VEN2_062 = 0x30BD,
    T_VEN2_064 = 0x3194,
    T_VEN2_055 = 0x31F1,
    T_VEN2_065 = 0x3275,
    T_VEN2_068 = 0x32CC,
    T_VEN2_050 = 0x3376,
    T_VEN2_003 = 0x33E8,
    T_VEN2_063 = 0x3439,
    T_VEN2_003_2 = 0x349B,
    T_VEN2_057 = 0x34EB,
    N_susemn_2 = 0x3573,
    T_VEN2_069 = 0x35A5,
    T_VEN2_035 = 0x365B,
    T_VEN2_091 = 0x36F6,
    N_LD6BMOFF = 0x3744,
    N_s7r2sh3 = 0x375E,
    T_VEN2_088 = 0x378B,
    T_VEN2_052 = 0x37DC,
    T_VEN2_033 = 0x389B,
    T_VEN2_089 = 0x38F1,
    T_VEN2_085 = 0x3948,
    N_s7r2sh = 0x399F,
    T_VEN2_092 = 0x39D6,
    T_VEN2_034 = 0x3A10,
    T_VEN2_018 = 0x3AAD,
    T_VEN2_077 = 0x3B1A,
    S_3BA9 = 0x3BA9,
    N_s7r2sh3_2 = 0x3C02,
    T_VEN2_087 = 0x3C2F,
    T_VEN2_071 = 0x3C82,
    S_3CD2 = 0x3CD2,
    T_VEN2_002 = 0x3D14,
    N_sscanw = 0x3D64,
    T_VEN2_079 = 0x3D9C,
    T_VEN2_024 = 0x3E37,
    T_VEN2_080 = 0x3E78,
    T_VEN2_045 = 0x3EFF,
    T_VEN2_074 = 0x3F6D,
    T_VEN2_096 = 0x4052,
    T_VEN2_059 = 0x40CA,
    T_VEN2_031 = 0x4165,
    N_susemn_3 = 0x4235,
    N_MUR4E1 = 0x425A,
    N_s7r2t1_2 = 0x429A,
    T_VEN2N017 = 0x42C1,
    T_VEN2_038 = 0x435B,
    N_susemn_4 = 0x43BC,
    N_MUR4E1_2 = 0x43E1,
    N_s7r2t2_2 = 0x4421,
    T_VEN2N016 = 0x4448,
    N_s7r2t3_2 = 0x44C2,
    T_VEN2_030 = 0x44EA,
    N_sscane = 0x45A1,
    T_VEN2_067 = 0x45D8,
    N_sscanw_2 = 0x4642,
    T_VEN2_056 = 0x467F,
    T_VEN2_051 = 0x46FA,
    T_VEN2_060 = 0x478B,
    N_kuseln = 0x4849,
    T_VEN2N039 = 0x488F,
    T_VEN2_047 = 0x48E5,
    N_susemn_5 = 0x49C5,
    T_VEN2N015 = 0x4A53,
    T_VEN2_047_2 = 0x4A97,
    N_susemn_6 = 0x4B77,
    T_VEN2N013 = 0x4C05,
    T_VEN2_098_2 = 0x4C78,
    T_VENA_F40 = 0x4CD7,
    T_VENA_F32 = 0x4D5E,
    T_VENA_F34 = 0x4E6C,
    T_VEN4N010 = 0x4F06,
    T_VEN6N007 = 0x4F47,
    N_Capt_Kirk = 0x5074,
    N_Dr_McCoy = 0x507F,
    N_Mr_Spock = 0x5089,
    N_Mr_Scott = 0x5093,
    N_Ensign_Chekov = 0x509D,
    N_Lt_Uhura = 0x50AB,
    S_50BE = 0x50BE,
    S_50D0 = 0x50D0,
    N_wes = 0x50E4,
    N_Ensign_Kije = 0x50E8,
    N_Elasi_Captain = 0x50F4,
    N_Elasi_Weapons_Master = 0x5102,
    N_Elasi_Crewman = 0x5117,
    S_5125 = 0x5125,
    T_VEN2_083 = 0x520B,
    T_VEN2_084 = 0x5308,
    T_VEN2_097 = 0x5462,
    T_VEN2_036 = 0x5980,
    T_VEN2_037 = 0x5AA7,
    T_VEN2N032_2 = 0x5BAE,
    talk_buf = 0x5C10,
    DefyElasi = 0x5C16,
    PowerChoices = 0x5C1E,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_beamed CS_B(0xCA)
#define fv_choice_jumps CS_W(0xE5)
#define fv_comm_talk CS_W(0xCB)
#define fv_talk_buf CS_W(0x5C10)
#define fv_talk_choice0 CS_W(0xCD)
#define fv_talk_choice1 CS_W(0xCF)
#define fv_talk_choice2 CS_W(0xD1)
#define fv_talk_end CS_W(0xD3)
#define rv_attack_timer ROOM_W(0x51)
#define rv_board_timer ROOM_W(0x49)
#define rv_clue_andrade ROOM_B(0x6A)
#define rv_clue_crewman ROOM_B(0x68)
#define rv_clue_log ROOM_B(0x6B)
#define rv_clue_marata ROOM_B(0x69)
#define rv_clue_records ROOM_B(0x6C)
#define rv_complied ROOM_B(0x45)
#define rv_comply_timer ROOM_W(0x4D)
#define rv_decks_taken ROOM_B(0x48)
#define rv_elasi_here ROOM_B(0x43)
#define rv_elasi_open ROOM_B(0x40)
#define rv_found_fusion ROOM_B(0x5A)
#define rv_hailed ROOM_B(0x47)
#define rv_hypo_oiled ROOM_B(0x3A)
#define rv_know_jam ROOM_B(0x66)
#define rv_logs_played ROOM_B(0x3E)
#define rv_need_tricorders ROOM_B(0x46)
#define rv_power_on ROOM_B(0x34)
#define rv_power_to ROOM_B(0x3C)
#define rv_read_journals ROOM_B(0x61)
#define rv_rescued ROOM_B(0x42)
#define rv_score ROOM_W(0x2E)
#define rv_stall_timer ROOM_W(0x4B)
#define rv_stalled ROOM_B(0x44)
#define rv_torpedo_loaded ROOM_B(0x41)
#define rv_torpedo_timer ROOM_W(0x4F)
#define rv_tricorders ROOM_B(0x3D)

/* functions */
static void veng2_StartRoom(void);
static void veng2_E_0355(void);
static void veng2_ElasiUncloak(void);
static void veng2_ElasiSeen(void);
static void veng2_ElasiHail(void);
static void veng2_ToCorridor(void);
static void veng2_E_0C58(void);
static void veng2_LookAtViewscreen(void);
static void veng2_E_0D7C(void);
static void veng2_LookAtHelm(void);
static void veng2_E_0E3E(void);
static void veng2_LookAtDamage(void);
static void veng2_LookAtDamage2(void);
static void veng2_LookAtSciTricorder(void);
static void veng2_LookAtMedTricorder(void);
static void veng2_E_1033(void);
static void veng2_LookAtTorpedoes(void);
static void veng2_LookAtFireControl(void);
static void veng2_E_1290(void);
static void veng2_LookAtPowerControl(void);
static void veng2_E_1547(void);
static void veng2_LookAtComputer(void);
static void veng2_LookAtDecks(void);
static void veng2_E_1745(void);
static void veng2_LookAtDeckSlot(void);
static void veng2_LookAtDeckSlot2(void);
static void veng2_LookAtKirk(void);
static void veng2_LookAtSpock(void);
static void veng2_LookAtMcCoy(void);
static void veng2_LookAtRedshirt(void);
static void veng2_LookAtRoom(void);
static void veng2_TalkToKirk(void);
static void veng2_TalkToSpock(void);
static void veng2_TalkToMcCoy(void);
static void veng2_TalkToRedshirt(void);
static void veng2_E_1DAD(void);
static void veng2_JournalsInSlot(void);
static void veng2_JournalsInSlot2(void);
static void veng2_SpockAtSlot(void);
static void veng2_ReadJournals(void);
static void veng2_E_1FCE(void);
static void veng2_DeckInSlot(void);
static void veng2_DeckInSlot2(void);
static void veng2_KirkAtSlot(void);
static void veng2_PlayLog(void);
static void veng2_CallEnterprise(void);
static void veng2_E_27C8(void);
static void veng2_FireTorpedo(void);
static void veng2_KirkAtFireControl(void);
static void veng2_Fired(void);
static void veng2_ElasiHit(void);
static void veng2_ElasiGone(void);
static void veng2_E_2E52(void);
static void veng2_BeamOut2(void);
static void veng2_BeamOut3(void);
static void veng2_BeamOut(void);
static void veng2_SpockScansTorpedoes(void);
static void veng2_E_2F1A(void);
static void veng2_SpockScansHelm(void);
static void veng2_E_300A(void);
static void veng2_ScanFireControl(void);
static void veng2_SpockOnFireControl(void);
static void veng2_E_33BC(void);
static void veng2_SpockOnPower(void);
static void veng2_KirkOnPower2(void);
static void veng2_AllocatePower(void);
static void veng2_SpockAtPower(void);
static void veng2_PowerTo(void);
static void veng2_E_3CF1(void);
static void veng2_SpockOnComputer(void);
static void veng2_KirkOnComputer2(void);
static void veng2_AskAboutComputer(void);
static void veng2_E_413F(void);
static void veng2_SciTricorderOnComputer(void);
static void veng2_SpockAtComputer(void);
static void veng2_WiredSciTricorder(void);
static void veng2_E_4342(void);
static void veng2_MedTricorderOnComputer(void);
static void veng2_SpockAtComputer2(void);
static void veng2_WiredMedTricorder(void);
static void veng2_part_44C0(void);
static void veng2_E_4597(void);
static void veng2_SpockScansDeckSlot(void);
static void veng2_E_4638(void);
static void veng2_SpockScansPower(void);
static void veng2_E_4777(void);
static void veng2_FusionOnComputer(void);
static void veng2_GetDecks(void);
static void veng2_KirkAtDecks(void);
static void veng2_TookDecks(void);
static void veng2_GetSciTricorder(void);
static void veng2_SpockAtComputer3(void);
static void veng2_UnwiredSciTricorder(void);
static void veng2_GetMedTricorder(void);
static void veng2_SpockAtComputer4(void);
static void veng2_UnwiredMedTricorder(void);
static void veng2_ElasiDeadlines(void);
static void veng2_E_4E58(void);
static void veng2_KillAnything(void);
static void veng2_LookAtHypo(void);
static uint16_t veng2_Talk(uint16_t ax);
static void veng2_Say(uint16_t ax, uint16_t bx);

/* The room starts: the decks, the wired tricorders, the Elasi on the
 * viewscreen, the shields, the torpedo light, and the dead consoles unless
 * there is power.
 * [MSG_TICK 01 00 00] */
static void veng2_StartRoom(void)
{
    uint16_t ax = 0;
    if (rv_decks_taken != 1) {
        svc_Anim(8, N_s7r2d1, 0xEE, 0x8C, 0);
    }
    ax = (uint16_t)svc_HaveItem(IT_STRICOR);
    if (ax != 1) {
        svc_Anim(9, N_s7r2t1, 0x45, 0x71, 0);
    }
    ax = (uint16_t)svc_HaveItem(IT_MTRICOR);
    if (ax != 1) {
        svc_Anim(0xA, N_s7r2t2, 0x55, 0x67, 0);
    }
    if (rv_tricorders == 3) {
        svc_Anim(0xD, N_s7r2t3, 0x4F, 0x71, 0);
    }
    if (rv_elasi_here == 1) {
        svc_Anim(0xB, N_s7r2u2, 0x9E, 0x63, 0);
    }
    if (rv_power_to == 2) {
        svc_Anim(0xC, N_s7r2sh2, 0xFA, 0x64, 0);
    }
    svc_Anim(0xE, N_s7r2pl, 0x97, 0x74, 0);
    if (rv_torpedo_loaded == 1) {
        svc_Anim(0xE, N_s7r2pm, 0x97, 0x74, 0);
    }
    if (rv_power_on != 0) {
    } else {
        svc_Anim(0xF, N_s7r2o1, 0xB, 0x87, 0);
        svc_Anim(0x14, N_s7r2o2, 0x36, 0x63, 0);
        svc_Anim(0x11, N_s7r2o3, 0x58, 0x5F, 0);
        svc_Anim(0x10, N_s7r2o4, 0x9E, 0x7B, 0);
        svc_Anim(0x15, N_s7r2o5, 0x112, 0x66, 0);
        svc_Anim(0x12, N_s7r2o6, 0x131, 0x86, 0);
        svc_Anim(0x13, N_s7r2o7, 0x9B, 0x14, 0);
        svc_Sfx(N_VEN2LOFF);
        goto L034E;
    }
    svc_Sfx(N_VEN2LON);
L034E:;
    return;
}

/* Timers 0 and 28h, once there is power and the tricorders run the
 * computer: the Elasi uncloak (msg 5). Once they are here, raising the
 * shields starts timer 2: they hail.
 * [MSG_TIMER 00 00 00] */
static void veng2_E_0355(void)
{
    { veng2_ElasiUncloak(); return; }
}

/* [MSG_TICK 28 00 00] */
static void veng2_ElasiUncloak(void)
{
    if (rv_tricorders != 3) {
        return;
    }
    if (rv_power_on != 1) {
        return;
    }
    if (rv_elasi_here == 1) {
        if (rv_hailed != 1) {
            if (rv_power_to == 2) {
                room.timers[2] = 0x2D;
            }
        }
        return;
    }
    rv_elasi_here = 1;
    svc_Sfx(N_UNCLOAK);
    svc_Anim(0xB, N_s7r2u1, 0x9E, 0x63, 5);
    room.busy = 1;
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void veng2_ElasiSeen(void)
{
    room.busy = 0;
    veng2_Say(N_Ensign_Kije, T_VEN2_093);
    veng2_Say(N_Capt_Kirk, T_VEN2_019);
    veng2_Say(N_Mr_Spock, T_VEN2_076);
    veng2_Say(N_Dr_McCoy, T_VEN2_042);
    if (rv_power_to == 2) {
        { veng2_ElasiHail(); return; }
    }
    veng2_Say(N_Mr_Spock, T_VEN2_032);
    rv_board_timer = 0x708;
    return;
}

/* Timer 2: the Elasi demand the locations of their imprisoned comrades
 * from the Republic's records. Kirk gets thirty minutes (stalled).
 * [MSG_TIMER 02 00 00] */
static void veng2_ElasiHail(void)
{
    uint16_t ax = 0;
    rv_hailed = 1;
    veng2_Say(N_Mr_Spock, T_VEN2_073);
    veng2_Say(N_Capt_Kirk, T_VEN2_016);
    svc_Anim(0xB, N_s7r2p1, 0x9E, 0x63, 0);
    veng2_Say(N_Elasi_Captain, T_VEN2_109);
    veng2_Say(N_Capt_Kirk, T_VEN2_027);
    veng2_Say(N_Elasi_Captain, T_VEN2_113);
    veng2_Say(N_Capt_Kirk, T_VEN2_010);
    veng2_Say(N_Elasi_Captain, T_VEN2_103);
    veng2_Say(N_Capt_Kirk, T_VEN2_006);
    veng2_Say(N_Elasi_Captain, T_VEN2_111);
    veng2_Say(N_Capt_Kirk, T_VEN2_020);
    veng2_Say(N_Elasi_Captain, T_VEN2_112);
    ax = (uint16_t)veng2_Talk(DefyElasi);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L09BE;
        }
        veng2_Say(N_wes, N_kirk_dialog_error);
        return;
    }
    veng2_Say(N_Elasi_Captain, T_VEN2_099);
    veng2_Say(N_Capt_Kirk, T_VEN2_008);
L09BE:;
    veng2_Say(N_Elasi_Captain, T_VEN2_105);
    veng2_Say(N_Capt_Kirk, T_VEN2_021);
    veng2_Say(N_Elasi_Captain, T_VEN2_102);
    veng2_Say(N_Mr_Spock, T_VEN2_072);
    veng2_Say(N_Dr_McCoy, T_VEN2_044);
    veng2_Say(N_Capt_Kirk, T_VEN2_022);
    svc_Anim(0xB, N_s7r2u2_2, 0x9E, 0x63, 0);
    rv_stall_timer = 0x6978;
    rv_stalled = 1;
    return;
}

/* Exit 1: to the corridor (room 1).
 * [MSG_EXIT 01 00 00] */
static void veng2_ToCorridor(void)
{
    svc_BridgeSfx(5);
    room.more = 1;
    return;
}

/* Looking.
 * [MSG_LOOK 0B 00 00] */
static void veng2_E_0C58(void)
{
    { veng2_LookAtViewscreen(); return; }
}

/* [MSG_LOOK 27 00 00] */
static void veng2_LookAtViewscreen(void)
{
    if (rv_elasi_here != 1) {
        veng2_Say(0, T_VEN2N022);
        return;
    }
    veng2_Say(0, T_VEN2N002);
    return;
}

/* [MSG_LOOK 10 00 00] */
static void veng2_E_0D7C(void)
{
    { veng2_LookAtHelm(); return; }
}

/* [MSG_LOOK 26 00 00] */
static void veng2_LookAtHelm(void)
{
    veng2_Say(0, T_VEN2N030);
    return;
}

/* [MSG_LOOK 0C 00 00] */
static void veng2_E_0E3E(void)
{
    { veng2_LookAtDamage(); return; }
}

/* [MSG_LOOK 15 00 00] */
static void veng2_LookAtDamage(void)
{
    { veng2_LookAtDamage2(); return; }
}

/* [MSG_LOOK 28 00 00] */
static void veng2_LookAtDamage2(void)
{
    if (rv_power_on == 1) {
    } else {
        veng2_Say(0, T_VEN2N035);
        return;
    }
    if (rv_power_to != 2) {
        veng2_Say(0, T_VEN2N032);
        return;
    }
    veng2_Say(S_5125, T_VEN2N032_2);
    return;
}

/* [MSG_LOOK 09 00 00] */
static void veng2_LookAtSciTricorder(void)
{
    veng2_Say(0, T_VEN2N027);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void veng2_LookAtMedTricorder(void)
{
    veng2_Say(0, T_VEN2N024);
    return;
}

/* [MSG_LOOK 0E 00 00] */
static void veng2_E_1033(void)
{
    { veng2_LookAtTorpedoes(); return; }
}

/* [MSG_LOOK 20 00 00] */
static void veng2_LookAtTorpedoes(void)
{
    veng2_Say(0, T_VEN2N033);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void veng2_LookAtFireControl(void)
{
    veng2_Say(0, T_VEN2N033_2);
    if (rv_power_on == 0) {
        veng2_Say(0, T_VEN2N001);
        return;
    }
    if (rv_torpedo_loaded != 0) {
    } else {
        veng2_Say(0, T_VEN2N000);
        if (rv_elasi_here != 1) {
            veng2_Say(N_Mr_Spock, T_VEN2_036);
            rv_know_jam = 1;
            return;
        }
        veng2_Say(N_Mr_Spock, T_VEN2_037);
        rv_know_jam = 1;
        return;
    }
    if (rv_rescued != 1) {
        veng2_Say(0, T_VEN2N021);
        return;
    }
    veng2_Say(0, T_VEN2N020);
    return;
}

/* [MSG_LOOK 0F 00 00] */
static void veng2_E_1290(void)
{
    { veng2_LookAtPowerControl(); return; }
}

/* [MSG_LOOK 23 00 00] */
static void veng2_LookAtPowerControl(void)
{
    veng2_Say(0, T_VEN2N034);
    if (rv_power_on != 1) {
        veng2_Say(0, T_VEN2N008);
        return;
    }
    if (rv_tricorders != 3) {
        veng2_Say(0, T_VEN2N009);
        return;
    }
    if (rv_power_to == 0) {
        veng2_Say(0, T_VEN2N009_2);
        return;
    }
    if (rv_power_to == 1) {
        veng2_Say(0, T_VEN2N007);
        return;
    }
    if (rv_power_to == 2) {
        veng2_Say(0, T_VEN2N005);
        return;
    }
    veng2_Say(0, T_VEN2N006);
    return;
}

/* [MSG_LOOK 0D 00 00] */
static void veng2_E_1547(void)
{
    { veng2_LookAtComputer(); return; }
}

/* [MSG_LOOK 24 00 00] */
static void veng2_LookAtComputer(void)
{
    if (rv_tricorders == 0) {
        veng2_Say(0, T_VEN2N003);
        return;
    }
    if (rv_tricorders == 1) {
        veng2_Say(0, T_VEN2N023);
        return;
    }
    if (rv_tricorders == 2) {
        veng2_Say(0, T_VEN2N027_2);
        return;
    }
    veng2_Say(0, T_VEN2N025);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void veng2_LookAtDecks(void)
{
    veng2_Say(0, T_VEN2N029);
    return;
}

/* [MSG_LOOK 12 00 00] */
static void veng2_E_1745(void)
{
    { veng2_LookAtDeckSlot(); return; }
}

/* [MSG_LOOK 25 00 00] */
static void veng2_LookAtDeckSlot(void)
{
    veng2_Say(0, T_VEN2N031);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void veng2_LookAtDeckSlot2(void)
{
    veng2_Say(0, T_VEN2N031_2);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void veng2_LookAtKirk(void)
{
    veng2_Say(0, T_VEN2N010);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void veng2_LookAtSpock(void)
{
    veng2_Say(0, T_VEN2N019);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void veng2_LookAtMcCoy(void)
{
    veng2_Say(0, T_VEN2N012);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void veng2_LookAtRedshirt(void)
{
    veng2_Say(0, T_VEN2N004);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void veng2_LookAtRoom(void)
{
    veng2_Say(0, T_VEN2N035_2);
    return;
}

/* [MSG_TALK 00 00 00] */
static void veng2_TalkToKirk(void)
{
    if (rv_stalled != 1) {
        veng2_Say(N_Capt_Kirk, T_VEN2_005);
        return;
    }
    veng2_Say(N_Capt_Kirk, T_VEN2_004);
    return;
}

/* [MSG_TALK 01 00 00] */
static void veng2_TalkToSpock(void)
{
    if (rv_stalled != 1) {
        veng2_Say(N_Mr_Spock, T_VEN4_017);
        return;
    }
    veng2_Say(N_Mr_Spock, T_VEN2_075);
    veng2_Say(N_Capt_Kirk, T_VEN2_009);
    return;
}

/* [MSG_TALK 02 00 00] */
static void veng2_TalkToMcCoy(void)
{
    if (rv_stalled != 1) {
        veng2_Say(N_Dr_McCoy, T_VEN2_039);
        return;
    }
    veng2_Say(N_Dr_McCoy, T_VEN2_041);
    veng2_Say(N_Capt_Kirk, T_VEN2_007);
    return;
}

/* [MSG_TALK 03 00 00] */
static void veng2_TalkToRedshirt(void)
{
    if (rv_stalled != 1) {
        veng2_Say(N_Ensign_Kije, T_VEN2_086);
        return;
    }
    veng2_Say(N_Ensign_Kije, T_VEN2_094);
    veng2_Say(N_Mr_Spock, T_VEN2_078);
    return;
}

/* The engineering journals in the deck slot (msgs 16h, 17h): Spock
 * learns to power the systems through linked tricorders.
 * [MSG_USE 88 12 00] */
static void veng2_E_1DAD(void)
{
    { veng2_JournalsInSlot(); return; }
}

/* [MSG_USE 88 25 00] */
static void veng2_JournalsInSlot(void)
{
    { veng2_JournalsInSlot2(); return; }
}

/* [MSG_USE 88 22 00] */
static void veng2_JournalsInSlot2(void)
{
    if (rv_read_journals == 1) {
        veng2_Say(N_Mr_Spock, T_VEN2_046);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x112, 0x98, 0x16);
    return;
}

/* [MSG_WALK_DONE 16 00 00] */
static void veng2_SpockAtSlot(void)
{
    svc_Anim(1, N_susemn, -1, -1, 0x17);
    return;
}

/* [MSG_ANIM_DONE 17 00 00] */
static void veng2_ReadJournals(void)
{
    room.busy = 0;
    veng2_Say(N_Mr_Spock, T_VEN2_070);
    veng2_Say(N_Dr_McCoy, T_VEN2_043);
    veng2_Say(N_Capt_Kirk, T_VEN2_017);
    veng2_Say(N_Mr_Spock, T_VEN2_081);
    rv_read_journals = 1;
    return;
}

/* The log decks in the slot (msgs 18h, 19h): each play is the next
 * entry; the fourth, Andrade's, says the Enterprise attacked (evidence).
 * [MSG_USE 87 12 00] */
static void veng2_E_1FCE(void)
{
    { veng2_DeckInSlot(); return; }
}

/* [MSG_USE 87 25 00] */
static void veng2_DeckInSlot(void)
{
    { veng2_DeckInSlot2(); return; }
}

/* [MSG_USE 87 22 00] */
static void veng2_DeckInSlot2(void)
{
    room.busy = 1;
    svc_Walk(0, 0x112, 0x98, 0x18);
    return;
}

/* [MSG_WALK_DONE 18 00 00] */
static void veng2_KirkAtSlot(void)
{
    svc_Anim(0, N_kusemn, -1, -1, 0x19);
    return;
}

/* [MSG_ANIM_DONE 19 00 00] */
static void veng2_PlayLog(void)
{
    svc_Sfx(N_COMPUTER);
    room.busy = 0;
    rv_logs_played++;
    if (rv_logs_played != 1) {
    } else {
        veng2_Say(S_50BE, T_VEN2_082);
        return;
    }
    if (rv_logs_played == 2) {
        veng2_Say(S_50BE, T_VEN2_083);
        return;
    }
    if (rv_logs_played == 3) {
        veng2_Say(S_50BE, T_VEN2_084);
        return;
    }
    if (rv_logs_played == 4) {
        veng2_Say(S_50D0, T_VEN2_097);
        rv_clue_andrade = 1;
    }
    veng2_Say(N_Mr_Spock, T_VEN2_061);
    return;
}

/* The communicator: jammed once the Elasi are here. After stalling,
 * Kirk can hail them: if the torpedo is loaded he tells them to beam over
 * for the data, and they must lower their shields to do it (complied).
 * [MSG_USE 47 00 00] */
static void veng2_CallEnterprise(void)
{
    uint16_t ax = 0, si = 0;
    if (rv_complied == 1) {
        veng2_Say(N_Elasi_Captain, T_VEN2_098);
        return;
    }
    if (rv_stalled == 1) {
    } else {
        if (rv_elasi_here == 1) {
            goto L2392;
        }
        veng2_Say(N_Capt_Kirk, S_222F);
        veng2_Say(N_Lt_Uhura, T_VEN2U093);
        veng2_Say(N_Capt_Kirk, T_VEN2_028);
        return;
    }
    fv_comm_talk = N_Capt_Kirk;
    fv_talk_choice0 = N_Hail_Elasi;
    fv_choice_jumps = 0x240C;
    fv_talk_choice1 = N_Hail_Enterprise;
    CS_W(0xE7) = 0x2392;
    fv_talk_choice2 = N_cancel;
    CS_W(0xE9) = 0x2406;
    fv_talk_end = 0;
    ax = (uint16_t)svc_Talk(comm_talk);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xE5 + si)) {
    case 0x2392: goto L2392;
    case 0x2406: goto L2406;
    case 0x240C: goto L240C;
    case 0x2650: goto L2650;
    case 0x26CE: goto L26CE;
    default: return;
    }
L2392:;
    veng2_Say(N_Mr_Spock, T_VEN2_066);
L2406:;
    return;
L240C:;
    if (rv_torpedo_loaded == 1) {
    } else {
        veng2_Say(N_Dr_McCoy, T_VEN2_040);
        return;
    }
    svc_Anim(0xB, N_s7r2p1_2, 0x9E, 0x63, 0);
    veng2_Say(N_Elasi_Captain, T_VEN2_108);
    fv_comm_talk = N_Capt_Kirk;
    fv_talk_choice0 = T_VEN2_015;
    fv_choice_jumps = 0x2650;
    fv_talk_choice1 = T_VEN2_029;
    CS_W(0xE7) = 0x26CE;
    fv_talk_choice2 = 0;
    ax = (uint16_t)svc_Talk(comm_talk);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xE5 + si)) {
    case 0x2392: goto L2392;
    case 0x2406: goto L2406;
    case 0x240C: goto L240C;
    case 0x2650: goto L2650;
    case 0x26CE: goto L26CE;
    default: return;
    }
L2650:;
    veng2_Say(N_Elasi_Captain, T_VEN2_100);
    svc_Anim(0xB, N_s7r2u2_3, 0x9E, 0x63, 0);
    return;
L26CE:;
    veng2_Say(N_Elasi_Captain, T_VEN2_114);
    veng2_Say(N_Capt_Kirk, T_VEN2_025);
    svc_Anim(0xB, N_s7r2u2_4, 0x9E, 0x63, 0);
    rv_comply_timer = 0x708;
    rv_complied = 1;
    return;
}

/* The fire control (msgs 12h, 13h, anims 7, 6): with power to weapons,
 * the torpedo loaded and the Elasi shields down, Kirk cripples their ship;
 * the Enterprise returns (rescued) and beams the party up (msgs 1-4).
 * [MSG_USE 00 0E 00] */
static void veng2_E_27C8(void)
{
    { veng2_FireTorpedo(); return; }
}

/* [MSG_USE 00 20 00] */
static void veng2_FireTorpedo(void)
{
    if (rv_power_to != 1) {
        veng2_Say(0, T_VEN2N028);
        return;
    }
    if (rv_torpedo_loaded != 1) {
        veng2_Say(N_Mr_Spock, T_VEN2_058);
        return;
    }
    if (rv_elasi_open == 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_054);
        return;
    }
    room.busy = 1;
    svc_Walk(0, 0x90, 0x95, 0x12);
    room.fall[0] = 0;
    return;
}

/* [MSG_WALK_DONE 12 00 00] */
static void veng2_KirkAtFireControl(void)
{
    svc_Anim(0, N_kusemn_2, -1, -1, 0x13);
    return;
}

/* [MSG_ANIM_DONE 13 00 00] */
static void veng2_Fired(void)
{
    svc_Sfx(N_DBLWHAM2);
    svc_Anim(0xB, N_s7r2ph, 0, 0, 7);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void veng2_ElasiHit(void)
{
    svc_Anim(0xB, N_s7r2p1_3, 0x9E, 0x63, 0);
    veng2_Say(N_Elasi_Captain, T_VEN2_104);
    veng2_Say(N_Elasi_Weapons_Master, T_VEN2_F29);
    veng2_Say(N_Elasi_Crewman, T_VEN2_F28);
    veng2_Say(N_Elasi_Captain, T_VEN2_101);
    veng2_Say(N_Capt_Kirk, T_VEN2_014);
    veng2_Say(N_Elasi_Captain, T_VEN2_106);
    svc_Anim(0xB, N_s7r2rc, 0, 0, 6);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void veng2_ElasiGone(void)
{
    veng2_Say(N_Lt_Uhura, T_VEN2U094);
    veng2_Say(N_Capt_Kirk, T_VEN2_026);
    veng2_Say(N_Ensign_Chekov, T_VEN2_115);
    veng2_Say(N_Capt_Kirk, T_VEN2_013);
    veng2_Say(N_Mr_Scott, T_VEN2_S09);
    rv_rescued = 1;
    svc_Walk(0, 0xEC, 0x9D, 1);
    svc_Walk(1, 0xD5, 0x96, 2);
    svc_Walk(2, 0x101, 0x95, 3);
    svc_Walk(3, 0xEC, 0x8C, 4);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void veng2_E_2E52(void)
{
    { veng2_BeamOut2(); return; }
}

/* [MSG_WALK_DONE 02 00 00] */
static void veng2_BeamOut2(void)
{
    { veng2_BeamOut3(); return; }
}

/* [MSG_WALK_DONE 03 00 00] */
static void veng2_BeamOut3(void)
{
    { veng2_BeamOut(); return; }
}

/* Once all four have walked to the spot: the score is 16 plus one per
 * piece of evidence.
 * [MSG_WALK_DONE 04 00 00] */
static void veng2_BeamOut(void)
{
    fv_beamed++;
    if (fv_beamed != 4) {
        return;
    }
    rv_score = 0x10;
    if (rv_clue_crewman == 1) {
        rv_score += 1;
    }
    if (rv_clue_marata == 1) {
        rv_score += 1;
    }
    if (rv_clue_andrade == 1) {
        rv_score += 1;
    }
    if (rv_clue_log == 1) {
        rv_score += 1;
    }
    if (rv_clue_records == 1) {
        rv_score += 1;
    }
    svc_BeamUp(rv_score, 0x15, 0);
    room.busy = 0;
    return;
}

/* Spock's tricorder and the fire control.
 * [MSG_USE 44 0E 00] */
static void veng2_SpockScansTorpedoes(void)
{
    veng2_Say(N_Mr_Spock, T_VEN2_F35);
    return;
}

/* [MSG_USE 44 10 00] */
static void veng2_E_2F1A(void)
{
    { veng2_SpockScansHelm(); return; }
}

/* [MSG_USE 44 26 00] */
static void veng2_SpockScansHelm(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    veng2_Say(0, T_VEN2N030_2);
    return;
}

/* [MSG_USE 00 21 00] */
static void veng2_E_300A(void)
{
    { veng2_ScanFireControl(); return; }
}

/* [MSG_USE 44 21 00] */
static void veng2_ScanFireControl(void)
{
    { veng2_SpockOnFireControl(); return; }
}

/* [MSG_USE 01 21 00] */
static void veng2_SpockOnFireControl(void)
{
    veng2_Say(N_Capt_Kirk, T_VEN2_001);
    svc_Anim(1, N_sscann_2, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_power_to == 1) {
    } else {
        if (rv_need_tricorders == 1) {
        } else {
            veng2_Say(N_Mr_Spock, T_VEN2_062);
            return;
        }
        veng2_Say(N_Mr_Spock, T_VEN2_064);
        return;
    }
    if (rv_tricorders != 3) {
        veng2_Say(N_Mr_Spock, T_VEN2_055);
        return;
    }
    if (rv_power_to != 1) {
        veng2_Say(N_Mr_Spock, T_VEN2_065);
        return;
    }
    if (rv_torpedo_loaded != 0) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_068);
        if (rv_elasi_here != 1) {
            veng2_Say(N_Mr_Spock, T_VEN2_036);
            rv_know_jam = 1;
            return;
        }
        veng2_Say(N_Mr_Spock, T_VEN2_037);
        rv_know_jam = 1;
        return;
    }
    veng2_Say(N_Mr_Spock, T_VEN2_050);
    return;
}

/* The power control (msgs 14h, 15h): once impulse power and the
 * tricorders are in, Spock allocates it to weapons, shields or the
 * transporter, which the Elasi watch.
 * [MSG_USE 00 0F 00] */
static void veng2_E_33BC(void)
{
    { veng2_SpockOnPower(); return; }
}

/* [MSG_USE 01 0F 00] */
static void veng2_SpockOnPower(void)
{
    { veng2_KirkOnPower2(); return; }
}

/* [MSG_USE 00 23 00] */
static void veng2_KirkOnPower2(void)
{
    { veng2_AllocatePower(); return; }
}

/* [MSG_USE 01 23 00] */
static void veng2_AllocatePower(void)
{
    if (rv_power_on != 0) {
    } else {
        veng2_Say(N_Capt_Kirk, T_VEN2_003);
        veng2_Say(N_Mr_Spock, T_VEN2_063);
        return;
    }
    if (rv_tricorders == 3) {
    } else {
        veng2_Say(N_Capt_Kirk, T_VEN2_003_2);
        veng2_Say(N_Mr_Spock, T_VEN2_057);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x22, 0x9B, 0x14);
    return;
}

/* [MSG_WALK_DONE 14 00 00] */
static void veng2_SpockAtPower(void)
{
    svc_Anim(1, N_susemn_2, -1, -1, 0x15);
    return;
}

/* [MSG_ANIM_DONE 15 00 00] */
static void veng2_PowerTo(void)
{
    uint16_t ax = 0;
    room.busy = 0;
    veng2_Say(N_Mr_Spock, T_VEN2_069);
    ax = (uint16_t)veng2_Talk(PowerChoices);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L38E0;
        }
        if (ax == 2) {
            goto L39FB;
        }
        if (ax == 3) {
            return;
        }
        goto L3CC7;
    }
    if (rv_complied == 1) {
    } else {
        if (rv_stalled != 1) {
        } else {
            veng2_Say(N_Mr_Spock, T_VEN2_035);
            return;
        }
        if (rv_power_to == 1) {
            veng2_Say(N_Ensign_Kije, T_VEN2_091);
            return;
        }
    }
    if (rv_power_to == 2) {
        svc_Sfx(N_LD6BMOFF);
        svc_Anim(0xC, N_s7r2sh3, 0xFA, 0x64, 0);
    }
    rv_power_to = 1;
    veng2_Say(N_Ensign_Kije, T_VEN2_088);
    if (rv_complied != 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_052);
        rv_elasi_open = 1;
        rv_attack_timer = 0x384;
    }
    if (rv_elasi_here == 1) {
        if (rv_hailed != 1) {
            veng2_Say(N_Mr_Spock, T_VEN2_033);
            rv_board_timer = 0x708;
        }
    }
    return;
L38E0:;
    if (rv_power_to == 2) {
        veng2_Say(N_Ensign_Kije, T_VEN2_089);
        return;
    }
    if (rv_complied == 1) {
        veng2_Say(N_Ensign_Kije, T_VEN2_085);
        return;
    }
    svc_Anim(0xC, N_s7r2sh, 0xFA, 0x64, 0);
    rv_power_to = 2;
    if (rv_elasi_here == 1) {
        room.timers[2] = 0x2D;
    }
    veng2_Say(N_Ensign_Kije, T_VEN2_092);
    return;
L39FB:;
    if (rv_stalled != 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_034);
        if (rv_found_fusion != 1) {
        } else {
            veng2_Say(N_Capt_Kirk, T_VEN2_018);
            veng2_Say(N_Mr_Spock, T_VEN2_077);
            goto L3B97;
        }
L3B97:;
        return;
    }
    if (rv_power_to == 3) {
        veng2_Say(N_Ensign_Kije, S_3BA9);
        return;
    }
    if (rv_power_to == 2) {
        svc_Anim(0xC, N_s7r2sh3_2, 0xFA, 0x64, 0);
    }
    rv_power_to = 3;
    veng2_Say(N_Ensign_Kije, T_VEN2_087);
    if (rv_elasi_here == 1) {
        if (rv_hailed != 1) {
            veng2_Say(N_Mr_Spock, T_VEN2_071);
            rv_board_timer = 0x708;
        }
    }
    return;
L3CC7:;
    veng2_Say(N_wes, S_3CD2);
    return;
}

/* The computer: too damaged, so Spock proposes the linked tricorders
 * (need_tricorders).
 * [MSG_USE 00 0D 00] */
static void veng2_E_3CF1(void)
{
    { veng2_SpockOnComputer(); return; }
}

/* [MSG_USE 01 0D 00] */
static void veng2_SpockOnComputer(void)
{
    { veng2_KirkOnComputer2(); return; }
}

/* [MSG_USE 00 24 00] */
static void veng2_KirkOnComputer2(void)
{
    { veng2_AskAboutComputer(); return; }
}

/* [MSG_USE 01 24 00] */
static void veng2_AskAboutComputer(void)
{
    veng2_Say(N_Capt_Kirk, T_VEN2_002);
    if (rv_tricorders == 3) {
    } else {
        svc_Anim(1, N_sscanw, -1, -1, 0);
        svc_BridgeSfx(4);
        veng2_Say(N_Mr_Spock, T_VEN2_079);
        veng2_Say(N_Capt_Kirk, T_VEN2_024);
        veng2_Say(N_Mr_Spock, T_VEN2_080);
        veng2_Say(N_Dr_McCoy, T_VEN2_045);
        veng2_Say(N_Mr_Spock, T_VEN2_074);
        veng2_Say(N_Ensign_Kije, T_VEN2_096);
        rv_need_tricorders = 1;
        return;
    }
    veng2_Say(N_Mr_Spock, T_VEN2_059);
    return;
}

/* Wiring in the tricorders (msgs 0Ah-0Dh); with both and power, timer 0
 * counts to the Elasi's arrival.
 * [MSG_USE 44 0D 00] */
static void veng2_E_413F(void)
{
    { veng2_SciTricorderOnComputer(); return; }
}

/* [MSG_USE 44 24 00] */
static void veng2_SciTricorderOnComputer(void)
{
    if (rv_need_tricorders != 1) {
        { veng2_AskAboutComputer(); return; }
    }
    if (rv_power_on == 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_031);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x52, 0x84, 0xA);
    return;
}

/* [MSG_WALK_DONE 0A 00 00] */
static void veng2_SpockAtComputer(void)
{
    svc_Anim(1, N_susemn_3, -1, -1, 0xB);
    return;
}

/* [MSG_ANIM_DONE 0B 00 00] */
static void veng2_WiredSciTricorder(void)
{
    svc_Sfx(N_MUR4E1);
    room.busy = 0;
    if (rv_tricorders != 1) {
        rv_tricorders = 2;
    } else {
        rv_tricorders = 3;
        room.timers[0] = 0x28;
    }
    svc_TakeItem(IT_STRICOR);
    svc_Anim(9, N_s7r2t1_2, 0x45, 0x71, 0);
    veng2_Say(0, T_VEN2N017);
    svc_Walk(1, 0x5F, 0xAA, 0);
    if (rv_tricorders == 3) {
        { veng2_part_44C0(); return; }
    }
    return;
}

/* [MSG_USE 45 0D 00] */
static void veng2_E_4342(void)
{
    { veng2_MedTricorderOnComputer(); return; }
}

/* [MSG_USE 45 24 00] */
static void veng2_MedTricorderOnComputer(void)
{
    if (rv_power_on != 1) {
        veng2_Say(N_Dr_McCoy, T_VEN2_038);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x52, 0x84, 0xC);
    return;
}

/* [MSG_WALK_DONE 0C 00 00] */
static void veng2_SpockAtComputer2(void)
{
    svc_Anim(1, N_susemn_4, -1, -1, 0xD);
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void veng2_WiredMedTricorder(void)
{
    svc_Sfx(N_MUR4E1_2);
    room.busy = 0;
    if (rv_tricorders != 2) {
        rv_tricorders = 1;
    } else {
        rv_tricorders = 3;
        room.timers[0] = 0x28;
    }
    svc_TakeItem(IT_MTRICOR);
    svc_Anim(0xA, N_s7r2t2_2, 0x55, 0x67, 0);
    veng2_Say(0, T_VEN2N016);
    svc_Walk(1, 0x5F, 0xAA, 0);
    if (rv_tricorders == 3)
        { veng2_part_44C0(); return; }
    return;
}

static void veng2_part_44C0(void)
{
    svc_Anim(0xD, N_s7r2t3_2, 0x4F, 0x71, 0);
    veng2_Say(N_Mr_Spock, T_VEN2_030);
    return;
}

/* Spock's tricorder and the fusion pack.
 * [MSG_USE 44 12 00] */
static void veng2_E_4597(void)
{
    { veng2_SpockScansDeckSlot(); return; }
}

/* [MSG_USE 44 22 00] */
static void veng2_SpockScansDeckSlot(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0);
    svc_BridgeSfx(4);
    veng2_Say(N_Mr_Spock, T_VEN2_067);
    return;
}

/* [MSG_USE 44 0F 00] */
static void veng2_E_4638(void)
{
    { veng2_SpockScansPower(); return; }
}

/* [MSG_USE 44 23 00] */
static void veng2_SpockScansPower(void)
{
    svc_Anim(1, N_sscanw_2, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_power_on != 1) {
        veng2_Say(N_Mr_Spock, T_VEN2_056);
        return;
    }
    veng2_Say(N_Mr_Spock, T_VEN2_051);
    return;
}

/* [MSG_USE 83 0D 00] */
static void veng2_E_4777(void)
{
    { veng2_FusionOnComputer(); return; }
}

/* [MSG_USE 83 24 00] */
static void veng2_FusionOnComputer(void)
{
    veng2_Say(N_Mr_Spock, T_VEN2_060);
    return;
}

/* Taking the decks (msgs 8, 9).
 * [MSG_GET 08 00 00] */
static void veng2_GetDecks(void)
{
    svc_Walk(0, 0xE7, 0x93, 8);
    room.busy = 1;
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void veng2_KirkAtDecks(void)
{
    svc_Anim(0, N_kuseln, -1, -1, 9);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void veng2_TookDecks(void)
{
    svc_StopAnim(8);
    room.busy = 0;
    rv_decks_taken = 1;
    veng2_Say(0, T_VEN2N039);
    svc_GiveItem(IT_DECK);
    return;
}

/* Taking the tricorders back out (msgs 0Eh-11h), not once the Elasi
 * have hailed.
 * [MSG_GET 09 00 00] */
static void veng2_GetSciTricorder(void)
{
    if (rv_stalled != 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_047);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x52, 0x84, 0xE);
    return;
}

/* [MSG_WALK_DONE 0E 00 00] */
static void veng2_SpockAtComputer3(void)
{
    svc_Anim(1, N_susemn_5, -1, -1, 0xF);
    return;
}

/* [MSG_ANIM_DONE 0F 00 00] */
static void veng2_UnwiredSciTricorder(void)
{
    room.busy = 0;
    if (rv_tricorders != 3) {
        rv_tricorders = 0;
    } else {
        rv_tricorders = 1;
    }
    svc_GiveItem(IT_STRICOR);
    svc_StopAnim(9);
    svc_StopAnim(0xD);
    svc_Walk(1, 0x5F, 0xAA, 0);
    veng2_Say(0, T_VEN2N015);
    return;
}

/* [MSG_GET 0A 00 00] */
static void veng2_GetMedTricorder(void)
{
    if (rv_stalled != 1) {
    } else {
        veng2_Say(N_Mr_Spock, T_VEN2_047_2);
        return;
    }
    room.busy = 1;
    svc_Walk(1, 0x52, 0x84, 0x10);
    return;
}

/* [MSG_WALK_DONE 10 00 00] */
static void veng2_SpockAtComputer4(void)
{
    svc_Anim(1, N_susemn_6, -1, -1, 0x11);
    return;
}

/* [MSG_ANIM_DONE 11 00 00] */
static void veng2_UnwiredMedTricorder(void)
{
    room.busy = 0;
    if (rv_tricorders != 3) {
        rv_tricorders = 0;
    } else {
        rv_tricorders = 2;
    }
    svc_GiveItem(IT_MTRICOR);
    svc_StopAnim(0xA);
    svc_StopAnim(0xD);
    svc_Walk(1, 0x5F, 0xAA, 0);
    veng2_Say(0, T_VEN2N013);
    return;
}

/* Every tick, common to the mission's rooms: the Elasi deadlines set on
 * the bridge (room 2). Until the Enterprise returns: boarding unless the
 * shields are up, the thirty minutes, the wait after Kirk complies (then
 * a warning and the torpedoes), the chance after they lower their
 * shields. When one runs out, the game is over.
 * [MSG_TICK FF FF FF] */
static void veng2_ElasiDeadlines(void)
{
    if (rv_board_timer != 0) {
        rv_board_timer--;
        if (rv_board_timer == 0) {
            if (rv_power_to != 2) {
                goto L4D48;
            }
        }
    }
    if (rv_stall_timer != 0) {
        rv_stall_timer--;
        if (rv_stall_timer == 0) {
            if (rv_complied != 1) {
                goto L4D48;
            }
        }
    }
    if (rv_comply_timer != 0) {
        rv_comply_timer--;
        if (rv_comply_timer == 0) {
            if (rv_rescued != 1) {
                veng2_Say(N_Elasi_Captain, T_VEN2_098_2);
                rv_torpedo_timer = 0xA8C;
            }
        }
    }
    if (rv_torpedo_timer != 0) {
        rv_torpedo_timer--;
        if (rv_torpedo_timer == 0) {
            if (rv_rescued != 1) {
                veng2_Say(0, T_VENA_F40);
                svc_GameOver();
            }
        }
    }
    if (rv_attack_timer != 0) {
        rv_attack_timer--;
        if (rv_attack_timer == 0) {
            if (rv_rescued != 1) {
                goto L4D48;
            }
        }
    }
    return;
L4D48:;
    veng2_Say(0, T_VENA_F32);
    svc_GameOver();
    return;
}

/* [MSG_USE 40 FF 00] */
static void veng2_E_4E58(void)
{
    { veng2_KillAnything(); return; }
}

/* [MSG_USE 41 FF 00] */
static void veng2_KillAnything(void)
{
    veng2_Say(N_Mr_Spock, T_VENA_F34);
    return;
}

/* The hypo, empty or full of oil.
 * [MSG_LOOK 82 00 00] */
static void veng2_LookAtHypo(void)
{
    if (rv_hypo_oiled != 1) {
        veng2_Say(0, T_VEN4N010);
        return;
    }
    veng2_Say(0, T_VEN6N007);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t veng2_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void veng2_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x5C12) = bx;
    veng2_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript veng2_scripts[] = {
    { 0x0101, veng2_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0355, veng2_E_0355 },    /* MSG_TIMER 00 00 00 */
    { 0x035D, veng2_ElasiUncloak },    /* MSG_TICK 28 00 00 */
    { 0x03D4, veng2_ElasiSeen },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x04F9, veng2_ElasiHail },    /* MSG_TIMER 02 00 00 */
    { 0x0C3D, veng2_ToCorridor },    /* MSG_EXIT 01 00 00 */
    { 0x0C58, veng2_E_0C58 },    /* MSG_LOOK 0B 00 00 */
    { 0x0C60, veng2_LookAtViewscreen },    /* MSG_LOOK 27 00 00 */
    { 0x0D7C, veng2_E_0D7C },    /* MSG_LOOK 10 00 00 */
    { 0x0D84, veng2_LookAtHelm },    /* MSG_LOOK 26 00 00 */
    { 0x0E3E, veng2_E_0E3E },    /* MSG_LOOK 0C 00 00 */
    { 0x0E46, veng2_LookAtDamage },    /* MSG_LOOK 15 00 00 */
    { 0x0E4E, veng2_LookAtDamage2 },    /* MSG_LOOK 28 00 00 */
    { 0x0F69, veng2_LookAtSciTricorder },    /* MSG_LOOK 09 00 00 */
    { 0x0FCE, veng2_LookAtMedTricorder },    /* MSG_LOOK 0A 00 00 */
    { 0x1033, veng2_E_1033 },    /* MSG_LOOK 0E 00 00 */
    { 0x103B, veng2_LookAtTorpedoes },    /* MSG_LOOK 20 00 00 */
    { 0x108F, veng2_LookAtFireControl },    /* MSG_LOOK 21 00 00 */
    { 0x1290, veng2_E_1290 },    /* MSG_LOOK 0F 00 00 */
    { 0x1298, veng2_LookAtPowerControl },    /* MSG_LOOK 23 00 00 */
    { 0x1547, veng2_E_1547 },    /* MSG_LOOK 0D 00 00 */
    { 0x154F, veng2_LookAtComputer },    /* MSG_LOOK 24 00 00 */
    { 0x16F8, veng2_LookAtDecks },    /* MSG_LOOK 08 00 00 */
    { 0x1745, veng2_E_1745 },    /* MSG_LOOK 12 00 00 */
    { 0x174D, veng2_LookAtDeckSlot },    /* MSG_LOOK 25 00 00 */
    { 0x17A5, veng2_LookAtDeckSlot2 },    /* MSG_LOOK 22 00 00 */
    { 0x17FD, veng2_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x1861, veng2_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x18A9, veng2_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x18F1, veng2_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x194A, veng2_LookAtRoom },    /* MSG_LOOK FF 00 00 */
    { 0x19D1, veng2_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x1A92, veng2_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x1B9B, veng2_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x1C6E, veng2_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x1DAD, veng2_E_1DAD },    /* MSG_USE 88 12 00 */
    { 0x1DB5, veng2_JournalsInSlot },    /* MSG_USE 88 25 00 */
    { 0x1DBD, veng2_JournalsInSlot2 },    /* MSG_USE 88 22 00 */
    { 0x1E5F, veng2_SpockAtSlot },    /* MSG_WALK_DONE 16 00 00 */
    { 0x1E84, veng2_ReadJournals },    /* MSG_ANIM_DONE 17 00 00 */
    { 0x1FCE, veng2_E_1FCE },    /* MSG_USE 87 12 00 */
    { 0x1FD6, veng2_DeckInSlot },    /* MSG_USE 87 25 00 */
    { 0x1FDE, veng2_DeckInSlot2 },    /* MSG_USE 87 22 00 */
    { 0x2005, veng2_KirkAtSlot },    /* MSG_WALK_DONE 18 00 00 */
    { 0x202A, veng2_PlayLog },    /* MSG_ANIM_DONE 19 00 00 */
    { 0x21C2, veng2_CallEnterprise },    /* MSG_USE 47 00 00 */
    { 0x27C8, veng2_E_27C8 },    /* MSG_USE 00 0E 00 */
    { 0x27D0, veng2_FireTorpedo },    /* MSG_USE 00 20 00 */
    { 0x293C, veng2_KirkAtFireControl },    /* MSG_WALK_DONE 12 00 00 */
    { 0x2961, veng2_Fired },    /* MSG_ANIM_DONE 13 00 00 */
    { 0x29A0, veng2_ElasiHit },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x2BE9, veng2_ElasiGone },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x2E52, veng2_E_2E52 },    /* MSG_WALK_DONE 01 00 00 */
    { 0x2E5A, veng2_BeamOut2 },    /* MSG_WALK_DONE 02 00 00 */
    { 0x2E62, veng2_BeamOut3 },    /* MSG_WALK_DONE 03 00 00 */
    { 0x2E6A, veng2_BeamOut },    /* MSG_WALK_DONE 04 00 00 */
    { 0x2ECB, veng2_SpockScansTorpedoes },    /* MSG_USE 44 0E 00 */
    { 0x2F1A, veng2_E_2F1A },    /* MSG_USE 44 10 00 */
    { 0x2F22, veng2_SpockScansHelm },    /* MSG_USE 44 26 00 */
    { 0x300A, veng2_E_300A },    /* MSG_USE 00 21 00 */
    { 0x3012, veng2_ScanFireControl },    /* MSG_USE 44 21 00 */
    { 0x301A, veng2_SpockOnFireControl },    /* MSG_USE 01 21 00 */
    { 0x33BC, veng2_E_33BC },    /* MSG_USE 00 0F 00 */
    { 0x33C4, veng2_SpockOnPower },    /* MSG_USE 01 0F 00 */
    { 0x33CC, veng2_KirkOnPower2 },    /* MSG_USE 00 23 00 */
    { 0x33D4, veng2_AllocatePower },    /* MSG_USE 01 23 00 */
    { 0x3571, veng2_SpockAtPower },    /* MSG_WALK_DONE 14 00 00 */
    { 0x3596, veng2_PowerTo },    /* MSG_ANIM_DONE 15 00 00 */
    { 0x3CF1, veng2_E_3CF1 },    /* MSG_USE 00 0D 00 */
    { 0x3CF9, veng2_SpockOnComputer },    /* MSG_USE 01 0D 00 */
    { 0x3D01, veng2_KirkOnComputer2 },    /* MSG_USE 00 24 00 */
    { 0x3D09, veng2_AskAboutComputer },    /* MSG_USE 01 24 00 */
    { 0x413F, veng2_E_413F },    /* MSG_USE 44 0D 00 */
    { 0x4147, veng2_SciTricorderOnComputer },    /* MSG_USE 44 24 00 */
    { 0x4233, veng2_SpockAtComputer },    /* MSG_WALK_DONE 0A 00 00 */
    { 0x4258, veng2_WiredSciTricorder },    /* MSG_ANIM_DONE 0B 00 00 */
    { 0x4342, veng2_E_4342 },    /* MSG_USE 45 0D 00 */
    { 0x434A, veng2_MedTricorderOnComputer },    /* MSG_USE 45 24 00 */
    { 0x43BA, veng2_SpockAtComputer2 },    /* MSG_WALK_DONE 0C 00 00 */
    { 0x43DF, veng2_WiredMedTricorder },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x4597, veng2_E_4597 },    /* MSG_USE 44 12 00 */
    { 0x459F, veng2_SpockScansDeckSlot },    /* MSG_USE 44 22 00 */
    { 0x4638, veng2_E_4638 },    /* MSG_USE 44 0F 00 */
    { 0x4640, veng2_SpockScansPower },    /* MSG_USE 44 23 00 */
    { 0x4777, veng2_E_4777 },    /* MSG_USE 83 0D 00 */
    { 0x477F, veng2_FusionOnComputer },    /* MSG_USE 83 24 00 */
    { 0x4820, veng2_GetDecks },    /* MSG_GET 08 00 00 */
    { 0x4847, veng2_KirkAtDecks },    /* MSG_WALK_DONE 08 00 00 */
    { 0x486C, veng2_TookDecks },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x48D0, veng2_GetSciTricorder },    /* MSG_GET 09 00 00 */
    { 0x49C3, veng2_SpockAtComputer3 },    /* MSG_WALK_DONE 0E 00 00 */
    { 0x49E8, veng2_UnwiredSciTricorder },    /* MSG_ANIM_DONE 0F 00 00 */
    { 0x4A82, veng2_GetMedTricorder },    /* MSG_GET 0A 00 00 */
    { 0x4B75, veng2_SpockAtComputer4 },    /* MSG_WALK_DONE 10 00 00 */
    { 0x4B9A, veng2_UnwiredMedTricorder },    /* MSG_ANIM_DONE 11 00 00 */
    { 0x4C34, veng2_ElasiDeadlines },    /* MSG_TICK FF FF FF */
    { 0x4E58, veng2_E_4E58 },    /* MSG_USE 40 FF 00 */
    { 0x4E60, veng2_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x4EF5, veng2_LookAtHypo },    /* MSG_LOOK 82 00 00 */
    { 0, 0 }
};
