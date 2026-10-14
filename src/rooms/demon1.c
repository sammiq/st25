/* DEMON1.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/demon1.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * DEMON1.RDF - "Demon World", the path up Mount Idyll, where three
 * "Klingons" ambush the landing party. They are androids: when shot,
 * Klingon 1 loses a hand with circuitry in its palm, which Kirk can take.
 * Any of the crew they stun get up again after the fight.
 *
 * Actors: 8, 9, 10 (0Ah) the Klingons; 11 (0Bh) a Klingon's shot;
 * 12 (0Ch) Kirk's phaser shot; 13 (0Dh) the hand on the ground.
 * Objects: 0Dh the hand, 20h cattails, 21h tulips, 22h Brandzite pods,
 * 23h a fern, 24h the stream, 25h the mine entrance, 26h Mount Idyll.
 * Exits: 0 up to the mine (room 2), 1 back to the colony (room 0).
 * Timers: 2 the ambush starts, 0 and 3 the other Klingons stand up,
 * 1 a Klingon fires, 4 Kirk has the hand, 5 Uhura calls, 6 the crew
 * come round.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 15C5: the engine reads its bytes
 * as key 2E A3 E7 16 and next 892E (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_klg1u = 0x00D8,
    N_klg1f = 0x00DE,
    N_klg1d = 0x00E4,
    N_klg1d2 = 0x00EA,
    N_klghnd = 0x00F1,
    N_klg2u = 0x00F8,
    N_klg2f = 0x00FE,
    N_klg2d = 0x0104,
    N_klg2d2 = 0x010A,
    N_klg3u = 0x0111,
    N_klg3f = 0x0117,
    N_klg3d = 0x011D,
    N_klg3d2 = 0x0123,
    N_klg1fr = 0x012A,
    N_klg1fs = 0x0131,
    N_klg1fm = 0x0138,
    N_klg1fk = 0x013F,
    N_klg2fr = 0x0146,
    N_klg2fs = 0x014D,
    N_klg2fm = 0x0154,
    N_klg2fk = 0x015B,
    N_klg3fr = 0x0162,
    N_klg3fs = 0x0169,
    N_klg3fm = 0x0170,
    N_klg3fk = 0x0177,
    N_s0ks1 = 0x017E,
    N_s0ks2 = 0x0184,
    N_s0ks3 = 0x018A,
    N_rstunn = 0x0190,
    N_sstunn = 0x0197,
    N_mstune = 0x019E,
    N_kstunn = 0x01A5,
    N_mscann = 0x01AC,
    N_sscann = 0x01B3,
    N_kfiren = 0x01BA,
    N_s5r1kg = 0x01C1,
    N_DEM1LOOP = 0x01D0,
    T_DEM1_011 = 0x05D4,
    N_EFX19 = 0x06B8,
    T_DEM1N020 = 0x0716,
    T_DEM1_025 = 0x0791,
    T_DEM1_F23 = 0x07EC,
    T_DEM1U077 = 0x099E,
    T_DEM1_003 = 0x0A06,
    T_DEM1U078 = 0x0A51,
    T_DEM1_002 = 0x0A96,
    T_DEM1_020 = 0x0ACC,
    T_DEM1_004 = 0x0B5A,
    T_DEM1_021 = 0x0B87,
    T_DEM1_007 = 0x0C46,
    T_DEM1_008 = 0x0CD5,
    T_DEM1_005 = 0x0D82,
    T_DEM1_006 = 0x0E55,
    T_DEM1_009 = 0x107E,
    T_DEM1N018 = 0x1186,
    T_DEM1N010 = 0x11D1,
    T_DEM1N019 = 0x1216,
    T_DEM1N009 = 0x1271,
    T_DEM1N011 = 0x12A4,
    T_DEM1N021 = 0x12F3,
    T_DEM1N016 = 0x1335,
    T_DEM1N023 = 0x1371,
    T_DEM1N017 = 0x13B4,
    T_DEM1N012 = 0x1419,
    T_DEM1N015 = 0x1467,
    T_DEM1N013 = 0x14B2,
    T_DEM1N014 = 0x14FD,
    N_Zzzzz = 0x15BA,
    rdf_bans = 0x16E7,
    N_Capt_Kirk = 0x16ED,
    N_Mr_Spock = 0x16F8,
    N_Dr_McCoy = 0x1702,
    N_Ensign_Everts = 0x170C,
    N_Lt_Uhura = 0x171A,
    N_Klingon = 0x1724,
    S_172C = 0x172C,
    T_DEM1N003 = 0x172D,
    T_DEM1N001 = 0x177F,
    T_DEM1N008 = 0x17E6,
    T_DEM1N002 = 0x1841,
    T_DEM1N000 = 0x1891,
    T_DEM1_018 = 0x18D6,
    T_DEM1_012 = 0x1A7B,
    T_DEM1_017 = 0x1AB5,
    T_DEM1_019 = 0x1B0E,
    T_DEM1_016 = 0x1BC7,
    T_DEM1N005 = 0x1C36,
    T_DEM1N007 = 0x1C71,
    T_DEM1N004 = 0x1CA6,
    T_DEM1_001 = 0x1CCE,
    T_DEM1_022 = 0x1D36,
    T_DEM1_013 = 0x1DDD,
    T_DEM1_024 = 0x1E37,
    T_DEM1_014 = 0x1E63,
    T_DEM1_015 = 0x1EE0,
    T_DEM1_023 = 0x1F2F,
    T_DEM1_026 = 0x1FB3,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_crew_shot CS_B(0xD0)
#define fv_kirk_firing CS_B(0xD1)
#define fv_klingon1_hit CS_B(0xCA)
#define fv_klingon2_hit CS_B(0xCB)
#define fv_klingon3_hit CS_B(0xCC)
#define fv_klingons_down CS_B(0xCD)
#define fv_rdf_bans CS_W(0x16E7)
#define fv_scanned_klingon CS_B(0xCF)
#define fv_shooter CS_W(0xD2)
#define fv_shooter_anim CS_W(0xD4)
#define fv_shot_anim CS_W(0xD6)
#define rv_ambushed ROOM_W(0x2F)
#define rv_came_from ROOM_B(0x3A)
#define rv_saw_klingons ROOM_B(0x37)
#define rv_score ROOM_W(0x58)
#define rv_took_hand ROOM_W(0x31)
#define rv_under_fire ROOM_B(0x48)

/* functions */
static void demon1_StartRoom(void);
static void demon1_WalkToMine(void);
static void demon1_E_02DB(void);
static void demon1_ExitToMine(void);
static void demon1_ExitToColony(void);
static void demon1_Ambush(void);
static void demon1_Klingon2StandsUp(void);
static void demon1_Klingon3StandsUp(void);
static void demon1_KlingonFires(void);
static void demon1_CrewMemberStunned(void);
static void demon1_KirkShot(void);
static void demon1_E_05B9(void);
static void demon1_PhaserAfterFight(void);
static void demon1_E_062A(void);
static void demon1_ShootKlingon1(void);
static void demon1_Klingon1Hit(void);
static void demon1_Klingon1Falls(void);
static void demon1_E_0818(void);
static void demon1_ShootKlingon2(void);
static void demon1_Klingon2Hit(void);
static void demon1_E_08C5(void);
static void demon1_ShootKlingon3(void);
static void demon1_Klingon3Hit(void);
static void demon1_CrewComeRound(void);
static void demon1_UhuraCalls(void);
static void demon1_McCoyScansHand(void);
static void demon1_SpockScansTulips(void);
static void demon1_SpockScansPods(void);
static void demon1_SpockScansCattails(void);
static void demon1_SpockScansFern(void);
static void demon1_SpockScansHand(void);
static void demon1_SpockScansKlingon1(void);
static void demon1_E_0F35(void);
static void demon1_SpockScansKlingon(void);
static void demon1_E_0F72(void);
static void demon1_McCoyScansKlingon1(void);
static void demon1_E_0FAF(void);
static void demon1_McCoyScansKlingon3(void);
static void demon1_McCoyScansKlingon3b(void);
static void demon1_McCoyScansKlingon(void);
static void demon1_SpockScansKlingon1Again(void);
static void demon1_McCoyScansKirk(void);
static void demon1_McCoyScansSpock(void);
static void demon1_McCoyScansRedshirt(void);
static void demon1_part_1052(void);
static void demon1_GetHand(void);
static void demon1_ReachHand(void);
static void demon1_PickUpHand(void);
static void demon1_GiveHand(void);
static void demon1_E_115B(void);
static void demon1_LookAtKlingon2(void);
static void demon1_LookAtKlingon(void);
static void demon1_LookAtCattails(void);
static void demon1_LookAtTulips(void);
static void demon1_LookAtPods(void);
static void demon1_LookAtFern(void);
static void demon1_LookAtStream(void);
static void demon1_LookAtMine(void);
static void demon1_LookAtMountain(void);
static void demon1_LookAtHand(void);
static void demon1_LookAtScene(void);
static void demon1_LookAtKirk(void);
static void demon1_LookAtSpock(void);
static void demon1_LookAtMcCoy(void);
static void demon1_LookAtRedshirt(void);
static void demon1_TalkToKirk(void);
static void demon1_TalkToSpock(void);
static void demon1_TalkToMcCoy(void);
static void demon1_TalkToRedshirt(void);
static void demon1_part_15AF(void);
static void demon1_Say(uint16_t ax, uint16_t bx);
static void demon1_Talk(uint16_t ax);

/* The room starts. The first time, the ambush is set for timer 2 and the
 * crew can't walk; afterwards the Klingons lie where they fell (and the hand,
 * until Kirk takes it).
 * [MSG_TICK 01 00 00] */
static void demon1_StartRoom(void)
{
    svc_Sfx(N_DEM1LOOP);
    if (rv_ambushed == 0) {
        svc_Music(1, -1);
    }
    if (rv_came_from == 1) {
        room.fall[0] = 0;
        room.fall[1] = 0;
        room.fall[2] = 0;
        room.fall[3] = 0;
    } else if (rv_came_from == 2) {
        room.fall[0] = 1;
        room.fall[1] = 1;
        room.fall[2] = 1;
        room.fall[3] = 1;
    }
    if (rv_ambushed == 0) {
        room.no_walk = 1;
        room.timers[2] = 0x32;
        return;
    }
    svc_Anim0(8, N_klg1d2, 0x120, 0x82, 0);
    if (rv_took_hand != 1) {
        svc_Anim0(0xD, N_klghnd, 0x10B, 0x8E, 0);
    }
    svc_Anim0(9, N_klg2d2, 0xAA, 0x7C, 0);
    svc_Anim0(0xA, N_klg3d2, 0, 0, 0);
    fv_klingon1_hit = 1;
    fv_klingon2_hit = 1;
    fv_klingon3_hit = 1;
    fv_klingons_down = 3;
    return;
}

/* Clicking the mine entrance after the fight: walk there (msg 2), then
 * leave as by exit 0.
 * [MSG_WALK 25 00 00] */
static void demon1_WalkToMine(void)
{
    if (fv_klingons_down != 3) {
        return;
    }
    svc_Walk(0, 0xA3, 0x56, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void demon1_E_02DB(void)
{
    { demon1_ExitToMine(); return; }
}

/* [MSG_EXIT 00 00 00] */
static void demon1_ExitToMine(void)
{
    if (rv_under_fire != 1) {
        svc_GotoRoom(2, 1);
    }
    return;
}

/* [MSG_EXIT 01 00 00] */
static void demon1_ExitToColony(void)
{
    if (rv_under_fire != 1) {
        svc_GotoRoom(0, 0);
    }
    return;
}

/* Timer 2: Klingon 1 stands up; the others follow on timers 0 and 3,
 * and timer 1 starts the shooting.
 * [MSG_TIMER 02 00 00] */
static void demon1_Ambush(void)
{
    if (rv_ambushed == 0) {
        rv_saw_klingons = 1;
        rv_ambushed--;
        rv_under_fire = 1;
        room.timers[0] = 5;
        room.timers[3] = 2;
        room.timers[1] = 0x64;
        svc_Anim0(8, N_klg1u, 0x120, 0x82, 0);
    }
    return;
}

/* [MSG_TIMER 00 00 00] */
static void demon1_Klingon2StandsUp(void)
{
    svc_Anim0(9, N_klg2u, 0xAA, 0x7C, 0);
    return;
}

/* [MSG_TIMER 03 00 00] */
static void demon1_Klingon3StandsUp(void)
{
    svc_Anim0(0xA, N_klg3u, 0xDB, 0x6D, 0);
    return;
}

/* Timer 1: the first Klingon still standing fires at the next crew
 * member (redshirt, McCoy, Spock, then Kirk). The jumps into the next
 * Klingon's tests when crew_shot doesn't match are in the original.
 * [MSG_TIMER 01 00 00] */
static void demon1_KlingonFires(void)
{
    uint16_t stk[32 + 8] = { 0 };       /* the top 8 are what a service call reads past the stack */
    int sp_ = 32;
    if (fv_klingon1_hit != 1) {
        fv_shooter_anim = N_klg1f;
        fv_shooter = 8;
        if (fv_crew_shot == 0) {
            fv_shot_anim = N_klg1fr;
            goto L049E;
        }
        if (fv_crew_shot == 1) {
            fv_shot_anim = N_klg1fm;
            goto L049E;
        }
        if (fv_crew_shot == 2) {
            fv_shot_anim = N_klg1fs;
            goto L049E;
        }
        if (fv_crew_shot != 3)
            goto L0416;
        fv_shot_anim = N_klg1fk;
        goto L049E;
    }
    if (fv_klingon2_hit == 1)
        goto L0449;
    fv_shooter_anim = N_klg2f;
    fv_shooter = 9;
    if (fv_crew_shot == 0) {
        fv_shot_anim = N_klg2fr;
        goto L049E;
    }
L0416:;
    if (fv_crew_shot == 1) {
        fv_shot_anim = N_klg2fm;
        goto L049E;
    }
    if (fv_crew_shot == 2) {
        fv_shot_anim = N_klg2fs;
        goto L049E;
    }
    if (fv_crew_shot != 3)
        goto L0468;
    fv_shot_anim = N_klg2fk;
    goto L049E;
L0449:;
    fv_shooter_anim = N_klg3f;
    fv_shooter = 0xA;
    if (fv_crew_shot == 0) {
        fv_shot_anim = N_klg3fr;
        goto L049E;
    }
L0468:;
    if (fv_crew_shot == 1) {
        fv_shot_anim = N_klg3fm;
    } else {
        if (fv_crew_shot == 2) {
            fv_shot_anim = N_klg3fs;
        } else {
            if (fv_crew_shot != 3) {
                goto L0517;
            }
            fv_shot_anim = N_klg3fk;
            goto L049E;
        }
    }
L049E:;
    stk[--sp_] = 2;
    stk[--sp_] = 0xFFFF;
    stk[--sp_] = 0xFFFF;
    stk[--sp_] = fv_shooter_anim;
    stk[--sp_] = fv_shooter;
    stk[--sp_] = 2;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 6;
    return;
L0517:;
    if (fv_crew_shot == 2) {
        stk[--sp_] = N_sstunn;
        stk[--sp_] = 1;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[1] = 0;
        room.timers[1] = 0x64;
        room.stun[1] = 0x28A;
        room.flags |= 2;
        return;
    }
    if (fv_crew_shot == 3) {
        stk[--sp_] = N_mstune;
        stk[--sp_] = 2;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[2] = 3;
        room.timers[1] = 0x64;
        room.stun[2] = 0x28A;
        room.flags |= 4;
        return;
    }
    if (fv_crew_shot == 4) {
        stk[--sp_] = 1;
        stk[--sp_] = 0xFFFF;
        stk[--sp_] = 0xFFFF;
        stk[--sp_] = N_kstunn;
        stk[--sp_] = 0;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[0] = 0;
        room.stun[0] = 0x28A;
        room.flags |= 1;
    }
    return;
}

/* The shot has hit (anim msg 2): that crew member falls, stunned for
 * 28Ah ticks (RS_STUN, RS_FLAGS), and timer 1 is set for the next shot.
 * When it's Kirk's turn (msg 1 when done), the game is over. The three
 * shared pushes are made before the test; the last case pops them again.
 * [MSG_ANIM_DONE 02 00 00] */
static void demon1_CrewMemberStunned(void)
{
    uint16_t stk[32 + 8] = { 0 };       /* the top 8 are what a service call reads past the stack */
    int sp_ = 32;
    stk[--sp_] = 0;
    stk[--sp_] = 0;
    stk[--sp_] = 0;
    stk[--sp_] = fv_shot_anim;
    stk[--sp_] = 0xB;
    stk[--sp_] = 0;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 6;
    stk[--sp_] = 6;
    stk[--sp_] = 0xF;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 2;
    stk[--sp_] = 0;
    stk[--sp_] = 0xFFFF;
    stk[--sp_] = 0xFFFF;
    fv_crew_shot++;
    if (fv_crew_shot == 1) {
        stk[--sp_] = N_rstunn;
        stk[--sp_] = 3;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[3] = 0;
        room.timers[1] = 0x64;
        room.stun[3] = 0x28A;
        room.flags |= 8;
        return;
    }
    if (fv_crew_shot == 2) {
        stk[--sp_] = N_sstunn;
        stk[--sp_] = 1;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[1] = 0;
        room.timers[1] = 0x64;
        room.stun[1] = 0x28A;
        room.flags |= 2;
        return;
    }
    if (fv_crew_shot == 3) {
        stk[--sp_] = N_mstune;
        stk[--sp_] = 2;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[2] = 3;
        room.timers[1] = 0x64;
        room.stun[2] = 0x28A;
        room.flags |= 4;
        return;
    }
    if (fv_crew_shot == 4) {
        stk[--sp_] = 1;
        stk[--sp_] = 0xFFFF;
        stk[--sp_] = 0xFFFF;
        stk[--sp_] = N_kstunn;
        stk[--sp_] = 0;
        stk[--sp_] = 2;
        ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
        sp_ += 6;
        room.fall[0] = 0;
        room.stun[0] = 0x28A;
        room.flags |= 1;
    }
    return;
}

/* Kirk has been shot: game over.
 * [MSG_ANIM_DONE 01 00 00] */
static void demon1_KirkShot(void)
{
    demon1_Say(S_172C, T_DEM1N000);
    svc_GameOver();
    return;
}

/* [MSG_USE 41 FF 00] */
static void demon1_E_05B9(void)
{
    { demon1_PhaserAfterFight(); return; }
}

/* [MSG_USE 40 FF 00] */
static void demon1_PhaserAfterFight(void)
{
    if (fv_klingons_down == 3) {
        demon1_Say(N_Dr_McCoy, T_DEM1_011);
        goto L0623;
    }
L0623:;
    return;
}

/* Kirk phasers Klingon 1 (anim msg 3), once.
 * [MSG_USE 41 08 00] */
static void demon1_E_062A(void)
{
    { demon1_ShootKlingon1(); return; }
}

/* [MSG_USE 40 08 00] */
static void demon1_ShootKlingon1(void)
{
    if (fv_klingon1_hit != 0) {
        return;
    }
    room.busy = 1;
    fv_klingon1_hit = 1;
    if (fv_kirk_firing == 1) {
        { demon1_Klingon1Hit(); return; }
    }
    svc_Anim(0, N_kfiren, -1, -1, 3);
    fv_kirk_firing = 1;
    return;
}

/* Klingon 1 falls (anim msg 7); the fight is over when all three are down.
 * [MSG_ANIM_DONE 03 00 00] */
static void demon1_Klingon1Hit(void)
{
    fv_klingons_down++;
    if (fv_klingons_down == 3) {
        room.timers[1] = 0;
        room.timers[5] = 0xB4;
        room.timers[6] = 1;
        rv_under_fire = 0;
        room.no_walk = 0;
    }
    svc_Anim0(0xC, N_s0ks1, 0, 0, 0);
    svc_BridgeSfx(6);
    svc_Sfx(N_EFX19);
    svc_Anim(8, N_klg1d, 0x120, 0x82, 7);
    return;
}

/* ...and loses his hand.
 * [MSG_ANIM_DONE 07 00 00] */
static void demon1_Klingon1Falls(void)
{
    svc_Anim0(0xD, N_klghnd, 0x10B, 0x8E, 0);
    room.busy = 0;
    room.timers[1] = 0;
    demon1_Say(0, T_DEM1N020);
    if ((room.flags & 8) != 0) {
    } else {
        demon1_Say(N_Ensign_Everts, T_DEM1_025);
        if (fv_klingons_down != 3) {
            demon1_Say(N_Klingon, T_DEM1_F23);
            room.timers[1] = 1;
        }
    }
    return;
}

/* The same for Klingons 2 and 3 (msgs 4 and 5).
 * [MSG_USE 41 09 00] */
static void demon1_E_0818(void)
{
    { demon1_ShootKlingon2(); return; }
}

/* [MSG_USE 40 09 00] */
static void demon1_ShootKlingon2(void)
{
    if (fv_klingon2_hit != 0) {
        return;
    }
    room.busy = 1;
    fv_klingon2_hit = 1;
    if (fv_kirk_firing == 1) {
        { demon1_Klingon2Hit(); return; }
    }
    svc_Anim(0, N_kfiren, -1, -1, 4);
    fv_kirk_firing = 1;
    return;
}

/* [MSG_ANIM_DONE 04 00 00] */
static void demon1_Klingon2Hit(void)
{
    fv_klingons_down++;
    if (fv_klingons_down == 3) {
        room.timers[1] = 0;
        room.timers[5] = 0xB4;
        room.timers[6] = 1;
        rv_under_fire = 0;
        room.no_walk = 0;
    }
    svc_Anim0(0xC, N_s0ks2, 0, 0, 0);
    svc_BridgeSfx(6);
    svc_Anim(9, N_klg2d, 0xAA, 0x7C, 0);
    room.busy = 0;
    return;
}

/* [MSG_USE 41 0A 00] */
static void demon1_E_08C5(void)
{
    { demon1_ShootKlingon3(); return; }
}

/* [MSG_USE 40 0A 00] */
static void demon1_ShootKlingon3(void)
{
    if (fv_klingon3_hit != 0) {
        return;
    }
    room.busy = 1;
    fv_klingon3_hit = 1;
    if (fv_kirk_firing == 1) {
        { demon1_Klingon3Hit(); return; }
    }
    svc_Anim(0, N_kfiren, -1, -1, 5);
    fv_kirk_firing = 1;
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void demon1_Klingon3Hit(void)
{
    fv_klingons_down++;
    if (fv_klingons_down == 3) {
        room.timers[1] = 0;
        room.timers[5] = 0xB4;
        room.timers[6] = 1;
        rv_under_fire = 0;
        room.no_walk = 0;
    }
    svc_Anim0(0xC, N_s0ks3, 0, 0, 0);
    svc_BridgeSfx(6);
    svc_Anim(0xA, N_klg3d, 0, 0, 0);
    room.busy = 0;
    return;
}

/* Timer 6: everyone stunned gets up in 45 ticks.
 * [MSG_TIMER 06 00 00] */
static void demon1_CrewComeRound(void)
{
    room.stun[0] = 0x2D;
    room.stun[1] = 0x2D;
    room.stun[2] = 0x2D;
    room.stun[3] = 0x2D;
    return;
}

/* Timer 5: Uhura asks what the shooting was.
 * [MSG_TIMER 05 00 00] */
static void demon1_UhuraCalls(void)
{
    if (room.flags != 0) {
        return;
    }
    demon1_Say(N_Lt_Uhura, T_DEM1U077);
    demon1_Say(N_Capt_Kirk, T_DEM1_003);
    demon1_Say(N_Lt_Uhura, T_DEM1U078);
    demon1_Say(N_Capt_Kirk, T_DEM1_002);
    demon1_Say(N_Mr_Spock, T_DEM1_020);
    demon1_Say(N_Capt_Kirk, T_DEM1_004);
    demon1_Say(N_Mr_Spock, T_DEM1_021);
    return;
}

/* Tricorders on the plants, the hand and the Klingons.
 * [MSG_USE 45 0D 00] */
static void demon1_McCoyScansHand(void)
{
    svc_Anim(2, N_mscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Dr_McCoy, T_DEM1_012);
    return;
}

/* [MSG_USE 44 21 00] */
static void demon1_SpockScansTulips(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_007);
    return;
}

/* [MSG_USE 44 22 00] */
static void demon1_SpockScansPods(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_008);
    return;
}

/* [MSG_USE 44 20 00] */
static void demon1_SpockScansCattails(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_005);
    return;
}

/* [MSG_USE 44 23 00] */
static void demon1_SpockScansFern(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_006);
    return;
}

/* [MSG_USE 44 0D 00] */
static void demon1_SpockScansHand(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_017);
    return;
}

/* Spock's first scan of Klingon 1 after the fight explains the androids,
 * once. Its scanning animation was taken out (the pushes are dropped).
 * [MSG_USE 44 08 00] */
static void demon1_SpockScansKlingon1(void)
{
    svc_BridgeSfx(4);
    if (fv_klingons_down == 3) {
        if (rv_took_hand != 1) {
            if (fv_scanned_klingon != 1) {
                fv_scanned_klingon = 1;
                demon1_Say(N_Mr_Spock, T_DEM1_018);
                return;
            }
        }
    }
    demon1_Say(N_Mr_Spock, T_DEM1_019);
    return;
}

/* [MSG_USE 44 0A 00] */
static void demon1_E_0F35(void)
{
    { demon1_SpockScansKlingon(); return; }
}

/* [MSG_USE 44 09 00] */
static void demon1_SpockScansKlingon(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_019);
    return;
}

/* [MSG_USE 7F 08 00] */
static void demon1_E_0F72(void)
{
    { demon1_McCoyScansKlingon1(); return; }
}

/* [MSG_USE 45 08 00] */
static void demon1_McCoyScansKlingon1(void)
{
    svc_Anim(2, N_mscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Dr_McCoy, T_DEM1_012);
    return;
}

/* [MSG_USE 7F 0A 00] */
static void demon1_E_0FAF(void)
{
    { demon1_McCoyScansKlingon3(); return; }
}

/* [MSG_USE 7F 09 00] */
static void demon1_McCoyScansKlingon3(void)
{
    { demon1_McCoyScansKlingon3b(); return; }
}

/* [MSG_USE 45 0A 00] */
static void demon1_McCoyScansKlingon3b(void)
{
    { demon1_McCoyScansKlingon(); return; }
}

/* [MSG_USE 45 09 00] */
static void demon1_McCoyScansKlingon(void)
{
    svc_Anim(2, N_mscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Dr_McCoy, T_DEM1_012);
    return;
}

/* [MSG_USE 44 08 00] */
static void demon1_SpockScansKlingon1Again(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Mr_Spock, T_DEM1_016);
    return;
}

/* McCoy on a stunned crew member.
 * [MSG_USE 45 00 00] */
static void demon1_McCoyScansKirk(void)
{
    if ((room.flags & 1) != 0)
        { demon1_part_1052(); return; }
    return;
}

/* [MSG_USE 45 01 00] */
static void demon1_McCoyScansSpock(void)
{
    if ((room.flags & 2) != 0)
        { demon1_part_1052(); return; }
    return;
}

/* [MSG_USE 45 03 00] */
static void demon1_McCoyScansRedshirt(void)
{
    if ((room.flags & 8) != 0)
        { demon1_part_1052(); return; }
    return;
}

static void demon1_part_1052(void)
{
    svc_Anim(2, N_mscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon1_Say(N_Dr_McCoy, T_DEM1_009);
    return;
}

/* Getting the hand: walk to it (msg 1), crouch (anim msg 6), +3 score,
 * and on timer 4 it is in the inventory.
 * [MSG_GET 0D 00 00] */
static void demon1_GetHand(void)
{
    if (fv_klingons_down != 3) {
        return;
    }
    svc_Walk(0, 0x104, 0x8E, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void demon1_ReachHand(void)
{
    svc_Anim(0, N_s5r1kg, -1, -1, 6);
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void demon1_PickUpHand(void)
{
    svc_StopAnim(0xD);
    rv_score += 3;
    svc_StopAnim(0);
    room.timers[4] = 2;
    return;
}

/* [MSG_TIMER 04 00 00] */
static void demon1_GiveHand(void)
{
    uint16_t bx = 0;
    bx = T_DEM1N005;
    if (rv_took_hand == 0) {
        rv_took_hand = 1;
        svc_GiveItem(IT_HAND);
        bx = T_DEM1N007;
    }
    demon1_Say(0, bx);
    return;
}

/* Looking at things.
 * [MSG_LOOK 08 00 00] */
static void demon1_E_115B(void)
{
    { demon1_LookAtKlingon2(); return; }
}

/* [MSG_LOOK 09 00 00] */
static void demon1_LookAtKlingon2(void)
{
    { demon1_LookAtKlingon(); return; }
}

/* [MSG_LOOK 0A 00 00] */
static void demon1_LookAtKlingon(void)
{
    demon1_Say(S_172C, T_DEM1N004);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void demon1_LookAtCattails(void)
{
    demon1_Say(0, T_DEM1N018);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void demon1_LookAtTulips(void)
{
    demon1_Say(0, T_DEM1N010);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void demon1_LookAtPods(void)
{
    demon1_Say(0, T_DEM1N019);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void demon1_LookAtFern(void)
{
    demon1_Say(0, T_DEM1N009);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void demon1_LookAtStream(void)
{
    demon1_Say(0, T_DEM1N011);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void demon1_LookAtMine(void)
{
    demon1_Say(0, T_DEM1N021);
    return;
}

/* [MSG_LOOK 26 00 00] */
static void demon1_LookAtMountain(void)
{
    demon1_Say(0, T_DEM1N016);
    return;
}

/* [MSG_LOOK 0D 00 00] */
static void demon1_LookAtHand(void)
{
    demon1_Say(0, T_DEM1N023);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void demon1_LookAtScene(void)
{
    demon1_Say(0, T_DEM1N017);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void demon1_LookAtKirk(void)
{
    if ((room.flags & 1) == 0) {
        demon1_Say(S_172C, T_DEM1N003);
        return;
    }
    demon1_Say(0, T_DEM1N012);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void demon1_LookAtSpock(void)
{
    if ((room.flags & 2) == 0) {
        demon1_Say(S_172C, T_DEM1N008);
        return;
    }
    demon1_Say(0, T_DEM1N015);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void demon1_LookAtMcCoy(void)
{
    if ((room.flags & 4) == 0) {
        demon1_Say(S_172C, T_DEM1N001);
        return;
    }
    demon1_Say(0, T_DEM1N013);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void demon1_LookAtRedshirt(void)
{
    if ((room.flags & 8) == 0) {
        demon1_Say(S_172C, T_DEM1N002);
        return;
    }
    demon1_Say(0, T_DEM1N014);
    return;
}

/* Talking to the crew.
 * [MSG_TALK 00 00 00] */
static void demon1_TalkToKirk(void)
{
    if ((room.flags & 1) != 0)
        { demon1_part_15AF(); return; }
    if (fv_klingons_down == 3) {
        demon1_Say(N_Capt_Kirk, T_DEM1_001);
    }
    return;
}

/* [MSG_TALK 01 00 00] */
static void demon1_TalkToSpock(void)
{
    if ((room.flags & 2) != 0)
        { demon1_part_15AF(); return; }
    demon1_Say(N_Mr_Spock, T_DEM1_022);
    demon1_Say(N_Dr_McCoy, T_DEM1_013);
    demon1_Say(N_Mr_Spock, T_DEM1_024);
    demon1_Say(N_Dr_McCoy, T_DEM1_014);
    return;
}

/* [MSG_TALK 02 00 00] */
static void demon1_TalkToMcCoy(void)
{
    if ((room.flags & 4) != 0)
        { demon1_part_15AF(); return; }
    demon1_Say(N_Dr_McCoy, T_DEM1_015);
    demon1_Say(N_Mr_Spock, T_DEM1_023);
    return;
}

/* [MSG_TALK 03 00 00] */
static void demon1_TalkToRedshirt(void)
{
    if ((room.flags & 8) != 0)
        { demon1_part_15AF(); return; }
    demon1_Say(N_Ensign_Everts, T_DEM1_026);
    return;
}

static void demon1_part_15AF(void)
{
    demon1_Say(S_172C, N_Zzzzz);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void demon1_Say(uint16_t ax, uint16_t bx)
{
    fv_rdf_bans = ax;
    CS_W(0x16E9) = bx;
    demon1_Talk(rdf_bans);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void demon1_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript demon1_scripts[] = {
    { 0x01CE, demon1_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x02B8, demon1_WalkToMine },    /* MSG_WALK 25 00 00 */
    { 0x02DB, demon1_E_02DB },    /* MSG_WALK_DONE 02 00 00 */
    { 0x02E3, demon1_ExitToMine },    /* MSG_EXIT 00 00 00 */
    { 0x02FE, demon1_ExitToColony },    /* MSG_EXIT 01 00 00 */
    { 0x0319, demon1_Ambush },    /* MSG_TIMER 02 00 00 */
    { 0x0356, demon1_Klingon2StandsUp },    /* MSG_TIMER 00 00 00 */
    { 0x0373, demon1_Klingon3StandsUp },    /* MSG_TIMER 03 00 00 */
    { 0x0390, demon1_KlingonFires },    /* MSG_TIMER 01 00 00 */
    { 0x04BF, demon1_CrewMemberStunned },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x059F, demon1_KirkShot },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x05B9, demon1_E_05B9 },    /* MSG_USE 41 FF 00 */
    { 0x05C1, demon1_PhaserAfterFight },    /* MSG_USE 40 FF 00 */
    { 0x062A, demon1_E_062A },    /* MSG_USE 41 08 00 */
    { 0x0632, demon1_ShootKlingon1 },    /* MSG_USE 40 08 00 */
    { 0x0671, demon1_Klingon1Hit },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x06EB, demon1_Klingon1Falls },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0818, demon1_E_0818 },    /* MSG_USE 41 09 00 */
    { 0x0820, demon1_ShootKlingon2 },    /* MSG_USE 40 09 00 */
    { 0x085F, demon1_Klingon2Hit },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x08C5, demon1_E_08C5 },    /* MSG_USE 41 0A 00 */
    { 0x08CD, demon1_ShootKlingon3 },    /* MSG_USE 40 0A 00 */
    { 0x090C, demon1_Klingon3Hit },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x0971, demon1_CrewComeRound },    /* MSG_TIMER 06 00 00 */
    { 0x098C, demon1_UhuraCalls },    /* MSG_TIMER 05 00 00 */
    { 0x0BE9, demon1_McCoyScansHand },    /* MSG_USE 45 0D 00 */
    { 0x0C1A, demon1_SpockScansTulips },    /* MSG_USE 44 21 00 */
    { 0x0CA9, demon1_SpockScansPods },    /* MSG_USE 44 22 00 */
    { 0x0D55, demon1_SpockScansCattails },    /* MSG_USE 44 20 00 */
    { 0x0E29, demon1_SpockScansFern },    /* MSG_USE 44 23 00 */
    { 0x0EAB, demon1_SpockScansHand },    /* MSG_USE 44 0D 00 */
    { 0x0EDC, demon1_SpockScansKlingon1 },    /* MSG_USE 44 08 00 */
    { 0x0F35, demon1_E_0F35 },    /* MSG_USE 44 0A 00 */
    { 0x0F3D, demon1_SpockScansKlingon },    /* MSG_USE 44 09 00 */
    { 0x0F72, demon1_E_0F72 },    /* MSG_USE 7F 08 00 */
    { 0x0F7A, demon1_McCoyScansKlingon1 },    /* MSG_USE 45 08 00 */
    { 0x0FAF, demon1_E_0FAF },    /* MSG_USE 7F 0A 00 */
    { 0x0FB7, demon1_McCoyScansKlingon3 },    /* MSG_USE 7F 09 00 */
    { 0x0FBF, demon1_McCoyScansKlingon3b },    /* MSG_USE 45 0A 00 */
    { 0x0FC7, demon1_McCoyScansKlingon },    /* MSG_USE 45 09 00 */
    { 0x0FFC, demon1_SpockScansKlingon1Again },    /* MSG_USE 44 08 00 */
    { 0x1031, demon1_McCoyScansKirk },    /* MSG_USE 45 00 00 */
    { 0x103E, demon1_McCoyScansSpock },    /* MSG_USE 45 01 00 */
    { 0x104B, demon1_McCoyScansRedshirt },    /* MSG_USE 45 03 00 */
    { 0x10CA, demon1_GetHand },    /* MSG_GET 0D 00 00 */
    { 0x10EE, demon1_ReachHand },    /* MSG_WALK_DONE 01 00 00 */
    { 0x110A, demon1_PickUpHand },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x1132, demon1_GiveHand },    /* MSG_TIMER 04 00 00 */
    { 0x115B, demon1_E_115B },    /* MSG_LOOK 08 00 00 */
    { 0x1163, demon1_LookAtKlingon2 },    /* MSG_LOOK 09 00 00 */
    { 0x116B, demon1_LookAtKlingon },    /* MSG_LOOK 0A 00 00 */
    { 0x117B, demon1_LookAtCattails },    /* MSG_LOOK 20 00 00 */
    { 0x11C6, demon1_LookAtTulips },    /* MSG_LOOK 21 00 00 */
    { 0x120B, demon1_LookAtPods },    /* MSG_LOOK 22 00 00 */
    { 0x1266, demon1_LookAtFern },    /* MSG_LOOK 23 00 00 */
    { 0x1299, demon1_LookAtStream },    /* MSG_LOOK 24 00 00 */
    { 0x12E8, demon1_LookAtMine },    /* MSG_LOOK 25 00 00 */
    { 0x132A, demon1_LookAtMountain },    /* MSG_LOOK 26 00 00 */
    { 0x1366, demon1_LookAtHand },    /* MSG_LOOK 0D 00 00 */
    { 0x13A9, demon1_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x13FE, demon1_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x144C, demon1_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x1497, demon1_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x14E2, demon1_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x1531, demon1_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x154F, demon1_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x1580, demon1_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x159F, demon1_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0, 0 }
};
