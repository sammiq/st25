/* TRIAL4.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/trial4.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * TRIAL4.RDF - "The Feathered Serpent", back in the court after the Test
 * of Life (from rooms 1-3). Vlict, furious, admits the trial was a sham;
 * Quetzecoatl gives himself up to spare the party, and the mission ends
 * with ending 2 (Quetzecoatl lost).
 *
 * Actors: 8 Vlict, 9 the guard, 10 (0Ah) Quetzecoatl.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The conversation at 00CA is built at run time.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    talk_buf = 0x00CA,
    N_Capt_Kirk = 0x0113,
    N_Dr_McCoy = 0x011E,
    N_Mr_Spock = 0x0128,
    N_Vlict = 0x014A,
    N_Quetzecoatl = 0x0150,
    N_TRI4LOOP = 0x0178,
    N_vlict1 = 0x0197,
    N_kgstnd = 0x01B6,
    N_qstand = 0x01D5,
    T_TRI4_061 = 0x021A,
    T_TRI4_022 = 0x02AB,
    T_TRI4_039 = 0x0306,
    T_TRI4_032 = 0x039A,
    T_TRI4_064 = 0x045D,
    T_TRI4_033 = 0x04BB,
    T_TRI4_009 = 0x04FB,
    T_TRI4_002 = 0x0549,
    T_TRI4_062 = 0x05B2,
    T_TRI4_014 = 0x0624,
    T_TRI4_066 = 0x066F,
    T_TRI4_043 = 0x0737,
    T_TRI4_029 = 0x07C8,
    T_TRI4_020 = 0x082E,
    T_TRI4_030 = 0x08A9,
    T_TRI4_063 = 0x0969,
    T_TRI4_040 = 0x09BF,
    T_TRI4_060 = 0x0A95,
    T_TRI4_013 = 0x0B67,
    T_TRI4_072 = 0x0BE4,
    T_TRI4_015 = 0x0C3B,
    T_TRI4_071 = 0x0CC5,
    T_TRI4_005 = 0x0D1E,
    T_TRI4_052 = 0x0D54,
    T_TRI4_041 = 0x0E19,
    T_TRI4_051 = 0x0E87,
    T_TRI4_053 = 0x0F1B,
    T_TRI4_050 = 0x0FED,
    T_TRI4_070 = 0x1099,
    T_TRI4_019 = 0x1120,
    T_TRI4_025 = 0x11C6,
    T_TRI4_008 = 0x1246,
    T_TRI4_069 = 0x1299,
    T_TRI4_004 = 0x12F9,
    T_TRI4_058 = 0x1337,
    T_TRI4_027 = 0x1382,
    T_TRI4_034 = 0x13D5,
    T_TRI4_018 = 0x143A,
    T_TRI4_075 = 0x14B5,
    T_TRI4_017 = 0x1510,
    T_TRI4_042 = 0x1531,
    T_TRI4_054 = 0x15CB,
    T_TRI4_028 = 0x160D,
    T_TRI4_021 = 0x163B,
    T_TRI4_031 = 0x169E,
    T_TRI4_068 = 0x172F,
    T_TRI4_036 = 0x1777,
    T_TRI4_056 = 0x17B9,
    T_TRI4_001 = 0x183F,
    T_TRI4_076 = 0x18AB,
    T_TRI4_037 = 0x190C,
    T_TRI4_073 = 0x195F,
    T_TRI4_011 = 0x19D7,
    T_TRI4_059 = 0x1A86,
    T_TRI4_065 = 0x1ACB,
    T_TRI4_016 = 0x1B60,
    T_TRI4_074 = 0x1BC5,
    T_TRI4_026 = 0x1C23,
    T_TRI4_024 = 0x1C98,
    T_TRI4_023 = 0x1CF7,
    T_TRI4_057 = 0x1D85,
    T_TRI4_038 = 0x1E0C,
    T_TRI4_044 = 0x1E3E,
    T_TRI4_006 = 0x1E9C,
    T_TRI4_048 = 0x1ED5,
    T_TRI4_055 = 0x1F07,
    T_TRI4_045 = 0x1F8D,
    T_TRI4_003 = 0x1FEB,
    T_TRI4_047 = 0x2028,
    T_TRI4_010 = 0x206C,
    T_TRI4_067 = 0x209B,
    T_TRI4_046 = 0x20F8,
    T_TRI4_007 = 0x2156,
    T_TRI4_049 = 0x218F,
    T_TRI4_012 = 0x21BF,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_choice_jumps CS_W(0xDE)
#define fv_talk_buf CS_W(0xCA)
#define rv_ending ROOM_W(0x5F)
#define rv_max_score ROOM_W(0x2B)
#define rv_score ROOM_W(0x29)

/* functions */
static void trial4_Talk(uint16_t ax);
static void trial4_Say(uint16_t ax, uint16_t bx);
static void trial4_StartRoom(void);
static void trial4_VlictRages(void);

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void trial4_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void trial4_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0xCC) = bx;
    CS_W(0xCE) = 0;
    trial4_Talk(talk_buf);
    return;
}

/* The room starts: Vlict, the guard, Quetzecoatl.
 * [MSG_TICK 01 00 00] */
static void trial4_StartRoom(void)
{
    svc_Sfx(N_TRI4LOOP);
    svc_Anim(8, N_vlict1, 0x9F, 0x48, 0);
    svc_Anim(9, N_kgstnd, 0xDC, 0x6A, 0);
    svc_Anim(0xA, N_qstand, 0x10E, 0xAA, 0);
    svc_Music(0x20, -1);
    return;
}

/* Tick 60: the confrontation, a series of conversations built at run
 * time; it ends with Quetzecoatl's sacrifice and BeamUp (ending 2).
 * [MSG_TICK 3C 00 00] */
static void trial4_VlictRages(void)
{
    uint16_t ax = 0, si = 0;
    trial4_Say(N_Vlict, T_TRI4_061);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_022;
    fv_choice_jumps = 0x44D;
    CS_W(0xCE) = T_TRI4_039;
    CS_W(0xE0) = 0xA84;
    CS_W(0xD0) = T_TRI4_032;
    CS_W(0xE2) = 0x1327;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L044D:;
    trial4_Say(N_Vlict, T_TRI4_064);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_033;
    fv_choice_jumps = 0x5A2;
    CS_W(0xCE) = T_TRI4_009;
    CS_W(0xE0) = 0x5A2;
    CS_W(0xD0) = T_TRI4_002;
    CS_W(0xE2) = 0x5A2;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L05A2:;
    trial4_Say(N_Vlict, T_TRI4_062);
    trial4_Say(N_Capt_Kirk, T_TRI4_014);
    trial4_Say(N_Vlict, T_TRI4_066);
    trial4_Say(N_Quetzecoatl, T_TRI4_043);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_029;
    fv_choice_jumps = 0x21AF;
    CS_W(0xCE) = T_TRI4_020;
    CS_W(0xE0) = 0x959;
    CS_W(0xD0) = T_TRI4_030;
    CS_W(0xE2) = 0x9AE;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L0959:;
    trial4_Say(N_Vlict, T_TRI4_063);
    goto L21AF;
L09AE:;
    trial4_Say(N_Quetzecoatl, T_TRI4_040);
    goto L21AF;
L0A84:;
    trial4_Say(N_Vlict, T_TRI4_060);
    trial4_Say(N_Capt_Kirk, T_TRI4_013);
    trial4_Say(N_Vlict, T_TRI4_072);
    trial4_Say(N_Capt_Kirk, T_TRI4_015);
    trial4_Say(N_Vlict, T_TRI4_071);
    trial4_Say(N_Capt_Kirk, T_TRI4_005);
    trial4_Say(N_Quetzecoatl, T_TRI4_052);
    trial4_Say(N_Quetzecoatl, T_TRI4_041);
    trial4_Say(N_Quetzecoatl, T_TRI4_051);
    trial4_Say(N_Quetzecoatl, T_TRI4_053);
    trial4_Say(N_Quetzecoatl, T_TRI4_050);
    trial4_Say(N_Vlict, T_TRI4_070);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_019;
    fv_choice_jumps = 0x21AF;
    CS_W(0xCE) = T_TRI4_025;
    CS_W(0xE0) = 0x1289;
    CS_W(0xD0) = T_TRI4_008;
    CS_W(0xE2) = 0x21AF;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L1289:;
    trial4_Say(N_Vlict, T_TRI4_069);
    trial4_Say(N_Capt_Kirk, T_TRI4_004);
    goto L21AF;
L1327:;
    trial4_Say(N_Vlict, T_TRI4_058);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_027;
    fv_choice_jumps = 0x14A5;
    CS_W(0xCE) = T_TRI4_034;
    CS_W(0xE0) = 0x189B;
    CS_W(0xD0) = T_TRI4_018;
    CS_W(0xE2) = 0x1ABA;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L14A5:;
    trial4_Say(N_Vlict, T_TRI4_075);
    trial4_Say(N_Capt_Kirk, T_TRI4_017);
    trial4_Say(N_Quetzecoatl, T_TRI4_042);
    trial4_Say(N_Vlict, T_TRI4_054);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_028;
    fv_choice_jumps = 0x171F;
    CS_W(0xCE) = T_TRI4_021;
    CS_W(0xE0) = 0x171F;
    CS_W(0xD0) = T_TRI4_031;
    CS_W(0xE2) = 0x17A9;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L171F:;
    trial4_Say(N_Vlict, T_TRI4_068);
    trial4_Say(N_Dr_McCoy, T_TRI4_036);
    goto L21AF;
L17A9:;
    trial4_Say(N_Vlict, T_TRI4_056);
    trial4_Say(N_Capt_Kirk, T_TRI4_001);
    goto L21AF;
L189B:;
    trial4_Say(N_Vlict, T_TRI4_076);
    trial4_Say(N_Dr_McCoy, T_TRI4_037);
    trial4_Say(N_Vlict, T_TRI4_073);
    trial4_Say(N_Capt_Kirk, T_TRI4_011);
    trial4_Say(N_Vlict, T_TRI4_059);
    goto L21AF;
L1ABA:;
    trial4_Say(N_Vlict, T_TRI4_065);
    trial4_Say(N_Capt_Kirk, T_TRI4_016);
    trial4_Say(N_Vlict, T_TRI4_074);
    fv_talk_buf = N_Capt_Kirk;
    CS_W(0xCC) = T_TRI4_026;
    fv_choice_jumps = 0x1D75;
    CS_W(0xCE) = T_TRI4_024;
    CS_W(0xE0) = 0x1EF7;
    CS_W(0xD0) = T_TRI4_023;
    CS_W(0xE2) = 0x208B;
    CS_W(0xD2) = 0;
    ax = (uint16_t)svc_Talk(talk_buf);
    ax = ax << 1;
    si = ax;
    switch (CS_W(0xDE + si)) {
    case 0x044D: goto L044D;
    case 0x05A2: goto L05A2;
    case 0x0959: goto L0959;
    case 0x09AE: goto L09AE;
    case 0x0A84: goto L0A84;
    case 0x1289: goto L1289;
    case 0x1327: goto L1327;
    case 0x14A5: goto L14A5;
    case 0x171F: goto L171F;
    case 0x17A9: goto L17A9;
    case 0x189B: goto L189B;
    case 0x1ABA: goto L1ABA;
    case 0x1D75: goto L1D75;
    case 0x1EF7: goto L1EF7;
    case 0x208B: goto L208B;
    case 0x21AF: goto L21AF;
    default: return;
    }
L1D75:;
    trial4_Say(N_Vlict, T_TRI4_057);
    trial4_Say(N_Mr_Spock, T_TRI4_038);
    trial4_Say(N_Quetzecoatl, T_TRI4_044);
    trial4_Say(N_Capt_Kirk, T_TRI4_006);
    trial4_Say(N_Quetzecoatl, T_TRI4_048);
    goto L21AF;
L1EF7:;
    trial4_Say(N_Vlict, T_TRI4_055);
    trial4_Say(N_Quetzecoatl, T_TRI4_045);
    trial4_Say(N_Capt_Kirk, T_TRI4_003);
    trial4_Say(N_Quetzecoatl, T_TRI4_047);
    trial4_Say(N_Capt_Kirk, T_TRI4_010);
    goto L21AF;
L208B:;
    trial4_Say(N_Vlict, T_TRI4_067);
    trial4_Say(N_Quetzecoatl, T_TRI4_046);
    trial4_Say(N_Capt_Kirk, T_TRI4_007);
    trial4_Say(N_Quetzecoatl, T_TRI4_049);
L21AF:;
    trial4_Say(N_Capt_Kirk, T_TRI4_012);
    rv_ending = 2;
    svc_BeamUp(rv_score, rv_max_score, 2);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript trial4_scripts[] = {
    { 0x0176, trial4_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x020E, trial4_VlictRages },    /* MSG_TICK 3C 00 00 */
    { 0, 0 }
};
