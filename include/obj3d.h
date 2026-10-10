/* obj3d.h - objects in the 3D space view: planets, ships and shots */

#ifndef OBJ3D_H
#define OBJ3D_H

#include "compat.h"

#include "mem.h"
#include "fixmath.h"

#define R3_PLANET   1
#define R3_SHIP     2
#define R3_SHOT     3
#define R3_EXPL     4       /* only while an explosion is drawn */

#ifndef VEC32_DEFINED
#define VEC32_DEFINED
typedef struct Vec32 {
    int32_t x;
    int32_t y;
    int32_t z;
} Vec32;
#endif

typedef struct Obj3D Obj3D;

/* A shield facing or a ship system. */
typedef struct ShipPart {
    int16_t unk;                            /* 00 */
    int16_t max;                            /* 02 */
    int16_t cur;                            /* 04 */
} ShipPart;

/* A ship's weapon mount. */
typedef struct Weapon {
    int32_t next;                          /* 00 blink_count when it may fire again */
    int16_t life;                           /* 04 frames a shot lasts */
    int16_t speed;                          /* 06 */
    int16_t delay;                          /* 08 frames between shots */
    Vec32 ofs;                          /* 0A position on the ship */
    int16_t kind;                           /* 16 shot kind */
} Weapon;

/* Ship movement. Angles and speeds are 16.16. */
typedef struct Motion {
    int32_t turn_max;                      /* 00 */
    int32_t turn_accel;                    /* 04 */
    int32_t max_speed;                     /* 08 */
    int32_t accel;                         /* 0C */
    int32_t target_speed;                  /* 10 */
    int32_t turn_target[3];                /* 14 pitch, yaw, roll */
    int32_t speed_limit;                   /* 20 what the power allows */
    int32_t speed;                         /* 24 */
    int32_t turn_rate[3];                  /* 28 */
    int32_t turn_sum[3];                   /* 34 turned so far */
    int16_t timer;                          /* 40 */
} Motion;

/* A ship's .dsc file. */
typedef struct ShipDesc {
    int16_t kind;                   /* 00 */
    int16_t reach;                  /* 02 */
    int16_t size;                   /* 04 */
    ShipPart shields[6];        /* 06 */
    ShipPart systems[10];       /* 2A */
    Motion mo;                  /* 66 */
    int32_t range;                 /* A8 */
    int16_t has_shields;            /* AC */
    Weapon weapons[4];          /* AE */
} ShipDesc;

/* The fields every object in the view has. */
#define R3_FIELDS \
    Vec32 pos;                          /* 00 world position */ \
    Matrix orient;                      /* 0C */ \
    int16_t visible;                        /* 1E R3_PLANET, R3_SHIP, R3_SHOT, or 0 */ \
    int16_t kind;                           /* 20 ship (shape_names) or shot type */ \
    int16_t reach;                          /* 22 radius for culling */ \
    int16_t size;                           /* 24 drawn radius */ \
    int16_t vx;                             /* 26 velocity per frame */ \
    int16_t vy;                             /* 28 */ \
    int16_t vz;                             /* 2A */ \
    int16_t (*draw)(Obj3D *o);          /* 2C */ \
    void (*free)(Obj3D *o);         /* 30 */ \
    int16_t drawn;                          /* 34 set when drawn this frame */ \
    Vec32 view;                         /* 36 position relative to the camera */ \
    Matrix view_m;                      /* 42 */ \
    int32_t scale;                         /* 54 distance from the camera */ \
    int16_t sx;                             /* 58 projected centre */ \
    int16_t sy;                             /* 5A */ \
    char shape[10];                     /* 5C name of the loaded .r3s */ \
    int16_t view_idx;                       /* 66 bitmap in it */ \
    MemHandle r3s;                      /* 68 bitmap drawn for the object */ \
    int16_t r3s_type;                       /* 6A offset of its header */ \
    Matrix rot;                         /* 6C orientation the bitmap shows */ \
    int16_t mirror;                         /* 7E bitmap drawn mirrored */ \
    double inv[4];                      /* 80 screen to bitmap matrix */

/* A shot (only these 0xA8 bytes; see Obj3D). */
typedef struct Shot {
    R3_FIELDS
    Obj3D *owner;                       /* A0 */
    int16_t life;                           /* A2 frames left; animates explosions */
    int16_t damage;                         /* A4 */
    int16_t color;                          /* A6 */
} Shot;

struct Obj3D {
    R3_FIELDS
    union {
        struct {                        /* planets */
            int16_t radius;                 /* A0 */
            int16_t small;                  /* A2 32x32 texture instead of 128x128 */
            char name[10];              /* A4 */
            int16_t generated;              /* AE fractal texture */
            uint16_t seed;              /* B0 */
            int16_t palette;                /* B2 */
        };
        struct {                        /* shots, as Shot */
            Obj3D *owner;               /* A0 */
            int16_t life;                   /* A2 */
            int16_t damage;                 /* A4 */
            int16_t color;                  /* A6 */
        };
        struct {                        /* ships */
            ShipPart shields[6];        /* A0 by side (FindSide) */
            ShipPart systems[10];       /* C4 SYS_ */
            Motion mo;                  /* 100 */
            int16_t power;                  /* 142 left this frame */
            int16_t shields_req;            /* 144 power wanted for shields */
            int16_t shields_pwr;            /* 146 and got */
            int16_t weapons_req;            /* 148 */
            int16_t weapons_pwr;            /* 14A */
            int32_t dist;                  /* 14C to the player */
            int32_t ddist;                 /* 150 change in dist */
            int16_t ai_busy;                /* 154 running ai */
            void (*ai)(Obj3D *o);   /* 156 manoeuvre */
            int16_t ai_state;               /* 15A */
            Weapon weapons[4];          /* 15C */
            int32_t range;                 /* 1BC fighting distance */
            int16_t has_shields;            /* 1C0 */
            int16_t shields_up;             /* 1C2 raised */
            int16_t shields_on;             /* 1C4 powered */
            int16_t shield_glow;            /* 1C6 0 to 28 */
            int16_t expl;                   /* 1C8 exploding */
            int16_t expl_frame;             /* 1CA */
            int16_t flee;                   /* 1CC */
        };
    };
};

/* The engines' power boost.  The game saves it as one block, so its
 * variables are kept together. */
typedef struct Boost {
    int16_t active;                     /* extra power from the engines */
    int32_t count;                      /* frames left */
    int16_t amount;                     /* power added each frame */
    int16_t power;                      /* amount at the next boost */
    int16_t time;                       /* frames a boost lasts */
} Boost;

extern Boost boost;
#define boost_active    boost.active
#define boost_count     boost.count
#define boost_amount    boost.amount
#define boost_power     boost.power
#define boost_time      boost.time

/* systems[] */
#define SYS_HULL        0
#define SYS_BRIDGE      1
#define SYS_TORP_PORT   2
#define SYS_TORP_STBD   3
#define SYS_PHASER_PORT 4
#define SYS_PHASER_STBD 5
#define SYS_SENSORS     6
#define SYS_IMPULSE     7
#define SYS_WARP_PORT   8
#define SYS_WARP_STBD   9

/* seg027 (assembly) */
void Vec32Sub(Vec32 *a, Vec32 *b);
void MatTVecMul32(Vec32 *v, Matrix *m, Vec32 *out);

#endif
