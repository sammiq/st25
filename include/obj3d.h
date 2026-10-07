/* obj3d.h - objects in the 3D views (layout only partly known) */

#ifndef OBJ3D_H
#define OBJ3D_H

#include "mem.h"
#include "fixmath.h"

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
    int visible;                        /* 1E */
    int unk20;                          /* 20 */
    int reach;                          /* 22 */
    int size;                           /* 24 drawn radius */
    char unk26[6];                      /* 26 */
    int (far *draw)(struct Obj3D *o);   /* 2C */
    void (far *free)(struct Obj3D *o);  /* 30 */
    int unk34;                          /* 34 */
    Vec32 view;                         /* 36 position relative to the camera */
    Matrix view_m;                      /* 42 */
    long scale;                         /* 54 */
    char unk58[0x10];                   /* 58 */
    MemHandle r3s;                      /* 68 bitmap drawn for the object */
    int r3s_type;                       /* 6A offset of its header */
    Matrix rot;                         /* 6C */
    char unk7E[0x22];                   /* 7E */
    /* planets */
    int radius;                         /* A0 */
    int small;                          /* A2 32x32 texture instead of 128x128 */
    char name[10];                      /* A4 */
    int generated;                      /* AE fractal texture */
    unsigned seed;                      /* B0 */
    int palette;                        /* B2 */
} Obj3D;

/* seg027 (assembly) */
void Vec32Sub(Vec32 *a, Vec32 *b);
void MatTVecMul32(Vec32 *v, Matrix *m, Vec32 *out);

#endif
