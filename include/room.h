/* room.h - the away team's rooms (seg008): the room state, the room data
 * file (.rdf) and its scripts */

#ifndef ROOM_H
#define ROOM_H

#include "compat.h"

/* The state of the current room, saved with the game and seen by the
 * room's scripts (SI points at it while they run). */
typedef struct RoomState {
    int16_t timers[8];              /* 00 count down; message 8 at zero */
    int16_t saved_x;                /* 10 pointer when the game was saved */
    int16_t saved_y;                /* 12 */
    int16_t stun[4];                /* 14 crew stunned for this many ticks */
    char no_walk;               /* 1C */
    char busy;                  /* 1D the player can't act */
    char redshirt_dead;         /* 1E */
    unsigned char action;       /* 1F 1 walk, 2 use, 3 get, 4 look, 5 talk */
    unsigned char target;       /* 20 object acted on */
    unsigned char target2;      /* 21 used on this */
    char unk22;                 /* 22 */
    char more;                  /* 23 a script asks for more matches */
    unsigned char flags;        /* 24 bits 0-3 crew stunned */
    unsigned char fall[4];      /* 25 where a stunned crew member falls */
    char vars[0x100];           /* 29 for the scripts */
} RoomState;

/* A room's .rdf file, locked at rdf. Offsets are from its start. */
typedef struct Rdf {
    uint32_t service;           /* 00 the scripts' far pointer to the game call (unused) */
    uint16_t state;             /* 04 the RoomState */
    int16_t y_max;                  /* 06 depth scaling */
    int16_t y_min;                  /* 08 */
    uint16_t scale_min;         /* 0A */
    uint16_t scale_max;         /* 0C */
    uint16_t scripts;           /* 0E */
    uint16_t scripts_end;       /* 10 */
    uint16_t objects;           /* 12 clickable polygons */
    uint16_t objects_end;       /* 14 */
    uint16_t exits;             /* 16 polygons */
    uint16_t exits_end;         /* 18 */
    uint16_t regions;           /* 1A */
    uint16_t regions_end;       /* 1C */
    uint16_t bans;              /* 1E names of the .ban animations */
    uint16_t bans_end;          /* 20 */
    int16_t exit_room[4];           /* 22 */
    int16_t walk_in[4][8][2];       /* 2A per entrance: from, to for each crew */
    int16_t beam_pos[4][2];         /* AA */
    int16_t stand_pos[4][2];        /* BA */
} Rdf;

extern RoomState room;
extern Rdf *rdf;
extern MemHandle rdf_handle;
extern char room_mission[16];

#endif
