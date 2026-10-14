/* FEATHER4.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/feather4.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * FEATHER4.RDF - "The Feathered Serpent", a dark forest path past
 * Tlaoxac (room 3), towards the river. Nothing to do but look around:
 * toxic mushrooms, a razor-edged fern, glowing insects.
 *
 * Objects: 20h eyes, 21h the mushrooms, 22h the fern, 23h-25h the lights.
 * Exits: 2 room 3, 3 room 5.
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 *
 * Quirks of the original:
 * - The script chain runs on into code at 03E4: the engine reads its bytes
 * as key 50 6A 03 2E and next 1EFF (beyond rdf_scripts_end, so the search
 * stops).
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_sscanw = 0x00CA,
    N_mscanw = 0x00D1,
    N_FEA4LOOP = 0x00E0,
    T_FEA4_002 = 0x02BC,
    N_Dr_McCoy = 0x04CC,
    N_Mr_Spock = 0x04D6,
    N_Lt_Stragey = 0x04E0,
    T_FEA4N001 = 0x04ED,
    T_FEA4N000 = 0x052C,
    T_FEA4N002 = 0x055A,
    T_FEA4N003 = 0x0592,
    T_FEA4N008 = 0x05CE,
    T_FEA4N008_2 = 0x0628,
    T_FEA4_011 = 0x0647,
    T_FEA4_008 = 0x0683,
    T_FEA4_006 = 0x06C1,
    T_FEA4_004 = 0x06F3,
    T_FEA4_007 = 0x0799,
    T_FEA4N004 = 0x07FD,
    T_FEA4N007 = 0x084B,
    T_FEA4N006 = 0x087F,
    T_FEA4N005 = 0x08BB,
    T_FEA4_013 = 0x08FA,
    T_FEA4_010 = 0x099A,
    T_FEA4_001 = 0x09DD,
    T_FEA4_012 = 0x0A07,
    T_FEA4_003 = 0x0A72,
    T_FEA4_009 = 0x0AD0,
    talk_buf = 0x0B57,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0xB57)

/* functions */
static void feather4_StartRoom(void);
static void feather4_UseCommunicator(void);
static void feather4_StunAnything(void);
static void feather4_KillAnything(void);
static void feather4_E_013B(void);
static void feather4_SpockScansLights2(void);
static void feather4_SpockScansLights(void);
static void feather4_SpockScansMushrooms(void);
static void feather4_SpockScansFern(void);
static void feather4_SpockScans(void);
static void feather4_E_020F(void);
static void feather4_McCoyScansLights2(void);
static void feather4_McCoyScansLights(void);
static void feather4_McCoyScansMushrooms(void);
static void feather4_McCoyScans(void);
static void feather4_UseMedkit(void);
static void feather4_TalkToMcCoy(void);
static void feather4_TalkToSpock(void);
static void feather4_TalkToRedshirt(void);
static void feather4_LookAtEyes(void);
static void feather4_LookAtScene(void);
static void feather4_LookAtMushrooms(void);
static void feather4_LookAtFern(void);
static void feather4_LookAtLight1(void);
static void feather4_LookAtLight2(void);
static void feather4_LookAtLight3(void);
static void feather4_LookAtKirk(void);
static void feather4_LookAtSpock(void);
static void feather4_LookAtMcCoy(void);
static void feather4_LookAtRedshirt(void);
static void feather4_Talk(uint16_t ax);
static void feather4_Say(uint16_t ax, uint16_t bx);

/* The room starts: the music.
 * [MSG_TICK 01 00 00] */
static void feather4_StartRoom(void)
{
    svc_Sfx(N_FEA4LOOP);
    svc_Music(0x1B, STALE_ARG);
    return;
}

/* The communicator and phasers still don't work.
 * [MSG_USE 47 FF 00] */
static void feather4_UseCommunicator(void)
{
    feather4_Say(N_Dr_McCoy, T_FEA4N008_2);
    return;
}

/* [MSG_USE 40 FF 00] */
static void feather4_StunAnything(void)
{
    feather4_Say(N_Mr_Spock, T_FEA4_011);
    return;
}

/* [MSG_USE 41 FF 00] */
static void feather4_KillAnything(void)
{
    feather4_Say(N_Mr_Spock, T_FEA4_011);
    return;
}

/* Tricorders.
 * [MSG_USE 44 23 00] */
static void feather4_E_013B(void)
{
    { feather4_SpockScansLights2(); return; }
}

/* [MSG_USE 44 24 00] */
static void feather4_SpockScansLights2(void)
{
    { feather4_SpockScansLights(); return; }
}

/* [MSG_USE 44 25 00] */
static void feather4_SpockScansLights(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Mr_Spock, T_FEA4_010);
    return;
}

/* [MSG_USE 44 21 00] */
static void feather4_SpockScansMushrooms(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Mr_Spock, T_FEA4_012);
    return;
}

/* [MSG_USE 44 22 00] */
static void feather4_SpockScansFern(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Mr_Spock, T_FEA4_009);
    return;
}

/* [MSG_USE 44 FF 00] */
static void feather4_SpockScans(void)
{
    svc_Anim(1, N_sscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Mr_Spock, T_FEA4_008);
    return;
}

/* [MSG_USE 45 23 00] */
static void feather4_E_020F(void)
{
    { feather4_McCoyScansLights2(); return; }
}

/* [MSG_USE 45 24 00] */
static void feather4_McCoyScansLights2(void)
{
    { feather4_McCoyScansLights(); return; }
}

/* [MSG_USE 45 25 00] */
static void feather4_McCoyScansLights(void)
{
    svc_Anim(2, N_mscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Dr_McCoy, T_FEA4_001);
    return;
}

/* [MSG_USE 45 21 00] */
static void feather4_McCoyScansMushrooms(void)
{
    svc_Anim(2, N_mscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Dr_McCoy, T_FEA4_003);
    return;
}

/* [MSG_USE 45 FF 00] */
static void feather4_McCoyScans(void)
{
    svc_Anim(2, N_mscanw, -1, -1, 0);
    svc_BridgeSfx(4);
    feather4_Say(N_Dr_McCoy, T_FEA4_006);
    return;
}

/* The medkit, talking and looking.
 * [MSG_USE 7F FF 00] */
static void feather4_UseMedkit(void)
{
    feather4_Say(N_Dr_McCoy, T_FEA4_002);
    return;
}

/* [MSG_TALK 02 00 00] */
static void feather4_TalkToMcCoy(void)
{
    feather4_Say(N_Dr_McCoy, T_FEA4_004);
    return;
}

/* [MSG_TALK 01 00 00] */
static void feather4_TalkToSpock(void)
{
    feather4_Say(N_Mr_Spock, T_FEA4_007);
    return;
}

/* [MSG_TALK 03 00 00] */
static void feather4_TalkToRedshirt(void)
{
    feather4_Say(N_Lt_Stragey, T_FEA4_013);
    return;
}

/* [MSG_LOOK 20 00 00] */
static void feather4_LookAtEyes(void)
{
    feather4_Say(0, T_FEA4N003);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void feather4_LookAtScene(void)
{
    feather4_Say(0, T_FEA4N008);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void feather4_LookAtMushrooms(void)
{
    feather4_Say(0, T_FEA4N001);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void feather4_LookAtFern(void)
{
    feather4_Say(0, T_FEA4N000);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void feather4_LookAtLight1(void)
{
    feather4_Say(0, T_FEA4N002);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void feather4_LookAtLight2(void)
{
    feather4_Say(0, T_FEA4N002);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void feather4_LookAtLight3(void)
{
    feather4_Say(0, T_FEA4N002);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void feather4_LookAtKirk(void)
{
    feather4_Say(0, T_FEA4N004);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void feather4_LookAtSpock(void)
{
    feather4_Say(0, T_FEA4N007);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void feather4_LookAtMcCoy(void)
{
    feather4_Say(0, T_FEA4N006);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void feather4_LookAtRedshirt(void)
{
    feather4_Say(0, T_FEA4N005);
    return;
}

/* Talk: conversation AX (speaker, choices, 0); returns the choice. */
static void feather4_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void feather4_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0xB59) = bx;
    feather4_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript feather4_scripts[] = {
    { 0x00DE, feather4_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x010B, feather4_UseCommunicator },    /* MSG_USE 47 FF 00 */
    { 0x011B, feather4_StunAnything },    /* MSG_USE 40 FF 00 */
    { 0x012B, feather4_KillAnything },    /* MSG_USE 41 FF 00 */
    { 0x013B, feather4_E_013B },    /* MSG_USE 44 23 00 */
    { 0x0143, feather4_SpockScansLights2 },    /* MSG_USE 44 24 00 */
    { 0x014B, feather4_SpockScansLights },    /* MSG_USE 44 25 00 */
    { 0x017C, feather4_SpockScansMushrooms },    /* MSG_USE 44 21 00 */
    { 0x01AD, feather4_SpockScansFern },    /* MSG_USE 44 22 00 */
    { 0x01DE, feather4_SpockScans },    /* MSG_USE 44 FF 00 */
    { 0x020F, feather4_E_020F },    /* MSG_USE 45 23 00 */
    { 0x0217, feather4_McCoyScansLights2 },    /* MSG_USE 45 24 00 */
    { 0x021F, feather4_McCoyScansLights },    /* MSG_USE 45 25 00 */
    { 0x0250, feather4_McCoyScansMushrooms },    /* MSG_USE 45 21 00 */
    { 0x0281, feather4_McCoyScans },    /* MSG_USE 45 FF 00 */
    { 0x02B2, feather4_UseMedkit },    /* MSG_USE 7F FF 00 */
    { 0x0315, feather4_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x0325, feather4_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x0335, feather4_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0345, feather4_LookAtEyes },    /* MSG_LOOK 20 00 00 */
    { 0x0354, feather4_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0363, feather4_LookAtMushrooms },    /* MSG_LOOK 21 00 00 */
    { 0x0372, feather4_LookAtFern },    /* MSG_LOOK 22 00 00 */
    { 0x0381, feather4_LookAtLight1 },    /* MSG_LOOK 23 00 00 */
    { 0x0390, feather4_LookAtLight2 },    /* MSG_LOOK 24 00 00 */
    { 0x039F, feather4_LookAtLight3 },    /* MSG_LOOK 25 00 00 */
    { 0x03AE, feather4_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x03BD, feather4_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x03CC, feather4_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x03DB, feather4_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0, 0 }
};
