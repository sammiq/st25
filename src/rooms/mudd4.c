/* MUDD4.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/mudd4.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * MUDD4.RDF - "Another Fine Mess", the alien ship's bridge. Two stations:
 * Spock works out that the aliens count in sixes, the clue for the library
 * (room 3); with the library decoded, the secondary station shows sensors,
 * navigation and engineering, and the primary works the viewscreen and
 * communications. Through these, once Spock has studied the weapons
 * battery (room 1), Kirk reaches Scott, ends the mission, and has a last
 * word with Mudd.
 *
 * Actors: 8 the viewscreen, 10 (0Ah) the doover, 11 (0Bh) Mudd.
 * Objects: 0Ah the doover, 20h the viewscreen, 21h-23h the stations.
 * Exits: 2 room 2, 3 room 3. Region 0: the doorsteps.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 0EAA: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s4crks = 0x00CE,
    N_s4crkl = 0x00D5,
    N_s4crkr = 0x00DC,
    N_s4crss = 0x00E3,
    N_s4crsl = 0x00EA,
    N_s4crsr = 0x00F1,
    N_s4crvs = 0x00F8,
    N_s4crvo = 0x00FF,
    N_s4crvf = 0x0106,
    N_kuseln = 0x010D,
    N_kgetde = 0x0114,
    N_sgetdn = 0x011B,
    N_mgetds = 0x0122,
    N_rgetdw = 0x0129,
    N_sscann = 0x0130,
    N_sscane = 0x0137,
    N_s4crhw = 0x013E,
    N_s4crdv = 0x0145,
    N_s4cbxp = 0x014C,
    N_MUD4LOOP = 0x015B,
    T_MUD4_018 = 0x01C6,
    N_Lt_Uhura = 0x01EB,
    T_STATICU1 = 0x01F5,
    N_SE2BIGAS = 0x0583,
    T_MUD4_043 = 0x09CE,
    N_EXPLO3 = 0x0A5D,
    S_0BB9 = 0x0BB9,
    S_0BDC = 0x0BDC,
    S_0BFF = 0x0BFF,
    T_MUD4N005 = 0x0C22,
    T_MUD4N007 = 0x0CE2,
    N_SMADOOR3 = 0x0E91,
    N_Capt_Kirk = 0x0FD0,
    N_Dr_McCoy = 0x0FDB,
    N_Mr_Spock = 0x0FE5,
    N_Lt_Buchert = 0x0FEF,
    N_Mr_Scott = 0x0FFB,
    N_Harry_Mudd = 0x1005,
    S_1010 = 0x1010,
    talk_buf = 0x1011,
    T_MUD4_061 = 0x1017,
    T_MUD4_013 = 0x109F,
    T_MUD4_062 = 0x10C0,
    T_MUD4_048 = 0x1119,
    T_MUD4_059 = 0x116B,
    T_MUD4_031 = 0x11FF,
    T_MUD4_060 = 0x1244,
    T_MUD4_057 = 0x131F,
    T_MUD4_058 = 0x139F,
    T_MUD4_053 = 0x14BD,
    T_MUD4_056 = 0x1593,
    T_MUD4_055 = 0x15E3,
    T_MUD4_049 = 0x169A,
    T_MUD4_052 = 0x16D0,
    T_MUD4_006 = 0x1746,
    T_MUD4_004 = 0x17F6,
    T_MUD4_054 = 0x184B,
    T_MUD4_025 = 0x18C9,
    T_MUD4_S02 = 0x18E9,
    T_MUD4_027 = 0x19BC,
    T_MUD4_028 = 0x1A9C,
    T_MUD4_S04 = 0x1B00,
    T_MUD4_020 = 0x1C6F,
    T_MUD4_S03 = 0x1C9B,
    T_MUD4_045 = 0x1D83,
    T_MUD4_075 = 0x1DBB,
    T_MUD4N000 = 0x1E79,
    T_MUD4_042 = 0x1EBD,
    T_MUD4N013 = 0x1F31,
    T_MUD4N008 = 0x1F5D,
    T_MUD4_050 = 0x1FF2,
    T_MUD4_065 = 0x204B,
    T_MUD4_066 = 0x20CC,
    T_MUD4_074 = 0x22B1,
    T_MUD4_017 = 0x22EB,
    T_MUD4_072 = 0x23F5,
    T_MUD4_008 = 0x242F,
    T_MUD4_068 = 0x24B3,
    T_MUD4_032 = 0x259A,
    T_MUD4_034 = 0x262C,
    T_MUD4_037 = 0x268D,
    T_MUD4_026 = 0x26E8,
    T_MUD4_071 = 0x27A1,
    T_MUD4_036 = 0x2875,
    T_MUD4_029 = 0x2897,
    T_MUD4_030 = 0x28DD,
    T_MUD4_064 = 0x29B5,
    T_MUD4_012 = 0x2A29,
    T_MUD4_070 = 0x2A42,
    T_MUD4_010 = 0x2A86,
    T_MUD4_067 = 0x2AA3,
    T_MUD4_011 = 0x2AC1,
    T_MUD4_073 = 0x2B54,
    T_MUD4_035 = 0x2B93,
    T_MUD4_069 = 0x2BD5,
    T_MUD4N012 = 0x2C0E,
    T_MUD2_002 = 0x2C38,
    T_MUD4N009 = 0x2CB8,
    T_MUD4N002 = 0x2D09,
    T_MUD4N003 = 0x2D5F,
    T_MUD4N006 = 0x2D9D,
    T_MUD4N001 = 0x2DE3,
    T_MUD4N004 = 0x2E10,
    T_MUD4N011 = 0x2E88,
    T_MUD4_005 = 0x2ED0,
    T_MUD4_063 = 0x2F28,
    T_MUD4_015 = 0x2F61,
    T_MUD4_047 = 0x2F8B,
    T_MUD4_046 = 0x2FCB,
    T_MUD4_076 = 0x301A,
    T_MUD4_051 = 0x307B,
    T_MUD4_001 = 0x30DF,
    SecondaryMenu = 0x3108,
    PrimaryMenu = 0x3114,
    KirkToScott = 0x311E,
    KirkToScottAgain = 0x3128,
    KirkOnDegrimer = 0x3132,
    KirkToMuddLeaving = 0x313A,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_collapsed CS_B(0xCC)
#define fv_going_door CS_B(0xCD)
#define fv_kirk_secondary CS_B(0xCB)
#define fv_spock_primary CS_B(0xCA)
#define fv_talk_buf CS_W(0x1011)
#define rv_air_countdown ROOM_W(0x52)
#define rv_air_failing ROOM_B(0x46)
#define rv_air_left ROOM_W(0x50)
#define rv_called_scott ROOM_B(0x44)
#define rv_know_physiology ROOM_B(0x36)
#define rv_knows_battery ROOM_B(0x34)
#define rv_library_erased ROOM_B(0x37)
#define rv_mudd_allowed ROOM_B(0x3B)
#define rv_mudd_incident ROOM_B(0x3F)
#define rv_read_library ROOM_B(0x41)
#define rv_score ROOM_W(0x5A)
#define rv_six_clue ROOM_B(0x35)
#define rv_take_weapon ROOM_W(0x3D)
#define rv_took_doover ROOM_B(0x42)
#define rv_used_device ROOM_B(0x3C)
#define rv_viewscreen_on ROOM_B(0x45)
#define rv_viewscreen_scored ROOM_B(0x57)

/* functions */
static void mudd4_StartRoom(void);
static void mudd4_CallEnterprise(void);
static void mudd4_Timer2Sound(void);
static void mudd4_Timer3Sound(void);
static void mudd4_E_0247(void);
static void mudd4_SpockToPrimary(void);
static void mudd4_SpockToSecondary(void);
static void mudd4_part_0263(void);
static void mudd4_SpockAtStation(void);
static void mudd4_SecondaryStation(void);
static void mudd4_part_035E(void);
static void mudd4_Sensors(void);
static void mudd4_Engineering(void);
static void mudd4_Navigation(void);
static void mudd4_SpockArrives(void);
static void mudd4_E_0424(void);
static void mudd4_KirkToSecondary(void);
static void mudd4_KirkToPrimary(void);
static void mudd4_part_0440(void);
static void mudd4_KirkAtStation(void);
static void mudd4_PrimaryStation(void);
static void mudd4_part_0506(void);
static void mudd4_ViewscreenToggled(void);
static void mudd4_ViewscreenOn(void);
static void mudd4_Communications(void);
static void mudd4_part_0671(void);
static void mudd4_MuddFarewell(void);
static void mudd4_E_0884(void);
static void mudd4_E_088C(void);
static void mudd4_McCoyAtStation(void);
static void mudd4_E_08BB(void);
static void mudd4_E_08C3(void);
static void mudd4_BuchertAtStation(void);
static void mudd4_SpockScansDoover(void);
static void mudd4_SpockScansConsole(void);
static void mudd4_SpockScansViewscreen(void);
static void mudd4_CleanLenses(void);
static void mudd4_UseMedkit(void);
static void mudd4_FireDevice(void);
static void mudd4_DeviceFired(void);
static void mudd4_GetDoover(void);
static void mudd4_KirkAtDoover(void);
static void mudd4_TookDoover(void);
static void mudd4_UseDegrimer(void);
static void mudd4_AirRunsOut(void);
static void mudd4_CrewCollapse(void);
static void mudd4_SpockCollapses(void);
static void mudd4_part_0CCB(void);
static void mudd4_LookAtConsole(void);
static void mudd4_LookAtViewscreen(void);
static void mudd4_LookAtKirk(void);
static void mudd4_LookAtSpock(void);
static void mudd4_LookAtMcCoy(void);
static void mudd4_LookAtRedshirt(void);
static void mudd4_LookAtDoover(void);
static void mudd4_TalkToKirk(void);
static void mudd4_TalkToSpock(void);
static void mudd4_TalkToMcCoy(void);
static void mudd4_TalkToRedshirt(void);
static void mudd4_WalkToRightDoor(void);
static void mudd4_WalkToLeftDoor(void);
static void mudd4_ReachDoor(void);
static uint16_t mudd4_Talk(uint16_t ax);
static void mudd4_Say(uint16_t ax, uint16_t bx);

/* The room starts: the doover, the viewscreen if on.
 * [MSG_TICK 01 00 00] */
static void mudd4_StartRoom(void)
{
    svc_Sfx(N_MUD4LOOP);
    if (rv_took_doover == 0) {
        svc_Anim0(0xA, N_s4crdv, 0xC9, 0x8E, 0);
    }
    if (rv_viewscreen_on == 1) {
        svc_Anim0(8, N_s4crvo, 0xA5, 0x76, 0);
    }
    return;
}

/* [MSG_USE 47 FF 00] */
static void mudd4_CallEnterprise(void)
{
    mudd4_Say(N_Capt_Kirk, T_MUD4_018);
    mudd4_Say(N_Lt_Uhura, T_STATICU1);
    return;
}

/* Timers 2, 3: sounds.
 * [MSG_TIMER 02 00 00] */
static void mudd4_Timer2Sound(void)
{
    svc_BridgeSfx(7);
    return;
}

/* [MSG_TIMER 03 00 00] */
static void mudd4_Timer3Sound(void)
{
    svc_BridgeSfx(0xA);
    return;
}

/* Spock at the stations: before the library, he notices the aliens count
 * in sixes (+1). After it, the secondary station (msg 1): sensors,
 * navigation, engineering (anim msgs 2-4).
 * [MSG_USE 01 21 00] */
static void mudd4_E_0247(void)
{
    { mudd4_SpockToPrimary(); return; }
}

/* [MSG_USE 01 22 00] */
static void mudd4_SpockToPrimary(void)
{
    fv_spock_primary = 1;
    { mudd4_part_0263(); return; }
}

/* [MSG_USE 01 23 00] */
static void mudd4_SpockToSecondary(void)
{
    fv_spock_primary = 0;
    { mudd4_part_0263(); return; }
}

static void mudd4_part_0263(void)
{
    room.fall[1] = 0;
    svc_StopAnim(1);
    if (rv_know_physiology == 0) {
        if (rv_six_clue == 0) {
            mudd4_Say(N_Mr_Spock, T_MUD4_061);
            mudd4_Say(N_Capt_Kirk, T_MUD4_013);
            mudd4_Say(N_Mr_Spock, T_MUD4_062);
            mudd4_Say(N_Dr_McCoy, T_MUD4_048);
            mudd4_Say(N_Mr_Spock, T_MUD4_059);
            mudd4_Say(N_Capt_Kirk, T_MUD4_031);
            mudd4_Say(N_Mr_Spock, T_MUD4_060);
            if (rv_six_clue != 1) {
                rv_six_clue = 1;
                rv_score += 1;
            }
            goto L02EA;
        }
        mudd4_Say(N_Mr_Spock, T_MUD4_062);
    } else if (fv_spock_primary == 0) {
        mudd4_Say(N_Mr_Spock, T_MUD4_057);
    }
L02EA:;
    room.fall[1] = 0;
    svc_Walk(1, 0x6D, 0x9A, 1);
    room.busy = 1;
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void mudd4_SpockAtStation(void)
{
    if (fv_spock_primary == 1) {
        if (rv_know_physiology == 1) {
            svc_Anim(1, N_s4crss, -1, -1, 1);
            return;
        }
    }
    room.busy = 0;
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void mudd4_SecondaryStation(void)
{
    svc_Anim(1, N_s4crsr, 0x55, 0x98, 0);
    room.busy = 0;
    mudd4_Say(N_Mr_Spock, T_MUD4_058);
    { mudd4_part_035E(); return; }
}

static void mudd4_part_035E(void)
{
    uint16_t ax = 0;
    uint16_t s0;
    ax = (uint16_t)mudd4_Talk(SecondaryMenu);
    if (ax == 3) {
        room.fall[1] = 0;
        svc_Walk(1, 0x69, 0xB7, 0);
        return;
    }
    if (ax == 1) {
        room.busy = 1;
        svc_Anim(1, N_s4crsl, 0x55, 0x98, 4);
        room.timers[2] = 5;
        return;
    }
    room.busy = 1;
    if (ax == 0) {
        s0 = 2;
    } else {
        s0 = 3;
    }
    svc_Anim(1, N_s4crsr, 0x55, 0x98, s0);
    room.timers[2] = 5;
    return;
}

/* [MSG_ANIM_DONE 02 00 00] */
static void mudd4_Sensors(void)
{
    room.busy = 0;
    mudd4_Say(N_Mr_Spock, T_MUD4_053);
    { mudd4_part_035E(); return; }
}

/* [MSG_ANIM_DONE 03 00 00] */
static void mudd4_Engineering(void)
{
    room.busy = 0;
    mudd4_Say(N_Mr_Spock, T_MUD4_056);
    { mudd4_part_035E(); return; }
}

/* [MSG_ANIM_DONE 04 00 00] */
static void mudd4_Navigation(void)
{
    room.busy = 0;
    mudd4_Say(N_Mr_Spock, T_MUD4_055);
    { mudd4_part_035E(); return; }
}

/* [MSG_WALK_DONE 03 00 00] */
static void mudd4_SpockArrives(void)
{
    room.busy = 0;
    { mudd4_SpockToPrimary(); return; }
}

/* Kirk at the stations: before the library, he calls Spock over (msg 3).
 * After it, the primary station (msgs 2, 5): the viewscreen (msg 6, +1
 * once) and communications (msg 7).
 * [MSG_USE 00 21 00] */
static void mudd4_E_0424(void)
{
    { mudd4_KirkToSecondary(); return; }
}

/* [MSG_USE 00 23 00] */
static void mudd4_KirkToSecondary(void)
{
    fv_kirk_secondary = 1;
    { mudd4_part_0440(); return; }
}

/* [MSG_USE 00 22 00] */
static void mudd4_KirkToPrimary(void)
{
    fv_kirk_secondary = 0;
    { mudd4_part_0440(); return; }
}

static void mudd4_part_0440(void)
{
    room.fall[0] = 0;
    svc_StopAnim(0);
    if (rv_know_physiology == 0) {
        mudd4_Say(N_Capt_Kirk, T_MUD4_006);
        room.fall[1] = 0;
        svc_Walk(1, 0x6D, 0x9A, 3);
        room.busy = 1;
        return;
    }
    if (fv_kirk_secondary == 0) {
        mudd4_Say(N_Capt_Kirk, T_MUD4_049);
    }
    room.fall[0] = 0;
    svc_Walk(0, 0x7E, 0x98, 2);
    room.busy = 1;
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void mudd4_KirkAtStation(void)
{
    if (fv_kirk_secondary == 1) {
        if (rv_know_physiology == 1) {
            svc_Anim(0, N_s4crks, -1, -1, 5);
            return;
        }
    }
    room.busy = 0;
    return;
}

/* [MSG_ANIM_DONE 05 00 00] */
static void mudd4_PrimaryStation(void)
{
    svc_Anim(0, N_s4crkl, 0x94, 0x98, 0);
    room.timers[2] = 5;
    room.busy = 0;
    mudd4_Say(N_Mr_Spock, T_MUD4_052);
    { mudd4_part_0506(); return; }
}

static void mudd4_part_0506(void)
{
    uint16_t ax = 0;
    ax = (uint16_t)mudd4_Talk(PrimaryMenu);
    if (ax != 0) {
        if (ax != 1) {
            room.fall[0] = 0;
            svc_Walk(0, 0x83, 0xB6, 0);
            return;
        }
        room.busy = 1;
        svc_Anim(0, N_s4crkl, 0x94, 0x98, 6);
        room.timers[2] = 5;
        return;
    }
    room.busy = 1;
    svc_Anim(0, N_s4crkr, 0x94, 0x98, 7);
    room.timers[2] = 5;
    return;
}

/* [MSG_ANIM_DONE 06 00 00] */
static void mudd4_ViewscreenToggled(void)
{
    if (rv_viewscreen_on == 0) {
        rv_viewscreen_on = 1;
        svc_Sfx(N_SE2BIGAS);
        svc_Anim0(8, N_s4crvs, 0xA5, 0x76, 0);
        room.timers[3] = 5;
        room.timers[1] = 0x32;
        if (rv_viewscreen_scored != 1) {
            rv_viewscreen_scored = 1;
            rv_score += 1;
        }
        return;
    }
    rv_viewscreen_on = 0;
    svc_Anim0(8, N_s4crvf, 0xA5, 0x76, 0);
    room.timers[3] = 5;
    room.timers[1] = 0x1E;
    return;
}

/* [MSG_TIMER 01 00 00] */
static void mudd4_ViewscreenOn(void)
{
    room.busy = 0;
    if (rv_viewscreen_on == 1) {
        mudd4_Say(N_Capt_Kirk, T_MUD4_004);
    }
    { mudd4_part_0506(); return; }
}

/* Communications, once Spock knows the weapons battery: Scott reports
 * (+1). Kirk can leave now (BeamUp: score, 27, take_weapon), later, or
 * after a word with Mudd, who comes in (walk msg 4).
 * [MSG_ANIM_DONE 07 00 00] */
static void mudd4_Communications(void)
{
    uint16_t ax = 0;
    room.busy = 0;
    if (rv_knows_battery == 0) {
        mudd4_Say(N_Mr_Spock, T_MUD4_054);
        { mudd4_part_0506(); return; }
    }
    if (rv_called_scott != 1) {
        rv_score += 1;
        mudd4_Say(N_Capt_Kirk, T_MUD4_025);
        mudd4_Say(N_Mr_Scott, T_MUD4_S02);
        if (rv_take_weapon == 1) {
            mudd4_Say(N_Capt_Kirk, T_MUD4_027);
        } else {
            mudd4_Say(N_Capt_Kirk, T_MUD4_028);
        }
        mudd4_Say(N_Mr_Scott, T_MUD4_S04);
        ax = (uint16_t)mudd4_Talk(KirkToScott);
        if (ax == 1) {
            { mudd4_part_0671(); return; }
        }
        if (ax != 0)
            goto L06B1;
        rv_called_scott = 1;
        return;
    }
    mudd4_Say(N_Capt_Kirk, T_MUD4_020);
    mudd4_Say(N_Mr_Scott, T_MUD4_S03);
    ax = (uint16_t)mudd4_Talk(KirkToScottAgain);
    if (ax == 1)
        { mudd4_part_0671(); return; }
    if (ax != 2) {
        return;
    }
L06B1:;
    if (rv_mudd_incident != 0) {
        return;
    }
    room.busy = 1;
    room.fall[0] = 2;
    svc_Walk(0, 0x83, 0xB4, 4);
    room.fall[1] = 2;
    svc_Walk(1, 0x69, 0xB7, 0);
    room.fall[2] = 0;
    svc_StopAnim(2);
    room.fall[3] = 2;
    svc_StopAnim(3);
    svc_Music(3, STALE_ARG);
    svc_Anim0(0xB, N_s4crhw, 0x13F, 0xC7, 0);
    return;
}

static void mudd4_part_0671(void)
{
    svc_BeamUp(rv_score, 0x1B, rv_take_weapon);
    return;
}

/* Mudd's farewell, depending on what he did in rooms 3 and 5.
 * [MSG_WALK_DONE 04 00 00] */
static void mudd4_MuddFarewell(void)
{
    uint16_t ax = 0;
    room.busy = 0;
    if (rv_used_device == 1)
        goto L079E;
    if (rv_mudd_allowed == 1)
        goto L079E;
    if (rv_library_erased == 1)
        goto L079E;
    if (rv_read_library == 0)
        goto L079E;
    mudd4_Say(N_Harry_Mudd, T_MUD4_065);
    ax = (uint16_t)mudd4_Talk(KirkOnDegrimer);
    if (ax == 0)
        goto L0777;
L0765:;
    mudd4_Say(N_Harry_Mudd, T_MUD4_074);
    mudd4_Say(N_Capt_Kirk, T_MUD4_017);
L0777:;
    mudd4_Say(N_Harry_Mudd, T_MUD4_072);
    mudd4_Say(N_Capt_Kirk, T_MUD4_008);
    mudd4_Say(N_Harry_Mudd, T_MUD4_068);
    mudd4_Say(N_Capt_Kirk, T_MUD4_032);
    { mudd4_part_0671(); return; }
L079E:;
    mudd4_Say(N_Harry_Mudd, T_MUD4_066);
    ax = (uint16_t)mudd4_Talk(KirkToMuddLeaving);
    if (ax == 1)
        goto L0765;
    if (rv_mudd_allowed != 1) {
        if (rv_library_erased != 1)
            goto L07C7;
    }
    mudd4_Say(N_Capt_Kirk, T_MUD4_034);
L07C7:;
    if (rv_used_device == 1) {
        mudd4_Say(N_Capt_Kirk, T_MUD4_037);
    }
    mudd4_Say(N_Capt_Kirk, T_MUD4_026);
    if (rv_mudd_allowed == 1) {
        if (rv_library_erased == 1) {
            mudd4_Say(N_Harry_Mudd, T_MUD4_071);
            mudd4_Say(N_Capt_Kirk, T_MUD4_036);
            mudd4_Say(N_Mr_Spock, T_MUD4_029);
            mudd4_Say(N_Capt_Kirk, T_MUD4_030);
            goto L0869;
        }
    }
    mudd4_Say(N_Harry_Mudd, T_MUD4_064);
    mudd4_Say(N_Capt_Kirk, T_MUD4_012);
    mudd4_Say(N_Harry_Mudd, T_MUD4_070);
    mudd4_Say(N_Capt_Kirk, T_MUD4_010);
    mudd4_Say(N_Harry_Mudd, T_MUD4_067);
    mudd4_Say(N_Capt_Kirk, T_MUD4_011);
    rv_score += 4;
    svc_Music(0x1E, STALE_ARG);
    mudd4_Say(N_Harry_Mudd, T_MUD4_073);
    mudd4_Say(N_Capt_Kirk, T_MUD4_035);
L0869:;
    mudd4_Say(N_Harry_Mudd, T_MUD4_069);
    mudd4_Say(N_Capt_Kirk, T_MUD4_001);
    { mudd4_part_0671(); return; }
}

/* McCoy and Buchert can't work the stations.
 * [MSG_USE 02 21 00] */
static void mudd4_E_0884(void)
{
    { mudd4_McCoyAtStation(); return; }
}

/* [MSG_USE 02 22 00] */
static void mudd4_E_088C(void)
{
    { mudd4_McCoyAtStation(); return; }
}

/* [MSG_USE 02 23 00] */
static void mudd4_McCoyAtStation(void)
{
    mudd4_Say(N_Dr_McCoy, T_MUD4_045);
    room.fall[2] = 0;
    svc_Walk(2, 0x7D, 0xC3, 0);
    return;
}

/* [MSG_USE 03 21 00] */
static void mudd4_E_08BB(void)
{
    { mudd4_BuchertAtStation(); return; }
}

/* [MSG_USE 03 22 00] */
static void mudd4_E_08C3(void)
{
    { mudd4_BuchertAtStation(); return; }
}

/* [MSG_USE 03 23 00] */
static void mudd4_BuchertAtStation(void)
{
    if (rv_know_physiology == 0) {
        mudd4_Say(N_Lt_Buchert, T_MUD4_075);
    } else {
        mudd4_Say(N_Lt_Buchert, T_MUD4_075);
    }
    room.fall[3] = 0;
    svc_Walk(3, 0x2A, 0xBB, 0);
    return;
}

/* Tricorders.
 * [MSG_USE 44 0A 00] */
static void mudd4_SpockScansDoover(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd4_Say(N_Mr_Spock, T_MUD4_051);
    return;
}

/* [MSG_USE 44 21 00] */
static void mudd4_SpockScansConsole(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd4_Say(N_Mr_Spock, T_MUD4_042);
    return;
}

/* [MSG_USE 44 20 00] */
static void mudd4_SpockScansViewscreen(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    mudd4_Say(N_Mr_Spock, T_MUD4_050);
    return;
}

/* Common to all of the mission's rooms: the de-grimer on the lenses makes
 * the alien device (+1); firing the device sets off an explosion at Kirk
 * (anim msg 9), +5 the first time anywhere; the de-grimer on anything else
 * just cleans it.
 * [MSG_USE 5F 5E 00] */
static void mudd4_CleanLenses(void)
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
L09A5:;
    ScriptService(stk[sp_], stk[sp_ + 1], stk[sp_ + 2], stk[sp_ + 3], stk[sp_ + 4], stk[sp_ + 5]);
    sp_ += 2;
    di = di - 1;
    if (di != 0)
        goto L09A5;
    rv_score += 1;
    mudd4_Say(S_1010, T_MUD4N009);
    return;
}

/* [MSG_USE 7F FF 00] */
static void mudd4_UseMedkit(void)
{
    mudd4_Say(N_Dr_McCoy, T_MUD4_043);
    return;
}

/* [MSG_USE 7D FF 00] */
static void mudd4_FireDevice(void)
{
    uint16_t ax = 0;
    room.busy = 1;
    room.fall[0] = 1;
    svc_StopAnim(0);
    ax = (uint16_t)svc_ActorPos(0);
    svc_Anim0(9, N_s4cbxp, ax, 0xA, 9);
    svc_Sfx(N_EXPLO3);
    return;
}

/* The alien device has gone off here (anim msg 9).
 * [MSG_ANIM_DONE 09 00 00] */
static void mudd4_DeviceFired(void)
{
    room.busy = 0;
    if (rv_used_device == 0) {
        rv_used_device = 1;
        rv_score += 5;
        mudd4_Say(N_Capt_Kirk, T_MUD2_002);
    }
    return;
}

/* The doover (msgs 5, 10): +1.
 * [MSG_GET 0A 00 00] */
static void mudd4_GetDoover(void)
{
    if (rv_took_doover == 0) {
        room.busy = 1;
        room.fall[0] = 0;
        svc_Walk(0, 0xCB, 0x9E, 5);
        return;
    }
    mudd4_Say(S_1010, T_MUD4N012);
    return;
}

/* [MSG_WALK_DONE 05 00 00] */
static void mudd4_KirkAtDoover(void)
{
    svc_Anim(0, N_kuseln, -1, -1, 0xA);
    return;
}

/* [MSG_ANIM_DONE 0A 00 00] */
static void mudd4_TookDoover(void)
{
    room.busy = 0;
    svc_StopAnim(0xA);
    rv_took_doover = 1;
    rv_score += 1;
    svc_GiveItem(IT_DOOVER);
    return;
}

/* [MSG_USE 5E FF 00] */
static void mudd4_UseDegrimer(void)
{
    mudd4_Say(S_1010, T_MUD4N002);
    return;
}

/* Every tick, common to all of the mission's rooms: while life support is
 * failing, the air runs down from 6978h, with warnings at 4F1Ah, 34BCh
 * and 1A5Eh; at 1 the crew collapse (walk msgs 9-12) and the game is
 * over.
 * [MSG_TICK FF FF FF] */
static void mudd4_AirRunsOut(void)
{
    if (rv_air_failing != 0) {
        if (rv_air_countdown != 1) {
            rv_air_countdown = 1;
            rv_air_left = 0x6978;
        }
        rv_air_left--;
        if (rv_air_left == 0x1A5E)
            goto L0BAF;
        if (rv_air_left == 0x34BC)
            goto L0BA5;
        if (rv_air_left == 0x4F1A)
            goto L0B9B;
        if (rv_air_left == 1) {
            room.busy = 1;
            svc_Walk(0, 0x8B, 0xB6, 9);
            svc_Walk(1, 0x69, 0xB7, 0xA);
            mudd4_Say(S_1010, T_MUD4N005);
            svc_GameOver();
        }
    }
    return;
L0B9B:;
    mudd4_Say(N_Mr_Spock, S_0BB9);
    return;
L0BA5:;
    mudd4_Say(N_Mr_Spock, S_0BDC);
    return;
L0BAF:;
    mudd4_Say(N_Mr_Spock, S_0BFF);
    return;
}

/* The air has run out: Kirk, McCoy and Buchert (walk msg 9), Spock (10).
 * [MSG_WALK_DONE 09 00 00] */
static void mudd4_CrewCollapse(void)
{
    svc_Anim(0, N_kgetde, -1, -1, 0);
    svc_Anim(2, N_mgetds, -1, -1, 0);
    svc_Anim(3, N_rgetdw, -1, -1, 0);
    { mudd4_part_0CCB(); return; }
}

/* [MSG_WALK_DONE 0A 00 00] */
static void mudd4_SpockCollapses(void)
{
    svc_Anim(1, N_sgetdn, -1, -1, 0);
    { mudd4_part_0CCB(); return; }
}

static void mudd4_part_0CCB(void)
{
    fv_collapsed++;
    if (fv_collapsed == 2) {
        mudd4_Say(S_1010, T_MUD4N007);
    }
    return;
}

/* Looking.
 * [MSG_LOOK 21 00 00] */
static void mudd4_LookAtConsole(void)
{
    mudd4_Say(0, T_MUD4N000);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void mudd4_LookAtViewscreen(void)
{
    if (rv_viewscreen_on == 0) {
        mudd4_Say(S_1010, T_MUD4N013);
        return;
    }
    mudd4_Say(S_1010, T_MUD4N008);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void mudd4_LookAtKirk(void)
{
    mudd4_Say(0, T_MUD4N003);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void mudd4_LookAtSpock(void)
{
    mudd4_Say(0, T_MUD4N006);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void mudd4_LookAtMcCoy(void)
{
    mudd4_Say(0, T_MUD4N001);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void mudd4_LookAtRedshirt(void)
{
    mudd4_Say(0, T_MUD4N004);
    return;
}

/* [MSG_LOOK 0A 00 00] */
static void mudd4_LookAtDoover(void)
{
    mudd4_Say(0, T_MUD4N011);
    return;
}

/* [MSG_TALK 00 00 00] */
static void mudd4_TalkToKirk(void)
{
    mudd4_Say(N_Capt_Kirk, T_MUD4_005);
    return;
}

/* [MSG_TALK 01 00 00] */
static void mudd4_TalkToSpock(void)
{
    mudd4_Say(N_Mr_Spock, T_MUD4_063);
    mudd4_Say(N_Capt_Kirk, T_MUD4_015);
    mudd4_Say(N_Dr_McCoy, T_MUD4_047);
    return;
}

/* [MSG_TALK 02 00 00] */
static void mudd4_TalkToMcCoy(void)
{
    mudd4_Say(N_Dr_McCoy, T_MUD4_046);
    return;
}

/* [MSG_TALK 03 00 00] */
static void mudd4_TalkToRedshirt(void)
{
    mudd4_Say(N_Lt_Buchert, T_MUD4_076);
    return;
}

/* The doors.
 * [MSG_WALK 24 00 00] */
static void mudd4_WalkToRightDoor(void)
{
    fv_going_door = 1;
    room.busy = 1;
    svc_Walk(0, 0xF0, 0xC7, 0);
    return;
}

/* [MSG_WALK 25 00 00] */
static void mudd4_WalkToLeftDoor(void)
{
    fv_going_door = 1;
    room.busy = 1;
    svc_Walk(0, 0x50, 0xC7, 0);
    return;
}

/* [MSG_REGION 00 00 00] */
static void mudd4_ReachDoor(void)
{
    if (fv_going_door == 1) {
        svc_Sfx(N_SMADOOR3);
    }
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t mudd4_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void mudd4_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x1013) = bx;
    mudd4_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript mudd4_scripts[] = {
    { 0x0159, mudd4_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x01B3, mudd4_CallEnterprise },    /* MSG_USE 47 FF 00 */
    { 0x0221, mudd4_Timer2Sound },    /* MSG_TIMER 02 00 00 */
    { 0x0234, mudd4_Timer3Sound },    /* MSG_TIMER 03 00 00 */
    { 0x0247, mudd4_E_0247 },    /* MSG_USE 01 21 00 */
    { 0x024F, mudd4_SpockToPrimary },    /* MSG_USE 01 22 00 */
    { 0x025D, mudd4_SpockToSecondary },    /* MSG_USE 01 23 00 */
    { 0x030C, mudd4_SpockAtStation },    /* MSG_WALK_DONE 01 00 00 */
    { 0x033B, mudd4_SecondaryStation },    /* MSG_ANIM_DONE 01 00 00 */
    { 0x03D5, mudd4_Sensors },    /* MSG_ANIM_DONE 02 00 00 */
    { 0x03EB, mudd4_Engineering },    /* MSG_ANIM_DONE 03 00 00 */
    { 0x0401, mudd4_Navigation },    /* MSG_ANIM_DONE 04 00 00 */
    { 0x0417, mudd4_SpockArrives },    /* MSG_WALK_DONE 03 00 00 */
    { 0x0424, mudd4_E_0424 },    /* MSG_USE 00 21 00 */
    { 0x042C, mudd4_KirkToSecondary },    /* MSG_USE 00 23 00 */
    { 0x043A, mudd4_KirkToPrimary },    /* MSG_USE 00 22 00 */
    { 0x04AE, mudd4_KirkAtStation },    /* MSG_WALK_DONE 02 00 00 */
    { 0x04DD, mudd4_PrimaryStation },    /* MSG_ANIM_DONE 05 00 00 */
    { 0x0577, mudd4_ViewscreenToggled },    /* MSG_ANIM_DONE 06 00 00 */
    { 0x05F5, mudd4_ViewscreenOn },    /* MSG_TIMER 01 00 00 */
    { 0x0611, mudd4_Communications },    /* MSG_ANIM_DONE 07 00 00 */
    { 0x0735, mudd4_MuddFarewell },    /* MSG_WALK_DONE 04 00 00 */
    { 0x0884, mudd4_E_0884 },    /* MSG_USE 02 21 00 */
    { 0x088C, mudd4_E_088C },    /* MSG_USE 02 22 00 */
    { 0x0894, mudd4_McCoyAtStation },    /* MSG_USE 02 23 00 */
    { 0x08BB, mudd4_E_08BB },    /* MSG_USE 03 21 00 */
    { 0x08C3, mudd4_E_08C3 },    /* MSG_USE 03 22 00 */
    { 0x08CB, mudd4_BuchertAtStation },    /* MSG_USE 03 23 00 */
    { 0x0903, mudd4_SpockScansDoover },    /* MSG_USE 44 0A 00 */
    { 0x0934, mudd4_SpockScansConsole },    /* MSG_USE 44 21 00 */
    { 0x0965, mudd4_SpockScansViewscreen },    /* MSG_USE 44 20 00 */
    { 0x0996, mudd4_CleanLenses },    /* MSG_USE 5F 5E 00 */
    { 0x09C4, mudd4_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0x0A27, mudd4_FireDevice },    /* MSG_USE 7D FF 00 */
    { 0x0A7A, mudd4_DeviceFired },    /* MSG_ANIM_DONE 09 00 00 */
    { 0x0A9C, mudd4_GetDoover },    /* MSG_GET 0A 00 00 */
    { 0x0ACF, mudd4_KirkAtDoover },    /* MSG_WALK_DONE 05 00 00 */
    { 0x0AEB, mudd4_TookDoover },    /* MSG_ANIM_DONE 0A 00 00 */
    { 0x0B16, mudd4_UseDegrimer },    /* MSG_USE 5E FF 00 */
    { 0x0B26, mudd4_AirRunsOut },    /* MSG_TICK FF FF FF */
    { 0x0C6F, mudd4_CrewCollapse },    /* MSG_WALK_DONE 09 00 00 */
    { 0x0CB6, mudd4_SpockCollapses },    /* MSG_WALK_DONE 0A 00 00 */
    { 0x0D72, mudd4_LookAtConsole },    /* MSG_LOOK 21 00 00 */
    { 0x0D81, mudd4_LookAtViewscreen },    /* MSG_LOOK 20 00 00 */
    { 0x0DA1, mudd4_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0DB0, mudd4_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0DBF, mudd4_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0DCE, mudd4_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0DDD, mudd4_LookAtDoover },    /* MSG_LOOK 0A 00 00 */
    { 0x0DEC, mudd4_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x0DFC, mudd4_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0E1E, mudd4_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0E2E, mudd4_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0E3E, mudd4_WalkToRightDoor },    /* MSG_WALK 24 00 00 */
    { 0x0E63, mudd4_WalkToLeftDoor },    /* MSG_WALK 25 00 00 */
    { 0x0E87, mudd4_ReachDoor },    /* MSG_REGION 00 00 00 */
    { 0, 0 }
};
