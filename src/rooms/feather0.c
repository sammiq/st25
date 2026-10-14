/* FEATHER0.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/feather0.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * FEATHER0.RDF - "The Feathered Serpent", a jungle clearing on Digifal.
 * Quetzecoatl, who raised the Aztecs and has now come to the Klingon world
 * of Digifal, greets the landing party. He refuses to believe what became
 * of his people on Earth, and whatever Kirk says, he sends the party into
 * the jungle (room 1).
 *
 * Actors: 8 Quetzecoatl.
 * Objects: 8 Quetzecoatl, 20h the moon, 21h the log, 22h the hut, 23h-2Bh
 * eyes in the dark, 2Ch insects.
 * Timers: 0 the party is sent away, 1 room 1.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 0574: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_s5r0qb = 0x00CA,
    N_s5r0qt = 0x00D1,
    N_ktele = 0x00D8,
    N_stele = 0x00DE,
    N_mtele = 0x00E4,
    N_rtele = 0x00EA,
    N_sscans = 0x00F0,
    N_mscans = 0x00F7,
    N_FEA0LOOP = 0x0106,
    N_SE2BIGEN = 0x025E,
    T_FEA0_108 = 0x033E,
    N_Quetzecoatl = 0x06FA,
    N_Dr_McCoy = 0x0706,
    N_Mr_Spock = 0x0710,
    N_Lt_Stragey = 0x071A,
    T_FEA0N000 = 0x0726,
    T_FEA0N003 = 0x0762,
    T_FEA0N001 = 0x079E,
    T_FEA0N006 = 0x07E5,
    T_FEA0N008 = 0x081C,
    T_FEA0N007 = 0x086E,
    T_FEA0N004 = 0x094F,
    T_FEA0N005 = 0x0979,
    T_FEA0N002 = 0x09AD,
    T_FEA0N009 = 0x09EA,
    T_FEA0_017 = 0x0A25,
    T_FEA0_021 = 0x0A68,
    T_FEA0_022 = 0x0AE9,
    T_FEA0_018 = 0x0B85,
    T_FEA0_023 = 0x0BDA,
    T_FEA0_020 = 0x0C0B,
    T_FEA0_015 = 0x0C83,
    T_FEA0_025 = 0x0D0F,
    T_FEA0_016 = 0x0D36,
    T_FEA0_019 = 0x17F7,
    N_dialog_error = 0x184E,
    talk_buf = 0x185B,
    QGreets = 0x1861,
    KirkIntroduces = 0x1867,
    QOnKlingons = 0x1871,
    QOnAnger = 0x1877,
    QWhoAreYou = 0x187D,
    KirkAccuses = 0x1883,
    QOnPeace = 0x188B,
    KirkOnName = 0x1891,
    QDisbelieves = 0x1897,
    QOnBloodshed = 0x189D,
    KirkOnSacrifice = 0x18A3,
    QImpossible = 0x18AB,
    QBegone = 0x18B1,
    KirkOnConquest = 0x18B7,
    KirkOnPlans = 0x18BD,
    KirkOnPeople = 0x18C5,
    QIntroduces = 0x18CD,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0x185B)

/* functions */
static void feather0_StartRoom(void);
static void feather0_MusicStarts(void);
static void feather0_TalkToQuetzecoatl(void);
static void feather0_Teleported(void);
static void feather0_ToTheJungle(void);
static void feather0_StunQuetzecoatl(void);
static void feather0_KillQuetzecoatl(void);
static void feather0_StunMcCoy(void);
static void feather0_KillMcCoy(void);
static void feather0_UseMedkit(void);
static void feather0_LookAtQuetzecoatl(void);
static void feather0_LookAtMoon(void);
static void feather0_LookAtLog(void);
static void feather0_LookAtHut(void);
static void feather0_LookAtScene(void);
static void feather0_LookAtEyes1(void);
static void feather0_LookAtEyes2(void);
static void feather0_LookAtEyes3(void);
static void feather0_LookAtEyes4(void);
static void feather0_LookAtEyes5(void);
static void feather0_LookAtEyes6(void);
static void feather0_LookAtEyes7(void);
static void feather0_LookAtEyes8(void);
static void feather0_LookAtEyes9(void);
static void feather0_LookAtInsects(void);
static void feather0_LookAtMcCoy(void);
static void feather0_LookAtSpock(void);
static void feather0_LookAtRedshirt(void);
static void feather0_TalkToMcCoy(void);
static void feather0_TalkToSpock(void);
static void feather0_TalkToRedshirt(void);
static void feather0_SpockScansQuetzecoatl(void);
static void feather0_SpockScans(void);
static void feather0_McCoyScansQuetzecoatl(void);
static uint16_t feather0_Talk(uint16_t ax);
static void feather0_Say(uint16_t ax, uint16_t bx);

/* The room starts: Quetzecoatl; tick 60, music.
 * [MSG_TICK 01 00 00] */
static void feather0_StartRoom(void)
{
    svc_Sfx(N_FEA0LOOP);
    svc_Anim0(8, N_s5r0qb, 0xBE, 0xA6, 0);
    return;
}

/* [MSG_TICK 3C 00 00] */
static void feather0_MusicStarts(void)
{
    svc_Music(0x21, STALE_ARG);
    return;
}

/* Talking to Quetzecoatl: a conversation tree; whatever Kirk says, it
 * ends with Quetzecoatl sending the party away (timer 0).
 * [MSG_TALK 08 00 00] */
static void feather0_TalkToQuetzecoatl(void)
{
    uint16_t ax = 0;
    feather0_Talk(QGreets);
    ax = (uint16_t)feather0_Talk(KirkIntroduces);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L017B;
        }
        if (ax == 2) {
            goto L0183;
        }
        goto L02E4;
    }
    feather0_Talk(QOnKlingons);
    goto L018B;
L017B:;
    feather0_Talk(QOnAnger);
    goto L01F1;
L0183:;
    feather0_Talk(QWhoAreYou);
    goto L0208;
L018B:;
    ax = (uint16_t)feather0_Talk(KirkAccuses);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L01B2;
        }
        goto L02E4;
    }
L01A2:;
    feather0_Talk(QOnPeace);
L01AA:;
    feather0_Talk(KirkOnName);
    goto L01BA;
L01B2:;
    feather0_Talk(QDisbelieves);
    goto L01AA;
L01BA:;
    feather0_Talk(QOnBloodshed);
    ax = (uint16_t)feather0_Talk(KirkOnSacrifice);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L01E1;
        }
        goto L02E4;
    }
    feather0_Talk(QImpossible);
    goto L01E9;
L01E1:;
    feather0_Talk(QBegone);
    goto L0227;
L01E9:;
    feather0_Talk(KirkOnConquest);
    goto L01E1;
L01F1:;
    ax = (uint16_t)feather0_Talk(KirkOnPlans);
    if (ax == 0) {
        goto L01A2;
    }
    if (ax == 1) {
        goto L01B2;
    }
    goto L02E4;
L0208:;
    ax = (uint16_t)feather0_Talk(KirkOnPeople);
    if (ax == 0) {
    } else {
        if (ax == 1) {
            goto L01B2;
        }
        goto L02E4;
    }
    feather0_Talk(QIntroduces);
L0227:;
    room.busy = 1;
    svc_Anim(8, N_s5r0qt, -1, -1, 0);
    svc_Music(1, STALE_ARG);
    room.timers[0] = 0x32;
    room.timers[1] = 0x40;
    return;
L02E4:;
    feather0_Say(0, N_dialog_error);
    return;
}

/* Timer 0: the crew vanish, and timer 1 puts them in room 1.
 * [MSG_TIMER 00 00 00] */
static void feather0_Teleported(void)
{
    svc_Sfx(N_SE2BIGEN);
    svc_Anim(0, N_ktele, -1, -1, 0);
    svc_Anim(1, N_stele, -1, -1, 0);
    svc_Anim(2, N_mtele, -1, -1, 0);
    svc_Anim(3, N_rtele, -1, -1, 0);
    return;
}

/* [MSG_TIMER 01 00 00] */
static void feather0_ToTheJungle(void)
{
    room.busy = 0;
    svc_GotoRoom(1, 5);
    return;
}

/* Phasers.
 * [MSG_USE 40 08 00] */
static void feather0_StunQuetzecoatl(void)
{
    feather0_Say(N_Quetzecoatl, T_FEA0_025);
    return;
}

/* [MSG_USE 41 08 00] */
static void feather0_KillQuetzecoatl(void)
{
    feather0_Say(N_Quetzecoatl, T_FEA0_025);
    return;
}

/* [MSG_USE 40 02 00] */
static void feather0_StunMcCoy(void)
{
    feather0_Say(N_Dr_McCoy, T_FEA0_016);
    return;
}

/* [MSG_USE 41 02 00] */
static void feather0_KillMcCoy(void)
{
    feather0_Say(N_Dr_McCoy, T_FEA0_016);
    return;
}

/* [MSG_USE 7F FF 00] */
static void feather0_UseMedkit(void)
{
    feather0_Say(N_Dr_McCoy, T_FEA0_108);
    return;
}

/* Looking and talking.
 * [MSG_LOOK 08 00 00] */
static void feather0_LookAtQuetzecoatl(void)
{
    feather0_Say(0, T_FEA0N008);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void feather0_LookAtMoon(void)
{
    feather0_Say(0, T_FEA0N007);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void feather0_LookAtLog(void)
{
    feather0_Say(0, T_FEA0N006);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void feather0_LookAtHut(void)
{
    feather0_Say(0, T_FEA0N001);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void feather0_LookAtScene(void)
{
    feather0_Say(0, T_FEA0N009);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void feather0_LookAtEyes1(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void feather0_LookAtEyes2(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void feather0_LookAtEyes3(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 26 00 00] */
static void feather0_LookAtEyes4(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 27 00 00] */
static void feather0_LookAtEyes5(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 28 00 00] */
static void feather0_LookAtEyes6(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 29 00 00] */
static void feather0_LookAtEyes7(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 2A 00 00] */
static void feather0_LookAtEyes8(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 2B 00 00] */
static void feather0_LookAtEyes9(void)
{
    feather0_Say(0, T_FEA0N000);
    return;
}

/* [MSG_LOOK 2C 00 00] */
static void feather0_LookAtInsects(void)
{
    feather0_Say(0, T_FEA0N003);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void feather0_LookAtMcCoy(void)
{
    feather0_Say(0, T_FEA0N004);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void feather0_LookAtSpock(void)
{
    feather0_Say(0, T_FEA0N005);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void feather0_LookAtRedshirt(void)
{
    feather0_Say(0, T_FEA0N002);
    return;
}

/* [MSG_TALK 02 00 00] */
static void feather0_TalkToMcCoy(void)
{
    feather0_Say(N_Dr_McCoy, T_FEA0_017);
    feather0_Say(N_Mr_Spock, T_FEA0_021);
    return;
}

/* [MSG_TALK 01 00 00] */
static void feather0_TalkToSpock(void)
{
    feather0_Say(N_Mr_Spock, T_FEA0_022);
    feather0_Say(N_Dr_McCoy, T_FEA0_018);
    return;
}

/* [MSG_TALK 03 00 00] */
static void feather0_TalkToRedshirt(void)
{
    feather0_Say(N_Lt_Stragey, T_FEA0_023);
    return;
}

/* Tricorders.
 * [MSG_USE 44 08 00] */
static void feather0_SpockScansQuetzecoatl(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    feather0_Say(N_Mr_Spock, T_FEA0_020);
    return;
}

/* [MSG_USE 44 FF 00] */
static void feather0_SpockScans(void)
{
    svc_Anim(1, N_sscans, -1, -1, 0);
    svc_BridgeSfx(4);
    feather0_Say(N_Mr_Spock, T_FEA0_019);
    return;
}

/* [MSG_USE 45 08 00] */
static void feather0_McCoyScansQuetzecoatl(void)
{
    svc_Anim(2, N_mscans, -1, -1, 0);
    svc_BridgeSfx(4);
    feather0_Say(N_Dr_McCoy, T_FEA0_015);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static uint16_t feather0_Talk(uint16_t ax)
{
    ax = (uint16_t)svc_Talk(ax);
    return ax;
}

/* Say: one line, BX, said by AX (0: narration). */
static void feather0_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x185D) = bx;
    feather0_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript feather0_scripts[] = {
    { 0x0104, feather0_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x013C, feather0_MusicStarts },    /* MSG_TICK 3C 00 00 */
    { 0x014F, feather0_TalkToQuetzecoatl },    /* MSG_TALK 08 00 00 */
    { 0x025C, feather0_Teleported },    /* MSG_TIMER 00 00 00 */
    { 0x02D1, feather0_ToTheJungle },    /* MSG_TIMER 01 00 00 */
    { 0x02F4, feather0_StunQuetzecoatl },    /* MSG_USE 40 08 00 */
    { 0x0304, feather0_KillQuetzecoatl },    /* MSG_USE 41 08 00 */
    { 0x0314, feather0_StunMcCoy },    /* MSG_USE 40 02 00 */
    { 0x0324, feather0_KillMcCoy },    /* MSG_USE 41 02 00 */
    { 0x0334, feather0_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0x0397, feather0_LookAtQuetzecoatl },    /* MSG_LOOK 08 00 00 */
    { 0x03A6, feather0_LookAtMoon },    /* MSG_LOOK 20 00 00 */
    { 0x03B5, feather0_LookAtLog },    /* MSG_LOOK 21 00 00 */
    { 0x03C4, feather0_LookAtHut },    /* MSG_LOOK 22 00 00 */
    { 0x03D3, feather0_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x03E2, feather0_LookAtEyes1 },    /* MSG_LOOK 23 00 00 */
    { 0x03F1, feather0_LookAtEyes2 },    /* MSG_LOOK 24 00 00 */
    { 0x0400, feather0_LookAtEyes3 },    /* MSG_LOOK 25 00 00 */
    { 0x040F, feather0_LookAtEyes4 },    /* MSG_LOOK 26 00 00 */
    { 0x041E, feather0_LookAtEyes5 },    /* MSG_LOOK 27 00 00 */
    { 0x042D, feather0_LookAtEyes6 },    /* MSG_LOOK 28 00 00 */
    { 0x043C, feather0_LookAtEyes7 },    /* MSG_LOOK 29 00 00 */
    { 0x044B, feather0_LookAtEyes8 },    /* MSG_LOOK 2A 00 00 */
    { 0x045A, feather0_LookAtEyes9 },    /* MSG_LOOK 2B 00 00 */
    { 0x0469, feather0_LookAtInsects },    /* MSG_LOOK 2C 00 00 */
    { 0x0478, feather0_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0487, feather0_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0496, feather0_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x04A5, feather0_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x04BE, feather0_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x04D7, feather0_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x04E7, feather0_SpockScansQuetzecoatl },    /* MSG_USE 44 08 00 */
    { 0x0518, feather0_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x0549, feather0_McCoyScansQuetzecoatl },    /* MSG_USE 45 08 00 */
    { 0, 0 }
};
