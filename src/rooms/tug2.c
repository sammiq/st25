/* TUG2.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/tug2.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * TUG2.RDF - "Hijacked", the Masada's brig. Two Elasi guards hold nine of
 * the crew behind a forcefield. When the party arrives the guards draw;
 * Kirk must stun (or kill) both before they fire. The guards shoot in
 * rounds (timer 0), stunning the crew, or killing them if the party has
 * used a phaser on kill. A stunned guard is tied up once the crew is free,
 * but wakes up if the party leaves before then.
 *
 * The forcefield switch is booby-trapped: wires lead to a bomb inside the
 * field. Pressing the switch, or pulling the wires, before the bomb is
 * defused (Spock or Christensen) kills the prisoners. Defused, the wires
 * are the wiring the transporter repair needs (room 0), and the bomb can
 * be taken. The freed crew tell Kirk how to open the bridge door (room 1).
 *
 * Actors: 8 and 9 the guards, 10 (0Ah) the forcefield and prisoners, 11
 * (0Bh) the bomb, 12 (0Ch) the wires, 13 (0Dh) the scorch on the wall.
 * Objects: 8, 9 the guards, 0Ah the prisoners, 0Bh the bomb, 0Ch the
 * wires, 20h the switch, 21h the door (to room 1), 22h the cell.
 * Timers: 0 the guards fire, 2 Kirk is dead.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_TUG2LOOP = 0x010E,
    N_t2bomb = 0x012E,
    N_t2wire = 0x0152,
    N_fld01 = 0x0176,
    N_fld10d = 0x0199,
    N_zapdon = 0x01BD,
    N_p5tied = 0x01E1,
    N_p5tied_2 = 0x020D,
    N_p5redy = 0x0261,
    N_p5hat = 0x0287,
    N_p6tied = 0x02AC,
    N_p6tied_2 = 0x02D8,
    N_p6draw = 0x032C,
    N_p5hat_2 = 0x0352,
    N_wallht = 0x0377,
    T_TUG2L085 = 0x03E1,
    T_TUG2N011 = 0x0423,
    T_TUG2N005 = 0x048D,
    T_TUG2N007 = 0x04CE,
    T_TUG2N004 = 0x0508,
    T_TUG2N024 = 0x055A,
    N_kusehn = 0x05CD,
    T_TUG2N000 = 0x062B,
    T_TUG2N001 = 0x067D,
    T_TUG2N013 = 0x06BB,
    T_TUG2N015 = 0x06F3,
    T_TUG2N106 = 0x072B,
    T_TUG2N037 = 0x0782,
    T_TUG2N014 = 0x07C0,
    T_TUG2N016 = 0x07F8,
    T_TUG2N106_2 = 0x0830,
    T_TUG2L002 = 0x0881,
    T_TUG2N010 = 0x08C4,
    N_sscane = 0x0924,
    T_TUG2_007 = 0x095B,
    T_TUG2_009 = 0x09E7,
    T_TUG2_011 = 0x0A5E,
    T_TUG2L007 = 0x0ACA,
    N_rusemn = 0x0B2C,
    N_Snip_snip_snip = 0x0B5A,
    T_TUG2L011 = 0x0B79,
    T_TUG2_003 = 0x0C1E,
    T_TUG2_023 = 0x0C5A,
    N_suseme = 0x0CE9,
    N_WIRESNIP = 0x0D07,
    T_TUG2_024 = 0x0D2A,
    T_TUG2_008 = 0x0D86,
    N_kuseme = 0x0E42,
    N_zap = 0x0E84,
    N_NM8EXPLO = 0x0E9F,
    N_t2bbo = 0x0EB9,
    N_kusehe = 0x0F88,
    N_susehe = 0x0FEE,
    N_musehe = 0x105E,
    N_rusehe = 0x10C4,
    N_zap_2 = 0x111F,
    N_NM8EXPLO_2 = 0x113A,
    N_t2bbo_2 = 0x1154,
    N_fld10 = 0x1195,
    N_EFX15 = 0x11B6,
    N_zapdon_2 = 0x11D4,
    T_TUG2N026 = 0x11FC,
    N_fld10d_2 = 0x1290,
    T_TUG2_029 = 0x12B7,
    T_TUG2_030 = 0x132D,
    T_TUG2_028 = 0x13E9,
    T_TUG2_006 = 0x14D7,
    N_p5draw = 0x1575,
    N_kdrawe = 0x15D8,
    N_p5stun = 0x1607,
    N_t2beem03 = 0x1639,
    N_kdrawe_2 = 0x1679,
    N_p6stun = 0x16A8,
    N_t2beem02 = 0x16DA,
    N_kdrawe_3 = 0x171A,
    N_p5kill = 0x174D,
    N_t2beem05 = 0x177F,
    N_kdrawe_4 = 0x17BF,
    N_p6kill = 0x17F2,
    N_t2beem04 = 0x1824,
    T_TUG2N006 = 0x187A,
    T_TUG2N009 = 0x18B9,
    T_TUG2_112 = 0x194B,
    T_TUG2_021 = 0x19F8,
    S_1A7F = 0x1A7F,
    T_TUG2N012 = 0x1ACA,
    T_TUG2N003 = 0x1B16,
    T_TUG2_001 = 0x1B70,
    T_TUG2_002 = 0x1BA6,
    T_TUG2_018 = 0x1C00,
    T_TUG2_010 = 0x1C58,
    T_TUG2L004 = 0x1CA4,
    T_TUG2J003 = 0x1CFE,
    T_TUG2_005 = 0x1D70,
    T_TUG2_020 = 0x1DB1,
    T_TUG1_011 = 0x1E0E,
    N_wallht_2 = 0x1F06,
    N_t2beem00 = 0x1F24,
    N_p6fire = 0x1F53,
    S_2061 = 0x2061,
    N_kstune = 0x209F,
    N_t2beem15 = 0x20BD,
    N_p5fire = 0x20EC,
    N_kstune_2 = 0x2140,
    N_t2beem07 = 0x215E,
    N_p6fire_2 = 0x218D,
    N_sstune = 0x21E1,
    N_t2beem14 = 0x21FF,
    N_p5fire_2 = 0x222E,
    N_sstune_2 = 0x2282,
    N_t2beem08 = 0x22A0,
    N_p6fire_3 = 0x22CF,
    N_mstune = 0x2323,
    N_t2beem17 = 0x2341,
    N_p5fire_3 = 0x2370,
    N_mstune_2 = 0x23C4,
    N_t2beem09 = 0x23E2,
    N_p6fire_4 = 0x2411,
    N_rstune = 0x2465,
    N_t2beem16 = 0x2483,
    N_p5fire_4 = 0x24B2,
    N_rstune_2 = 0x2506,
    N_t2beem06 = 0x2524,
    N_p6fire_5 = 0x2553,
    N_kKille = 0x259A,
    N_t2beem19 = 0x25B8,
    N_p5fire_5 = 0x25E7,
    N_kKille_2 = 0x2621,
    N_t2beem11 = 0x263F,
    N_p6fire_6 = 0x266E,
    N_sKille = 0x26A8,
    N_t2beem20 = 0x26C6,
    N_p5fire_6 = 0x26F5,
    N_sKille_2 = 0x272F,
    N_t2beem12 = 0x274D,
    N_p6fire_7 = 0x277C,
    N_mKille = 0x27B6,
    N_t2beem21 = 0x27D4,
    N_p5fire_7 = 0x2803,
    N_mKille_2 = 0x283D,
    N_t2beem13 = 0x285B,
    N_p6fire_8 = 0x288A,
    N_rKille = 0x28C7,
    N_t2beem18 = 0x28E5,
    N_p5fire_8 = 0x2914,
    N_rKille_2 = 0x2955,
    N_t2beem10 = 0x2973,
    N_p6fire_9 = 0x29A2,
    N_sscane_2 = 0x29D3,
    T_TUG2_004 = 0x2A0B,
    N_mscane = 0x2AED,
    T_TUG2_016 = 0x2B25,
    N_mscane_2 = 0x2BD4,
    S_2C0C = 0x2C0C,
    T_TUG2L086 = 0x2CB0,
    T_TUG2_012 = 0x2D48,
    T_TUG2L086_2 = 0x2DA0,
    T_TUG2_013 = 0x2E38,
    T_TUG2_017 = 0x2E8E,
    T_TUG2_014 = 0x2EEA,
    T_TUG2_014_2 = 0x2F4D,
    T_TUG2N019 = 0x2FD6,
    T_TUG2N017 = 0x30A5,
    T_TUG2N020 = 0x3164,
    T_TUG2N018 = 0x3220,
    T_TUG2N021 = 0x32CC,
    T_TUG2N022 = 0x3379,
    T_TUG2N023 = 0x341B,
    N_fld10d_3 = 0x3468,
    T_TUG2_026 = 0x348F,
    T_TUG2_027 = 0x351A,
    N_mscane_3 = 0x356F,
    T_TUG2_019 = 0x35A6,
    T_TUG2_022 = 0x35FE,
    N_mscane_4 = 0x36B3,
    T_TUG2_015 = 0x36EA,
    N_mscane_5 = 0x3764,
    T_TUG2_015_2 = 0x379B,
    T_TUG2_025 = 0x3817,
    T_TUG2_025_2 = 0x386F,
    N_Capt_Kirk = 0x390F,
    N_Dr_McCoy = 0x391A,
    N_Mr_Spock = 0x3924,
    N_Lt_Christensen = 0x392E,
    N_Elasi_Guard = 0x393E,
    N_Masada_Crewman = 0x394A,
    T_TUG2N025 = 0x39FE,
    talk_buf = 0x3AED,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_killing CS_B(0x1EC3)
#define fv_killing2 CS_B(0x1EC4)
#define fv_shooter CS_B(0x1EC1)
#define fv_talk_buf CS_W(0x3AED)
#define fv_target CS_B(0x1EC2)
#define rv_been_here ROOM_B(0x5B)
#define rv_bomb ROOM_B(0x35)
#define rv_crew_freed ROOM_B(0x58)
#define rv_found_bomb ROOM_B(0x49)
#define rv_guard1 ROOM_B(0x4C)
#define rv_guard1_drew ROOM_B(0x59)
#define rv_guard2 ROOM_B(0x4D)
#define rv_guard_round ROOM_B(0x4B)
#define rv_heard_about_door ROOM_B(0x48)
#define rv_kirk_drew ROOM_B(0x5D)
#define rv_kirk_killed ROOM_B(0x4F)
#define rv_mccoy_killed ROOM_B(0x51)
#define rv_redshirt_killed ROOM_B(0x52)
#define rv_score ROOM_W(0x29)
#define rv_spock_assessed ROOM_B(0x45)
#define rv_spock_killed ROOM_B(0x50)
#define rv_talked_to_crew ROOM_B(0x5F)
#define rv_took_wiring ROOM_B(0x37)
#define rv_used_kill ROOM_B(0x4A)
#define rv_wall_scorched ROOM_B(0x4E)

/* functions */
static void tug2_StartRoom(void);
static void tug2_GuardsChallenge(void);
static void tug2_LookAtSwitch(void);
static void tug2_LookAtMcCoy(void);
static void tug2_LookAtSpock(void);
static void tug2_LookAtRedshirt(void);
static void tug2_GetBomb(void);
static void tug2_KirkAtBomb(void);
static void tug2_TookBomb(void);
static void tug2_LookAtBomb(void);
static void tug2_LookAtGuard1(void);
static void tug2_LookAtGuard2(void);
static void tug2_TalkToRedshirt(void);
static void tug2_LookAtWires(void);
static void tug2_E_090D(void);
static void tug2_SpockScansSwitch(void);
static void tug2_McCoyOnWires(void);
static void tug2_McCoyOnBomb(void);
static void tug2_ChristensenOnWires(void);
static void tug2_ChristensenAtWires(void);
static void tug2_ChristensenDefuses(void);
static void tug2_ChristensenBack(void);
static void tug2_KirkOnWires(void);
static void tug2_KirkAtWires(void);
static void tug2_SpockOnWires(void);
static void tug2_SpockDefuses(void);
static void tug2_SpockBack(void);
static void tug2_GetWires(void);
static void tug2_KirkAtWires2(void);
static void tug2_PulledWires(void);
static void tug2_KirkPressesSwitch(void);
static void tug2_KirkAtSwitch(void);
static void tug2_SpockPressesSwitch(void);
static void tug2_SpockAtSwitch(void);
static void tug2_McCoyPressesSwitch(void);
static void tug2_McCoyAtSwitch(void);
static void tug2_ChristensenPressesSwitch(void);
static void tug2_ChristensenAtSwitch(void);
static void tug2_E_10E7(void);
static void tug2_E_10EF(void);
static void tug2_E_10F7(void);
static void tug2_SwitchPressed(void);
static void tug2_BombWentOff(void);
static void tug2_CrewFreed(void);
static void tug2_E_14BA(void);
static void tug2_PhaserField(void);
static void tug2_Guard1Draws(void);
static void tug2_GuardsDown(void);
static void tug2_StunGuard1(void);
static void tug2_Guard1Stunned(void);
static void tug2_StunGuard2(void);
static void tug2_Guard2Stunned(void);
static void tug2_KillGuard1(void);
static void tug2_Guard1Killed(void);
static void tug2_KillGuard2(void);
static void tug2_Guard2Killed(void);
static void tug2_E_184F(void);
static void tug2_ChargeWelder(void);
static void tug2_WeldWireScraps(void);
static void tug2_WeldJunkMetal(void);
static void tug2_FitCombBit(void);
static void tug2_E_1A59(void);
static void tug2_KirkKilled(void);
static void tug2_WalkToDoor(void);
static void tug2_LookAtDoor(void);
static void tug2_LookAtKirk(void);
static void tug2_TalkToKirk(void);
static void tug2_TalkToMcCoy(void);
static void tug2_TalkToChristensen(void);
static void tug2_TalkToSpock(void);
static void tug2_UseCommunicator(void);
static void tug2_PickShooter(void);
static void tug2_GuardsFire(void);
static void tug2_GameOver(void);
static void tug2_SpockScansBomb(void);
static void tug2_McCoyScansGuard1(void);
static void tug2_McCoyScansGuard2(void);
static void tug2_TalkToGuard1(void);
static void tug2_TalkToGuard2(void);
static void tug2_E_2E7B(void);
static void tug2_MedkitOnWires(void);
static void tug2_MedkitOnGuard1(void);
static void tug2_MedkitOnGuard2(void);
static void tug2_E_2F9F(void);
static void tug2_E_2FA7(void);
static void tug2_LookAtScene(void);
static void tug2_TalkToPrisoners(void);
static void tug2_McCoyScansPrisoners(void);
static void tug2_part_36A4(void);
static void tug2_McCoyScansCell(void);
static void tug2_KillPrisoners(void);
static void tug2_StunPrisoners(void);
static void tug2_StunAnything(void);
static void tug2_KillAnything(void);
static void tug2_Talk(uint16_t ax);
static void tug2_Say(uint16_t ax, uint16_t bx);

/* The room starts: the bomb, the wires, the forcefield (on, off, or the
 * explosion's aftermath), and the guards. A stunned guard is tied up if
 * the crew is free, else he wakes and the party's injuries are forgotten.
 * The first time (been_here), the guards are ready: no walking, music 32.
 * [MSG_TICK 01 00 00] */
static void tug2_StartRoom(void)
{
    svc_Sfx(N_TUG2LOOP);
    if (rv_found_bomb != 1) {
        svc_Anim(0xB, N_t2bomb, -3, 1, 0);
    }
    if (rv_took_wiring != 1) {
        svc_Anim(0xC, N_t2wire, 0, 1, 0);
    }
    if (rv_crew_freed == 0) {
        svc_Anim(0xA, N_fld01, 0, 0, 0);
    }
    if (rv_crew_freed == 1) {
        svc_Anim(0xA, N_fld10d, 0, 0, 0);
    }
    if (rv_bomb == 6) {
        svc_Anim(0xA, N_zapdon, 0, 0, 0);
    }
    if (rv_guard1 == 8) {
        svc_Anim(8, N_p5tied, 0xE0, 0xB1, 0);
    }
    if (rv_guard1 == 1) {
        if (rv_crew_freed == 1) {
            svc_Anim(8, N_p5tied_2, 0xE0, 0xB1, 0);
            rv_guard1 = 8;
        }
    }
    if (rv_guard1 == 1) {
        if (rv_crew_freed == 0) {
            rv_guard1 = 0;
            rv_kirk_killed = 0;
            rv_spock_killed = 0;
            rv_mccoy_killed = 0;
            if (room.redshirt_dead == 0) {
                rv_redshirt_killed = 0;
            }
            rv_been_here = 0;
        }
    }
    if (rv_guard1 == 0) {
        svc_Anim(8, N_p5redy, 0xE0, 0xB1, 0);
    }
    if (rv_guard1 == 2) {
        svc_Anim(8, N_p5hat, 0xE0, 0xB1, 0);
    }
    if (rv_guard2 == 8) {
        svc_Anim(9, N_p6tied, 0xFB, 0xC5, 0);
    }
    if (rv_guard2 == 1) {
        if (rv_crew_freed == 1) {
            svc_Anim(9, N_p6tied_2, 0xFB, 0xC5, 0);
            rv_guard2 = 8;
        }
    }
    if (rv_guard2 == 1) {
        if (rv_crew_freed == 0) {
            rv_guard2 = 0;
            rv_kirk_killed = 0;
            rv_spock_killed = 0;
            rv_mccoy_killed = 0;
            if (room.redshirt_dead == 0) {
                rv_redshirt_killed = 0;
            }
            rv_been_here = 0;
        }
    }
    if (rv_guard2 == 0) {
        svc_Anim(9, N_p6draw, 0xFB, 0xC5, 0);
    }
    if (rv_guard2 == 2) {
        svc_Anim(9, N_p5hat_2, 0xFB, 0xC5, 0);
    }
    if (rv_wall_scorched != 0) {
        svc_Anim(0xD, N_wallht, 0, 0, 0);
    }
    if (rv_been_here == 0) {
        room.no_walk = 1;
        svc_Music(0x20, -1);
    }
    rv_been_here = 1;
    rv_kirk_drew = 0;
    return;
}

/* Tick 60, with a guard up: he challenges the party and timer 0 starts
 * the shooting.
 * [MSG_TICK 3C 00 00] */
static void tug2_GuardsChallenge(void)
{
    if (rv_guard1 != 0) {
        if (rv_guard2 != 0) {
            return;
        }
    }
    room.timers[0] = 0x3C;
    tug2_Say(N_Elasi_Guard, T_TUG2L085);
    tug2_Guard1Draws();
    return;
}

/* Looking at things.
 * [MSG_LOOK 20 00 00] */
static void tug2_LookAtSwitch(void)
{
    tug2_Say(0, T_TUG2N011);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void tug2_LookAtMcCoy(void)
{
    tug2_Say(0, T_TUG2N005);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void tug2_LookAtSpock(void)
{
    tug2_Say(0, T_TUG2N007);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void tug2_LookAtRedshirt(void)
{
    tug2_Say(0, T_TUG2N004);
    return;
}

/* Taking the bomb, once the field is off (msgs 12, 13).
 * [MSG_GET 0B 00 00] */
static void tug2_GetBomb(void)
{
    if (rv_crew_freed != 1) {
        tug2_Say(0, T_TUG2N024);
        return;
    }
    room.busy = 1;
    room.fall[0] = 0;
    svc_Walk(0, 0xC9, 0xA0, 0xC);
    return;
}

/* [MSG_WALK_DONE 0C 00 00] */
static void tug2_KirkAtBomb(void)
{
    svc_Anim(0, N_kusehn, -1, -1, 0xD);
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void tug2_TookBomb(void)
{
    svc_StopAnim(0xB);
    svc_GiveItem(IT_BOMB);
    rv_found_bomb = 1;
    room.busy = 0;
    return;
}

/* [MSG_LOOK 0B 00 00] */
static void tug2_LookAtBomb(void)
{
    tug2_Say(0, T_TUG2N000);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void tug2_LookAtGuard1(void)
{
    if (rv_guard1 == 0) {
        tug2_Say(0, T_TUG2N001);
        return;
    }
    if (rv_guard1 == 1) {
        tug2_Say(0, T_TUG2N013);
        return;
    }
    if (rv_guard1 == 8) {
        tug2_Say(0, T_TUG2N015);
        return;
    }
    if (rv_guard1 == 2) {
        tug2_Say(0, T_TUG2N106);
        return;
    }
    return;
}

/* [MSG_LOOK 09 00 00] */
static void tug2_LookAtGuard2(void)
{
    if (rv_guard2 == 0) {
        tug2_Say(0, T_TUG2N037);
        return;
    }
    if (rv_guard2 == 1) {
        tug2_Say(0, T_TUG2N014);
        return;
    }
    if (rv_guard2 == 8) {
        tug2_Say(0, T_TUG2N016);
        return;
    }
    if (rv_guard2 == 2) {
        tug2_Say(0, T_TUG2N106_2);
        return;
    }
    return;
}

/* Christensen.
 * [MSG_TALK 03 00 00] */
static void tug2_TalkToRedshirt(void)
{
    tug2_Say(N_Lt_Christensen, T_TUG2L002);
    return;
}

/* [MSG_LOOK 0C 00 00] */
static void tug2_LookAtWires(void)
{
    tug2_Say(0, T_TUG2N010);
    return;
}

/* The wires: Spock finds the booby trap. Spock or Christensen defuse it
 * (+2 score); McCoy won't, and Kirk asks Spock.
 * [MSG_USE 44 0C 00] */
static void tug2_E_090D(void)
{
    { tug2_SpockScansSwitch(); return; }
}

/* [MSG_USE 44 20 00] */
static void tug2_SpockScansSwitch(void)
{
    if (rv_bomb != 0) {
    } else {
        room.fall[1] = 2;
        svc_Anim(1, N_sscane, -1, -1, 0);
        svc_BridgeSfx(4);
        tug2_Say(N_Mr_Spock, T_TUG2_007);
        goto L09CF;
    }
L09CF:;
    return;
}

/* [MSG_USE 02 0C 00] */
static void tug2_McCoyOnWires(void)
{
    if (rv_bomb == 0) {
        tug2_Say(N_Dr_McCoy, T_TUG2_009);
        goto L0A4C;
    }
L0A4C:;
    return;
}

/* [MSG_USE 02 0B 00] */
static void tug2_McCoyOnBomb(void)
{
    tug2_Say(N_Dr_McCoy, T_TUG2_011);
    return;
}

/* [MSG_USE 03 0C 00] */
static void tug2_ChristensenOnWires(void)
{
    if (room.no_walk == 0) {
        if (rv_bomb == 0) {
            tug2_Say(N_Lt_Christensen, T_TUG2L007);
            room.fall[3] = 0;
            room.busy = 1;
            svc_Walk(3, 0xC9, 0xA0, 6);
        }
    }
    return;
}

/* [MSG_WALK_DONE 06 00 00] */
static void tug2_ChristensenAtWires(void)
{
    svc_Anim(3, N_rusemn, -1, -1, 7);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void tug2_ChristensenDefuses(void)
{
    tug2_Say(0, N_Snip_snip_snip);
    tug2_Say(N_Lt_Christensen, T_TUG2L011);
    rv_bomb = 4;
    room.fall[3] = 2;
    svc_Walk(3, 0x44, 0xC2, 8);
    return;
}

/* [MSG_WALK_DONE 08 00 00] */
static void tug2_ChristensenBack(void)
{
    room.busy = 0;
    rv_score += 2;
    return;
}

/* [MSG_USE 00 0C 00] */
static void tug2_KirkOnWires(void)
{
    if (room.no_walk == 0) {
        if (rv_bomb == 0) {
            room.busy = 1;
            room.fall[0] = 2;
            svc_Walk(0, 0xC9, 0xA0, 5);
        }
    }
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void tug2_KirkAtWires(void)
{
    tug2_Say(N_Capt_Kirk, T_TUG2_003);
    tug2_Say(N_Mr_Spock, T_TUG2_023);
    room.fall[0] = 2;
    svc_Walk(0, 0x66, 0xB8, 0);
    room.busy = 0;
    { tug2_SpockOnWires(); return; }
}

/* [MSG_USE 01 0C 00] */
static void tug2_SpockOnWires(void)
{
    if (room.no_walk == 0) {
        if (rv_bomb == 0) {
            room.fall[1] = 2;
            room.busy = 1;
            svc_Walk(1, 0xC9, 0xA0, 9);
        }
    }
    return;
}

/* [MSG_WALK_DONE 09 00 00] */
static void tug2_SpockDefuses(void)
{
    svc_Anim(1, N_suseme, -1, -1, 0);
    svc_Sfx(N_WIRESNIP);
    tug2_Say(N_Mr_Spock, T_TUG2_024);
    rv_bomb = 4;
    if (rv_spock_assessed == 1) {
        tug2_Say(N_Mr_Spock, T_TUG2_008);
        goto L0DD9;
    }
L0DD9:;
    room.fall[1] = 2;
    svc_Walk(1, 0x56, 0xA9, 0xB);
    return;
}

/* [MSG_WALK_DONE 0B 00 00] */
static void tug2_SpockBack(void)
{
    room.busy = 0;
    rv_score += 2;
    return;
}

/* Pulling the wires (msgs 28, 29): if armed, the bomb goes off; if
 * defused, Kirk has the wiring, +3 score.
 * [MSG_GET 0C 00 00] */
static void tug2_GetWires(void)
{
    if (room.no_walk == 0) {
        room.busy = 1;
        room.fall[0] = 2;
        svc_Walk(0, 0xC9, 0xA0, 0x1C);
    }
    return;
}

/* [MSG_WALK_DONE 1C 00 00] */
static void tug2_KirkAtWires2(void)
{
    svc_Anim(0, N_kuseme, -1, -1, 0x1D);
    return;
}

/* [MSG_ANIM_DONE 1D 00 00] */
static void tug2_PulledWires(void)
{
    if (rv_bomb != 0) {
    } else {
        svc_Music(2, -1);
        svc_Anim(0xA, N_zap, 0, 0, 1);
        svc_Sfx(N_NM8EXPLO);
        svc_Anim(0xB, N_t2bbo, -3, 1, 0);
        svc_BridgeSfx(6);
        svc_BridgeSfx(0x16);
        rv_bomb = 6;
        svc_StopAnim(0xC);
        svc_GiveItem(IT_WIRING);
        rv_took_wiring = 1;
        room.busy = 0;
        rv_score += 3;
        return;
    }
    svc_StopAnim(0xC);
    svc_GiveItem(IT_WIRING);
    rv_took_wiring = 1;
    room.busy = 0;
    rv_score += 3;
    return;
}

/* The switch (msgs 14-21): if armed, the bomb goes off; if defused, the
 * field goes down and the crew are free, +6 score.
 * [MSG_USE 00 20 00] */
static void tug2_KirkPressesSwitch(void)
{
    if (room.no_walk == 0) {
        room.busy = 1;
        room.fall[0] = 2;
        svc_Walk(0, 0xC9, 0xA0, 0xE);
    }
    return;
}

/* [MSG_WALK_DONE 0E 00 00] */
static void tug2_KirkAtSwitch(void)
{
    svc_Anim(0, N_kusehe, -1, -1, 0xF);
    svc_BridgeSfx(7);
    return;
}

/* [MSG_USE 01 20 00] */
static void tug2_SpockPressesSwitch(void)
{
    if (room.no_walk == 0) {
        room.busy = 1;
        room.fall[1] = 2;
        svc_Walk(1, 0xC9, 0xA0, 0x10);
    }
    return;
}

/* [MSG_WALK_DONE 10 00 00] */
static void tug2_SpockAtSwitch(void)
{
    svc_Anim(1, N_susehe, -1, -1, 0x11);
    svc_BridgeSfx(7);
    return;
}

/* [MSG_USE 02 20 00] */
static void tug2_McCoyPressesSwitch(void)
{
    room.busy = 1;
    room.fall[2] = 2;
    svc_Walk(2, 0xC9, 0xA0, 0x12);
    return;
}

/* [MSG_WALK_DONE 12 00 00] */
static void tug2_McCoyAtSwitch(void)
{
    svc_BridgeSfx(7);
    svc_Anim(2, N_musehe, -1, -1, 0x13);
    return;
}

/* [MSG_USE 03 20 00] */
static void tug2_ChristensenPressesSwitch(void)
{
    if (room.no_walk == 0) {
        room.busy = 1;
        room.fall[3] = 2;
        svc_Walk(3, 0xC9, 0xA0, 0x14);
    }
    return;
}

/* [MSG_WALK_DONE 14 00 00] */
static void tug2_ChristensenAtSwitch(void)
{
    svc_BridgeSfx(7);
    svc_Anim(3, N_rusehe, -1, -1, 0x15);
    return;
}

/* [MSG_ANIM_DONE 0F 00 00] */
static void tug2_E_10E7(void)
{
    { tug2_SwitchPressed(); return; }
}

/* [MSG_ANIM_DONE 11 00 00] */
static void tug2_E_10EF(void)
{
    { tug2_SwitchPressed(); return; }
}

/* [MSG_ANIM_DONE 13 00 00] */
static void tug2_E_10F7(void)
{
    { tug2_SwitchPressed(); return; }
}

/* [MSG_ANIM_DONE 15 00 00] */
static void tug2_SwitchPressed(void)
{
    room.busy = 0;
    if (rv_bomb == 0) {
        svc_Music(2, -1);
        svc_Anim(0xA, N_zap_2, 0, 0, 1);
        svc_Sfx(N_NM8EXPLO_2);
        svc_Anim(0xB, N_t2bbo_2, -3, 1, 0);
        rv_bomb = 6;
        return;
    }
    if (rv_bomb == 4) {
        if (rv_crew_freed == 0) {
            rv_crew_freed = 1;
            svc_Anim(0xA, N_fld10, 0, 0, 2);
            rv_score += 6;
            svc_Sfx(N_EFX15);
        }
    }
    return;
}

/* The bomb has gone off (anim msg 1): the field held, the prisoners died.
 * [MSG_ANIM_DONE 01 00 00] */
static void tug2_BombWentOff(void)
{
    svc_Anim(0xA, N_zapdon_2, 0, 0, 0);
    tug2_Say(0, T_TUG2N026);
    return;
}

/* The field is down (anim msg 2): the crew tell of the bridge door.
 * [MSG_ANIM_DONE 02 00 00] */
static void tug2_CrewFreed(void)
{
    svc_Anim(0xA, N_fld10d_2, 0, 0, 0);
    tug2_Say(N_Masada_Crewman, T_TUG2_029);
    tug2_Say(N_Masada_Crewman, T_TUG2_030);
    tug2_Say(N_Masada_Crewman, T_TUG2_028);
    rv_heard_about_door = 1;
    return;
}

/* Phasers on the field.
 * [MSG_USE 40 0A 00] */
static void tug2_E_14BA(void)
{
    { tug2_PhaserField(); return; }
}

/* [MSG_USE 41 0A 00] */
static void tug2_PhaserField(void)
{
    if (rv_bomb != 0) {
    } else {
        tug2_Say(N_Mr_Spock, T_TUG2_006);
        goto L1566;
    }
L1566:;
    return;
}

/* Guard 1 draws his weapon, once (near call). */
static void tug2_Guard1Draws(void)
{
    if (rv_guard1 == 0) {
        if (rv_guard1_drew == 0) {
            svc_Anim(8, N_p5draw, -1, -1, 0);
            rv_guard1_drew = 1;
        }
    }
    return;
}

/* Both guards down: walking again, and Kirk puts his phaser away. */
static void tug2_GuardsDown(void)
{
    if (rv_guard1 != 0) {
        if (rv_guard2 != 0) {
            room.no_walk = 0;
            if (rv_kirk_drew == 1) {
                svc_StopAnim(0);
            }
        }
    }
    return;
}

/* Phasers on the guards: Kirk draws (once) and fires (msgs 22-25). When
 * both guards are down the party can walk again.
 * [MSG_USE 40 08 00] */
static void tug2_StunGuard1(void)
{
    if (rv_guard1 != 0)
        { tug2_Guard1Stunned(); return; }
    tug2_Guard1Draws();
    room.busy = 1;
    if (rv_kirk_drew != 0)
        { tug2_Guard1Stunned(); return; }
    svc_Anim(0, N_kdrawe, -1, -1, 0x16);
    rv_kirk_drew = 1;
    return;
}

/* [MSG_ANIM_DONE 16 00 00] */
static void tug2_Guard1Stunned(void)
{
    if (rv_guard1 == 0) {
        svc_Anim(8, N_p5stun, -1, -1, 0);
        rv_guard1 = 1;
        svc_BridgeSfx(6);
        svc_FlashBitmap(N_t2beem03);
        tug2_GuardsDown();
    }
    room.busy = 0;
    return;
}

/* [MSG_USE 40 09 00] */
static void tug2_StunGuard2(void)
{
    if (rv_guard2 != 0)
        { tug2_Guard2Stunned(); return; }
    tug2_Guard1Draws();
    room.busy = 1;
    if (rv_kirk_drew != 0)
        { tug2_Guard2Stunned(); return; }
    svc_Anim(0, N_kdrawe_2, -1, -1, 0x17);
    rv_kirk_drew = 1;
    return;
}

/* [MSG_ANIM_DONE 17 00 00] */
static void tug2_Guard2Stunned(void)
{
    if (rv_guard2 == 0) {
        svc_Anim(9, N_p6stun, -1, -1, 0);
        rv_guard2 = 1;
        svc_BridgeSfx(6);
        svc_FlashBitmap(N_t2beem02);
        tug2_GuardsDown();
    }
    room.busy = 0;
    return;
}

/* [MSG_USE 41 08 00] */
static void tug2_KillGuard1(void)
{
    if (rv_guard1 != 0)
        { tug2_Guard1Killed(); return; }
    tug2_Guard1Draws();
    room.busy = 1;
    if (rv_kirk_drew != 0)
        { tug2_Guard1Killed(); return; }
    svc_Anim(0, N_kdrawe_3, -1, -1, 0x18);
    rv_kirk_drew = 1;
    rv_used_kill = 1;
    return;
}

/* [MSG_ANIM_DONE 18 00 00] */
static void tug2_Guard1Killed(void)
{
    if (rv_guard1 == 0) {
        svc_Anim(8, N_p5kill, -1, -1, 0);
        rv_guard1 = 2;
        svc_BridgeSfx(6);
        svc_FlashBitmap(N_t2beem05);
        tug2_GuardsDown();
    }
    room.busy = 0;
    return;
}

/* [MSG_USE 41 09 00] */
static void tug2_KillGuard2(void)
{
    if (rv_guard2 != 0)
        { tug2_Guard2Killed(); return; }
    tug2_Guard1Draws();
    room.busy = 1;
    if (rv_kirk_drew != 0)
        { tug2_Guard2Killed(); return; }
    svc_Anim(0, N_kdrawe_4, -1, -1, 0x19);
    rv_kirk_drew = 1;
    rv_used_kill = 1;
    return;
}

/* [MSG_ANIM_DONE 19 00 00] */
static void tug2_Guard2Killed(void)
{
    if (rv_guard2 == 0) {
        svc_Anim(9, N_p6kill, -1, -1, 0);
        rv_guard2 = 2;
        svc_BridgeSfx(6);
        svc_FlashBitmap(N_t2beem04);
        tug2_GuardsDown();
    }
    room.busy = 0;
    return;
}

/* Inventory, as in rooms 0 and 1.
 * [MSG_USE 40 56 00] */
static void tug2_E_184F(void)
{
    { tug2_ChargeWelder(); return; }
}

/* [MSG_USE 41 56 00] */
static void tug2_ChargeWelder(void)
{
    svc_TakeItem(IT_PWE);
    svc_GiveItem(IT_PWF);
    tug2_Say(0, T_TUG2N006);
    return;
}

/* [MSG_USE 55 54 00] */
static void tug2_WeldWireScraps(void)
{
    tug2_Say(0, T_TUG2N009);
    svc_TakeItem(IT_WIRSCRP);
    return;
}

/* [MSG_USE 55 52 00] */
static void tug2_WeldJunkMetal(void)
{
    tug2_Say(N_Mr_Spock, T_TUG2_112);
    svc_TakeItem(IT_JNKMETL);
    svc_GiveItem(IT_COMBBIT);
    return;
}

/* [MSG_USE 51 4F 00] */
static void tug2_FitCombBit(void)
{
    tug2_Say(N_Mr_Spock, T_TUG2_021);
    svc_TakeItem(IT_COMBBIT);
    svc_TakeItem(IT_RT);
    svc_GiveItem(IT_RTWB);
    return;
}

/* Kirk has been killed (anim msgs 26, 27): game over.
 * [MSG_ANIM_DONE 1A 00 00] */
static void tug2_E_1A59(void)
{
    { tug2_KirkKilled(); return; }
}

/* [MSG_ANIM_DONE 1B 00 00] */
static void tug2_KirkKilled(void)
{
    tug2_Say(0, T_TUG2N025);
    svc_GameOver();
    tug2_Say(0, S_1A7F);
    return;
}

/* The door, and looking and talking.
 * [MSG_WALK 21 00 00] */
static void tug2_WalkToDoor(void)
{
    svc_Walk(0, 0xB, 0xAB, 0);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void tug2_LookAtDoor(void)
{
    tug2_Say(0, T_TUG2N012);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void tug2_LookAtKirk(void)
{
    tug2_Say(0, T_TUG2N003);
    return;
}

/* [MSG_TALK 00 00 00] */
static void tug2_TalkToKirk(void)
{
    if (rv_bomb == 6) {
        tug2_Say(N_Capt_Kirk, T_TUG2_001);
        return;
    }
    tug2_Say(N_Capt_Kirk, T_TUG2_002);
    return;
}

/* [MSG_TALK 02 00 00] */
static void tug2_TalkToMcCoy(void)
{
    if (rv_bomb == 6) {
        tug2_Say(N_Dr_McCoy, T_TUG2_018);
        return;
    }
    tug2_Say(N_Dr_McCoy, T_TUG2_010);
    return;
}

/* [MSG_TALK 03 00 00] */
static void tug2_TalkToChristensen(void)
{
    if (rv_bomb == 6) {
        tug2_Say(N_Lt_Christensen, T_TUG2L004);
        return;
    }
    tug2_Say(N_Lt_Christensen, T_TUG2J003);
    return;
}

/* [MSG_TALK 01 00 00] */
static void tug2_TalkToSpock(void)
{
    if (rv_bomb == 6) {
        tug2_Say(N_Mr_Spock, T_TUG2_005);
        return;
    }
    tug2_Say(N_Mr_Spock, T_TUG2_020);
    return;
}

/* [MSG_USE 47 FF 00] */
static void tug2_UseCommunicator(void)
{
    tug2_Say(N_Mr_Spock, T_TUG1_011);
    return;
}

/* The guard still standing fires: guard 1 if both are. */
static void tug2_PickShooter(void)
{
    if (rv_guard2 == 0) {
        fv_shooter = 9;
    }
    if (rv_guard1 == 0) {
        fv_shooter = 8;
    }
    return;
}

/* Timer 0, while a guard is up: the guards fire in rounds (guard_round)
 * at Kirk, Christensen, McCoy, then Kirk or Spock again: stunning them,
 * or killing them if `killing'. A dead Kirk (timer 2) ends the game.
 * [MSG_TIMER 00 00 00] */
static void tug2_GuardsFire(void)
{
    uint16_t ax = 0;
    if (rv_guard1 != 0) {
        if (rv_guard2 != 0) {
            return;
        }
    }
    fv_killing = 0;
    if (rv_used_kill == 1) {
        fv_killing = 1;
    }
    if (rv_guard_round != 0) {
    } else {
        rv_guard_round += 1;
        if (rv_guard2 != 0) {
            return;
        }
        if (rv_guard2 == 0) {
            svc_Anim(0xD, N_wallht_2, 0, 0, 0);
            svc_FlashBitmap(N_t2beem00);
            svc_BridgeSfx(6);
            ax = (uint16_t)svc_Anim(9, N_p6fire, -1, -1, 0);
            SETLO(ax, fv_killing);
            rv_wall_scorched = LO(ax);
            room.timers[0] = 0x3C;
            return;
        }
        return;
    }
    if (rv_guard_round == 1) {
        rv_guard_round += 1;
        if (rv_redshirt_killed == 0) {
            tug2_PickShooter();
            fv_target = 3;
            room.timers[0] = 0x3C;
            goto L2078;
        }
        return;
    }
    if (rv_guard_round == 2) {
        rv_guard_round += 1;
        fv_killing2 = 0;
        if (rv_used_kill == 1) {
            fv_killing2 = 1;
        }
        if (fv_killing2 == 0) {
            tug2_PickShooter();
            fv_target = 1;
            room.timers[0] = 0x3C;
            goto L2078;
        }
        if (fv_killing2 == 1) {
            tug2_PickShooter();
            fv_target = 0;
            room.timers[0] = 0x3C;
            goto L2078;
        }
        return;
    }
    if (rv_guard_round == 3) {
        rv_guard_round += 1;
        if (rv_mccoy_killed == 0) {
            tug2_PickShooter();
            fv_target = 2;
            room.timers[0] = 0x50;
            goto L2078;
        }
        return;
    }
    if (rv_guard_round == 4) {
        rv_guard_round += 1;
        if (fv_killing2 == 0) {
            tug2_PickShooter();
            fv_target = 0;
            room.timers[2] = 0x28;
            goto L2078;
        }
        if (fv_killing2 == 1) {
            tug2_PickShooter();
            fv_target = 1;
            room.timers[2] = 0x28;
            goto L2078;
        }
        return;
    }
    return;
L2078:;
    if (fv_shooter != 8) {
    } else {
        if (fv_target != 0) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[0] = 3;
                svc_Anim(0, N_kstune, -1, -1, 0x1A);
                svc_FlashBitmap(N_t2beem15);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire, -1, -1, 0);
                rv_kirk_killed = 0;
                room.fall[0] = 3;
                room.stun[0] = 0x78;
                room.flags |= 1;
            }
        }
    }
    if (fv_shooter != 9) {
    } else {
        if (fv_target != 0) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[0] = 3;
                svc_Anim(0, N_kstune_2, -1, -1, 0x1A);
                svc_FlashBitmap(N_t2beem07);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_2, -1, -1, 0);
                rv_kirk_killed = 0;
                room.fall[0] = 3;
                room.stun[0] = 0x78;
                room.flags |= 1;
            }
        }
    }
    if (fv_shooter != 8) {
    } else {
        if (fv_target != 1) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[1] = 3;
                svc_Anim(1, N_sstune, -1, -1, 0);
                svc_FlashBitmap(N_t2beem14);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_2, -1, -1, 0);
                rv_spock_killed = 0;
                room.fall[1] = 3;
                room.stun[1] = 0x78;
                room.flags |= 2;
            }
        }
    }
    if (fv_shooter != 9) {
    } else {
        if (fv_target != 1) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[1] = 3;
                svc_Anim(1, N_sstune_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem08);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_3, -1, -1, 0);
                rv_spock_killed = 0;
                room.fall[1] = 3;
                room.stun[1] = 0x78;
                room.flags |= 2;
            }
        }
    }
    if (fv_shooter != 8) {
    } else {
        if (fv_target != 2) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[2] = 3;
                svc_Anim(2, N_mstune, -1, -1, 0);
                svc_FlashBitmap(N_t2beem17);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_3, -1, -1, 0);
                rv_mccoy_killed = 0;
                room.fall[2] = 3;
                room.stun[2] = 0x78;
                room.flags |= 4;
            }
        }
    }
    if (fv_shooter != 9) {
    } else {
        if (fv_target != 2) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[2] = 3;
                svc_Anim(2, N_mstune_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem09);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_4, -1, -1, 0);
                rv_mccoy_killed = 0;
                room.fall[2] = 3;
                room.stun[2] = 0x78;
                room.flags |= 4;
            }
        }
    }
    if (fv_shooter != 8) {
    } else {
        if (fv_target != 3) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[3] = 3;
                svc_Anim(3, N_rstune, -1, -1, 0);
                svc_FlashBitmap(N_t2beem16);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_4, -1, -1, 0);
                rv_redshirt_killed = 0;
                room.fall[3] = 3;
                room.stun[3] = 0x78;
                room.flags |= 8;
            }
        }
    }
    if (fv_shooter != 9) {
    } else {
        if (fv_target != 3) {
        } else {
            if (fv_killing != 0) {
            } else {
                room.fall[3] = 3;
                svc_Anim(3, N_rstune_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem06);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_5, -1, -1, 0);
                rv_redshirt_killed = 0;
                room.fall[3] = 3;
                room.stun[3] = 0x78;
                room.flags |= 8;
            }
        }
    }
    if (fv_shooter == 8) {
        if (fv_target == 0) {
            if (fv_killing == 1) {
                svc_Anim(0, N_kKille, -1, -1, 0x1B);
                svc_FlashBitmap(N_t2beem19);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_5, -1, -1, 0);
                rv_kirk_killed = 1;
            }
        }
    }
    if (fv_shooter == 9) {
        if (fv_target == 0) {
            if (fv_killing == 1) {
                svc_Anim(0, N_kKille_2, -1, -1, 0x1B);
                svc_FlashBitmap(N_t2beem11);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_6, -1, -1, 0);
                rv_kirk_killed = 1;
            }
        }
    }
    if (fv_shooter == 8) {
        if (fv_target == 1) {
            if (fv_killing == 1) {
                svc_Anim(1, N_sKille, -1, -1, 0);
                svc_FlashBitmap(N_t2beem20);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_6, -1, -1, 0);
                rv_spock_killed = 1;
            }
        }
    }
    if (fv_shooter == 9) {
        if (fv_target == 1) {
            if (fv_killing == 1) {
                svc_Anim(1, N_sKille_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem12);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_7, -1, -1, 0);
                rv_spock_killed = 1;
            }
        }
    }
    if (fv_shooter == 8) {
        if (fv_target == 2) {
            if (fv_killing == 1) {
                svc_Anim(2, N_mKille, -1, -1, 0);
                svc_FlashBitmap(N_t2beem21);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_7, -1, -1, 0);
                rv_mccoy_killed = 1;
            }
        }
    }
    if (fv_shooter == 9) {
        if (fv_target == 2) {
            if (fv_killing == 1) {
                svc_Anim(2, N_mKille_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem13);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_8, -1, -1, 0);
                rv_mccoy_killed = 1;
            }
        }
    }
    if (fv_shooter != 8) {
    } else {
        if (fv_target == 3) {
            if (fv_killing == 1) {
                svc_Anim(3, N_rKille, -1, -1, 0);
                svc_FlashBitmap(N_t2beem18);
                svc_BridgeSfx(6);
                svc_Anim(8, N_p5fire_8, -1, -1, 0);
                rv_redshirt_killed = 1;
                room.redshirt_dead = 1;
            }
        }
    }
    if (fv_shooter != 9) {
    } else {
        if (fv_target == 3) {
            if (fv_killing == 1) {
                svc_Anim(3, N_rKille_2, -1, -1, 0);
                svc_FlashBitmap(N_t2beem10);
                svc_BridgeSfx(6);
                svc_Anim(9, N_p6fire_9, -1, -1, 0);
                rv_redshirt_killed = 1;
                room.redshirt_dead = 1;
            }
        }
    }
    return;
}

/* [MSG_TIMER 02 00 00] */
static void tug2_GameOver(void)
{
    svc_GameOver();
    tug2_Say(0, S_2061);
    return;
}

/* Tricorders, talking to the guards, the medkit.
 * [MSG_USE 44 0B 00] */
static void tug2_SpockScansBomb(void)
{
    room.fall[1] = 2;
    svc_Anim(1, N_sscane_2, -1, -1, 0);
    svc_BridgeSfx(4);
    tug2_Say(N_Mr_Spock, T_TUG2_004);
    return;
}

/* [MSG_USE 45 08 00] */
static void tug2_McCoyScansGuard1(void)
{
    if (rv_guard1 != 1) {
    } else {
        room.fall[2] = 2;
        svc_Anim(2, N_mscane, -1, -1, 0);
        svc_BridgeSfx(4);
        tug2_Say(N_Dr_McCoy, T_TUG2_016);
        goto L2BBE;
    }
L2BBE:;
    return;
}

/* [MSG_USE 45 09 00] */
static void tug2_McCoyScansGuard2(void)
{
    if (rv_guard2 != 1) {
    } else {
        room.fall[2] = 2;
        svc_Anim(2, N_mscane_2, -1, -1, 0);
        svc_BridgeSfx(4);
        tug2_Say(N_Dr_McCoy, S_2C0C);
        goto L2C94;
    }
L2C94:;
    return;
}

/* [MSG_TALK 08 00 00] */
static void tug2_TalkToGuard1(void)
{
    if (rv_guard1 != 8) {
    } else {
        tug2_Say(N_Elasi_Guard, T_TUG2L086);
        goto L2D37;
    }
L2D37:;
    if (rv_guard1 == 1) {
        tug2_Say(N_Dr_McCoy, T_TUG2_012);
        goto L2D84;
    }
L2D84:;
    return;
}

/* [MSG_TALK 09 00 00] */
static void tug2_TalkToGuard2(void)
{
    if (rv_guard2 != 8) {
    } else {
        tug2_Say(N_Elasi_Guard, T_TUG2L086_2);
        goto L2E27;
    }
L2E27:;
    if (rv_guard2 == 1) {
        tug2_Say(N_Dr_McCoy, T_TUG2_013);
        goto L2E74;
    }
L2E74:;
    return;
}

/* [MSG_USE 7F 0B 00] */
static void tug2_E_2E7B(void)
{
    { tug2_MedkitOnWires(); return; }
}

/* [MSG_USE 7F 0C 00] */
static void tug2_MedkitOnWires(void)
{
    tug2_Say(N_Dr_McCoy, T_TUG2_017);
    return;
}

/* [MSG_USE 7F 08 00] */
static void tug2_MedkitOnGuard1(void)
{
    if (rv_guard1 == 1) {
        tug2_Say(N_Dr_McCoy, T_TUG2_014);
        goto L2F35;
    }
L2F35:;
    return;
}

/* [MSG_USE 7F 09 00] */
static void tug2_MedkitOnGuard2(void)
{
    if (rv_guard2 == 1) {
        tug2_Say(N_Dr_McCoy, T_TUG2_014_2);
        goto L2F98;
    }
L2F98:;
    return;
}

/* The room, depending on the guards, the bomb and the prisoners.
 * [MSG_LOOK 0A 00 00] */
static void tug2_E_2F9F(void)
{
    { tug2_LookAtScene(); return; }
}

/* [MSG_LOOK 22 00 00] */
static void tug2_E_2FA7(void)
{
    { tug2_LookAtScene(); return; }
}

/* [MSG_LOOK FF 00 00] */
static void tug2_LookAtScene(void)
{
    if (rv_guard1 != 2) {
    } else {
        if (rv_guard2 != 2) {
        } else {
            if (rv_bomb != 6) {
            } else {
                tug2_Say(0, T_TUG2N019);
                return;
            }
        }
    }
    if (rv_guard1 != 2) {
    } else {
        if (rv_guard2 != 2) {
        } else {
            if (rv_crew_freed != 0) {
            } else {
                tug2_Say(0, T_TUG2N017);
                return;
            }
        }
    }
    if (rv_guard1 != 1) {
    } else {
        if (rv_guard2 != 1) {
        } else {
            if (rv_bomb != 6) {
            } else {
                tug2_Say(0, T_TUG2N020);
                return;
            }
        }
    }
    if (rv_guard1 != 1) {
    } else {
        if (rv_guard2 != 1) {
        } else {
            if (rv_crew_freed != 0) {
            } else {
                tug2_Say(0, T_TUG2N018);
                return;
            }
        }
    }
    if (rv_guard1 != 1) {
    } else {
        if (rv_guard2 != 1) {
        } else {
            if (rv_crew_freed != 1) {
            } else {
                tug2_Say(0, T_TUG2N021);
                return;
            }
        }
    }
    if (rv_guard1 != 2) {
    } else {
        if (rv_guard2 != 2) {
        } else {
            if (rv_crew_freed != 1) {
            } else {
                tug2_Say(0, T_TUG2N022);
                return;
            }
        }
    }
    tug2_Say(0, T_TUG2N023);
    return;
}

/* The prisoners.
 * [MSG_TALK 0A 00 00] */
static void tug2_TalkToPrisoners(void)
{
    if (rv_crew_freed != 1) {
    } else {
        if (rv_talked_to_crew != 0) {
        } else {
            svc_Anim(0xA, N_fld10d_3, 0, 0, 0);
            tug2_Say(N_Masada_Crewman, T_TUG2_026);
            rv_talked_to_crew = 1;
        }
    }
    if (rv_crew_freed != 1) {
        { tug2_part_36A4(); return; }
    }
    if (rv_talked_to_crew != 1) {
        { tug2_part_36A4(); return; }
    }
    tug2_Say(N_Masada_Crewman, T_TUG2_027);
    return;
}

/* [MSG_USE 45 0A 00] */
static void tug2_McCoyScansPrisoners(void)
{
    if (rv_bomb != 6) {
        { tug2_part_36A4(); return; }
    }
    room.fall[2] = 2;
    svc_Anim(2, N_mscane_3, -1, -1, 0);
    svc_BridgeSfx(4);
    tug2_Say(N_Dr_McCoy, T_TUG2_019);
    tug2_Say(N_Mr_Spock, T_TUG2_022);
    return;
}

static void tug2_part_36A4(void)
{
    if (rv_crew_freed != 0) {
    } else {
        room.fall[2] = 2;
        svc_Anim(2, N_mscane_4, -1, -1, 0);
        svc_BridgeSfx(4);
        tug2_Say(N_Dr_McCoy, T_TUG2_015);
        goto L374E;
    }
L374E:;
    return;
}

/* [MSG_USE 45 22 00] */
static void tug2_McCoyScansCell(void)
{
    if (rv_crew_freed != 1) {
    } else {
        room.fall[2] = 2;
        svc_Anim(2, N_mscane_5, -1, -1, 0);
        svc_BridgeSfx(4);
        tug2_Say(N_Dr_McCoy, T_TUG2_015_2);
        goto L37FF;
    }
L37FF:;
    return;
}

/* [MSG_USE 41 0A 00] */
static void tug2_KillPrisoners(void)
{
    if (rv_crew_freed == 1) {
        tug2_Say(N_Masada_Crewman, T_TUG2_025);
        goto L385D;
    }
L385D:;
    return;
}

/* [MSG_USE 40 0A 00] */
static void tug2_StunPrisoners(void)
{
    tug2_Say(N_Masada_Crewman, T_TUG2_025_2);
    return;
}

/* Phasers on anything else: nothing.
 * [MSG_USE 40 FF 00] */
static void tug2_StunAnything(void)
{
    return;
}

/* [MSG_USE 41 FF 00] */
static void tug2_KillAnything(void)
{
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void tug2_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void tug2_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x3AEF) = bx;
    tug2_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript tug2_scripts[] = {
    { 0x010C, tug2_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x03C0, tug2_GuardsChallenge },    /* MSG_TICK 3C 00 00 */
    { 0x0418, tug2_LookAtSwitch },    /* MSG_LOOK 20 00 00 */
    { 0x0482, tug2_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x04C3, tug2_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x04FD, tug2_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0549, tug2_GetBomb },    /* MSG_GET 0B 00 00 */
    { 0x05CB, tug2_KirkAtBomb },    /* MSG_WALK_DONE 0C 00 00 */
    { 0x05F0, tug2_TookBomb },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x0620, tug2_LookAtBomb },    /* MSG_LOOK 0B 00 00 */
    { 0x066C, tug2_LookAtGuard1 },    /* MSG_LOOK 08 00 00 */
    { 0x0771, tug2_LookAtGuard2 },    /* MSG_LOOK 09 00 00 */
    { 0x0876, tug2_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x08B9, tug2_LookAtWires },    /* MSG_LOOK 0C 00 00 */
    { 0x090D, tug2_E_090D },    /* MSG_USE 44 0C 00 */
    { 0x0915, tug2_SpockScansSwitch },    /* MSG_USE 44 20 00 */
    { 0x09D6, tug2_McCoyOnWires },    /* MSG_USE 02 0C 00 */
    { 0x0A53, tug2_McCoyOnBomb },    /* MSG_USE 02 0B 00 */
    { 0x0AB3, tug2_ChristensenOnWires },    /* MSG_USE 03 0C 00 */
    { 0x0B2A, tug2_ChristensenAtWires },    /* MSG_WALK_DONE 06 00 00 */
    { 0x0B4F, tug2_ChristensenDefuses },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0BCD, tug2_ChristensenBack },    /* MSG_WALK_DONE 08 00 00 */
    { 0x0BDC, tug2_KirkOnWires },    /* MSG_USE 00 0C 00 */
    { 0x0C13, tug2_KirkAtWires },    /* MSG_WALK_DONE 05 00 00 */
    { 0x0CB0, tug2_SpockOnWires },    /* MSG_USE 01 0C 00 */
    { 0x0CE7, tug2_SpockDefuses },    /* MSG_WALK_DONE 09 00 00 */
    { 0x0E00, tug2_SpockBack },    /* MSG_WALK_DONE 0B 00 00 */
    { 0x0E0F, tug2_GetWires },    /* MSG_GET 0C 00 00 */
    { 0x0E40, tug2_KirkAtWires2 },    /* MSG_WALK_DONE 1C 00 00 */
    { 0x0E65, tug2_PulledWires },    /* MSG_ANIM_DONE 1D 00 00 */
    { 0x0F55, tug2_KirkPressesSwitch },    /* MSG_USE 00 20 00 */
    { 0x0F86, tug2_KirkAtSwitch },    /* MSG_WALK_DONE 0E 00 00 */
    { 0x0FBB, tug2_SpockPressesSwitch },    /* MSG_USE 01 20 00 */
    { 0x0FEC, tug2_SpockAtSwitch },    /* MSG_WALK_DONE 10 00 00 */
    { 0x1021, tug2_McCoyPressesSwitch },    /* MSG_USE 02 20 00 */
    { 0x104C, tug2_McCoyAtSwitch },    /* MSG_WALK_DONE 12 00 00 */
    { 0x1081, tug2_ChristensenPressesSwitch },    /* MSG_USE 03 20 00 */
    { 0x10B2, tug2_ChristensenAtSwitch },    /* MSG_WALK_DONE 14 00 00 */
    { 0x10E7, tug2_E_10E7 },    /* MSG_ANIM_DONE 0F 00 00 */
    { 0x10EF, tug2_E_10EF },    /* MSG_ANIM_DONE 11 00 00 */
    { 0x10F7, tug2_E_10F7 },    /* MSG_ANIM_DONE 13 00 00 */
    { 0x10FF, tug2_SwitchPressed },    /* MSG_ANIM_DONE 15 00 00 */
    { 0x11D2, tug2_BombWentOff },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x128E, tug2_CrewFreed },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x14BA, tug2_E_14BA },    /* MSG_USE 40 0A 00 */
    { 0x14C2, tug2_PhaserField },    /* MSG_USE 41 0A 00 */
    { 0x15C3, tug2_StunGuard1 },    /* MSG_USE 40 08 00 */
    { 0x15FF, tug2_Guard1Stunned },    /* MSG_ANIM_DONE 16 00 00 */
    { 0x1664, tug2_StunGuard2 },    /* MSG_USE 40 09 00 */
    { 0x16A0, tug2_Guard2Stunned },    /* MSG_ANIM_DONE 17 00 00 */
    { 0x1705, tug2_KillGuard1 },    /* MSG_USE 41 08 00 */
    { 0x1745, tug2_Guard1Killed },    /* MSG_ANIM_DONE 18 00 00 */
    { 0x17AA, tug2_KillGuard2 },    /* MSG_USE 41 09 00 */
    { 0x17EA, tug2_Guard2Killed },    /* MSG_ANIM_DONE 19 00 00 */
    { 0x184F, tug2_E_184F },    /* MSG_USE 40 56 00 */
    { 0x1857, tug2_ChargeWelder },    /* MSG_USE 41 56 00 */
    { 0x18AE, tug2_WeldWireScraps },    /* MSG_USE 55 54 00 */
    { 0x193F, tug2_WeldJunkMetal },    /* MSG_USE 55 52 00 */
    { 0x19ED, tug2_FitCombBit },    /* MSG_USE 51 4F 00 */
    { 0x1A59, tug2_E_1A59 },    /* MSG_ANIM_DONE 1A 00 00 */
    { 0x1A61, tug2_KirkKilled },    /* MSG_ANIM_DONE 1B 00 00 */
    { 0x1A9C, tug2_WalkToDoor },    /* MSG_WALK 21 00 00 */
    { 0x1ABF, tug2_LookAtDoor },    /* MSG_LOOK 21 00 00 */
    { 0x1B0B, tug2_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x1B5F, tug2_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x1BEF, tug2_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x1C93, tug2_TalkToChristensen },    /* MSG_TALK 03 00 00 */
    { 0x1D5F, tug2_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x1E02, tug2_UseCommunicator },    /* MSG_USE 47 FF 00 */
    { 0x1ECB, tug2_GuardsFire },    /* MSG_TIMER 00 00 00 */
    { 0x204C, tug2_GameOver },    /* MSG_TIMER 02 00 00 */
    { 0x29CD, tug2_SpockScansBomb },    /* MSG_USE 44 0B 00 */
    { 0x2ADE, tug2_McCoyScansGuard1 },    /* MSG_USE 45 08 00 */
    { 0x2BC5, tug2_McCoyScansGuard2 },    /* MSG_USE 45 09 00 */
    { 0x2C9B, tug2_TalkToGuard1 },    /* MSG_TALK 08 00 00 */
    { 0x2D8B, tug2_TalkToGuard2 },    /* MSG_TALK 09 00 00 */
    { 0x2E7B, tug2_E_2E7B },    /* MSG_USE 7F 0B 00 */
    { 0x2E83, tug2_MedkitOnWires },    /* MSG_USE 7F 0C 00 */
    { 0x2ED9, tug2_MedkitOnGuard1 },    /* MSG_USE 7F 08 00 */
    { 0x2F3C, tug2_MedkitOnGuard2 },    /* MSG_USE 7F 09 00 */
    { 0x2F9F, tug2_E_2F9F },    /* MSG_LOOK 0A 00 00 */
    { 0x2FA7, tug2_E_2FA7 },    /* MSG_LOOK 22 00 00 */
    { 0x2FAF, tug2_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x3454, tug2_TalkToPrisoners },    /* MSG_TALK 0A 00 00 */
    { 0x3560, tug2_McCoyScansPrisoners },    /* MSG_USE 45 0A 00 */
    { 0x3755, tug2_McCoyScansCell },    /* MSG_USE 45 22 00 */
    { 0x3806, tug2_KillPrisoners },    /* MSG_USE 41 0A 00 */
    { 0x3864, tug2_StunPrisoners },    /* MSG_USE 40 0A 00 */
    { 0x38BC, tug2_StunAnything },    /* MSG_USE 40 FF 00 */
    { 0x38C3, tug2_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0, 0 }
};
