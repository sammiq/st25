/* TUG0.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/tug0.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * TUG0.RDF - "Hijacked", the transporter room of the Masada, taken by
 * Elasi pirates. The Elasi bombed the transporter controls; Crewman
 * Simpson lies stunned by the pad. McCoy revives him, which opens the
 * cabinet with the Runcinate Transmogrifier. Spock repairs the controls
 * with the transmogrifier fitted with a comb bit (welded from junk metal)
 * and a length of wiring, setting them for just inside the bridge door.
 * Then the pad beams the party onto the bridge (room 3). Beaming the bomb
 * there instead destroys the ship, hostages and all: the mission's
 * failure ending.
 *
 * Actors: 8 Simpson, 9 the cabinet, 10 (0Ah) the bomb, 11 (0Bh) the
 * controls.
 * Objects: 8 Simpson, 9 the cabinet, 0Bh/21h the controls, 22h the pad,
 * 23h the door.
 * Exit 0: the door (room 1). Timer 0: after the bomb.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 2675: the engine reads its bytes
 * as key 50 B8 03 00 and next 2E50 (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_TUG0LOOP = 0x012E,
    N_t0con2 = 0x0167,
    N_tcdown = 0x018D,
    N_tcupd = 0x01AD,
    N_pempty = 0x01D6,
    N_popend = 0x01F6,
    N_MUR4E6S = 0x0215,
    T_TUG0N002 = 0x0245,
    T_TUG0N014 = 0x0271,
    T_TUG0_019 = 0x02E1,
    T_TUG0N012 = 0x0337,
    T_TUG0N001 = 0x03B0,
    N_sscane = 0x0432,
    T_TUG0_040 = 0x0470,
    T_TUG0_005 = 0x04BF,
    T_TUG0_043 = 0x04F6,
    T_TUG0_013 = 0x057B,
    T_TUG0_038 = 0x061C,
    T_TUG0_008 = 0x06AD,
    T_TUG0_014 = 0x071B,
    T_TUG0_029 = 0x07CE,
    N_susehn = 0x0822,
    N_t0con2_2 = 0x087B,
    T_TUG0_034 = 0x08B4,
    N_susehn_2 = 0x093A,
    N_t0con1 = 0x0973,
    T_TUG0_039 = 0x099C,
    T_TUG0_010 = 0x09FD,
    T_TUG0_022 = 0x0A34,
    T_TUG0_002 = 0x0AF7,
    T_TUG0L009 = 0x0B2C,
    T_TUG0L008 = 0x0BD7,
    T_TUG0L010 = 0x0C6C,
    T_TUG0_021 = 0x0D24,
    T_TUG0_041 = 0x0D9D,
    N_museln = 0x0EA7,
    N_tcup = 0x0ECC,
    T_TUG0_F31 = 0x0F22,
    N_tcshow = 0x0FC5,
    N_popen = 0x0FE4,
    N_MUR4E6S_2 = 0x1002,
    N_kuselw = 0x107B,
    N_pempty_2 = 0x10B5,
    T_TUG0N000 = 0x1106,
    T_TUG0N008 = 0x11CC,
    T_TUG0N006 = 0x1223,
    T_TUG0N010 = 0x1262,
    T_TUG0_032 = 0x12F4,
    T_TUG0_035 = 0x13A1,
    N_susehw = 0x14A0,
    T_TUG0_001 = 0x14EE,
    N_kteled = 0x157D,
    N_steled = 0x159B,
    N_mteled = 0x15B9,
    N_rteled = 0x15DD,
    N_kpickw = 0x1689,
    N_bomb1 = 0x16BA,
    N_susehw_2 = 0x171E,
    N_bomb2 = 0x1773,
    T_TUG0_023 = 0x17D8,
    T_TUG0_037 = 0x1853,
    T_TUG0_027 = 0x18DD,
    T_TUG0_004 = 0x1917,
    T_TUG0_S11 = 0x1953,
    T_TUG0_003 = 0x1984,
    T_TUG0_S06 = 0x19B3,
    N_mscans = 0x1A14,
    T_TUG0_016 = 0x1A4B,
    N_sscans = 0x1A97,
    T_TUG0_015 = 0x1ACE,
    N_mscann = 0x1B8E,
    T_TUG0_017 = 0x1BC2,
    T_TUG0_018 = 0x1C32,
    N_sscans_2 = 0x1CA8,
    T_TUG0_030 = 0x1CDF,
    T_TUG0N003 = 0x1D5E,
    T_TUG0N007 = 0x1DAA,
    T_TUG0N005 = 0x1E0E,
    T_TUG0N004 = 0x1E6B,
    T_TUG0N013 = 0x1EB1,
    T_TUG0N011 = 0x1F02,
    T_TUG0_006 = 0x1F6A,
    T_TUG0_036 = 0x1FB9,
    T_TUG0_011 = 0x1FFD,
    T_TUG0_025 = 0x204E,
    T_TUG0_007 = 0x20AA,
    T_TUG0_024 = 0x20FD,
    T_TUG0_042 = 0x213E,
    T_TUG0_009 = 0x21DB,
    T_TUG0L006 = 0x2225,
    T_TUG0_F30 = 0x2298,
    T_TUG0_026 = 0x2301,
    T_TUG0_012 = 0x2368,
    T_TUG0_031 = 0x23C2,
    T_TUG0N009 = 0x246D,
    N_sscans_3 = 0x24C6,
    T_TUG0_028 = 0x24FE,
    N_mscans_2 = 0x25B1,
    T_TUG0_020 = 0x25E9,
    N_Capt_Kirk = 0x26A8,
    N_Dr_McCoy = 0x26B3,
    N_Mr_Spock = 0x26BD,
    N_Lt_Christensen = 0x26C7,
    N_Crewman_Simpson = 0x26D7,
    N_Mr_Scott = 0x26F1,
    talk_buf = 0x26FB,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0x26FB)
#define rv_found_bomb ROOM_B(0x49)
#define rv_max_score ROOM_W(0x2B)
#define rv_outcome ROOM_W(0x2D)
#define rv_revived_simpson ROOM_B(0x30)
#define rv_score ROOM_W(0x29)
#define rv_spock_assessed ROOM_B(0x45)
#define rv_took_transmogrifier ROOM_B(0x43)
#define rv_transmogrifier_fitted ROOM_B(0x46)
#define rv_transporter_fixed ROOM_B(0x44)

/* functions */
static void tug0_StartRoom(void);
static void tug0_LookAtSimpson(void);
static void tug0_GetSimpson(void);
static void tug0_E_031E(void);
static void tug0_LookAtControls(void);
static void tug0_SpockUsesControls(void);
static void tug0_SpockAtControls(void);
static void tug0_SpockAssesses(void);
static void tug0_UseTransmogrifierOnControls(void);
static void tug0_UseFittedTransmogrifier(void);
static void tug0_SpockFitsIt(void);
static void tug0_TransmogrifierFitted(void);
static void tug0_ControlsLit(void);
static void tug0_UseWireScrapsOnControls(void);
static void tug0_UseWiringOnControls(void);
static void tug0_part_0931(void);
static void tug0_SpockWires(void);
static void tug0_ControlsRepaired(void);
static void tug0_HealSimpson(void);
static void tug0_McCoyAtSimpson(void);
static void tug0_SimpsonWakes(void);
static void tug0_SimpsonSitsUp(void);
static void tug0_GetTransmogrifier(void);
static void tug0_KirkAtCabinet(void);
static void tug0_TookTransmogrifier(void);
static void tug0_LookAtCabinet(void);
static void tug0_E_11F8(void);
static void tug0_ChargeWelder(void);
static void tug0_WeldWireScraps(void);
static void tug0_WeldJunkMetal(void);
static void tug0_FitCombBit(void);
static void tug0_E_1402(void);
static void tug0_UsePad(void);
static void tug0_SpockAtControls2(void);
static void tug0_SpockEnergizes(void);
static void tug0_EveryoneOnPad(void);
static void tug0_BeamedToBridge(void);
static void tug0_UseBombOnPad(void);
static void tug0_KirkAtPad(void);
static void tug0_BombOnPad(void);
static void tug0_SpockAtControls3(void);
static void tug0_BombBeamed(void);
static void tug0_BombExplodes(void);
static void tug0_ShipDestroyed(void);
static void tug0_E_1A06(void);
static void tug0_McCoyScansControls(void);
static void tug0_SpockScansControls(void);
static void tug0_McCoyScansSimpson(void);
static void tug0_McCoyAtSimpson2(void);
static void tug0_SimpsonScanned(void);
static void tug0_McCoyBack(void);
static void tug0_E_1C9A(void);
static void tug0_SpockScansSimpson(void);
static void tug0_WalkToDoor(void);
static void tug0_LookAtKirk(void);
static void tug0_LookAtSpock(void);
static void tug0_LookAtMcCoy(void);
static void tug0_LookAtRedshirt(void);
static void tug0_LookAtPad(void);
static void tug0_LookAtDoor(void);
static void tug0_TalkToKirk(void);
static void tug0_TalkToMcCoy(void);
static void tug0_TalkToSpock(void);
static void tug0_TalkToRedshirt(void);
static void tug0_TalkToSimpson(void);
static void tug0_UseCommunicator(void);
static void tug0_LookAtScene(void);
static void tug0_SpockScans(void);
static void tug0_McCoyScans(void);
static void tug0_Talk(uint16_t ax);
static void tug0_Say(uint16_t ax, uint16_t bx);

/* The room starts: the controls (lit once repaired), Simpson lying or
 * sitting up, the cabinet closed or open (empty once Kirk has the tool).
 * [MSG_TICK 01 00 00] */
static void tug0_StartRoom(void)
{
    svc_Sfx(N_TUG0LOOP);
    svc_Music(0, -1);
    rv_max_score = 0x23;
    if (rv_transporter_fixed == 1) {
        svc_Anim(0xB, N_t0con2, 0x105, 0xC2, 0);
    }
    if (rv_revived_simpson == 0) {
        svc_Anim(8, N_tcdown, 0x27, 0xA8, 0);
        return;
    }
    svc_Anim(8, N_tcupd, 0x27, 0xA8, 0);
    if (rv_took_transmogrifier == 1) {
        svc_Anim(9, N_pempty, 0x27, 0xA9, 0);
        return;
    }
    svc_Anim(9, N_popend, 0x27, 0xA9, 0);
    svc_Sfx(N_MUR4E6S);
    return;
}

/* Simpson.
 * [MSG_LOOK 08 00 00] */
static void tug0_LookAtSimpson(void)
{
    if (rv_revived_simpson == 1) {
        tug0_Say(0, T_TUG0N002);
        return;
    }
    tug0_Say(0, T_TUG0N014);
    return;
}

/* [MSG_GET 08 00 00] */
static void tug0_GetSimpson(void)
{
    tug0_Say(N_Dr_McCoy, T_TUG0_019);
    return;
}

/* The controls.
 * [MSG_LOOK 0B 00 00] */
static void tug0_E_031E(void)
{
    { tug0_LookAtControls(); return; }
}

/* [MSG_LOOK 21 00 00] */
static void tug0_LookAtControls(void)
{
    if (rv_transporter_fixed == 1) {
        tug0_Say(0, T_TUG0N012);
        return;
    }
    tug0_Say(0, T_TUG0N001);
    return;
}

/* Spock looks at the controls (msgs 16, 17): only a jury-rig will do.
 * [MSG_USE 01 21 00] */
static void tug0_SpockUsesControls(void)
{
    room.busy = 1;
    room.fall[1] = 2;
    svc_Walk(1, 0xDB, 0xC5, 0x10);
    return;
}

/* [MSG_WALK_DONE 10 00 00] */
static void tug0_SpockAtControls(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0x11);
    svc_BridgeSfx(4);
    return;
}

/* [MSG_ANIM_DONE 11 00 00] */
static void tug0_SpockAssesses(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_040);
    tug0_Say(N_Capt_Kirk, T_TUG0_005);
    tug0_Say(N_Mr_Spock, T_TUG0_043);
    tug0_Say(N_Capt_Kirk, T_TUG0_013);
    tug0_Say(N_Mr_Spock, T_TUG0_038);
    tug0_Say(N_Capt_Kirk, T_TUG0_008);
    rv_spock_assessed = 1;
    room.busy = 0;
    return;
}

/* Repairing the controls: the transmogrifier needs its comb bit first;
 * with it, Spock fits it (msgs 6, 7), then needs wiring (the scraps are
 * too short), and wires them up (msgs 8, 9). +1 score each step. Then
 * Christensen sums up the plan of attack.
 * [MSG_USE 4F 21 00] */
static void tug0_UseTransmogrifierOnControls(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_014);
    return;
}

/* [MSG_USE 50 21 00] */
static void tug0_UseFittedTransmogrifier(void)
{
    room.busy = 1;
    room.fall[1] = 2;
    svc_Walk(1, 0xDB, 0xC5, 6);
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void tug0_SpockFitsIt(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_029);
    svc_Anim(1, N_susehn, -1, -1, 7);
    svc_BridgeSfx(0xB);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void tug0_TransmogrifierFitted(void)
{
    svc_TakeItem(IT_RTWB);
    rv_transmogrifier_fitted = 1;
    room.busy = 0;
    rv_score += 1;
    return;
}

/* [MSG_ANIM_DONE 16 00 00] */
static void tug0_ControlsLit(void)
{
    svc_Anim(0xB, N_t0con2_2, 0x105, 0xC2, 0);
    return;
}

/* [MSG_USE 54 21 00] */
static void tug0_UseWireScrapsOnControls(void)
{
    if (rv_transmogrifier_fitted != 1) {
        { tug0_part_0931(); return; }
    }
    tug0_Say(N_Mr_Spock, T_TUG0_034);
    return;
}

/* [MSG_USE 53 21 00] */
static void tug0_UseWiringOnControls(void)
{
    if (rv_transmogrifier_fitted != 1)
        { tug0_part_0931(); return; }
    room.busy = 1;
    room.fall[1] = 2;
    svc_Walk(1, 0xDB, 0xC5, 8);
    { tug0_part_0931(); return; }
}

static void tug0_part_0931(void)
{
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void tug0_SpockWires(void)
{
    svc_Anim(1, N_susehn_2, -1, -1, 9);
    return;
}

/* [MSG_ANIM_DONE 09 00 00] */
static void tug0_ControlsRepaired(void)
{
    room.busy = 0;
    svc_TakeItem(IT_WIRING);
    rv_score += 1;
    svc_Anim(0xB, N_t0con1, 0x105, 0xC2, 0x16);
    tug0_Say(N_Mr_Spock, T_TUG0_039);
    rv_transporter_fixed = 1;
    tug0_Say(N_Capt_Kirk, T_TUG0_010);
    tug0_Say(N_Dr_McCoy, T_TUG0_022);
    tug0_Say(N_Capt_Kirk, T_TUG0_002);
    tug0_Say(N_Lt_Christensen, T_TUG0L009);
    tug0_Say(N_Lt_Christensen, T_TUG0L008);
    if (rv_found_bomb != 1) {
    } else {
        tug0_Say(N_Lt_Christensen, T_TUG0L010);
        tug0_Say(N_Dr_McCoy, T_TUG0_021);
        tug0_Say(N_Mr_Spock, T_TUG0_041);
        goto L0DEA;
    }
L0DEA:;
    return;
}

/* McCoy revives Simpson (msgs 2, 13, 1), +1 score: he tells them about
 * the cabinet, which opens.
 * [MSG_USE 7F 08 00] */
static void tug0_HealSimpson(void)
{
    if (rv_revived_simpson != 1) {
        room.busy = 1;
        room.fall[2] = 0;
        svc_Walk(2, 0x2D, 0xAF, 2);
    }
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void tug0_McCoyAtSimpson(void)
{
    svc_Anim(2, N_museln, -1, -1, 0xD);
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void tug0_SimpsonWakes(void)
{
    svc_Anim(8, N_tcup, 0x27, 0xA8, 1);
    room.fall[2] = 2;
    svc_Walk(2, 0x41, 0xAF, 0);
    room.busy = 0;
    rv_score += 1;
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void tug0_SimpsonSitsUp(void)
{
    tug0_Say(N_Crewman_Simpson, T_TUG0_F31);
    rv_revived_simpson = 1;
    svc_Anim(8, N_tcshow, 0x27, 0xA8, 0);
    svc_Anim(9, N_popen, 0x27, 0xA9, 0);
    svc_Sfx(N_MUR4E6S_2);
    return;
}

/* Taking the transmogrifier from the cabinet (msgs 3, 4), +1 score.
 * [MSG_GET 09 00 00] */
static void tug0_GetTransmogrifier(void)
{
    if (rv_took_transmogrifier == 0) {
        room.busy = 1;
        svc_Walk(0, 0x1E, 0xB1, 3);
        return;
    }
    return;
}

/* [MSG_WALK_DONE 03 00 00] */
static void tug0_KirkAtCabinet(void)
{
    svc_Anim(0, N_kuselw, -1, -1, 4);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void tug0_TookTransmogrifier(void)
{
    rv_took_transmogrifier = 1;
    svc_GiveItem(IT_RT);
    svc_Anim(9, N_pempty_2, 0x27, 0xA9, 0);
    svc_StopAnim(0);
    room.busy = 0;
    rv_score += 1;
    return;
}

/* [MSG_LOOK 09 00 00] */
static void tug0_LookAtCabinet(void)
{
    if (rv_took_transmogrifier != 0) {
    } else {
        tug0_Say(0, T_TUG0N000);
        return;
    }
    tug0_Say(0, T_TUG0N008);
    return;
}

/* Inventory: phaser + empty welder = charged welder; welder + junk metal
 * = comb bit (wire scraps just melt); comb bit + transmogrifier.
 * [MSG_USE 40 56 00] */
static void tug0_E_11F8(void)
{
    { tug0_ChargeWelder(); return; }
}

/* [MSG_USE 41 56 00] */
static void tug0_ChargeWelder(void)
{
    svc_TakeItem(IT_PWE);
    svc_GiveItem(IT_PWF);
    tug0_Say(0, T_TUG0N006);
    return;
}

/* [MSG_USE 55 54 00] */
static void tug0_WeldWireScraps(void)
{
    tug0_Say(0, T_TUG0N010);
    svc_TakeItem(IT_WIRSCRP);
    return;
}

/* [MSG_USE 55 52 00] */
static void tug0_WeldJunkMetal(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_032);
    svc_TakeItem(IT_JNKMETL);
    svc_GiveItem(IT_COMBBIT);
    return;
}

/* [MSG_USE 51 4F 00] */
static void tug0_FitCombBit(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_035);
    svc_TakeItem(IT_COMBBIT);
    svc_TakeItem(IT_RT);
    svc_GiveItem(IT_RTWB);
    return;
}

/* The pad, once the controls are fixed: everyone onto it, Spock energizes
 * (msgs 14, 18, 20, 21), +3 score, and they appear on the bridge.
 * [MSG_USE 01 0B 00] */
static void tug0_E_1402(void)
{
    { tug0_UsePad(); return; }
}

/* [MSG_USE 00 22 00] */
static void tug0_UsePad(void)
{
    if (rv_transporter_fixed != 1) {
    } else {
        room.busy = 1;
        room.fall[0] = 1;
        svc_Walk(0, 0x62, 0x7F, 0);
        room.fall[1] = 3;
        svc_Walk(1, 0x11E, 0xC7, 0xE);
        room.fall[2] = 1;
        svc_Walk(2, 0x7B, 0x7A, 0);
        room.fall[3] = 1;
        svc_Walk(3, 0x4A, 0x7D, 0);
    }
    return;
}

/* [MSG_WALK_DONE 0E 00 00] */
static void tug0_SpockAtControls2(void)
{
    svc_Anim(1, N_susehw, -1, -1, 0x12);
    svc_BridgeSfx(7);
    svc_BridgeSfx(0xA);
    return;
}

/* [MSG_ANIM_DONE 12 00 00] */
static void tug0_SpockEnergizes(void)
{
    tug0_Say(N_Capt_Kirk, T_TUG0_001);
    room.fall[1] = 1;
    svc_Walk(1, 0x9A, 0x7E, 0x14);
    return;
}

/* [MSG_WALK_DONE 14 00 00] */
static void tug0_EveryoneOnPad(void)
{
    svc_BridgeSfx(8);
    svc_Anim(0, N_kteled, -1, -1, 0x15);
    svc_Anim(1, N_steled, -1, -1, 0x15);
    svc_Anim(2, N_mteled, -1, -1, 0x15);
    if (room.redshirt_dead == 0) {
        svc_Anim(3, N_rteled, -1, -1, 0x15);
    }
    return;
}

/* [MSG_ANIM_DONE 15 00 00] */
static void tug0_BeamedToBridge(void)
{
    rv_score += 3;
    room.busy = 1;
    svc_GotoRoom(3, 4);
    return;
}

/* The bomb on the pad, once the controls are fixed: Kirk puts it there,
 * Spock beams it to the bridge (msgs 5, 10, 11, 15, 19, 12). Timer 0:
 * the Masada is destroyed; score 0, outcome 1.
 * [MSG_USE 58 22 00] */
static void tug0_UseBombOnPad(void)
{
    if (rv_transporter_fixed == 1) {
        room.busy = 1;
        svc_Walk(0, 0x62, 0x7F, 5);
        room.fall[2] = 0;
        svc_Walk(2, 0x46, 0xBE, 0);
        room.fall[3] = 0;
        svc_Walk(3, 0x78, 0xBE, 0);
    }
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void tug0_KirkAtPad(void)
{
    svc_Anim(0, N_kpickw, -1, -1, 0xA);
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void tug0_BombOnPad(void)
{
    svc_TakeItem(IT_BOMB);
    svc_Anim(0xA, N_bomb1, 0x5A, 0x7F, 0);
    room.fall[0] = 0;
    svc_Walk(0, 0xC8, 0xBE, 0xB);
    room.fall[1] = 3;
    svc_Walk(1, 0x11E, 0xC7, 0xF);
    return;
}

/* [MSG_WALK_DONE 0F 00 00] */
static void tug0_SpockAtControls3(void)
{
    svc_Anim(1, N_susehw_2, -1, -1, 0x13);
    svc_BridgeSfx(7);
    svc_BridgeSfx(0xA);
    return;
}

/* [MSG_ANIM_DONE 13 00 00] */
static void tug0_BombBeamed(void)
{
    svc_BridgeSfx(8);
    svc_Anim(0xA, N_bomb2, 0x5A, 0x7F, 0xC);
    return;
}

/* [MSG_ANIM_DONE 0C 00 00] */
static void tug0_BombExplodes(void)
{
    svc_BridgeSfx(0x14);
    rv_score += 0;
    rv_score = 0;
    room.timers[0] = 0x40;
    return;
}

/* [MSG_TIMER 00 00 00] */
static void tug0_ShipDestroyed(void)
{
    uint16_t ax = 0;
    svc_Music(2, -1);
    tug0_Say(N_Dr_McCoy, T_TUG0_023);
    tug0_Say(N_Mr_Spock, T_TUG0_037);
    tug0_Say(N_Dr_McCoy, T_TUG0_027);
    tug0_Say(N_Capt_Kirk, T_TUG0_004);
    tug0_Say(N_Mr_Scott, T_TUG0_S11);
    tug0_Say(N_Capt_Kirk, T_TUG0_003);
    tug0_Say(N_Mr_Scott, T_TUG0_S06);
    room.busy = 0;
    rv_outcome = 1;
    room.busy = 1;
    ax = rv_score;
    ax = ax & 0x8000;
    if (ax != 0) {
        rv_score = 0;
    }
    svc_BeamUp(rv_score, rv_max_score, rv_outcome);
    return;
}

/* Tricorders.
 * [MSG_USE 02 21 00] */
static void tug0_E_1A06(void)
{
    { tug0_McCoyScansControls(); return; }
}

/* [MSG_USE 45 21 00] */
static void tug0_McCoyScansControls(void)
{
    room.fall[2] = 1;
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    tug0_Say(N_Dr_McCoy, T_TUG0_016);
    return;
}

/* [MSG_USE 44 21 00] */
static void tug0_SpockScansControls(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0x17);
    svc_BridgeSfx(4);
    tug0_Say(N_Mr_Spock, T_TUG0_015);
    return;
}

/* [MSG_USE 45 08 00] */
static void tug0_McCoyScansSimpson(void)
{
    room.busy = 1;
    room.fall[2] = 0;
    svc_Walk(2, 0x2D, 0xAF, 0x18);
    return;
}

/* [MSG_WALK_DONE 18 00 00] */
static void tug0_McCoyAtSimpson2(void)
{
    svc_BridgeSfx(4);
    svc_Anim(2, N_mscann, -1, -1, 0x19);
    return;
}

/* [MSG_ANIM_DONE 19 00 00] */
static void tug0_SimpsonScanned(void)
{
    if (rv_revived_simpson == 1) {
        tug0_Say(N_Dr_McCoy, T_TUG0_017);
        room.fall[2] = 3;
        svc_Walk(2, 0x41, 0xAF, 0x1A);
        room.busy = 0;
        return;
    }
    tug0_Say(N_Dr_McCoy, T_TUG0_018);
    room.fall[2] = 3;
    svc_Walk(2, 0x41, 0xAF, 0x1A);
    room.busy = 0;
    return;
}

/* [MSG_WALK_DONE 1A 00 00] */
static void tug0_McCoyBack(void)
{
    return;
}

/* [MSG_USE 01 08 00] */
static void tug0_E_1C9A(void)
{
    { tug0_SpockScansSimpson(); return; }
}

/* [MSG_USE 44 08 00] */
static void tug0_SpockScansSimpson(void)
{
    room.fall[1] = 1;
    svc_Anim(1, N_sscans_2, -1, -1, 0);
    svc_BridgeSfx(4);
    tug0_Say(N_Mr_Spock, T_TUG0_030);
    return;
}

/* Walking to the door.
 * [MSG_WALK 23 00 00] */
static void tug0_WalkToDoor(void)
{
    svc_Walk(0, 0x138, 0xAA, 0);
    return;
}

/* Looking at things.
 * [MSG_LOOK 00 00 00] */
static void tug0_LookAtKirk(void)
{
    tug0_Say(0, T_TUG0N003);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void tug0_LookAtSpock(void)
{
    tug0_Say(0, T_TUG0N007);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void tug0_LookAtMcCoy(void)
{
    tug0_Say(0, T_TUG0N005);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void tug0_LookAtRedshirt(void)
{
    tug0_Say(0, T_TUG0N004);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void tug0_LookAtPad(void)
{
    tug0_Say(0, T_TUG0N013);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void tug0_LookAtDoor(void)
{
    tug0_Say(0, T_TUG0N011);
    return;
}

/* Talking.
 * [MSG_TALK 00 00 00] */
static void tug0_TalkToKirk(void)
{
    tug0_Say(N_Capt_Kirk, T_TUG0_006);
    tug0_Say(N_Mr_Spock, T_TUG0_036);
    tug0_Say(N_Capt_Kirk, T_TUG0_011);
    return;
}

/* [MSG_TALK 02 00 00] */
static void tug0_TalkToMcCoy(void)
{
    tug0_Say(N_Dr_McCoy, T_TUG0_025);
    tug0_Say(N_Capt_Kirk, T_TUG0_007);
    tug0_Say(N_Dr_McCoy, T_TUG0_024);
    return;
}

/* [MSG_TALK 01 00 00] */
static void tug0_TalkToSpock(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_042);
    tug0_Say(N_Capt_Kirk, T_TUG0_009);
    return;
}

/* [MSG_TALK 03 00 00] */
static void tug0_TalkToRedshirt(void)
{
    tug0_Say(N_Lt_Christensen, T_TUG0L006);
    return;
}

/* [MSG_TALK 08 00 00] */
static void tug0_TalkToSimpson(void)
{
    if (rv_revived_simpson != 1) {
    } else {
        tug0_Say(N_Crewman_Simpson, T_TUG0_F30);
        tug0_Say(N_Dr_McCoy, T_TUG0_026);
        tug0_Say(N_Capt_Kirk, T_TUG0_012);
        goto L23AF;
    }
L23AF:;
    return;
}

/* The communicator: Spock advises silence.
 * [MSG_USE 47 FF 00] */
static void tug0_UseCommunicator(void)
{
    tug0_Say(N_Mr_Spock, T_TUG0_031);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void tug0_LookAtScene(void)
{
    tug0_Say(0, T_TUG0N009);
    return;
}

/* [MSG_USE 44 FF 00] */
static void tug0_SpockScans(void)
{
    room.fall[1] = 1;
    svc_Anim(1, N_sscans_3, -1, -1, 0);
    svc_BridgeSfx(4);
    tug0_Say(N_Mr_Spock, T_TUG0_028);
    return;
}

/* [MSG_USE 45 FF 00] */
static void tug0_McCoyScans(void)
{
    room.fall[2] = 1;
    svc_Anim(2, N_mscans_2, -1, -1, 0);
    svc_BridgeSfx(4);
    tug0_Say(N_Dr_McCoy, T_TUG0_020);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void tug0_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void tug0_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x26FD) = bx;
    tug0_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript tug0_scripts[] = {
    { 0x012C, tug0_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0234, tug0_LookAtSimpson },    /* MSG_LOOK 08 00 00 */
    { 0x02D6, tug0_GetSimpson },    /* MSG_GET 08 00 00 */
    { 0x031E, tug0_E_031E },    /* MSG_LOOK 0B 00 00 */
    { 0x0326, tug0_LookAtControls },    /* MSG_LOOK 21 00 00 */
    { 0x0405, tug0_SpockUsesControls },    /* MSG_USE 01 21 00 */
    { 0x0430, tug0_SpockAtControls },    /* MSG_WALK_DONE 10 00 00 */
    { 0x0465, tug0_SpockAssesses },    /* MSG_ANIM_DONE 11 00 00 */
    { 0x0710, tug0_UseTransmogrifierOnControls },    /* MSG_USE 4F 21 00 */
    { 0x0798, tug0_UseFittedTransmogrifier },    /* MSG_USE 50 21 00 */
    { 0x07C3, tug0_SpockFitsIt },    /* MSG_WALK_DONE 06 00 00 */
    { 0x0855, tug0_TransmogrifierFitted },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0879, tug0_ControlsLit },    /* MSG_ANIM_DONE 16 00 00 */
    { 0x08A0, tug0_UseWireScrapsOnControls },    /* MSG_USE 54 21 00 */
    { 0x0907, tug0_UseWiringOnControls },    /* MSG_USE 53 21 00 */
    { 0x0938, tug0_SpockWires },    /* MSG_WALK_DONE 08 00 00 */
    { 0x095D, tug0_ControlsRepaired },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x0E74, tug0_HealSimpson },    /* MSG_USE 7F 08 00 */
    { 0x0EA5, tug0_McCoyAtSimpson },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0ECA, tug0_SimpsonWakes },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x0F16, tug0_SimpsonSitsUp },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x1046, tug0_GetTransmogrifier },    /* MSG_GET 09 00 00 */
    { 0x1079, tug0_KirkAtCabinet },    /* MSG_WALK_DONE 03 00 00 */
    { 0x109E, tug0_TookTransmogrifier },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x10F1, tug0_LookAtCabinet },    /* MSG_LOOK 09 00 00 */
    { 0x11F8, tug0_E_11F8 },    /* MSG_USE 40 56 00 */
    { 0x1200, tug0_ChargeWelder },    /* MSG_USE 41 56 00 */
    { 0x1257, tug0_WeldWireScraps },    /* MSG_USE 55 54 00 */
    { 0x12E8, tug0_WeldJunkMetal },    /* MSG_USE 55 52 00 */
    { 0x1396, tug0_FitCombBit },    /* MSG_USE 51 4F 00 */
    { 0x1402, tug0_E_1402 },    /* MSG_USE 01 0B 00 */
    { 0x140A, tug0_UsePad },    /* MSG_USE 00 22 00 */
    { 0x149E, tug0_SpockAtControls2 },    /* MSG_WALK_DONE 0E 00 00 */
    { 0x14E3, tug0_SpockEnergizes },    /* MSG_ANIM_DONE 12 00 00 */
    { 0x156B, tug0_EveryoneOnPad },    /* MSG_WALK_DONE 14 00 00 */
    { 0x1600, tug0_BeamedToBridge },    /* MSG_ANIM_DONE 15 00 00 */
    { 0x161A, tug0_UseBombOnPad },    /* MSG_USE 58 22 00 */
    { 0x1687, tug0_KirkAtPad },    /* MSG_WALK_DONE 05 00 00 */
    { 0x16AC, tug0_BombOnPad },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x171C, tug0_SpockAtControls3 },    /* MSG_WALK_DONE 0F 00 00 */
    { 0x1761, tug0_BombBeamed },    /* MSG_ANIM_DONE 13 00 00 */
    { 0x1795, tug0_BombExplodes },    /* MSG_ANIM_DONE 0C 00 00 */
    { 0x17B9, tug0_ShipDestroyed },    /* MSG_TIMER 00 00 00 */
    { 0x1A06, tug0_E_1A06 },    /* MSG_USE 02 21 00 */
    { 0x1A0E, tug0_McCoyScansControls },    /* MSG_USE 45 21 00 */
    { 0x1A95, tug0_SpockScansControls },    /* MSG_USE 44 21 00 */
    { 0x1B51, tug0_McCoyScansSimpson },    /* MSG_USE 45 08 00 */
    { 0x1B7C, tug0_McCoyAtSimpson2 },    /* MSG_WALK_DONE 18 00 00 */
    { 0x1BB1, tug0_SimpsonScanned },    /* MSG_ANIM_DONE 19 00 00 */
    { 0x1C93, tug0_McCoyBack },    /* MSG_WALK_DONE 1A 00 00 */
    { 0x1C9A, tug0_E_1C9A },    /* MSG_USE 01 08 00 */
    { 0x1CA2, tug0_SpockScansSimpson },    /* MSG_USE 44 08 00 */
    { 0x1D30, tug0_WalkToDoor },    /* MSG_WALK 23 00 00 */
    { 0x1D53, tug0_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x1D9F, tug0_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x1E03, tug0_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x1E60, tug0_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x1EA6, tug0_LookAtPad },    /* MSG_LOOK 22 00 00 */
    { 0x1EF7, tug0_LookAtDoor },    /* MSG_LOOK 23 00 00 */
    { 0x1F5F, tug0_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x2043, tug0_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x2132, tug0_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x221A, tug0_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x2284, tug0_TalkToSimpson },    /* MSG_TALK 08 00 00 */
    { 0x23B6, tug0_UseCommunicator },    /* MSG_USE 47 FF 00 */
    { 0x2462, tug0_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x24C0, tug0_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x25AB, tug0_McCoyScans },    /* MSG_USE 45 FF 00 */
    { 0, 0 }
};
