/* rooms.h - what the room scripts (src/rooms) run on.
 *
 * The scripts are the .RDF files' machine code translated to C.  They keep their variables, strings and tables in the
 * loaded room file (rdf_image: CS_B and CS_W read and write it by offset),
 * use the room state through `room`, and call the game through the svc_
 * macros, which are ScriptService's commands (src/seg008.c).
 */

#ifndef ROOMS_H
#define ROOMS_H

#include "compat.h"
#include "mem.h"
#include "room.h"
#include "rooms_const.h"

/* The loaded room file; set by LoadRoom. */
extern uint8_t *rdf_image;

typedef uint16_t __attribute__((aligned(1))) u16_unaligned;

/* offsets are 16 bit, like the segment they were in: an index that overshoots wraps */
#define CS_B(off)       (*(uint8_t *)(rdf_image + (uint16_t)(off)))
#define CS_W(off)       (*(u16_unaligned *)(rdf_image + (uint16_t)(off)))
#define ROOM_B(off)     (*((uint8_t *)&room + (off)))
#define ROOM_W(off)     (*(u16_unaligned *)((uint8_t *)&room + (off)))

/* A few scripts use SI as a table index and then still write through it, as if it were the
 * room: the machine hit some other data of the game, harmlessly.  Those go here. */
extern uint8_t stray_mem[0x400];
#define STRAY_B(off)    (stray_mem[(uint16_t)(off) & 0x3FF])
#define STRAY_W(off)    (*(u16_unaligned *)(stray_mem + ((uint16_t)(off) & 0x3FE)))

/* the halves of a 16 bit register variable */
#define LO(r)           ((uint8_t)(r))
#define HI(r)           ((uint8_t)((r) >> 8))
#define SETLO(r, v)     ((r) = (uint16_t)(((r) & 0xFF00) | ((v) & 0xFF)))
#define SETHI(r, v)     ((r) = (uint16_t)(((r) & 0x00FF) | (((v) & 0xFF) << 8)))

/* a script: where its code was in the room file, and what it does now */
typedef struct RoomScript {
    uint16_t offset;
    void (*fn)(void);
} RoomScript;

/* The scripts of a room, by name ("DEMON1"); 0 if there are none. */
const RoomScript *FindRoomScripts(const char *room_name);

/* An argument the script did not push: the engine read whatever was on the stack there (in
 * practice the return address into CallScript), so it is not a meaningful value. */
#define STALE_ARG   0x7FFF

/* ScriptService commands */
enum {
    RC_ANIM0, RC_STOPANIM, RC_ANIM2, RC_TALK, RC_GIVEITEM, RC_LEVERS, RC_GOTOROOM, RC_TAKEITEM,
    RC_WALK, RC_LOADMAP, RC_FLASHBMP, RC_HAVEITEM, RC_ITEMCASE, RC_ACTORPOS, RC_RANDOM,
    RC_BRIDGESFX, RC_MUSIC, RC_BEAMUP, RC_GAMEOVER, RC_INPUTCODE, RC_TURBOLIFT, RC_SFX, RC_BAD,
    RC_DIGIOFF
};

int32_t ScriptService(int16_t cmd, int16_t a1, int16_t a2, int16_t a3, int16_t a4, uint16_t a5);

#define svc_Anim0(actor, name, x, y, msg)   ScriptService(RC_ANIM0, actor, name, x, y, msg)
#define svc_StopAnim(actor)                 ScriptService(RC_STOPANIM, actor, 0, 0, 0, 0)
#define svc_Anim(actor, name, x, y, msg)    ScriptService(RC_ANIM2, actor, name, x, y, msg)
#define svc_Talk(conv)                      ScriptService(RC_TALK, conv, 0, 0, 0, 0)
#define svc_GiveItem(item)                  ScriptService(RC_GIVEITEM, item, 0, 0, 0, 0)
#define svc_Levers()                        ScriptService(RC_LEVERS, 0, 0, 0, 0, 0)
#define svc_GotoRoom(room, entrance)        ScriptService(RC_GOTOROOM, room, entrance, 0, 0, 0)
#define svc_TakeItem(item)                  ScriptService(RC_TAKEITEM, item, 0, 0, 0, 0)
#define svc_Walk(crew, x, y, msg)           ScriptService(RC_WALK, crew, x, y, msg, 0)
#define svc_LoadMap(name)                   ScriptService(RC_LOADMAP, name, 0, 0, 0, 0)
#define svc_FlashBitmap(name)               ScriptService(RC_FLASHBMP, name, 0, 0, 0, 0)
#define svc_HaveItem(item)                  ScriptService(RC_HAVEITEM, item, 0, 0, 0, 0)
#define svc_ItemCase(mask)                  ScriptService(RC_ITEMCASE, mask, 0, 0, 0, 0)
#define svc_ActorPos(actor)                 ScriptService(RC_ACTORPOS, actor, 0, 0, 0, 0)
#define svc_Random(lo, hi)                  ScriptService(RC_RANDOM, lo, hi, 0, 0, 0)
#define svc_BridgeSfx(n)                    ScriptService(RC_BRIDGESFX, n, 0, 0, 0, 0)
#define svc_Music(a, b)                     ScriptService(RC_MUSIC, a, b, 0, 0, 0)
#define svc_BeamUp(score, max_score, outcome) ScriptService(RC_BEAMUP, score, max_score, outcome, 0, 0)
#define svc_GameOver()                      ScriptService(RC_GAMEOVER, 0, 0, 0, 0, 0)
#define svc_InputCode(codes)                ScriptService(RC_INPUTCODE, codes, 0, 0, 0, 0)
#define svc_Turbolift(unused, deck)         ScriptService(RC_TURBOLIFT, unused, deck, 0, 0, 0)
#define svc_Sfx(name)                       ScriptService(RC_SFX, name, 0, 0, 0, 0)
#define svc_Bad()                           ScriptService(RC_BAD, 0, 0, 0, 0, 0)
#define svc_DigiOff()                       ScriptService(RC_DIGIOFF, 0, 0, 0, 0, 0)

#endif
