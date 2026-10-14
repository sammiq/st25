/* TUG3.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/tug3.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * TUG3.RDF - "Hijacked", the Masada's bridge: Elasi Cereth and three
 * clansmen. The party arrives by transporter (room 0) or the door (room
 * 1). The first shot starts the fight: the clansmen draw and fire (timer
 * 0), killing a crew member each time, Christensen first. Stunning or
 * killing them (killing costs points) ends it, and the rest surrender.
 * Talking to Cereth first can win his surrender (+8), or he plays a
 * "gambit": he shoots the helm, and the Masada's orbit decays (timer 1).
 * Calling Sulu in time stabilizes it; too late, the party can only beam
 * out. With every Elasi down, Kirk calls for a security team and the
 * mission ends. If a brig guard (room 2) is still up when the fight
 * starts, the alarm has gone out and the score is lost.
 *
 * Actors: 8 Cereth, 9-11 (0Bh) the clansmen, 12 (0Ch) sparks, 13-15
 * (0Fh) the security team.
 * Timers: 0 the clansmen fire, 1 the orbit decays.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_TUG3LOOP = 0x00D2,
    N_p1turn = 0x00F0,
    N_p2turn = 0x0110,
    N_p3turn = 0x0130,
    N_p4turn = 0x0150,
    N_kdraws = 0x0176,
    T_TUG3N007 = 0x01A5,
    N_p1surr = 0x0243,
    N_p2surr = 0x0270,
    N_p3surr = 0x029D,
    N_p4surr = 0x02CA,
    T_TUG3L081 = 0x0318,
    N_p1draw = 0x036E,
    N_p2draw = 0x0397,
    N_p3draw = 0x03C0,
    N_p4draw = 0x03E9,
    N_p1stun = 0x042E,
    N_t3beem05 = 0x045C,
    N_p2stun = 0x04A0,
    N_t3beem06 = 0x04CE,
    N_p3stun = 0x0512,
    N_t3beem07 = 0x0540,
    N_p4stun = 0x0584,
    N_t3beem04 = 0x05B2,
    N_p1Kill = 0x05F0,
    N_t3beem25 = 0x061E,
    N_p2Kill = 0x0660,
    N_t3beem27 = 0x068E,
    N_p3Kill = 0x06D0,
    N_t3beem26 = 0x06FE,
    N_p4Kill = 0x0740,
    N_t3beem24 = 0x076E,
    T_TUG3L084 = 0x0809,
    N_p1surr_2 = 0x086D,
    T_TUG3_F27 = 0x08B0,
    N_p1draw_2 = 0x0902,
    N_t3beem41 = 0x0937,
    N_sparks = 0x0966,
    T_TUG3L083 = 0x09B0,
    N_p1draw_3 = 0x09EF,
    N_t3beem41_2 = 0x0A24,
    N_sparks_2 = 0x0A43,
    N_SHOOTCON = 0x0A63,
    T_TUG3L080 = 0x0A8D,
    T_TUG3N001 = 0x0B1D,
    T_TUG3_013 = 0x0BE2,
    S_0C19 = 0x0C19,
    T_TUG3_011 = 0x0C2D,
    T_TUG3_009 = 0x0C9F,
    T_TUG3_008 = 0x0D15,
    T_TUG3L003 = 0x0D8E,
    T_TUG3_007 = 0x0DE1,
    T_TUG3_015 = 0x0E22,
    T_COMPA180 = 0x0E8C,
    T_TUG3_006 = 0x0EE2,
    T_TUG3_S07 = 0x0F1B,
    N_rkills = 0x109B,
    N_kkills = 0x10CF,
    N_skills = 0x10FF,
    N_mkills = 0x112F,
    S_118A = 0x118A,
    T_COMPU182 = 0x11B7,
    T_COMPU181 = 0x120A,
    T_TUG3N006 = 0x1275,
    S_12F7 = 0x12F7,
    T_TUG3_001 = 0x1332,
    N_rteleb = 0x139F,
    N_rteleb_2 = 0x13BE,
    N_rteleb_3 = 0x13DE,
    N_rdrawe = 0x1405,
    N_rdraws = 0x1423,
    N_rfiren = 0x1441,
    T_TUG3_005 = 0x1468,
    T_TUG3_S08 = 0x14A2,
    N_Capt_Kirk = 0x1532,
    N_Dr_McCoy = 0x153D,
    N_Mr_Spock = 0x1547,
    N_Mr_Sulu = 0x1551,
    N_Ship_s_Computer = 0x155A,
    N_Lt_Christensen = 0x156A,
    N_Elasi_Cereth = 0x157A,
    N_Elasi_Clansman = 0x1587,
    N_Mr_Scott = 0x15A0,
    S_15AA = 0x15AA,
    T_TUG3N008 = 0x15AB,
    T_TUG3N003 = 0x1602,
    T_TUG3N005 = 0x163E,
    T_TUG3N004 = 0x1677,
    T_TUG3N000 = 0x16C0,
    T_TUG3N002 = 0x16F1,
    talk_buf = 0x185C,
    KirkToCereth = 0x1862,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_shooter CS_W(0x1055)
#define fv_talk_buf CS_W(0x185C)
#define fv_target CS_W(0x1057)
#define rv_cereth ROOM_B(0x53)
#define rv_cereth_talk ROOM_B(0x5E)
#define rv_elasi2 ROOM_B(0x54)
#define rv_elasi3 ROOM_B(0x55)
#define rv_elasi4 ROOM_B(0x56)
#define rv_fight_started ROOM_B(0x3D)
#define rv_guard1 ROOM_B(0x4C)
#define rv_guard2 ROOM_B(0x4D)
#define rv_kirk_killed ROOM_B(0x4F)
#define rv_max_score ROOM_W(0x2B)
#define rv_mccoy_killed ROOM_B(0x51)
#define rv_orbit_decay ROOM_B(0x3B)
#define rv_outcome ROOM_W(0x2D)
#define rv_redshirt_killed ROOM_B(0x52)
#define rv_score ROOM_W(0x29)
#define rv_spock_killed ROOM_B(0x50)
#define rv_surrendered ROOM_B(0x5C)
#define rv_talked_to_cereth ROOM_B(0x3E)

/* functions */
static void tug3_StartRoom(void);
static void tug3_KirkDraws(void);
static void tug3_LookAtScene(void);
static void tug3_ElasiSurrender(void);
static void tug3_FightStarts(void);
static void tug3_StunAnything(void);
static void tug3_KillAnything(void);
static void tug3_StunCereth(void);
static void tug3_StunElasi2(void);
static void tug3_StunElasi3(void);
static void tug3_StunElasi4(void);
static void tug3_KillCereth(void);
static void tug3_part_064B(void);
static void tug3_KillElasi2(void);
static void tug3_KillElasi3(void);
static void tug3_KillElasi4(void);
static void tug3_ElasiHit(void);
static void tug3_TalkToCereth(void);
static void tug3_GambitShot(void);
static void tug3_HelmHit(void);
static void tug3_GambitShot2(void);
static void tug3_HelmHit2(void);
static void tug3_AnimDone1(void);
static void tug3_LookAtMcCoy(void);
static void tug3_LookAtSpock(void);
static void tug3_LookAtRedshirt(void);
static void tug3_LookAtCerethDead(void);
static void tug3_LookAtElasi2(void);
static void tug3_LookAtElasi3(void);
static void tug3_LookAtElasi4(void);
static void tug3_part_0B91(void);
static void tug3_part_0B9B(void);
static void tug3_LookAtCereth(void);
static void tug3_TalkToMcCoy(void);
static void tug3_TalkToSpock(void);
static void tug3_TalkToRedshirt(void);
static void tug3_CallForHelp(void);
static void tug3_ElasiFire(void);
static void tug3_part_1054(void);
static void tug3_PickShooter(void);
static void tug3_PickTarget(void);
static void tug3_McCoyKilled(void);
static void tug3_OrbitDecays(void);
static void tug3_CallSecurity(void);
static void tug3_SecurityArrives(void);
static uint16_t tug3_Talk(uint16_t ax);
static void tug3_Say(uint16_t ax, uint16_t bx);

/* The room starts: the Elasi turn to face the party, who can't walk.
 * [MSG_TICK 01 00 00] */
static void tug3_StartRoom(void)
{
    svc_Sfx(N_TUG3LOOP);
    room.no_walk = 1;
    svc_Anim(8, N_p1turn, 0xA4, 0x98, 0);
    svc_Anim(9, N_p2turn, 0xD1, 0x88, 0);
    svc_Anim(0xA, N_p3turn, 0xFB, 0xC4, 0);
    svc_Anim(0xB, N_p4turn, 0x5A, 0x9E, 0);
    return;
}

/* Tick 40: Kirk draws.
 * [MSG_TICK 28 00 00] */
static void tug3_KirkDraws(void)
{
    svc_Anim(0, N_kdraws, -1, -1, 0);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void tug3_LookAtScene(void)
{
    tug3_Say(0, T_TUG3N007);
    return;
}

/* The Elasi still standing surrender, +4 score (near call). */
static void tug3_ElasiSurrender(void)
{
    if (rv_fight_started == 1) {
        return;
    }
    rv_surrendered = 1;
    svc_Anim(8, N_p1surr, -1, -1, 0);
    rv_cereth = 4;
    if (rv_elasi2 == 0) {
        svc_Anim(9, N_p2surr, -1, -1, 0);
        rv_elasi2 = 4;
    }
    if (rv_elasi3 == 0) {
        svc_Anim(0xA, N_p3surr, -1, -1, 0);
        rv_elasi3 = 4;
    }
    if (rv_elasi4 == 0) {
        svc_Anim(0xB, N_p4surr, -1, -1, 0);
        rv_elasi4 = 4;
    }
    rv_score += 4;
    return;
}

/* The fight starts, once: if a brig guard is still up he has raised the
 * alarm, score 0. Timer 0, and the Elasi still up draw. */
static void tug3_FightStarts(void)
{
    if (rv_fight_started == 1) {
        return;
    }
    rv_fight_started = 1;
    if (rv_guard1 != 0) {
        if (rv_guard2 != 0) {
            goto L0353;
        }
    }
    tug3_Say(N_Elasi_Clansman, T_TUG3L081);
    rv_score += 0;
    rv_score = 0;
L0353:;
    if (room.timers[0] == 0) {
        room.timers[0] = 0xA;
    }
    if (rv_cereth == 0) {
        svc_Anim(8, N_p1draw, -1, -1, 0);
    }
    if (rv_elasi2 == 0) {
        svc_Anim(9, N_p2draw, -1, -1, 0);
    }
    if (rv_elasi3 == 0) {
        svc_Anim(0xA, N_p3draw, -1, -1, 0);
    }
    if (rv_elasi4 == 0) {
        svc_Anim(0xB, N_p4draw, -1, -1, 0);
    }
    return;
}

/* Phasers on anything else: nothing.
 * [MSG_USE 40 FF 00] */
static void tug3_StunAnything(void)
{
    return;
}

/* [MSG_USE 41 FF 00] */
static void tug3_KillAnything(void)
{
    return;
}

/* Phasers on the Elasi, while Kirk is alive: stun, or kill (-2 score
 * each). The first shot starts the fight (msg 12 when done).
 * [MSG_USE 40 08 00] */
static void tug3_StunCereth(void)
{
    if (rv_kirk_killed != 0) {
        { tug3_part_064B(); return; }
    }
    if (rv_cereth != 0) {
        { tug3_part_064B(); return; }
    }
    svc_Anim(8, N_p1stun, -1, -1, 0xC);
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem05);
    rv_cereth = 1;
    tug3_FightStarts();
    return;
}

/* [MSG_USE 40 09 00] */
static void tug3_StunElasi2(void)
{
    if (rv_kirk_killed != 0) {
        { tug3_part_064B(); return; }
    }
    if (rv_elasi2 != 0) {
        { tug3_part_064B(); return; }
    }
    svc_Anim(9, N_p2stun, -1, -1, 0xC);
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem06);
    rv_elasi2 = 1;
    tug3_FightStarts();
    return;
}

/* [MSG_USE 40 0A 00] */
static void tug3_StunElasi3(void)
{
    if (rv_kirk_killed != 0) {
        { tug3_part_064B(); return; }
    }
    if (rv_elasi3 != 0) {
        { tug3_part_064B(); return; }
    }
    svc_Anim(0xA, N_p3stun, -1, -1, 0xC);
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem07);
    rv_elasi3 = 1;
    tug3_FightStarts();
    return;
}

/* [MSG_USE 40 0B 00] */
static void tug3_StunElasi4(void)
{
    if (rv_kirk_killed != 0) {
        { tug3_part_064B(); return; }
    }
    if (rv_elasi4 != 0) {
        { tug3_part_064B(); return; }
    }
    svc_Anim(0xB, N_p4stun, -1, -1, 0xC);
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem04);
    rv_elasi4 = 1;
    tug3_FightStarts();
    return;
}

/* [MSG_USE 41 08 00] */
static void tug3_KillCereth(void)
{
    if (rv_kirk_killed != 0)
        { tug3_part_064B(); return; }
    if (rv_cereth != 0)
        { tug3_part_064B(); return; }
    svc_Anim(8, N_p1Kill, -1, -1, 0xC);
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem25);
    rv_cereth = 2;
    rv_score += 0xFFFE;
    tug3_FightStarts();
    { tug3_part_064B(); return; }
}

static void tug3_part_064B(void)
{
    return;
}

/* [MSG_USE 41 09 00] */
static void tug3_KillElasi2(void)
{
    if (rv_kirk_killed == 0) {
        if (rv_elasi2 == 0) {
            svc_Anim(9, N_p2Kill, -1, -1, 0xC);
            svc_BridgeSfx(6);
            svc_FlashBitmap(N_t3beem27);
            rv_elasi2 = 2;
            rv_score += 0xFFFE;
            tug3_FightStarts();
        }
    }
    return;
}

/* [MSG_USE 41 0A 00] */
static void tug3_KillElasi3(void)
{
    if (rv_kirk_killed == 0) {
        if (rv_elasi3 == 0) {
            svc_Anim(0xA, N_p3Kill, -1, -1, 0xC);
            svc_BridgeSfx(6);
            svc_FlashBitmap(N_t3beem26);
            rv_elasi3 = 2;
            rv_score += 0xFFFE;
            tug3_FightStarts();
        }
    }
    return;
}

/* [MSG_USE 41 0B 00] */
static void tug3_KillElasi4(void)
{
    if (rv_kirk_killed == 0) {
        if (rv_elasi4 == 0) {
            svc_Anim(0xB, N_p4Kill, -1, -1, 0xC);
            svc_BridgeSfx(6);
            svc_FlashBitmap(N_t3beem24);
            rv_elasi4 = 2;
            rv_score += 0xFFFE;
            tug3_FightStarts();
        }
    }
    return;
}

/* An Elasi is down: once none is standing, call security.
 * [MSG_ANIM_DONE 0C 00 00] */
static void tug3_ElasiHit(void)
{
    if (rv_cereth_talk != 1) {
        if (rv_cereth != 0) {
            if (rv_elasi2 != 0) {
                if (rv_elasi3 != 0) {
                    if (rv_elasi4 != 0) {
                        rv_cereth_talk = 1;
                        { tug3_CallSecurity(); return; }
                    }
                }
            }
        }
    }
    return;
}

/* Talking to Cereth, once, before the fight: a choice (+8 and surrender),
 * or his gambit: he shoots the helm (msgs 3-6) and the orbit decays.
 * [MSG_TALK 08 00 00] */
static void tug3_TalkToCereth(void)
{
    uint16_t ax = 0;
    if (rv_cereth != 0) {
        return;
    }
    if (rv_talked_to_cereth == 1) {
        return;
    }
    rv_talked_to_cereth = 1;
    ax = (uint16_t)tug3_Talk(KirkToCereth);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L08A1;
        }
        if (ax == 2) {
            goto L09A1;
        }
        return;
    }
    tug3_Say(N_Elasi_Cereth, T_TUG3L084);
    rv_cereth = 4;
    svc_Anim(8, N_p1surr_2, -1, -1, 0);
    rv_cereth = 4;
    tug3_ElasiSurrender();
    rv_cereth_talk = 2;
    rv_score += 8;
    { tug3_CallSecurity(); return; }
L08A1:;
    room.busy = 1;
    tug3_Say(N_Elasi_Cereth, T_TUG3_F27);
    svc_Anim(8, N_p1draw_2, -1, -1, 3);
    return;
L09A1:;
    room.busy = 1;
    tug3_Say(N_Elasi_Cereth, T_TUG3L083);
    svc_Anim(8, N_p1draw_3, -1, -1, 5);
    return;
}

/* [MSG_ANIM_DONE 03 00 00] */
static void tug3_GambitShot(void)
{
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem41);
    svc_BridgeSfx(0x16);
    svc_Anim(0xC, N_sparks, 0xA0, 0xAD, 4);
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void tug3_HelmHit(void)
{
    tug3_FightStarts();
    room.busy = 0;
    rv_cereth_talk = 3;
    room.timers[1] = 0xA;
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void tug3_GambitShot2(void)
{
    svc_BridgeSfx(6);
    svc_FlashBitmap(N_t3beem41_2);
    svc_Anim(0xC, N_sparks_2, 0xA0, 0xAD, 6);
    svc_Sfx(N_SHOOTCON);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void tug3_HelmHit2(void)
{
    tug3_Say(N_Elasi_Cereth, T_TUG3L080);
    tug3_ElasiSurrender();
    room.busy = 0;
    rv_cereth_talk = 3;
    room.timers[1] = 0xA;
    return;
}

/* Nothing.
 * [MSG_ANIM_DONE 01 00 00] */
static void tug3_AnimDone1(void)
{
    return;
}

/* Looking. A dead Elasi first, else (RS_MORE) the next script.
 * [MSG_LOOK 02 00 00] */
static void tug3_LookAtMcCoy(void)
{
    tug3_Say(0, T_TUG3N000);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void tug3_LookAtSpock(void)
{
    tug3_Say(0, T_TUG3N002);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void tug3_LookAtRedshirt(void)
{
    tug3_Say(0, T_TUG3N001);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void tug3_LookAtCerethDead(void)
{
    if (rv_cereth == 2)
        { tug3_part_0B9B(); return; }
    { tug3_part_0B91(); return; }
}

/* [MSG_LOOK 09 00 00] */
static void tug3_LookAtElasi2(void)
{
    if (rv_elasi2 == 2)
        { tug3_part_0B9B(); return; }
    { tug3_part_0B91(); return; }
}

/* [MSG_LOOK 0A 00 00] */
static void tug3_LookAtElasi3(void)
{
    if (rv_elasi3 == 2)
        { tug3_part_0B9B(); return; }
    { tug3_part_0B91(); return; }
}

/* [MSG_LOOK 0B 00 00] */
static void tug3_LookAtElasi4(void)
{
    if (rv_elasi4 == 2)
        { tug3_part_0B9B(); return; }
    { tug3_part_0B91(); return; }
}

static void tug3_part_0B91(void)
{
    tug3_Say(S_15AA, T_TUG3N005);
    return;
}

static void tug3_part_0B9B(void)
{
    tug3_Say(S_15AA, T_TUG3N004);
    return;
}

/* [MSG_LOOK 08 00 00] */
static void tug3_LookAtCereth(void)
{
    if (rv_cereth != 1) {
        tug3_Say(S_15AA, T_TUG3N008);
        return;
    }
    tug3_Say(S_15AA, T_TUG3N003);
    return;
}

/* Talking, depending on the orbit.
 * [MSG_TALK 02 00 00] */
static void tug3_TalkToMcCoy(void)
{
    if (rv_orbit_decay == 0) {
        if (rv_cereth == 0) {
            tug3_Say(N_Dr_McCoy, T_TUG3_013);
            return;
        }
    }
    if ((int8_t)rv_orbit_decay < (int8_t)0xA) {
        tug3_Say(N_Dr_McCoy, S_0C19);
        return;
    }
    if ((int8_t)rv_orbit_decay < (int8_t)0x10) {
        tug3_Say(N_Dr_McCoy, T_TUG3_011);
        return;
    }
    return;
}

/* [MSG_TALK 01 00 00] */
static void tug3_TalkToSpock(void)
{
    if (rv_orbit_decay == 0) {
        return;
    }
    if ((int8_t)rv_orbit_decay < (int8_t)0xA) {
        tug3_Say(N_Mr_Spock, T_TUG3_009);
        return;
    }
    if ((int8_t)rv_orbit_decay < (int8_t)0x10) {
        tug3_Say(N_Mr_Spock, T_TUG3_008);
        return;
    }
    return;
}

/* [MSG_TALK 03 00 00] */
static void tug3_TalkToRedshirt(void)
{
    tug3_Say(N_Lt_Christensen, T_TUG3L003);
    return;
}

/* The communicator while the orbit decays: Sulu stabilizes it; once it
 * is too late, Scott beams the party out (score 0).
 * [MSG_USE 47 FF 00] */
static void tug3_CallForHelp(void)
{
    uint16_t ax = 0;
    if (rv_orbit_decay == 0) {
        return;
    }
    if ((int8_t)rv_orbit_decay >= (int8_t)0xA) {
    } else {
        tug3_Say(N_Capt_Kirk, T_TUG3_007);
        tug3_Say(N_Mr_Sulu, T_TUG3_015);
        tug3_Say(N_Ship_s_Computer, T_COMPA180);
        room.timers[1] = 0;
        { tug3_CallSecurity(); return; }
    }
    if ((int8_t)rv_orbit_decay >= (int8_t)0x10) {
        { tug3_part_1054(); return; }
    }
    tug3_Say(N_Capt_Kirk, T_TUG3_006);
    tug3_Say(N_Mr_Scott, T_TUG3_S07);
    svc_Music(-1, -1);
    room.busy = 1;
    ax = rv_score;
    ax = ax & 0x8000;
    if (ax != 0) {
        rv_score = 0;
    }
    svc_BeamUp(rv_score, rv_max_score, rv_outcome);
    return;
}

/* Timer 0: the next clansman standing fires at the next crew member
 * still alive; McCoy's death (msg 13) ends the game.
 * [MSG_TIMER 00 00 00] */
static void tug3_ElasiFire(void)
{
    uint16_t ax = 0, si = 0;
    uint16_t t0;
    uint16_t t1;
    uint16_t t2;
    uint16_t t3;
    tug3_PickShooter();
    if (fv_shooter == 0xFFFF)
        { tug3_part_1054(); return; }
    tug3_PickTarget();
    if (fv_target == 0xFFFF)
        { tug3_part_1054(); return; }
    svc_BridgeSfx(6);
    ax = fv_shooter;
    ax = ax << 1;
    t0 = ax;
    ax = ax << 1;
    t1 = ax;
    ax = ax + fv_target;
    t2 = ax;
    ax = ax << 1;
    si = ax;
    t3 = ax;
    ax = CS_W(0xF74 + si);
    svc_FlashBitmap(ax);
    room.timers[0] = 0x32;
    { tug3_part_1054(); return; }
}

static void tug3_part_1054(void)
{
    return;
}

/* The first clansman still up (Cereth never fires). */
static void tug3_PickShooter(void)
{
    if (rv_elasi2 == 0) {
        fv_shooter = 0;
        return;
    }
    if (rv_elasi3 == 0) {
        fv_shooter = 1;
        return;
    }
    if (rv_elasi4 == 0) {
        fv_shooter = 2;
        return;
    }
    fv_shooter = 0xFFFF;
    return;
}

/* The first of Christensen, Kirk, Spock, McCoy still alive is killed. */
static void tug3_PickTarget(void)
{
    if (rv_redshirt_killed == 0) {
        rv_redshirt_killed = 2;
        room.redshirt_dead = 1;
        svc_Anim(3, N_rkills, -1, -1, 0);
        fv_target = 3;
        return;
    }
    if (rv_kirk_killed == 0) {
        rv_kirk_killed = 2;
        room.busy = 1;
        svc_Anim(0, N_kkills, -1, -1, 0);
        fv_target = 0;
        return;
    }
    if (rv_spock_killed == 0) {
        rv_spock_killed = 2;
        svc_Anim(1, N_skills, -1, -1, 0);
        fv_target = 1;
        return;
    }
    if (rv_mccoy_killed == 0) {
        rv_mccoy_killed = 2;
        svc_Anim(2, N_mkills, -1, -1, 0xD);
        fv_target = 2;
        return;
    }
    fv_target = 0xFFFF;
    return;
}

/* [MSG_ANIM_DONE 0D 00 00] */
static void tug3_McCoyKilled(void)
{
    svc_Music(2, -1);
    svc_GameOver();
    tug3_Say(0, S_118A);
    return;
}

/* Timer 1: the orbit decays; at 10h the Masada is lost, game over.
 * [MSG_TIMER 01 00 00] */
static void tug3_OrbitDecays(void)
{
    if ((int8_t)rv_orbit_decay < (int8_t)0xA) {
        tug3_Say(N_Ship_s_Computer, T_COMPU182);
        room.timers[1] = 0x64;
        rv_orbit_decay++;
        return;
    }
    if ((int8_t)rv_orbit_decay < (int8_t)0x10) {
        tug3_Say(N_Ship_s_Computer, T_COMPU181);
        room.timers[1] = 0x64;
        rv_orbit_decay++;
        return;
    }
    tug3_Say(0, T_TUG3N006);
    svc_GameOver();
    tug3_Say(0, S_12F7);
    return;
}

/* The Elasi are all down (or the communicator): Scott beams a security
 * team over (msgs 7-10), then the party out. The mission's end.
 * [MSG_USE 47 FF 00] */
static void tug3_CallSecurity(void)
{
    svc_Music(0x1C, -1);
    tug3_Say(N_Capt_Kirk, T_TUG3_001);
    svc_BridgeSfx(9);
    svc_Anim(0xD, N_rteleb, 0x14, 0xA0, 7);
    svc_Anim(0xE, N_rteleb_2, 0x118, 0xA0, 0);
    svc_Anim(0xF, N_rteleb_3, 0x96, 0xBE, 0);
    return;
}

/* [MSG_ANIM_DONE 07 00 00] */
static void tug3_SecurityArrives(void)
{
    uint16_t ax = 0;
    svc_Anim(0xD, N_rdrawe, -1, -1, 8);
    svc_Anim(0xE, N_rdraws, -1, -1, 9);
    svc_Anim(0xF, N_rfiren, -1, -1, 0xA);
    tug3_Say(N_Capt_Kirk, T_TUG3_005);
    tug3_Say(N_Mr_Scott, T_TUG3_S08);
    svc_Music(-1, -1);
    room.busy = 1;
    ax = rv_score;
    ax = ax & 0x8000;
    if (ax != 0) {
        rv_score = 0;
    }
    svc_BeamUp(rv_score, rv_max_score, rv_outcome);
    return;
}

static uint16_t tug3_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void tug3_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x185E) = bx;
    tug3_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript tug3_scripts[] = {
    { 0x00D0, tug3_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0174, tug3_KirkDraws },    /* MSG_TICK 28 00 00 */
    { 0x0199, tug3_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x040C, tug3_StunAnything },    /* MSG_USE 40 FF 00 */
    { 0x0413, tug3_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x041A, tug3_StunCereth },    /* MSG_USE 40 08 00 */
    { 0x048C, tug3_StunElasi2 },    /* MSG_USE 40 09 00 */
    { 0x04FE, tug3_StunElasi3 },    /* MSG_USE 40 0A 00 */
    { 0x0570, tug3_StunElasi4 },    /* MSG_USE 40 0B 00 */
    { 0x05E2, tug3_KillCereth },    /* MSG_USE 41 08 00 */
    { 0x0652, tug3_KillElasi2 },    /* MSG_USE 41 09 00 */
    { 0x06C2, tug3_KillElasi3 },    /* MSG_USE 41 0A 00 */
    { 0x0732, tug3_KillElasi4 },    /* MSG_USE 41 0B 00 */
    { 0x07A2, tug3_ElasiHit },    /* MSG_ANIM_DONE 0C 00 00 */
    { 0x07CE, tug3_TalkToCereth },    /* MSG_TALK 08 00 00 */
    { 0x0925, tug3_GambitShot },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x098B, tug3_HelmHit },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0A12, tug3_GambitShot2 },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x0A82, tug3_HelmHit2 },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x0AED, tug3_AnimDone1 },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x0AF4, tug3_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0B03, tug3_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0B12, tug3_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0B61, tug3_LookAtCerethDead },    /* MSG_LOOK 08 00 00 */
    { 0x0B6F, tug3_LookAtElasi2 },    /* MSG_LOOK 09 00 00 */
    { 0x0B7D, tug3_LookAtElasi3 },    /* MSG_LOOK 0A 00 00 */
    { 0x0B8B, tug3_LookAtElasi4 },    /* MSG_LOOK 0B 00 00 */
    { 0x0BAB, tug3_LookAtCereth },    /* MSG_LOOK 08 00 00 */
    { 0x0BCB, tug3_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0C87, tug3_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0D83, tug3_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0DC6, tug3_CallForHelp },    /* MSG_USE 47 FF 00 */
    { 0x0FFE, tug3_ElasiFire },    /* MSG_TIMER 00 00 00 */
    { 0x1161, tug3_McCoyKilled },    /* MSG_ANIM_DONE 0D 00 00 */
    { 0x11A6, tug3_OrbitDecays },    /* MSG_TIMER 01 00 00 */
    { 0x1313, tug3_CallSecurity },    /* MSG_USE 47 FF 00 */
    { 0x1403, tug3_SecurityArrives },    /* MSG_ANIM_DONE 07 00 00 */
    { 0, 0 }
};
