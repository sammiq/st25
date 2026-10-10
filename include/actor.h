/* actor.h - animated actors in the away-team rooms (seg000) */

#ifndef ACTOR_H
#define ACTOR_H

#include "compat.h"

#include "mem.h"
#include "sprite.h"

#define MAX_ACTORS  32

/* Actor.type */
#define ANIM_STAND  0       /* standing animation */
#define ANIM_WALK   1
#define ANIM_PLAY   2       /* any other .anm */

typedef struct Actor {
    int16_t state;              /* 00 1 in use */
    char name[16];          /* 02 the .anm */
    int16_t type;               /* 12 ANIM_ */
    Sprite sprite;          /* 14 */
    char bitmap[10];        /* 48 shown now */
    int16_t scale;              /* 52 */
    MemHandle anim;         /* 54 the .anm data (AnimFrame[]) */
    int16_t nframes;            /* 56 */
    int16_t frame;              /* 58 */
    int32_t next_time;         /* 5A blink_count of the next frame */
    int16_t x;                  /* 5E */
    int16_t y;                  /* 60 */
    int16_t loops;              /* 62 times the last frame was reached */
    int16_t notify;             /* 64 report to the room when done */
    int16_t notify_arg;         /* 66 */
    char walk_name[16];     /* 68 walk bitmaps, then the direction letter */
    int16_t walk_node;          /* 78 walk graph node being walked to, or -1 */
    int16_t dest_node;          /* 7A */
    int32_t fx;                /* 7C 16.16 position while walking */
    int32_t fy;                /* 80 */
    int32_t dx;                /* 84 step */
    int32_t dy;                /* 88 */
    int16_t dest_x;             /* 8C */
    int16_t dest_y;             /* 8E */
    int16_t steps;              /* 90 left to the next node */
    unsigned char step;     /* 92 walk frame counter */
    unsigned char dir;      /* 93 'N', 'S', 'E', 'W' or 0 */
    char unk94[4];          /* 94 */
    char stand[16];         /* 98 standing animation prefix */
} Actor;

/* A frame of a .anm file. */
typedef struct AnimFrame {
    char bitmap[10];        /* 00 */
    int16_t x;                  /* 0A */
    int16_t y;                  /* 0C */
    int16_t z;                  /* 0E added to the depth */
    int16_t delay;              /* 10 ticks */
    char next[4];           /* 12 frames that may follow, picked at random */
} AnimFrame;

/* A playing .ban (run-length coded screen changes). */
typedef struct Ban {
    MemHandle h;            /* 00 */
    int16_t pos;                /* 02 offset of the next frame */
} Ban;

extern Actor actors[MAX_ACTORS];
extern Ban bans[16];

int16_t StartAnim(int16_t n, char *name, int16_t x, int16_t y, int16_t scale);
int16_t StartWalk(int16_t n, char *name, int16_t x, int16_t y, int16_t dx, int16_t dy);
void EndAnim(int16_t n);
void init_stand_anim(int16_t n);

#endif
