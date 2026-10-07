/* obj3d.h - objects in the 3D space view (layout only partly known) */

#ifndef OBJ3D_H
#define OBJ3D_H

#include "mem.h"
#include "fixmath.h"

#define R3_PLANET   1
#define R3_SHIP     2
#define R3_SHOT     3
#define R3_EXPL     4       /* only while an explosion is drawn */

#ifndef VEC32_DEFINED
#define VEC32_DEFINED
typedef struct Vec32 {
    long x;
    long y;
    long z;
} Vec32;
#endif

typedef struct Obj3D {
    Vec32 pos;                          /* 00 world position */
    Matrix orient;                      /* 0C */
    int visible;                        /* 1E R3_PLANET, R3_SHIP, R3_SHOT, or 0 */
    int kind;                           /* 20 ship (shape_names) or shot type */
    int reach;                          /* 22 radius for culling */
    int size;                           /* 24 drawn radius */
    int vx;                             /* 26 velocity per frame */
    int vy;                             /* 28 */
    int vz;                             /* 2A */
    int (far *draw)(struct Obj3D *o);   /* 2C */
    void (far *free)(struct Obj3D *o);  /* 30 */
    int drawn;                          /* 34 set when drawn this frame */
    Vec32 view;                         /* 36 position relative to the camera */
    Matrix view_m;                      /* 42 */
    long scale;                         /* 54 distance from the camera */
    int sx;                             /* 58 projected centre */
    int sy;                             /* 5A */
    char shape[10];                     /* 5C name of the loaded .r3s */
    int view_idx;                       /* 66 bitmap in it */
    MemHandle r3s;                      /* 68 bitmap drawn for the object */
    int r3s_type;                       /* 6A offset of its header */
    Matrix rot;                         /* 6C orientation the bitmap shows */
    int mirror;                         /* 7E bitmap drawn mirrored */
    double inv[4];                      /* 80 screen to bitmap matrix */
    union {
        struct {                        /* planets */
            int radius;                 /* A0 */
            int small;                  /* A2 32x32 texture instead of 128x128 */
            char name[10];              /* A4 */
            int generated;              /* AE fractal texture */
            unsigned seed;              /* B0 */
            int palette;                /* B2 */
        };
        struct {                        /* shots (only 0xA8 bytes are saved) */
            int unkA0;                  /* A0 */
            int sub;                    /* A2 variant */
            int unkA4;                  /* A4 */
            int color;                  /* A6 */
        };
    };
    char unkB4[0x5C];                   /* B4 */
    Vec32 move;                         /* 110 */
    long move_speed;                    /* 11C */
    long unk120;                        /* 120 */
    Vec32 turn;                         /* 124 */
    long turn_speed;                    /* 130 */
    char unk134[0x20];                  /* 134 */
    int unk154;                         /* 154 */
    char unk156[0x6A];                  /* 156 */
    int shielded;                       /* 1C0 */
    int unk1C2;                         /* 1C2 */
    int shield_up;                      /* 1C4 */
    int shield;                         /* 1C6 shield glow, 0 to 28 */
    int expl;                           /* 1C8 exploding */
    int expl_frame;                     /* 1CA */
    int unk1CC;                         /* 1CC */
} Obj3D;

/* seg027 (assembly) */
void Vec32Sub(Vec32 *a, Vec32 *b);
void MatTVecMul32(Vec32 *v, Matrix *m, Vec32 *out);

#endif
