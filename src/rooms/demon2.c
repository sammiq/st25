/* DEMON2.RDF: the room's scripts, translated from the machine code in the room file
 * (src/rdf/demon2.asm) by tools/rdf2c.py.  They run on the loaded file (CS_B/CS_W) and the
 * room state (`room`), and call the game through the svc_ macros of rooms.h.
 *
 * DEMON2.RDF - "Demon World", the mine entrance on Mount Idyll. Laraxian
 * berries grow here: the cure for Brother Chub (room 5) is made from them.
 *
 * Objects: 20h, 27h the mine entrance, 21h/22h the mountain, 23h the berry
 * bushes, 24h a fern, 25h the moss, 26h the tunnel lights.
 * Exits: 0 into the mine (room 3), 1 back down the path (room 1).
 *
 * Labels: H_ script headers, L branch targets, C_ conversations,
 * T_/N_/S_ strings; talk_buf: the conversation buffer of Say.
 */

#include "rooms.h"

/* where the file keeps its strings, conversations and tables */
enum {
    N_kusehe = 0x00CA,
    N_mscane = 0x00D8,
    N_sscann = 0x00DF,
    N_sscane = 0x00E6,
    N_DEM2LOOP = 0x00F5,
    T_DEM2N009 = 0x0165,
    T_DEM2N007 = 0x0195,
    T_DEM2N008 = 0x01C6,
    T_DEM2N012 = 0x01F7,
    T_DEM2N005 = 0x024A,
    T_DEM2N010 = 0x027D,
    T_DEM2N006 = 0x02C6,
    T_DEM2N011 = 0x0311,
    T_DEM2_011 = 0x041A,
    T_DEM2_008 = 0x0489,
    T_DEM2_007 = 0x0538,
    T_DEM2_006 = 0x05BA,
    T_DEM2_005 = 0x064D,
    T_DEM2_013 = 0x074D,
    T_DEM2_009 = 0x07AB,
    T_DEM2_010 = 0x0817,
    T_DEM2_012 = 0x08A4,
    T_DEM2_014 = 0x0950,
    N_PIKBERRY = 0x09B7,
    T_DEM2N013 = 0x09E1,
    N_Capt_Kirk = 0x0B4A,
    N_Dr_McCoy = 0x0B55,
    N_Mr_Spock = 0x0B5F,
    N_Ensign_Everts = 0x0B69,
    T_DEM2N002 = 0x0B77,
    T_DEM2N004 = 0x0BC6,
    T_DEM2N003 = 0x0C2D,
    T_DEM2N000 = 0x0C7D,
    T_DEM2_001 = 0x0CCB,
    T_DEM2_015 = 0x0D12,
    T_DEM2_017 = 0x0D56,
    T_DEM2_016 = 0x0DE9,
    T_DEM2_019 = 0x0E22,
    T_DEM2_002 = 0x0E94,
    T_DEM2_020 = 0x0ECF,
    T_DEM2_004 = 0x0F48,
    T_DEM2_018 = 0x100F,
    T_DEM2_003 = 0x1084,
    talk_buf = 0x10DF,
};

/* the room's variables: in the room state, and in the file itself */
#define fv_talk_buf CS_W(0x10DF)
#define rv_came_from ROOM_B(0x3A)
#define rv_cured_chub ROOM_B(0x3D)
#define rv_got_berries ROOM_B(0x50)
#define rv_made_hypo ROOM_B(0x51)
#define rv_need_berries ROOM_B(0x3F)
#define rv_score ROOM_W(0x58)

/* functions */
static void demon2_StartRoom(void);
static void demon2_WalkIntoMine(void);
static void demon2_EnterMine(void);
static void demon2_ExitDownPath(void);
static void demon2_E_0152(void);
static void demon2_LookAtEntrance(void);
static void demon2_LookAtMountain(void);
static void demon2_LookAtMountain2(void);
static void demon2_LookAtBerries(void);
static void demon2_LookAtFern(void);
static void demon2_LookAtMoss(void);
static void demon2_LookAtLights(void);
static void demon2_LookAtScene(void);
static void demon2_LookAtKirk(void);
static void demon2_LookAtSpock(void);
static void demon2_LookAtMcCoy(void);
static void demon2_LookAtRedshirt(void);
static void demon2_TalkToKirk(void);
static void demon2_TalkToSpock(void);
static void demon2_TalkToMcCoy(void);
static void demon2_TalkToRedshirt(void);
static void demon2_E_0407(void);
static void demon2_PhaserBerries(void);
static void demon2_SpockScansBerries(void);
static void demon2_SpockScansMoss(void);
static void demon2_SpockScansFern(void);
static void demon2_E_0610(void);
static void demon2_SpockScansMountain(void);
static void demon2_SpockScansMountain2(void);
static void demon2_E_0713(void);
static void demon2_McCoyScansBerries(void);
static void demon2_GetBerries(void);
static void demon2_ReachBerries(void);
static void demon2_PickBerries(void);
static void demon2_Talk(uint16_t ax);
static void demon2_Say(uint16_t ax, uint16_t bx);

/* The room starts: the ambient sound.
 * [MSG_TICK 01 00 00] */
static void demon2_StartRoom(void)
{
    svc_Sfx(N_DEM2LOOP);
    return;
}

/* Clicking the mine entrance: walk in (msg 2), then on to room 3.
 * [MSG_WALK 27 00 00] */
static void demon2_WalkIntoMine(void)
{
    svc_Walk(0, 0x83, 0x5C, 2);
    return;
}

/* [MSG_WALK_DONE 02 00 00] */
static void demon2_EnterMine(void)
{
    svc_GotoRoom(3, 1);
    return;
}

/* Leaving down the path: the engine does the rest (RS_MORE).
 * [MSG_EXIT 01 00 00] */
static void demon2_ExitDownPath(void)
{
    rv_came_from = 2;
    room.more = 1;
    return;
}

/* Looking at things.
 * [MSG_LOOK 27 00 00] */
static void demon2_E_0152(void)
{
    { demon2_LookAtEntrance(); return; }
}

/* [MSG_LOOK 20 00 00] */
static void demon2_LookAtEntrance(void)
{
    demon2_Say(0, T_DEM2N009);
    return;
}

/* [MSG_LOOK 21 00 00] */
static void demon2_LookAtMountain(void)
{
    demon2_Say(0, T_DEM2N007);
    return;
}

/* [MSG_LOOK 22 00 00] */
static void demon2_LookAtMountain2(void)
{
    demon2_Say(0, T_DEM2N008);
    return;
}

/* [MSG_LOOK 23 00 00] */
static void demon2_LookAtBerries(void)
{
    demon2_Say(0, T_DEM2N012);
    return;
}

/* [MSG_LOOK 24 00 00] */
static void demon2_LookAtFern(void)
{
    demon2_Say(0, T_DEM2N005);
    return;
}

/* [MSG_LOOK 25 00 00] */
static void demon2_LookAtMoss(void)
{
    demon2_Say(0, T_DEM2N010);
    return;
}

/* [MSG_LOOK 26 00 00] */
static void demon2_LookAtLights(void)
{
    demon2_Say(0, T_DEM2N006);
    return;
}

/* [MSG_LOOK FF 00 00] */
static void demon2_LookAtScene(void)
{
    demon2_Say(0, T_DEM2N011);
    return;
}

/* [MSG_LOOK 00 00 00] */
static void demon2_LookAtKirk(void)
{
    demon2_Say(0, T_DEM2N002);
    return;
}

/* [MSG_LOOK 01 00 00] */
static void demon2_LookAtSpock(void)
{
    demon2_Say(0, T_DEM2N004);
    return;
}

/* [MSG_LOOK 02 00 00] */
static void demon2_LookAtMcCoy(void)
{
    demon2_Say(0, T_DEM2N003);
    return;
}

/* [MSG_LOOK 03 00 00] */
static void demon2_LookAtRedshirt(void)
{
    demon2_Say(0, T_DEM2N000);
    return;
}

/* Talking to the crew.
 * [MSG_TALK 00 00 00] */
static void demon2_TalkToKirk(void)
{
    demon2_Say(N_Capt_Kirk, T_DEM2_001);
    return;
}

/* [MSG_TALK 01 00 00] */
static void demon2_TalkToSpock(void)
{
    demon2_Say(N_Mr_Spock, T_DEM2_018);
    demon2_Say(N_Capt_Kirk, T_DEM2_003);
    return;
}

/* [MSG_TALK 02 00 00] */
static void demon2_TalkToMcCoy(void)
{
    demon2_Say(N_Dr_McCoy, T_DEM2_015);
    demon2_Say(N_Mr_Spock, T_DEM2_017);
    demon2_Say(N_Dr_McCoy, T_DEM2_016);
    return;
}

/* [MSG_TALK 03 00 00] */
static void demon2_TalkToRedshirt(void)
{
    demon2_Say(N_Ensign_Everts, T_DEM2_019);
    demon2_Say(N_Capt_Kirk, T_DEM2_002);
    demon2_Say(N_Ensign_Everts, T_DEM2_020);
    demon2_Say(N_Capt_Kirk, T_DEM2_004);
    return;
}

/* Phasers and tricorders. What McCoy says about the berries depends on
 * how far the cure has got.
 * [MSG_USE 41 23 00] */
static void demon2_E_0407(void)
{
    { demon2_PhaserBerries(); return; }
}

/* [MSG_USE 40 23 00] */
static void demon2_PhaserBerries(void)
{
    demon2_Say(N_Dr_McCoy, T_DEM2_011);
    return;
}

/* [MSG_USE 44 23 00] */
static void demon2_SpockScansBerries(void)
{
    svc_Anim(1, N_sscane, -1, -1, 0);
    svc_BridgeSfx(4);
    demon2_Say(N_Mr_Spock, T_DEM2_008);
    return;
}

/* [MSG_USE 44 25 00] */
static void demon2_SpockScansMoss(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon2_Say(N_Mr_Spock, T_DEM2_007);
    return;
}

/* [MSG_USE 44 24 00] */
static void demon2_SpockScansFern(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon2_Say(N_Mr_Spock, T_DEM2_006);
    return;
}

/* [MSG_USE 44 20 00] */
static void demon2_E_0610(void)
{
    { demon2_SpockScansMountain(); return; }
}

/* [MSG_USE 44 21 00] */
static void demon2_SpockScansMountain(void)
{
    { demon2_SpockScansMountain2(); return; }
}

/* [MSG_USE 44 22 00] */
static void demon2_SpockScansMountain2(void)
{
    svc_Anim(1, N_sscann, -1, -1, 0);
    svc_BridgeSfx(4);
    demon2_Say(N_Mr_Spock, T_DEM2_005);
    return;
}

/* [MSG_USE 02 23 00] */
static void demon2_E_0713(void)
{
    { demon2_McCoyScansBerries(); return; }
}

/* [MSG_USE 45 23 00] */
static void demon2_McCoyScansBerries(void)
{
    svc_Anim(2, N_mscane, -1, -1, 0);
    svc_BridgeSfx(4);
    if (rv_cured_chub == 1) {
        demon2_Say(N_Dr_McCoy, T_DEM2_013);
        return;
    }
    if (rv_made_hypo == 1) {
        demon2_Say(N_Dr_McCoy, T_DEM2_009);
        return;
    }
    if (rv_need_berries != 1) {
    } else {
        demon2_Say(N_Dr_McCoy, T_DEM2_010);
        return;
    }
    demon2_Say(N_Dr_McCoy, T_DEM2_012);
    return;
}

/* Getting the berries, once: walk there (msg 1), reach (anim msg 1),
 * +1 score.
 * [MSG_GET 23 00 00] */
static void demon2_GetBerries(void)
{
    if (rv_got_berries == 1) {
        demon2_Say(N_Dr_McCoy, T_DEM2_014);
        return;
    }
    svc_Walk(0, 0xE8, 0x97, 1);
    return;
}

/* [MSG_WALK_DONE 01 00 00] */
static void demon2_ReachBerries(void)
{
    svc_Anim(0, N_kusehe, -1, -1, 1);
    svc_Sfx(N_PIKBERRY);
    return;
}

/* [MSG_ANIM_DONE 01 00 00] */
static void demon2_PickBerries(void)
{
    demon2_Say(0, T_DEM2N013);
    svc_GiveItem(IT_BERRY);
    rv_got_berries = 1;
    rv_score += 1;
    return;
}

static void demon2_Talk(uint16_t ax)
{
    svc_Talk(ax);
    return;
}

/* Say: one line, BX, said by AX (0: narration). */
static void demon2_Say(uint16_t ax, uint16_t bx)
{
    fv_talk_buf = ax;
    CS_W(0x10E1) = bx;
    demon2_Talk(talk_buf);
    return;
}

/* the scripts, by the offset of their code in the room file */
const RoomScript demon2_scripts[] = {
    { 0x00F3, demon2_StartRoom },    /* MSG_TICK 01 00 00 */
    { 0x0114, demon2_WalkIntoMine },    /* MSG_WALK 27 00 00 */
    { 0x012E, demon2_EnterMine },    /* MSG_WALK_DONE 02 00 00 */
    { 0x0143, demon2_ExitDownPath },    /* MSG_EXIT 01 00 00 */
    { 0x0152, demon2_E_0152 },    /* MSG_LOOK 27 00 00 */
    { 0x015A, demon2_LookAtEntrance },    /* MSG_LOOK 20 00 00 */
    { 0x018A, demon2_LookAtMountain },    /* MSG_LOOK 21 00 00 */
    { 0x01BB, demon2_LookAtMountain2 },    /* MSG_LOOK 22 00 00 */
    { 0x01EC, demon2_LookAtBerries },    /* MSG_LOOK 23 00 00 */
    { 0x023F, demon2_LookAtFern },    /* MSG_LOOK 24 00 00 */
    { 0x0272, demon2_LookAtMoss },    /* MSG_LOOK 25 00 00 */
    { 0x02BB, demon2_LookAtLights },    /* MSG_LOOK 26 00 00 */
    { 0x0306, demon2_LookAtScene },    /* MSG_LOOK FF 00 00 */
    { 0x0355, demon2_LookAtKirk },    /* MSG_LOOK 00 00 00 */
    { 0x0364, demon2_LookAtSpock },    /* MSG_LOOK 01 00 00 */
    { 0x0373, demon2_LookAtMcCoy },    /* MSG_LOOK 02 00 00 */
    { 0x0382, demon2_LookAtRedshirt },    /* MSG_LOOK 03 00 00 */
    { 0x0391, demon2_TalkToKirk },    /* MSG_TALK 00 00 00 */
    { 0x03A1, demon2_TalkToSpock },    /* MSG_TALK 01 00 00 */
    { 0x03BA, demon2_TalkToMcCoy },    /* MSG_TALK 02 00 00 */
    { 0x03DC, demon2_TalkToRedshirt },    /* MSG_TALK 03 00 00 */
    { 0x0407, demon2_E_0407 },    /* MSG_USE 41 23 00 */
    { 0x040F, demon2_PhaserBerries },    /* MSG_USE 40 23 00 */
    { 0x045D, demon2_SpockScansBerries },    /* MSG_USE 44 23 00 */
    { 0x050C, demon2_SpockScansMoss },    /* MSG_USE 44 25 00 */
    { 0x058E, demon2_SpockScansFern },    /* MSG_USE 44 24 00 */
    { 0x0610, demon2_E_0610 },    /* MSG_USE 44 20 00 */
    { 0x0618, demon2_SpockScansMountain },    /* MSG_USE 44 21 00 */
    { 0x0620, demon2_SpockScansMountain2 },    /* MSG_USE 44 22 00 */
    { 0x0713, demon2_E_0713 },    /* MSG_USE 02 23 00 */
    { 0x071B, demon2_McCoyScansBerries },    /* MSG_USE 45 23 00 */
    { 0x093F, demon2_GetBerries },    /* MSG_GET 23 00 00 */
    { 0x09A0, demon2_ReachBerries },    /* MSG_WALK_DONE 01 00 00 */
    { 0x09D6, demon2_PickBerries },    /* MSG_ANIM_DONE 01 00 00 */
    { 0, 0 }
};
