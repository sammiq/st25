/* seg008: the away team - rooms, their scripts and objects, conversation
 * boxes and the player's actions */

#include "compat.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "event.h"
#include "gfx.h"
#include "sound.h"
#include "sprite.h"
#include "resource.h"
#define MenuWait MenuWait_
#include "menu.h"
#undef MenuWait
#include "actor.h"
#include "room.h"
#include "obj3d.h"
#include "rooms.h"

/* Globals this file owns (declared in the headers). */
Actor actors[MAX_ACTORS];
MemHandle rdf_handle;
Rdf *rdf;
uint8_t *rdf_image;
uint8_t stray_mem[0x400];            /* where scripts that use SI as an index write (rooms.h) */                  /* the loaded .rdf, which the scripts' variables live in */
static const RoomScript *room_scripts;   /* this room's scripts (src/rooms) */
int32_t ScriptService(int16_t cmd, int16_t a1, int16_t a2, int16_t a3, int16_t a4, uint16_t a5);
char room_mission[16];
RoomState room;

int16_t MenuWait();
int16_t ItemGive();
int16_t ItemHave();
void ItemTake(int16_t id);
void ItemsInit(void);
void ItemsSave(void);
void ItemsLoad(void);
int16_t PickItem(int16_t x, int16_t y, int16_t restore);
char *WrapLine(char *src, char *dst, int16_t width);
MemHandle getanyfile(char *name, char *ext);
void FreeAnyFile(MemHandle h);
int32_t GetFileSize(char *name, char *ext);
char *InputCode(void);
void GameOverMenu(void);
int16_t ActionPanel(void);
void SaveWrite(void *p, uint16_t n);
void SaveRead(void *p, uint16_t n);
void SaveClose(void);
void SaveSpace(void);
void LoadSpace(void);
void SaveMusicState(void);
void LoadMusicState(void);
void PlayVoc(char *name);
void StopVoc(void);
void SaveActors(void);
void LoadActors(void);
void InitActors(void);
void FreeActors(void);
void UpdateActors(void);
void EndOrStand(int16_t n);
void LoadBan(char *name);
void LoadWalkGraph(char *name);
void DrawBans(void);
void UpdateBans(void);
void BridgeSfx(int16_t n);              /* seg001 */
void BridgeLeave(void);
void BridgeEnter(int16_t reset);
void SetTextFile(char *name);
char *GetText(int16_t n);
void FreeText(char *p);
void SendScript(int16_t n);
int16_t LeverPuzzle(void);              /* seg012 */
int16_t Turbolift(int16_t unused, int16_t deck);
int16_t ItemCase(int16_t mask);
void FlashBitmap(char *name, int16_t z);         /* seg030 */
void StartCrew(void);

typedef struct Msg {
    unsigned char type;
    unsigned char arg;
    unsigned char b2;
    unsigned char b3;
} Msg;

/* A room script: a key (action, target, target2, extra; FF matches any)
 * and the machine code to run. */
typedef struct Script {
    int32_t key;
    uint16_t next;
    char code[1];
} Script;

typedef struct Point {
    int16_t x;
    int16_t y;
} Point;

typedef struct Item {
    int16_t have;                   /* 00 */
    int16_t code;                   /* 02 */
    char name[10];              /* 04 bitmap name */
    int16_t text;                   /* 0E description */
} Item;

/* Deferred messages: posted when the crew member walking there arrives. */
typedef struct Timer {
    unsigned char active;
    unsigned char msg[4];
} Timer;

extern Bitmap *screen;
extern Bitmap *backbuf;
extern unsigned char *mask_buf;
extern uint32_t blink_count;
extern int16_t kbd_mouse;
extern int16_t in_orbit;
extern int16_t under_attack;
extern int16_t encounter_kind;
extern int16_t encounter_delay;
extern uint32_t bridge_ticks;
extern int16_t word_45AB2;
extern int16_t word_45AB4;
extern int16_t view_dir;
extern int16_t word_45ABA;
char mission[16];
int16_t mission_vars[6];
extern int16_t last_mission;
extern int16_t mission_ranks[7];
char sfx_name[10];
extern void (*bridge_tick)(void);
extern void (*tick_fns[32])(void);
extern int16_t game_mode;
extern int16_t prev_mode;
extern int16_t key_accel;
int16_t voc_wait;
extern int16_t sfx_on;
extern int16_t digi_ok;
jmp_buf game_jmp;
jmp_buf room_jmp;
Msg msg_queue[16];
extern int16_t msg_head;
int16_t msg_tail;
char room_name[16];
char room_file[16];
int16_t room_num;
int32_t room_ticks;
int32_t scale_k;
MemHandle map_handle;
int16_t cur_region;
int16_t cur_exit;
Timer timers[31];
int16_t hit_valid;
int16_t hit_x;
int16_t hit_y;
Sprite item_sprite;
Sprite inv_sprite;
int16_t word_52C14;
int16_t word_53798;
int16_t word_50CA6;
int16_t word_52C1E;
extern int16_t word_51156;

int16_t HasScript(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
int16_t RunScripts(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
int16_t RunWildScripts(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
char *RdfPtr(uint16_t off);
void PlayScaled(int16_t n, char *name, int16_t x, int16_t y);
int16_t DepthScale(int16_t y);
void PostRoomMsg(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
int32_t PopRoomMsg(void);
void TraceMsg(int32_t msg);
void RoomMsg(void);
void CallScript(void *code);
int32_t ScriptService(int16_t cmd, int16_t a1, int16_t a2, int16_t a3, int16_t a4, uint16_t a5);
void CentreLine(char *dst, char *src);
void ChoiceTitle(char *dst, char *name, int16_t n);
int32_t Converse(int16_t text, int16_t x, int16_t y);
int32_t TextBox(void *fn, void *arg, int16_t x, int16_t y,
               int16_t color, int16_t choice, int16_t maxl, int16_t flag);
void MsgBox(char *title, char *text, int16_t x, int16_t y, int16_t color, int16_t msg);
char *SkipVoice(char *p);
char *PlayVoiceTag(char *p);
int16_t WrapCount(char *s);
void WrapText(char *dst, char *s);
Bitmap *MakeTextBox(int16_t *px, int16_t *py, int16_t color, int16_t lines, uint16_t titled, Sprite *spr);
void BoxLines(Bitmap *box, int16_t n, int16_t max, char *src, uint16_t titled);
void BoxTitle(Bitmap *box, char *title);
char *LayoutChoice(void *fn, void *arg, uint16_t sel,
                Bitmap *box, int16_t max, int16_t *nlines);
void ActionCursor(int16_t action, int16_t on);
int16_t FindObject(int16_t x, int16_t y);
int16_t InPoly(int16_t *poly, int16_t x, int16_t y);
int16_t *NextPoly(int16_t *poly);
void CheckRegions(int16_t x, int16_t y);
void RoomTimers(void);
int16_t WalkBlocked(int16_t x, int16_t y);
int16_t PickUseTarget(void);
Event WaitAction(void);
void CrewAnim(char *buf, int16_t crew, char *what);
void HiliteCursor(void);
int16_t WalkToHit(void);
int16_t CantAct(int16_t obj, int16_t action);
void StunTick(void);
void ShowInvIcon(int16_t on);
void HideInvIcon(void);
int16_t AwayInput(void);
void LoadRoom(char *mission, int16_t n);
void EnterRoom(int16_t entry);
void BeamUp(void);
void FreeRoom(void);
void SaveRoom(void);
void RestoreRoom(void);

Item items[137] = {
    { 1, 0x40, "iphasers", 0 },
    { 1, 0x41, "iphaserk", 1 },
    { 0, 0x42, "ihand", 2 },
    { 0, 0x43, "irock", 3 },
    { 1, 0x44, "istricor", 4 },
    { 1, 0x45, "imtricor", 5 },
    { 0, 0x46, "ideadguy", 6 },
    { 1, 0x47, "icomm", 7 },
    { 0, 0x48, "ipbc", 8 },
    { 0, 0x49, "iRLG", 9 },
    { 0, 0x4A, "iWrench", 10 },
    { 0, 0x4B, "iInsulat", 11 },
    { 0, 0x4C, "iSample", 12 },
    { 0, 0x4D, "iCure", 13 },
    { 0, 0x4E, "iDishes", 14 },
    { 0, 0x4F, "iRT", 15 },
    { 0, 0x50, "iRTWB", 16 },
    { 0, 0x51, "iCombBit", 17 },
    { 0, 0x52, "iJnkmetl", 18 },
    { 0, 0x53, "iWiring", 19 },
    { 0, 0x54, "iWirscrp", 20 },
    { 0, 0x55, "iPWF", 21 },
    { 0, 0x56, "iPWE", 22 },
    { 0, 0x57, "iDeadPh", 23 },
    { 0, 0x58, "iBomb", 24 },
    { 0, 0x59, "iMetal", 25 },
    { 0, 0x5A, "iSkull", 26 },
    { 0, 0x5B, "iMineral", 27 },
    { 0, 0x5C, "iMeteor", 28 },
    { 0, 0x5D, "iShells", 29 },
    { 0, 0x5E, "iDeGrime", 30 },
    { 0, 0x5F, "iLenses", 31 },
    { 0, 0x60, "iDisks", 32 },
    { 0, 0x61, "iAntigra", 33 },
    { 0, 0x62, "iN2gas", 34 },
    { 0, 0x63, "iO2gas", 35 },
    { 0, 0x64, "iH2gas", 36 },
    { 0, 0x65, "iN2O", 37 },
    { 0, 0x66, "iNH3", 38 },
    { 0, 0x67, "iH2O", 39 },
    { 0, 0x68, "iWRod", 40 },
    { 0, 0x69, "iIRod", 41 },
    { 0, 0x6A, "iRedGem", 42 },
    { 0, 0x6B, "iRedGem", 43 },
    { 0, 0x6C, "iRedGem", 44 },
    { 0, 0x6D, "iGrnGem", 45 },
    { 0, 0x6E, "iGrnGem", 46 },
    { 0, 0x6F, "iGrnGem", 47 },
    { 0, 0x70, "iBluGem", 48 },
    { 0, 0x71, "iBluGem", 49 },
    { 0, 0x72, "iBluGem", 50 },
    { 0, 0x73, "iConect", 51 },
    { 0, 0x74, "iS8Rocks", 52 },
    { 0, 0x75, "iIDCard", 53 },
    { 0, 0x76, "iSnake", 54 },
    { 0, 0x77, "iFern", 55 },
    { 0, 0x78, "iCrystal", 56 },
    { 0, 0x79, "iKnife", 57 },
    { 0, 0x7A, "idetoxin", 58 },
    { 0, 0x7B, "iberry", 59 },
    { 0, 0x7C, "idoover", 60 },
    { 0, 0x7D, "ialiendv", 61 },
    { 0, 0x7E, "icapsule", 62 },
    { 1, 0x7F, "imedkit", 63 },
    { 0, 0x80, "iBeam", 64 },
    { 0, 0x81, "iDrill", 65 },
    { 0, 0x82, "iHypo", 66 },
    { 0, 0x83, "iFusion", 67 },
    { 0, 0x84, "iCable1", 68 },
    { 0, 0x85, "iCable2", 69 },
    { 0, 0x86, "iLMD", 70 },
    { 0, 0x87, "iDeck", 71 },
    { 0, 0x88, "iTech", 72 }
};
int16_t exits_open = 1;                 /* the crew may leave by an exit */
char dir_letters[6] = "nsew";
int16_t mccoy_lines[8] = { 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E };
int16_t spock_lines[8] = { 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66 };
int16_t redshirt_lines[8] = { 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E };
int16_t misc_lines[7] = { 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75 };
uint32_t last_talk = 0;

/* Save the room: its state, then the bytes of the .rdf the scripts
 * changed (with a checksum of the original file). */
void SaveRoom(void)
{
    uint16_t size;
    uint16_t i;
    uint32_t sum;
    char *orig;
    char *cur;
    char c;

    SaveWrite(room_mission, 16);
    SaveWrite(&room_num, 2);
    SaveWrite(&room_ticks, 4);
    SaveWrite(&blink_count, 4);
    SaveActors();
    SaveWrite(room_file, 16);
    SaveWrite(&room, sizeof(room));
    SaveWrite(msg_queue, sizeof(msg_queue));
    SaveWrite(&msg_head, 2);
    SaveWrite(&msg_tail, 2);
    size = GetFileSize(room_name, "rdf");
    sum = 0;
    orig = backbuf->data;
    LoadFilePacked(room_name, "rdf", orig);
    rdf = LockMem(rdf_handle);
    cur = (char *)rdf;
    for (i = 6; i < size; i++) {
        sum = sum << 1 | sum >> 31;
        sum += orig[i];
    }
    SaveWrite(&sum, 4);
    for (i = 6; i < size; i++)
        if (orig[i] != cur[i]) {
            SaveWrite(&i, 2);
            c = cur[i];
            SaveWrite(&c, 1);
        }
    i = -1;
    SaveWrite(&i, 2);
    UnlockMem(rdf_handle);
    SaveWrite(&hit_valid, 2);
    SaveWrite(&hit_x, 2);
    SaveWrite(&hit_y, 2);
    SaveWrite(timers, sizeof(timers));
    SaveWrite(&exits_open, 2);
}

/* Write the game state to the save file, the room last when on an away
 * mission. */
void SaveGameState(void)
{
    int16_t i;

    SaveWrite(&game_mode, 2);
    SaveSpace();
    SaveMusicState();
    SaveWrite(&blink_count, 4);
    SaveWrite(&kbd_mouse, 2);
    SaveWrite(&in_orbit, 2);
    SaveWrite(&under_attack, 2);
    SaveWrite(&encounter_kind, 2);
    SaveWrite(&encounter_delay, 2);
    SaveWrite(&bridge_ticks, 4);
    SaveWrite(&word_45AB2, 2);
    SaveWrite(&word_45AB4, 2);
    SaveWrite(&view_dir, 2);
    SaveWrite(mission, 16);
    SaveWrite(&boost, sizeof(boost));
    SaveWrite(mission_vars, 12);
    SaveWrite(&last_mission, 2);
    SaveWrite(mission_ranks, 14);
    SaveWrite(sfx_name, 10);
    for (i = 0; ; i++)
        if (bridge_tick == tick_fns[i]) {
            SaveWrite(&i, 2);
            break;
        }
    ItemsSave();
    if (game_mode == 2)
        SaveRoom();
}

/* Load the room saved by SaveRoom, patching the .rdf the same way. */
void RestoreRoom(void)
{
    uint16_t size;
    uint16_t i;
    uint32_t saved;
    uint32_t sum;
    char *cur;
    char c;

    FadeOut();
    SaveRead(room_mission, 16);
    SaveRead(&room_num, 2);
    SetTextFile("ground");
    LoadRoom(room_mission, room_num);
    SaveRead(&room_ticks, 4);
    SaveRead(&blink_count, 4);
    LoadActors();
    SaveRead(room_file, 16);
    FreeAnyFile(map_handle);
    map_handle = getanyfile(room_file, "map");
    LoadWalkGraph(room_file);
    SaveRead(&room, sizeof(room));
    room.action = 1;
    SaveRead(msg_queue, sizeof(msg_queue));
    SaveRead(&msg_head, 2);
    SaveRead(&msg_tail, 2);
    size = GetFileSize(room_name, "rdf");
    sum = 0;
    rdf = LockMem(rdf_handle);
    cur = (char *)rdf;
    for (i = 6; i < size; i++) {
        sum = sum << 1 | sum >> 31;
        sum += cur[i];
    }
    SaveRead(&saved, 4);
    if (saved == sum) {
        while (SaveRead(&i, 2), i != 0xFFFF) {
            SaveRead(&c, 1);
            cur[i] = c;
        }
    } else {
        while (SaveRead(&i, 2), i != 0xFFFF)
            SaveRead(&c, 1);
    }
    UnlockMem(rdf_handle);
    if (saved != sum) {
        FadeIn();
        MsgBox("Warning", "Code for current room patched after game saved.", 20, 20, 0xB0, 0);
    }
    SaveRead(&hit_valid, 2);
    SaveRead(&hit_x, 2);
    SaveRead(&hit_y, 2);
    SaveRead(timers, sizeof(timers));
    SaveRead(&exits_open, 2);
}

/* Read the game state back from the save file. */
void LoadGameState(void)
{
    int16_t i;

    if (prev_mode == 1)
        BridgeLeave();
    else
        FreeRoom();
    SaveRead(&game_mode, 2);
    LoadSpace();
    LoadMusicState();
    SaveRead(&blink_count, 4);
    SaveRead(&kbd_mouse, 2);
    SaveRead(&in_orbit, 2);
    SaveRead(&under_attack, 2);
    SaveRead(&encounter_kind, 2);
    SaveRead(&encounter_delay, 2);
    SaveRead(&bridge_ticks, 4);
    SaveRead(&word_45AB2, 2);
    SaveRead(&word_45AB4, 2);
    SaveRead(&view_dir, 2);
    SaveRead(mission, 16);
    SaveRead(&boost, sizeof(boost));
    SaveRead(mission_vars, 12);
    SaveRead(&last_mission, 2);
    SaveRead(mission_ranks, 14);
    SaveRead(sfx_name, 10);
    if (sfx_name[0])
        PlaySfx(sfx_name);
    SaveRead(&i, 2);
    bridge_tick = tick_fns[i];
    prev_mode = game_mode;
    ItemsLoad();
    if (game_mode == 2) {
        RestoreRoom();
        SaveClose();
        longjmp(game_jmp, 2);
    } else if (game_mode == -1) {
        BridgeEnter(1);
        prev_mode = 1;
        SaveClose();
        longjmp(game_jmp, 1);
    } else {
        SetTextFile(mission);
        BridgeEnter(0);
        SaveClose();
        longjmp(game_jmp, 1);
    }
}

/* Start the away mission. */
void StartAway(void)
{
    MemSet(&room, sizeof(room), 0);
    MemSet(screen, 0xFA00, 0);
    SetTextFile("ground");
    ItemsInit();
    LoadMusic("ground");
    LoadRoom(mission, word_45ABA);
    EnterRoom(4);
}

/* The away team's main loop. */
void AwayLoop(void)
{
    int16_t unused[2];
    int16_t r;

    r = setjmp(room_jmp);
    for (;;) {
        while (msg_head != msg_tail) {
            TraceMsg(PopRoomMsg());
            RoomMsg();
        }
        AwayInput();
        room.saved_x = mouse_x;
        room.saved_y = mouse_y;
    }
}

/* Leave the room for the bridge. */
void EndAway(void)
{
    FreeRoom();
    SetTextFile(mission);
}

/* Beam the landing party up. */
void BeamUp(void)
{
    char buf[12];
    int16_t i;
    int16_t n;

    room.busy = 1;
    n = room.redshirt_dead ? 3 : 4;
    for (i = 0; i != n; i++) {
        CrewAnim(buf, i, "teled");
        PlayScaled(i, buf, actors[i].sprite.x, actors[i].sprite.y);
    }
    actors[0].stand[0] = 0;
    actors[2].stand[0] = 0;
    actors[3].stand[0] = 0;
    actors[1].stand[0] = 0;
    BridgeSfx(8);
    while (actors[0].state)
        AwayInput();
    room.busy = 0;
    longjmp(game_jmp, 4);
}

/* Load room n of mission. */
void LoadRoom(char *mission, int16_t n)
{
    key_accel = 1;
    strcpy(room_mission, mission);
    room_num = n;
    room_ticks = 0;
    room.busy = 0;
    FadeOut();
    FreeAllSfxFiles();
    sprintf(room_name, "%s%d", room_mission, n);
    LoadFilePacked(room_name, "bmp", screen);
    LoadFilePacked(room_name, "pri", mask_buf);
    LoadPalette("palette");
    MemCpy(backbuf->data, screen->data, 0xFA00);
    BlitToScreen(&screen_rect);
    rdf_handle = getanyfile(room_name, "rdf");
    rdf = LockMem(rdf_handle);
    rdf->service = 0;                   /* the scripts are C now: they call ScriptService directly */
    rdf->state = 0;
    rdf_image = (uint8_t *)rdf;
    room_scripts = FindRoomScripts(room_name);
    if (room_scripts == 0)
        Fatal("no scripts for room %s", room_name);
    UnlockMem(rdf_handle);
    room.action = 1;
    strcpy(room_file, room_name);
    map_handle = getanyfile(room_file, "map");
    LoadWalkGraph(room_file);
    FreeActors();
    InitActors();
    rdf = LockMem(rdf_handle);
    scale_k = (rdf->scale_max - rdf->scale_min) * 256.0 / (rdf->y_max - rdf->y_min + 1);
    if (rdf->bans != rdf->bans_end) {
        uint16_t i;
        uint16_t last;
        char *p;
        char name[16];

        i = rdf->bans;
        last = rdf->bans_end;
        while (i != last) {
            p = RdfPtr(i);
            _fstrcpy(name, p);
            while (i++, *p++)
                ;
            UnlockMem(rdf_handle);
            LoadBan(name);
            rdf = LockMem(rdf_handle);
        }
    }
    UnlockMem(rdf_handle);
    msg_head = msg_tail = 0;
}

/* Bring the landing party into the room: by entrance 0 to 3, beamed in
 * (4), standing (5), or the bridge crew (6). */
void EnterRoom(int16_t entry)
{
    int16_t off;
    int16_t i;
    int16_t n;
    char buf[16];
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;

    FreeAllSfxFiles();
    MemSet(room.fall, 4, 0xFF);
    switch (entry) {
    case 0:
    case 1:
    case 2:
    case 3:
        n = room.redshirt_dead ? 6 : 8;
        for (off = i = 0; off != n; off += 2, ++i) {
            CrewAnim(buf, i, "walk");
            rdf = LockMem(rdf_handle);
            x0 = rdf->walk_in[entry][off][0];
            y0 = rdf->walk_in[entry][off][1];
            x1 = rdf->walk_in[entry][off + 1][0];
            y1 = rdf->walk_in[entry][off + 1][1];
            UnlockMem(rdf_handle);
            StartWalk(i, buf, x0, y0, x1, y1);
        }
        actors[0].notify = 1;
        actors[0].notify_arg = 0xFF;
        room.busy = 1;
        exits_open = 0;
        break;
    case 4:
        n = room.redshirt_dead ? 3 : 4;
        entry -= 4;
        for (i = 0; i != n; i++) {
            CrewAnim(buf, i, "tele");
            rdf = LockMem(rdf_handle);
            x0 = rdf->beam_pos[i][0];
            y0 = rdf->beam_pos[i][1];
            UnlockMem(rdf_handle);
            PlayScaled(i, buf, x0, y0);
        }
        actors[0].notify = 1;
        actors[0].notify_arg = 0xFF;
        room.busy = 1;
        exits_open = 0;
        BridgeSfx(9);
        break;
    case 5:
        n = room.redshirt_dead ? 3 : 4;
        for (i = 0; i != n; i++) {
            CrewAnim(buf, i, "stnds");
            rdf = LockMem(rdf_handle);
            x0 = rdf->stand_pos[i][0];
            y0 = rdf->stand_pos[i][1];
            UnlockMem(rdf_handle);
            PlayScaled(i, buf, x0, y0);
        }
        exits_open = 1;
        break;
    case 6:
        StartCrew();
    }
}

/* The away team's input. */
int16_t AwayInput(void)
{
    Event ev;
    int16_t obj;
    int16_t k;
    char buf[14];

    if (GetEvent(&ev)) {
        switch (ev.type) {
        case EV_TICK:
            UpdateActors();
            StunTick();
            HiliteCursor();
            DrawBans();
            UpdateSprites();
            UpdateBans();
            MusicTick();
            RoomTimers();
            blink_count++;
            room_ticks++;
            PostRoomMsg(0, (int16_t)room_ticks & 0xFF, (int16_t)(room_ticks >> 8) & 0xFF, 0);
            if (room_ticks >= 2)
                FadeUp();
            break;
        case EV_MOVE:
            SpriteSetPos(&mouse_sprite, ev.x, ev.y, 15);
            break;
        case EV_DOWN1:
        click:
            if (room.busy)
                break;
            switch (room.action) {
            case 1:
                if (room.no_walk)
                    break;
                actors[0].sprite.type = 1;
                actors[1].sprite.type = 1;
                actors[2].sprite.type = 1;
                actors[3].sprite.type = 1;
                obj = FindObject(mouse_x, mouse_y);
                actors[0].sprite.type = 0;
                actors[1].sprite.type = 0;
                actors[2].sprite.type = 0;
                actors[3].sprite.type = 0;
                if (WalkToHit())
                    break;
                if (obj < 0x40 && obj > 0)
                    PostRoomMsg(1, obj & 0xFF, 0, 0);
                else {
                    CrewAnim(buf, 0, "walk");
                    StartWalk(0, buf, actors[0].x, actors[0].y, mouse_x, mouse_y);
                }
                break;
            case 3:
            case 4:
            case 5:
                obj = FindObject(mouse_x, mouse_y);
                if (CantAct(obj, room.action))
                    break;
                HideInvIcon();
                if (obj == 0x1F) {
                    obj = PickItem(50, 50, 0);
                look_item:
                    if (obj == -1)
                        obj = -2;
                }
                room.target = obj;
                if (WalkToHit())
                    break;
                if (obj != -2)
                    PostRoomMsg(room.action, room.target, 0, 0);
                if (room.action == 4 && (room.flags & 1) == 0)
                    ShowInvIcon(0);
                break;
            case 2:
                if (room.target == 3 && (room.redshirt_dead || (room.flags & 8))) {
                    HideInvIcon();
                    room.action = 1;
                    break;
                }
                obj = FindObject(mouse_x, mouse_y);
                HideInvIcon();
                if (obj == 0x1F) {
                    obj = PickItem(50, 50, 0);
                use_item:
                    if (obj == -1)
                        obj = -2;
                }
                room.target2 = obj;
                if (obj != -2 && !HasScript(2, room.target, room.target2, 0)
                    && !(room.target == 2 && (HasScript(2, 0x7F, room.target2, 0)
                                              || HasScript(2, 0x45, room.target2, 0)))
                    && !(room.target == 1 && HasScript(2, 0x44, room.target2, 0))
                    && ((0 <= room.target && room.target <= 3 && 0 <= room.target2 && room.target2 <= 3)
                        || (0 <= room.target && room.target <= 3 && 0x40 <= room.target2 && room.target2 <= 0x89)
                        || (0x40 <= room.target && room.target < 0x89 && 0x40 <= room.target2 && room.target2 <= 0x89))) {
                    if (room.target2 == 0x47) {
                        if (WalkToHit())
                            break;
                        PostRoomMsg(2, 0x47, 0, 0);
                        PlaySfx("commun30");
                        if (room.target < 0 || 3 < room.target) {
                            room.action = 1;
                            break;
                        }
                    } else
                        room.target = room.target2;
                } else {
                    if (WalkToHit())
                        break;
                    if (obj != -2)
                        PostRoomMsg(room.action, room.target, room.target2, 0);
                }
                if ((room.flags & 1) == 0)
                    ShowInvIcon(1);
                break;
            }
            break;
        case EV_DOWN2:
        menu:
            if (room.busy)
                break;
            HideInvIcon();
            BridgeSfx(7);
            room.action = ActionPanel();
        action:
            if (room.action == 2) {
                obj = PickUseTarget();
                if (obj == -1)
                    return room.action == 1;
                room.target = obj;
            }
            if (room.action == 2 && room.target == 0x47 && (room.flags & 1) == 0) {
                if (WalkToHit())
                    break;
                PostRoomMsg(room.action, room.target, 0, 0);
                PlaySfx("communic");
                room.action = 1;
                break;
            }
            if (room.action == 4 || (room.action == 2 && (room.flags & 1) == 0))
                ShowInvIcon(room.action == 2);
            break;
        case EV_KEY:
            if (room.busy)
                break;
            switch (k = ((uint16_t)ev.key >> 8) & 0xFF) {
            case 0x1C:
            case 0x3B:
                goto click;
            case 1:
            case 0x39:
            case 0x3C:
                goto menu;
            case 0x11:
                HideInvIcon();
                room.action = 1;
                break;
            case 0x14:
                HideInvIcon();
                room.action = 5;
                goto action;
            case 0x26:
                HideInvIcon();
                room.action = 4;
                goto action;
            case 0x22:
                HideInvIcon();
                room.action = 3;
                goto action;
            case 0x16:
                HideInvIcon();
                room.action = 2;
                goto action;
            case 0x17:
                if (room.action == 2) {
                    HideInvIcon();
                    obj = PickItem(50, 50, 1);
                    goto use_item;
                } else if (room.action == 4) {
                    HideInvIcon();
                    obj = PickItem(50, 50, 1);
                    goto look_item;
                }
                break;
            }
        }
    }
    return 0;
}

/* Free the room and its actors. */
void FreeRoom(void)
{
    FadeOut();
    HideInvIcon();
    FreeActors();
    FreeAnyFile(rdf_handle);
    FreeAnyFile(map_handle);
}

/* A pointer into the .rdf. */
char *RdfPtr(uint16_t off)
{
    char *p;

    p = (char *)rdf;
    p += off;
    return p;
}

/* Play an animation scaled for its depth. */
void PlayScaled(int16_t n, char *name, int16_t x, int16_t y)
{
    int16_t scale;

    scale = DepthScale(y);
    StartAnim(n, name, x, y, scale);
}

/* The scale for depth y. */
int16_t DepthScale(int16_t y)
{
    rdf = LockMem(rdf_handle);
    y = min(rdf->y_max, y);
    y = max(rdf->y_min, y);
    y = ((int32_t)(y - rdf->y_min) * scale_k >> 8) + (uint32_t)rdf->scale_min;
    UnlockMem(rdf_handle);
    return y;
}

/* The priority at y. */
int16_t DepthPri(int16_t y)
{
    return ((unsigned char)mask_buf[y * 160] >> 4) & 0x0F;
}

/* Queue a room message. */
void PostRoomMsg(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    Msg *m;

    m = &msg_queue[msg_tail++];
    m->type = a;
    m->arg = b;
    m->b2 = c;
    m->b3 = d;
    msg_tail %= 16;
}

/* The next room message, as a long. */
int32_t PopRoomMsg(void)
{
    Msg *m;

    m = &msg_queue[msg_head];
    msg_head %= 16;
    return *(int32_t *)m;
}

/* A message trace, compiled out (an empty switch). */
void TraceMsg(int32_t msg)
{
    char unused[10];
    int32_t m;

    m = msg;
    switch (m) {
    }
}

/* Is there a script for this message? */
int16_t HasScript(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    uint32_t key;

    key = ((uint32_t)a & 0xFF) << 0 | ((uint32_t)b & 0xFF) << 8
        | ((uint32_t)c & 0xFF) << 16 | ((uint32_t)d & 0xFF) << 24;
    rdf = LockMem(rdf_handle);
    {
        Script *p;
        Script *end;

        p = (Script *)RdfPtr(rdf->scripts);
        end = (Script *)RdfPtr(rdf->scripts_end);
        while (p < end) {
            if (p->key == key) {
                UnlockMem(rdf_handle);
                return 1;
            }
            p = (Script *)RdfPtr(p->next);
        }
    }
    UnlockMem(rdf_handle);
    return 0;
}

/* Run the scripts for this message. */
int16_t RunScripts(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    uint32_t key;

    key = ((uint32_t)a & 0xFF) << 0 | ((uint32_t)b & 0xFF) << 8
        | ((uint32_t)c & 0xFF) << 16 | ((uint32_t)d & 0xFF) << 24;
    rdf = LockMem(rdf_handle);
    {
        Script *p;
        Script *end;

        p = (Script *)RdfPtr(rdf->scripts);
        end = (Script *)RdfPtr(rdf->scripts_end);
        while (p < end) {
            if (p->key == key) {
                room.more = 0;
                CallScript(p->code);
                if (room.more == 0) {
                    UnlockMem(rdf_handle);
                    return 1;
                }
            }
            p = (Script *)RdfPtr(p->next);
        }
    }
    UnlockMem(rdf_handle);
    return 0;
}

/* As RunScripts, with FF bytes in a script's key matching anything. */
int16_t RunWildScripts(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    uint32_t key;

    key = ((uint32_t)a & 0xFF) << 0 | ((uint32_t)b & 0xFF) << 8
        | ((uint32_t)c & 0xFF) << 16 | ((uint32_t)d & 0xFF) << 24;
    rdf = LockMem(rdf_handle);
    {
        Script *p;
        Script *end;
        uint32_t mask;
        uint32_t k;

        p = (Script *)RdfPtr(rdf->scripts);
        end = (Script *)RdfPtr(rdf->scripts_end);
        while (p < end) {
            k = p->key;
            mask = (k & 0xFF000000L) == 0xFF000000L ? 0 : 0xFF000000L;
            mask |= (k & 0x00FF0000L) == 0x00FF0000L ? 0 : 0x00FF0000L;
            mask |= (k & 0x0000FF00L) == 0x0000FF00L ? 0 : 0x0000FF00L;
            mask |= (k & 0x000000FFL) == 0x000000FFL ? 0 : 0x000000FFL;
            if ((p->key & mask) == (key & mask)) {
                room.more = 0;
                CallScript(p->code);
                if (room.more == 0) {
                    UnlockMem(rdf_handle);
                    return 1;
                }
            }
            p = (Script *)RdfPtr(p->next);
        }
    }
    UnlockMem(rdf_handle);
    return 0;
}

/* Handle the next room message: scripts first, then the defaults. */
void RoomMsg(void)
{
    Msg *m;

    m = &msg_queue[msg_head++];
    msg_head %= 16;
    if ((m->type == 12 || m->type == 10) && m->arg == 0xFF) {
        if (room.busy == 1)
            room.busy = 0;
        exits_open = 1;
        return;
    }
    if (m->type == 12 && m->arg >= 0xE0) {
        int16_t n;

        n = m->arg - 0xE0;
        PostRoomMsg(timers[n].msg[0], timers[n].msg[1], timers[n].msg[2], timers[n].msg[3]);
        timers[n].active = 0;
        return;
    }
    if (RunScripts(m->type, m->arg, m->b2, m->b3))
        return;
    switch (m->type) {
    case 1: {
        char buf[16];

        if (RunWildScripts(m->type, m->arg, m->b2, m->b3))
            break;
        CrewAnim(buf, 0, "walk");
        StartWalk(0, buf, actors[0].x, actors[0].y, mouse_x, mouse_y);
        break;
    }
    case 6: {
        uint16_t n;

        if (RunWildScripts(m->type, m->arg, m->b2, m->b3))
            break;
        rdf = LockMem(rdf_handle);
        n = rdf->exit_room[m->arg];
        UnlockMem(rdf_handle);
        FreeRoom();
        LoadMusic("ground");
        LoadRoom(room_mission, n);
        EnterRoom(m->arg ^ 1);
        break;
    }
    case 4:
        if (0x40 <= m->arg && m->arg < 0x89)
            MsgBox("", GetText(items[m->arg - 0x40].text), 20, 20, 0xB0, 0);
        else
            switch (m->arg) {
            case 0:
                MsgBox("", GetText(0x49), 20, 20, 0xB0, 0);
                break;
            case 1:
                MsgBox("", GetText(0x4A), 20, 20, 0xB0, 0);
                break;
            case 2:
                MsgBox("", GetText(0x4B), 20, 20, 0xB0, 0);
                break;
            case 3:
                MsgBox("", GetText(0x4C), 20, 20, 0xB0, 0);
            default:
                MsgBox("", GetText(0x4D), 20, 20, 0xB0, 0);
                break;
            }
        break;
    case 5:
        switch (m->arg) {
        case 0:
        case 1:
        case 2:
        case 3:
            MsgBox("", GetText(0x4E), 20, 20, 0xB0, 0);
            break;
        default:
            MsgBox("", GetText(0x4F), 20, 20, 0xB0, 0);
            break;
        }
        break;
    case 2:
        if (m->arg == m->b2)
            break;
        switch (m->arg) {
        case 0:
            if (!RunScripts(1, m->b2, 0, 0) && !RunScripts(3, m->b2, 0, 0))
                MsgBox("Capt. Kirk", GetText(0x50), 20, 20, 0xB0, 0);
            break;
        case 1:
            if (!RunScripts(2, 0x44, m->b2, 0))
                MsgBox("Spock", GetText(0x51), 20, 20, 0xC0, 0);
            break;
        case 2:
            if (!RunScripts(2, 0x7F, m->b2, 0) && !RunScripts(2, 0x45, m->b2, 0))
                MsgBox("McCoy", GetText(0x52), 20, 20, 0xC0, 0);
            break;
        case 3:
            MsgBox("", GetText(0x53), 20, 20, 0xB0, 0);
            break;
        case 0x44:
            MsgBox("Spock", GetText(0x54), 20, 20, 0xC0, 0);
            break;
        case 0x45:
            MsgBox("McCoy", GetText(0x55), 20, 20, 0xC0, 0);
            break;
        case 0x7F:
            MsgBox("McCoy", GetText(0x56), 20, 20, 0xC0, 0);
            break;
        case 0x40:
        case 0x41: {
            char *who;
            int16_t *tab;
            int16_t color;
            int16_t r;
            char *text;

            switch (m->b2) {
            case 2:
                tab = mccoy_lines;
                r = (uint16_t)rand() % 8;
                who = "McCoy";
                color = 0xC0;
                text = GetText(tab[r]);
                break;
            case 1:
                tab = spock_lines;
                r = (uint16_t)rand() % 8;
                who = "Spock";
                color = 0xC0;
                text = GetText(tab[r]);
                break;
            case 3:
                tab = redshirt_lines;
                r = (uint16_t)rand() % 8;
                who = "Security Officer";
                color = 0xA1;
                text = GetText(tab[r]);
                text[6] = mission[0];
                text[7] = mission[1];
                text[8] = mission[2];
                break;
            default:
                if (RunWildScripts(m->type, m->arg, m->b2, m->b3))
                    return;
                tab = misc_lines;
                r = (uint16_t)rand() % 7;
                who = r & 1 ? "McCoy" : "Spock";
                color = r & 1 ? 0xC0 : 0xC0;
                text = GetText(tab[r]);
                break;
            }
            MsgBox(who, (char *)text, 20, 20, color, 0);
            break;
        }
        case 0x47:
            if (!RunScripts(2, 0x47, 0xFF, 0)) {
                PlaySfx("communic");
                MsgBox("Uhura", GetText(0x76), 20, 20, 0xA1, 0);
            }
            break;
        default:
            if (RunWildScripts(m->type, m->arg, m->b2, m->b3))
                return;
            MsgBox("", GetText(0x77), 20, 20, 0xB0, 0);
            break;
        }
        break;
    case 3:
        if (RunWildScripts(m->type, m->arg, m->b2, m->b3))
            break;
        MsgBox("", GetText(0x78), 20, 20, 0xB0, 0);
        break;
    default:
        RunWildScripts(m->type, m->arg, m->b2, m->b3);
        break;
    }
}

/* Run a script: code is where it sits in the room file (the offset the
 * machine code ran from); room_scripts has the C that replaces it. */
void CallScript(void *code)
{
    uint16_t offset = (char *)code - (char *)rdf;
    const RoomScript *p;

    rdf_image = (uint8_t *)rdf;
    for (p = room_scripts; p->fn; p++)
        if (p->offset == offset) {
            p->fn();
            return;
        }
    Fatal("room %s has no script at %04X", room_name, offset);
}

/* The scripts' service call (rdf->service). */
int32_t ScriptService(int16_t cmd, int16_t a1, int16_t a2, int16_t a3, int16_t a4, uint16_t a5)
{
    int32_t r;
    Actor *a;

    r = 0;
    UnlockMem(rdf_handle);
    switch (cmd) {
    case 0:
    case 2: {
        char name[10];

        a = actors;
        if (a3 == -1 || a4 == -1) {
            a3 = actors[a1].sprite.x;
            a4 = actors[a1].sprite.y;
        }
        rdf = LockMem(rdf_handle);
        MemCpy(name, RdfPtr(a2), 10);
        UnlockMem(rdf_handle);
        if (a1 >= 0 && a1 <= 7) {
            PlayScaled(a1, name, a3, a4);
            a += a1;
        } else
            a += StartAnim(a1, name, a3, a4, 256);
        if (a5) {
            a->notify = 1;
            a->notify_arg = a5;
        }
        break;
    }
    case 1:
        if (room.redshirt_dead && a1 == 3)
            EndAnim(a1);
        else
            EndOrStand(a1);
        break;
    case 3:
        r = Converse(a1, 20, 20);
        break;
    case 4:
        r = ItemGive(a1);
        break;
    case 5:
        r = LeverPuzzle();
        break;
    case 6:
        if (room.flags)
            break;
        FreeRoom();
        LoadMusic("ground");
        LoadRoom(room_mission, a1);
        EnterRoom(a2 % 6);
        longjmp(room_jmp, 1);
    case 7:
        ItemTake(a1);
        if (room.action == 2 && a1 == room.target) {
            room.action = 1;
            ActionCursor(1, 0);
            HideInvIcon();
        }
        break;
    case 8: {
        char buf[14];
        int16_t unused;
        Actor *b;
        int16_t ok;

        b = &actors[a1];
        if (a1 >= 0 && a1 <= 3) {
            CrewAnim(buf, a1, "walk");
            ok = StartWalk(a1, buf, b->x, b->y, a2, a3);
            if (ok && a4) {
                b->notify = 1;
                b->notify_arg = (uint16_t)a4;
            }
        } else
            Fatal("Tried to walk a non PC");
        break;
    }
    case 10: {
        char name[12];

        rdf = LockMem(rdf_handle);
        MemCpy(name, RdfPtr(a1), 10);
        UnlockMem(rdf_handle);
        FlashBitmap(name, 5);
        break;
    }
    case 9:
        FreeAnyFile(map_handle);
        rdf = LockMem(rdf_handle);
        MemCpy(room_file, RdfPtr(a1), 16);
        UnlockMem(rdf_handle);
        map_handle = getanyfile(room_file, "map");
        LoadWalkGraph(room_file);
        break;
    case 11:
        r = ItemHave(a1);
        break;
    case 12:
        r = ItemCase(a1);
        break;
    case 13:
        a = &actors[a1];
        r = a->sprite.x | (int32_t)a->sprite.y << 16;
        break;
    case 14:
        r = rand() % (a2 - a1 + 1) + a1;
        break;
    case 21: {
        char name[12];

        rdf = LockMem(rdf_handle);
        MemCpy(name, RdfPtr(a1), 10);
        UnlockMem(rdf_handle);
        PlaySfx(name);
        break;
    }
    case 23:
        DigiShutdown();
        break;
    case 15:
        BridgeSfx(a1);
        break;
    case 16:
        PlayMusic((uint16_t)a1, (uint16_t)a2);
        break;
    case 17:
        word_50CA6 = a1;
        word_52C1E = a2;
        word_51156 = a3;
        BeamUp();
        break;
    case 18:
        GameOverMenu();
    case 19: {
        char buf[32];
        char *p;
        char *code;
        int16_t i;

        rdf = LockMem(rdf_handle);
        MemCpy(buf, RdfPtr(a1), 30);
        UnlockMem(rdf_handle);
        p = buf;
        code = InputCode();
        i = 1;
        r = 0;
        while (*p) {
            if (strcmp(p, code) == 0)
                r = i;
            p += strlen(p) + 1;
            ++i;
        }
        break;
    }
    case 20:
        r = Turbolift(a1, a2);
        break;
    case 22:
    default:
        Fatal("unknown room cmd");
    }
    rdf = LockMem(rdf_handle);
    return r;
}

/* Centre src in a 24 character line. */
void CentreLine(char *dst, char *src)
{
    int16_t len;

    MemSet(dst, 24, ' ');
    len = strlen(src);
    if (len > 24)
        len = 24;
    MemCpy(dst + (24 - len) / 2, src, len);
}

/* A choice's title: the speaker, numbered when there are choices. */
void ChoiceTitle(char *dst, char *name, int16_t n)
{
    char buf[44];
    int16_t len;

    len = _fstrlen(name) + 1;
    MemCpy(buf, name, min(len, 25));
    buf[24] = 0;
    if (n) {
        sprintf(buf + strlen(buf), " choice %d", n);
        buf[24] = 0;
    }
    CentreLine(dst, buf);
}

/* Choice n of a conversation (for TextBox): its text, and its title
 * in title. */
char *TalkChoice(uint16_t n, int16_t *list, char *title)
{
    uint16_t *entry;
    char *str;
    char *buf;
    int16_t len;

    rdf = LockMem(rdf_handle);
    entry = (uint16_t *)RdfPtr(*list);
    if (entry[n + 1] == 0) {
        UnlockMem(rdf_handle);
        return 0;
    }
    str = RdfPtr(entry[n + 1]);
    len = _fstrlen(str) + 1;
    buf = AllocMemPtr(len);
    MemCpy(buf, str, len);
    if (entry[0] && (str = RdfPtr(entry[0]), *str)) {
        if (entry[2])
            ChoiceTitle(title, str, n + 1);
        else
            ChoiceTitle(title, str, 0);
    } else
        *title = 0;
    UnlockMem(rdf_handle);
    return buf;
}

/* A conversation from the .rdf. Returns the choice. */
int32_t Converse(int16_t text, int16_t x, int16_t y)
{
    uint16_t *entry;
    char name[26];
    int16_t color;
    int16_t list;

    rdf = LockMem(rdf_handle);
    entry = (uint16_t *)RdfPtr(text);
    if (entry[0])
        MemCpy(name, RdfPtr(entry[0]), 25);
    if (entry[0] && name[0]) {
        if (strcmpi(name, "Capt. Kirk") == 0)
            color = 0xB0;
        else if (strcmpi(name, "Mr. Spock") == 0)
            color = 0xC0;
        else if (strcmpi(name, "Dr. McCoy") == 0)
            color = 0xC0;
        else if (strcmpi(name, "Mr. Chekov") == 0)
            color = 0xB0;
        else if (strcmpi(name, "Mr. Scott") == 0)
            color = 0xA1;
        else if (strnicmp(name, "Lt", 2) == 0)
            color = 0xA1;
        else if (strnicmp(name, "Ensign", 6) == 0)
            color = 0xA1;
        else
            color = 0x88;
    } else
        color = 0xB0;
    UnlockMem(rdf_handle);
    list = text;
    return TextBox(TalkChoice, &list, x, y, color, 1, 0, 0);
}

typedef struct TextArg {
    char title[24];
    char *text;
} TextArg;

/* The single choice of a message box (for TextBox). */
char *MsgChoice(uint16_t n, TextArg *arg, char *title)
{
    int16_t len;
    char *buf;

    if (n)
        return 0;
    MemCpy(title, arg, 24);
    len = strlen(arg->text) + 1;
    buf = AllocMemPtr(len);
    MemCpy(buf, arg->text, len);
    return buf;
}

/* A message box: title (a speaker) and text. Then post msg. */
void MsgBox(char *title, char *text, int16_t x, int16_t y, int16_t color, int16_t msg)
{
    TextArg arg;
    int16_t n;

    if (title && *title)
        CentreLine(arg.title, title);
    else
        arg.title[0] = 0;
    arg.text = text;
    n = msg > 0 ? msg : 0;
    msg = msg < 0 ? -msg : 0;
    TextBox(MsgChoice, &arg, x, y, color, 0, msg, 0);
    FreeText(text);
    SendScript(n);
}

/* Skip a #voice# tag. */
char *SkipVoice(char *p)
{
    char *q;

    q = p;
    if (*q == '#') {
        for (++q; *q != '#'; ++q)
            if (*q == 0)
                return p;
        ++q;
    }
    return q;
}

/* Play a #voice# tag; returns the text after it. */
char *PlayVoiceTag(char *p)
{
    char *q;
    char name[256];
    int16_t i;

    q = p;
    if (*q == '#') {
        i = 0;
        for (++q; *q != '#'; ++q, ++i) {
            if (*q == 0 || i > 250)
                return p;
            name[i] = *q;
        }
        ++q;
        name[i] = 0;
        PlayVoc(name);
    }
    return q;
}

/* A conversation box: fn gives the choices. Returns the one picked, -1
 * if cancelled (flag). */
int32_t TextBox(void *fn, void *arg, int16_t x, int16_t y,
               int16_t color, int16_t choice, int16_t maxl, int16_t flag)
{
    char *lines;
    char buf[24];
    int16_t ntitled;
    int16_t n;
    int16_t maxlines;
    int16_t sel;
    int16_t nlines;
    char *text;
    int16_t top;
    Sprite box;
    Bitmap *boxbuf;
    char unused[14];
    int16_t done;
    int16_t mx;
    int16_t my;
    MemHandle bmp;
    int16_t kbd;
    int16_t vw;
    uint32_t timeout;

    vw = voc_wait;
    timeout = blink_count > last_talk + 1 ? 16 : 8;
    ntitled = 0;
    maxlines = 0;
    for (n = 0; (text = ((char *(*)())fn)(n, arg, buf)) != 0; n++) {
        int16_t r;

        r = WrapCount(SkipVoice(text));
        if (r > maxlines)
            maxlines = r;
        if (buf[0])
            ntitled++;
        FreeMemPtr(text);
    }
    if (maxl == 0 || maxl > 12)
        maxl = 12;
    if (maxlines > maxl)
        maxlines = maxl;
    if (ntitled && ntitled != n)
        Fatal("not all choices have titles");
    boxbuf = MakeTextBox(&x, &y, color, maxlines, ntitled, &box);
    sel = 0;
    top = 0;
    if (vw == 0 || vw == 1 || n != 1 || sfx_on == 0 || digi_ok == 0)
        word_52C14 = 1;
    else
        word_52C14 = 0;
    if ((vw == 0 || vw == 1 || vw == 2) && sfx_on == 1 && digi_ok == 1)
        word_53798 = 1;
    else
        word_53798 = 0;
    lines = LayoutChoice(fn, arg, sel, boxbuf, maxlines, &nlines);
    if (lines == 0) {
        DelSprite(&box);
        for (done = 0; !done; ) {
            int16_t r;

            r = MenuWait(timeout, 1);
            if (timeout)
                MenuEnable(1L);
            switch (r) {
            case -4:
            case -2:
                if (timeout)
                    break;
                done = 1;
                if (flag)
                    sel = -1;
                break;
            case 9:
                if (n == 1)
                    done = 1;
                break;
            case 0:
                done = 1;
                break;
            }
            timeout = 0;
        }
        FreeMemPtr(boxbuf);
    } else {
        LoadMenu("textbtns", x + 150, y - 17);
        mx = mouse_x;
        my = mouse_y;
        bmp = mouse_sprite.handle;
        mouse_x = x + 0xDE;
        mouse_y = y - 8;
        SpriteSetPos(&mouse_sprite, mouse_x, mouse_y, 15);
        SpriteSetBitmap(&mouse_sprite, LoadBitmap("pushbtn"));
        SetPointer(mouse_x, mouse_y);
        kbd = kbd_mouse;
        kbd_mouse = 0;
        MenuShow((nlines > maxlines ? 6L : 0L) | 1 | (n > 1 ? 0x18L : 0L));
        MenuDisable(2L);
        if (timeout)
            MenuDisable(1L);
        if (choice == 0)
            MenuDisable(8L);
        for (done = 0; !done; ) {
            int16_t r;

            r = MenuLoop(timeout, 1);
            if (timeout)
                MenuEnable(1L);
            switch (r) {
            case 3:
                --sel;
                if (choice == 0 && sel == 0)
                    MenuDisable(8L);
                else
                    sel = sel < 0 ? n - 1 : sel;
                MenuEnable(16L);
                goto redraw;
            case -4:
            case -2:
                if (timeout)
                    break;
                done = 1;
                if (flag)
                    sel = -1;
                break;
            case 4:
                MenuEnable(8L);
                ++sel;
                if (choice == 0 && sel == n - 1)
                    MenuDisable(16L);
                else
                    sel %= n;
            redraw:
                FreeMemPtr(lines);
                top = 0;
                lines = LayoutChoice(fn, arg, sel, boxbuf, maxlines, &nlines);
                if (nlines <= maxlines)
                    MenuShow(0x19L);
                else
                    MenuShow(0x1FL);
                MenuEnable(4L);
                MenuDisable(2L);
                box.dirty = 1;
                break;
            case 5:
                top--;
                goto up;
            case 7:
                top = 0;
                goto up;
            case 1:
                top -= maxlines;
            up:
                MenuEnable(4L);
                top = max(0, top);
                if (top == 0)
                    MenuDisable(2L);
                goto draw;
            case 6:
                top++;
                goto down;
            case 8:
                top = nlines - maxlines;
                goto down;
            case 2:
                top += maxlines;
            down:
                MenuEnable(2L);
                if (top >= nlines)
                    top -= maxlines;
                top = min(nlines - 1, top);
                if (top + maxlines >= nlines)
                    MenuDisable(4L);
            draw:
                box.dirty = 1;
                BoxLines(boxbuf, nlines - top, maxlines, lines + top * 24, ntitled);
                break;
            case 9:
                if (n == 1)
                    done = 1;
                break;
            case 0:
                done = 1;
                break;
            }
            timeout = 0;
        }
        mouse_x = mx;
        mouse_y = my;
        SpriteSetPos(&mouse_sprite, mouse_x, mouse_y, 15);
        ReleaseBitmap(mouse_sprite.handle);
        SpriteSetBitmap(&mouse_sprite, bmp);
        SetPointer(mouse_x, mouse_y);
        kbd_mouse = kbd;
        MenuClose();
        {
            MemHandle h;

            h = box.handle;
            SpriteErase(&box);
            UpdateSprites();
            DelSprite(&box);
            FreeMemHandle(h);
        }
        FreeMemPtr(boxbuf);
        FreeMemPtr(lines);
    }
    last_talk = blink_count;
    StopVoc();
    return sel;
}

/* The number of 24 character lines s wraps to. */
int16_t WrapCount(char *s)
{
    char buf[26];
    int16_t n;

    for (n = 0; (s = WrapLine(s, buf, 24)) != 0; n++)
        ;
    return n;
}

/* Wrap s into 24 character lines at dst. */
void WrapText(char *dst, char *s)
{
    char buf[26];
    int16_t i;

    s = WrapLine(s, buf, 24);
    while (s) {
        for (i = strlen(buf); i != 24; ++i)
            buf[i] = ' ';
        MemCpy(dst, buf, 24);
        dst += 24;
        s = WrapLine(s, buf, 24);
    }
}

/* Make the text box sprite: a frame of font characters, placed near
 * (*px, *py) and on screen. */
Bitmap *MakeTextBox(int16_t *px, int16_t *py, int16_t color, int16_t lines, uint16_t titled, Sprite *spr)
{
    int16_t unused;
    Bitmap *bmp;
    char *p;
    int16_t i;
    int16_t extra;
    int16_t t;
    int16_t x;
    int16_t y;

    extra = titled ? 4 : 2;
    x = *px;
    y = *py;
    bmp = AllocMemPtr((lines + extra) * 26 + 8);
    MemSet(spr, sizeof(Sprite), 0);
    spr->z = 15;
    spr->layer = 8;
    spr->handle = AllocMemHandle(bmp);
    spr->colour = color;
    p = bmp->data;
    bmp->x = 0;
    bmp->y = 0;
    bmp->w = 0xD0;
    bmp->h = (lines + extra) * 8;
    MemSet(p, (lines + extra) * 26, ' ');
    if ((t = 319 - x - (bmp->w + 29) / 2) < 0)
        x += t;
    if ((t = x - (bmp->w + 29) / 2) < 1)
        x -= t - 1;
    if ((t = y - (bmp->h + 17) - 20) < 0)
        y -= t;
    x -= (bmp->w + 29) / 2;
    y -= bmp->h;
    *p = 0x10;
    MemSet(p + 1, 24, 0x11);
    p[25] = 0x12;
    p += 26;
    if (titled) {
        *p = 0x13;
        p[25] = 0x14;
        p += 26;
        *p = 0x13;
        MemSet(p + 1, 24, 0x19);
        p[25] = 0x14;
        p += 26;
    }
    for (i = lines; i != 0; --i) {
        *p = 0x13;
        p[25] = 0x14;
        p += 26;
    }
    *p = 0x15;
    MemSet(p + 1, 24, 0x16);
    p[25] = 0x17;
    p += 26;
    AddSprite(spr);
    spr->type = SPRITE_TEXT;
    SpriteSetPos(spr, x, y, 15);
    *px = x;
    *py = y;
    return bmp;
}

/* Put n lines of src in the box (blank the rest of max). */
void BoxLines(Bitmap *box, int16_t n, int16_t max, char *src, uint16_t titled)
{
    int16_t i;
    char *p;

    p = (char *)box + 35;
    n = min(n, max);
    if (titled)
        p += 52;
    for (i = 0; i != n; i++) {
        MemCpy(p, src, 24);
        src += 24;
        p += 26;
    }
    for (; i != max; i++) {
        MemSet(p, 24, ' ');
        p += 26;
    }
}

/* Put the title (a speaker) on the box's top line. */
void BoxTitle(Bitmap *box, char *title)
{
    char *p;

    if (*title) {
        p = (char *)box + 35;
        MemCpy(p, title, 24);
    }
}

/* Lay out choice sel in the box, speaking its voice tag. Returns the
 * lines (0 when only spoken). */
char *LayoutChoice(void *fn, void *arg, uint16_t sel,
                Bitmap *box, int16_t max, int16_t *nlines)
{
    char *text;
    char *q;
    char *out;
    char title[24];

    text = ((char *(*)())fn)(sel, arg, title);
    if (voc_wait == 2 && sfx_on == 1 && digi_ok == 1) {
        q = PlayVoiceTag(text);
        if (q == text)
            word_52C14 = 1;
    } else if ((voc_wait == 1 || voc_wait == 0) && sfx_on == 1 && digi_ok == 1)
        q = PlayVoiceTag(text);
    else
        q = SkipVoice(text);
    if (word_52C14) {
        *nlines = WrapCount(q);
        out = AllocMemPtr(*nlines * 24);
        WrapText(out, q);
        BoxLines(box, *nlines, max, out, title[0]);
        BoxTitle(box, title);
        FreeMemPtr(text);
        return out;
    } else {
        FreeMemPtr(text);
        return 0;
    }
}

/* The pointer for action (highlighted when on something). */
void ActionCursor(int16_t action, int16_t on)
{
    char *name;

    switch (action) {
    case 2:
        name = on ? "useh" : "usen";
        break;
    case 3:
        name = on ? "geth" : "getn";
        break;
    case 4: {
        static char *look_cursors[8] = {
            "lookh0", "lookh0", "lookh0", "lookh0", "lookh0", "lookh1", "lookh2", "lookh3"
        };
        static int16_t look_idx = 0;

        if (on) {
            if ((rand() & 7) == 0)
                look_idx = rand() & 7;
            name = look_cursors[look_idx];
        } else
            name = "lookn";
        break;
    }
    case 5:
        if (on) {
            if (rand() & 3)
                name = "talkh0";
            else
                name = "talkh1";
        } else
            name = "talkn";
        break;
    case 13:
        name = "options";
        break;
    default:
        name = "walk";
        break;
    }
    ReleaseBitmap(mouse_sprite.handle);
    SpriteSetBitmap(&mouse_sprite, LoadBitmap(name));
}

/* What's at (x, y): a crew member or other actor, the inventory icon,
 * the selected item, an object polygon, or -1. */
int16_t FindObject(int16_t x, int16_t y)
{
    Sprite *s;
    int16_t *e;
    int16_t *end;
    int16_t *q;
    int16_t mask;
    int16_t i;
    Actor *a;

    s = SpriteAt(x, y);
    if (s != 0) {
        if (s == &inv_sprite)
            return 0x1F;
        if (s == &item_sprite)
            return room.target;
        for (i = 0, a = actors; i != 32; ++i, ++a)
            if (s == &a->sprite)
                return i;
        Fatal("FindObject: clicked on unknown sprite.");
    }
    rdf = LockMem(rdf_handle);
    e = (int16_t *)RdfPtr(rdf->objects);
    end = (int16_t *)RdfPtr(rdf->objects_end);
    hit_valid = 0;
    mask = 1 << (room.action - 1);
    while (e != end) {
        q = e;
        if (*q & 0x8000) {
            if ((*q & mask) && InPoly(q + 3, x, y)) {
                i = q[3];
                hit_valid = 1;
                hit_x = q[1];
                hit_y = q[2];
                UnlockMem(rdf_handle);
                return i;
            }
            e = NextPoly(q + 3);
        } else if (InPoly(e, x, y) == 1) {
            i = *e;
            UnlockMem(rdf_handle);
            return i;
        } else
            e = NextPoly(e);
    }
    UnlockMem(rdf_handle);
    return -1;
}

/* Is (x, y) inside the convex polygon poly (id, n, points)? */
int16_t InPoly(int16_t *poly, int16_t x, int16_t y)
{
    int16_t i;
    int16_t n;
    Point *p;
    Point e;
    Point d;
    Point a;

    p = (Point *)(poly + 2);
    n = poly[1] - 1;
    for (i = 0; i != n; ++i, ++p) {
        a = p[0];
        e = p[1];
        e.x -= a.x;
        e.y -= a.y;
        d.x = x - a.x;
        d.y = y - a.y;
        if (e.x * d.y - e.y * d.x < 0)
            return 0;
    }
    a = p[0];
    e = *(Point *)(poly + 2);
    e.x -= a.x;
    e.y -= a.y;
    d.x = x - a.x;
    d.y = y - a.y;
    if (e.x * d.y - e.y * d.x < 0)
        return 0;
    return 1;
}

/* The polygon after poly. */
int16_t *NextPoly(int16_t *poly)
{
    char *p;

    p = (char *)poly;
    p += poly[1] * 4 + 4;
    return (int16_t *)p;
}

/* Report the region, else the exit, the pointer moves into. */
void CheckRegions(int16_t x, int16_t y)
{
    uint16_t *e;
    uint16_t *end;

    rdf = LockMem(rdf_handle);
    e = (uint16_t *)RdfPtr(rdf->regions);
    end = (uint16_t *)RdfPtr(rdf->regions_end);
    while (e != end) {
        if (InPoly(e, x, y) == 1) {
            if (*e != cur_region) {
                cur_region = *e;
                PostRoomMsg(7, *e & 0xFF, 0, 0);
            }
            UnlockMem(rdf_handle);
            return;
        }
        e = NextPoly(e);
    }
    cur_region = -1;
    if (room.flags == 0 && exits_open) {
        e = (uint16_t *)RdfPtr(rdf->exits);
        end = (uint16_t *)RdfPtr(rdf->exits_end);
        while (e != end) {
            if (InPoly(e, x, y) == 1) {
                if (*e != cur_exit) {
                    cur_exit = *e;
                    PostRoomMsg(6, *e & 0xFF, 0, 0);
                }
                UnlockMem(rdf_handle);
                return;
            }
            e = NextPoly(e);
        }
    }
    cur_exit = -1;
    UnlockMem(rdf_handle);
}

/* Count down the room timers. */
void RoomTimers(void)
{
    int16_t i;

    for (i = 0; i != 8; i++)
        if (room.timers[i] && --room.timers[i] == 0)
            PostRoomMsg(8, i & 0xFF, 0, 0);
}

/* Is (x, y) blocked in the walk map? */
int16_t WalkBlocked(int16_t x, int16_t y)
{
    unsigned char bit;
    int16_t off;
    int16_t r;
    unsigned char *map;

    map = LockMem(map_handle);
    bit = 0x80 >> (x & 7);
    off = y * 40;
    off += x >> 3;
    r = map[off] & bit;
    UnlockMem(map_handle);
    return r;
}

/* Pick what to use the selected object on. Returns it or -1. */
int16_t PickUseTarget(void)
{
    Event ev;
    int16_t obj;

    for (;;) {
        if ((room.flags & 1) == 0)
            ShowInvIcon(0);
        ev = WaitAction();
        if (ev.type == EV_KEY && (((uint16_t)ev.key >> 8) & 0xFF) == 0x17) {
            HideInvIcon();
            obj = PickItem(50, 50, 1);
            if (obj == -1)
                continue;
            return obj;
        } else if (ev.type != EV_DOWN1) {
            HideInvIcon();
            return -1;
        } else {
            obj = FindObject(mouse_x, mouse_y);
            HideInvIcon();
            if (obj == -1)
                continue;
            if (CantAct(obj, 2))
                continue;
        }
        if (obj == 0x1F) {
            obj = PickItem(50, 50, 0);
            if (obj == -1)
                continue;
        } else if (obj > 3) {
            if (CantAct(0, 2))
                continue;
            if (!HasScript(2, 0, obj & 0xFF, 0) && !HasScript(3, obj & 0xFF, 0, 0)
                && !HasScript(1, obj & 0xFF, 0, 0))
                continue;
            room.target = 0;
            room.target2 = obj;
            room.action = 2;
            obj = 0;
            if (!WalkToHit())
                PostRoomMsg(room.action, room.target, room.target2, 0);
        }
        return obj;
    }
}

/* Wait for a click or an action key. */
Event WaitAction(void)
{
    Event *e;

    for (;; PopEvent()) {
        if ((e = PeekEvent()) != 0) {
            switch (e->type) {
            case EV_TICK:
                HiliteCursor();
                UpdateSprites();
                MusicTick();
                break;
            case EV_MOVE:
                SpriteSetPos(&mouse_sprite, e->x, e->y, 15);
                break;
            case EV_DOWN1:
                PopEvent();
                return *e;
            case EV_DOWN2:
                return *e;
            case EV_KEY:
                switch (((uint16_t)e->key >> 8) & 0xFF) {
                case 0x17:
                    PopEvent();
                    return *e;
                case 0x1C:
                case 0x3B:
                    PopEvent();
                    e->type = EV_DOWN1;
                    return *e;
                case 1:
                case 0x11:
                case 0x14:
                case 0x16:
                case 0x22:
                case 0x26:
                case 0x39:
                case 0x3C:
                    return *e;
                default:
                    break;
                }
                break;
            }
        }
    }
}

char crew_letters[6] = "ksmr";
int16_t action_masks[5] = { 0, 7, 6, 7, 3 };

/* Crew member crew's animation name for what. */
void CrewAnim(char *buf, int16_t crew, char *what)
{
    sprintf(buf, "%c%s", crew_letters[crew], what);
}

/* Highlight the pointer when it's over something the action applies to. */
void HiliteCursor(void)
{
    int16_t mask;
    int16_t obj;
    int16_t on;

    mask = action_masks[room.action - 1];
    obj = FindObject(mouse_x, mouse_y);
    on = (obj >= 0 && obj <= 3 && (mask & 1)) || (obj > 3 && obj < 32 && (mask & 2))
        || (obj >= 32 && obj < 64 && (mask & 4));
    ActionCursor(room.action, on);
}

/* Walk the crew member to the spot clicked, with the action posted
 * when they get there. Returns 0 if the spot isn't a walk-to spot. */
int16_t WalkToHit(void)
{
    int16_t crew;
    uint16_t timer;
    int16_t i;
    char buf[16];

    if (hit_valid) {
    if (room.action != 2)
        crew = 0;
    else if (room.action == 2 && room.target >= 0 && room.target <= 3)
        crew = room.target;
    else if (room.action == 2 && room.target >= 0x40 && room.target <= 0x89) {
        if (room.target == 0x44)
            crew = 1;
        else if (room.target == 0x45)
            crew = 2;
        else
            crew = 0;
    } else
        Fatal("Jay didn't think about pmcheck.");
    timer = 0;
    if (room.action != 1) {
        Script *e;
        Script *end;
        uint32_t key;
        int16_t found;

        found = 0;
        rdf = LockMem(rdf_handle);
        e = (Script *)RdfPtr(rdf->scripts);
        end = (Script *)RdfPtr(rdf->scripts_end);
        key = room.action | (uint32_t)room.target << 8;
        if (room.action == 2)
            key |= (uint32_t)room.target2 << 16;
        while (e < end) {
            if (e->key == key) {
                for (i = 0; i != 31; i++)
                    if (timers[i].active == 0) {
                        timer = i + 0xE0;
                        timers[i].active = timer;
                        timers[i].msg[0] = room.action;
                        timers[i].msg[1] = room.target;
                        if (room.action == 2)
                            timers[i].msg[2] = room.target2;
                        else
                            timers[i].msg[2] = 0;
                        timers[i].msg[3] = 0;
                        found = 1;
                        break;
                    }
                break;
            }
            e = (Script *)RdfPtr(e->next);
        }
        UnlockMem(rdf_handle);
        if (found == 0) {
            hit_valid = 0;
            return 0;
        }
    }
    CrewAnim(buf, crew, "walk");
    StartWalk(crew, buf, actors[crew].x, actors[crew].y, hit_x, hit_y);
    if (timer) {
        actors[crew].notify = 1;
        actors[crew].notify_arg = timer;
    }
    hit_valid = 0;
    return 1;
    } else
        return 0;
}

/* Can't act on obj (a stunned or dead crew member)? */
int16_t CantAct(int16_t obj, int16_t action)
{
    if (action == 4)
        return 0;
    if (obj == 3 && room.redshirt_dead)
        return 1;
    if (obj >= 0 && obj <= 3 && ((1 << obj) & room.flags))
        return 1;
    if (obj == 0x45 && (room.flags & 4))
        return 1;
    if (obj == 0x44 && (room.flags & 2))
        return 1;
    return 0;
}

/* Stunned crew members get up when their time is up. */
void StunTick(void)
{
    int16_t bit;
    int16_t i;
    char buf[10];
    int16_t dx;
    int16_t dy;
    int16_t scale;

    if (room.flags == 0)
        return;
    for (bit = 1, i = 0; i <= 3; ++i, bit <<= 1) {
        if (room.flags & bit) {
        if (--room.stun[i] <= 0) {
        CrewAnim(buf, i, "getu");
        if (room.fall[i] == 0xFF)
            buf[5] = actors[i].dir;
        else {
            dx = 0;
            dy = 0;
            scale = DepthScale(actors[i].sprite.y);
            buf[5] = dir_letters[room.fall[i]];
            if (buf[5] == 'n')
                dx = -24, dy = -8;
            if (buf[5] == 'w')
                dx = -35, dy = -12;
            actors[i].sprite.x += scale * dx >> 8;
            actors[i].sprite.y += scale * dy >> 8;
        }
        buf[6] = 0;
        PlayScaled(i, buf, actors[i].sprite.x, actors[i].sprite.y);
        room.flags &= ~bit;
        }
        }
    }
}

/* Show the inventory icon, and the selected object for use. */
void ShowInvIcon(int16_t on)
{
    char *name;
    int16_t x;

    name = 0;
    if (on) {
        switch (room.target) {
        case 0:
            name = "ikirk";
            break;
        case 1:
            name = "ispock";
            break;
        case 2:
            name = "imccoy";
            break;
        case 3:
            name = "iredshir";
            break;
        default:
            name = items[room.target - 0x40].name;
            break;
        }
    }
    if (name) {
        AddSprite(&item_sprite);
        item_sprite.type = SPRITE_SHADOW;
        SpriteSetPos(&item_sprite, 10, 10, 15);
        item_sprite.handle = CacheLoad(name, "bmp");
        item_sprite.layer = 8;
        x = 46;
    } else
        x = 10;
    AddSprite(&inv_sprite);
    inv_sprite.type = SPRITE_SHADOW;
    SpriteSetPos(&inv_sprite, x, 10, 15);
    inv_sprite.handle = CacheLoad("inv00", "bmp");
    inv_sprite.layer = 8;
}

/* Take down the inventory icons. */
void HideInvIcon(void)
{
    if (item_sprite.type == SPRITE_SHADOW)
        SpriteErase(&item_sprite);
    if (inv_sprite.type == SPRITE_SHADOW)
        SpriteErase(&inv_sprite);
    UpdateSprites();
    if (item_sprite.type == SPRITE_SHADOW) {
        DelSprite(&item_sprite);
        item_sprite.type = 0;
        CacheRelease(item_sprite.handle);
    }
    if (inv_sprite.type == SPRITE_SHADOW) {
        DelSprite(&inv_sprite);
        inv_sprite.type = 0;
        CacheRelease(inv_sprite.handle);
    }
}

/* A crew member who fell stands facing the way they fell. */
void StandUp(Actor *a, int16_t n)
{
    char c;

    if (n >= 0 && n <= 3 && ((c = room.fall[n]) & 0xFF) != 0xFF) {
        a->dir = dir_letters[c];
        room.fall[n] = 0xFF;
    }
}
