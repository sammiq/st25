/* seg023: starfield */

#include "compat.h"
#include <stdlib.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"

typedef struct Vec32 {
    int32_t x;
    int32_t y;
    int32_t z;
} Vec32;

typedef struct Rect {
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
} Rect;

typedef struct Star {
    int16_t active;             /* 00 */
    Vec32 pos;              /* 02 world position */
} Star;

#define NUM_STARS 16

extern float star_near;
extern int16_t view_w;
extern int16_t view_h;
extern int32_t proj_dist;
extern struct { int16_t e[3][3]; } cam_matrix;
extern Vec32 cam_pos;
extern int16_t view_cx;
extern int16_t view_cy;
extern Rect clip_rect;
extern Bitmap *screen;

/* seg027 */
void Vec32FromScreen(Vec32 *out, int16_t x, int16_t y, int16_t z);
void MatVecMul32(void *m, Vec32 *v, Vec32 *out);
void Vec32Add(Vec32 *a, Vec32 *b);
void Vec32Sub(Vec32 *a, Vec32 *b);
void MatTVecMul32(Vec32 *v, void *m, Vec32 *out);
int16_t Project(int16_t a, int16_t z);

int16_t ClipRect(Rect *out, Rect *a, Rect *b);      /* seg007: clip a to b */

MemHandle star_bmp = 0;
Star stars[NUM_STARS];

void BlitClipped(Bitmap *bmp, Rect *r, Rect *clip);

void StarsReset(void)
{
    int32_t unused;

    if (star_bmp)
        CacheRelease(star_bmp);
    MemSet(stars, sizeof(stars), 0);
}

/* Respawn, move and draw the stars around the camera. stars.shp holds
 * the star frames back to back. */
void DrawStars(void)
{
    int16_t i;
    int16_t x;
    int16_t y;
    int16_t z;
    Vec32 v2;
    Vec32 v;
    int32_t unused1;
    Star *s;
    int16_t wr;
    int16_t hr;
    int16_t wr2;
    int16_t hr2;
    Rect r;
    Rect rc;
    char unused2[16];
    Bitmap *bmp;
    char *cur;
    char *base;

    wr = (view_w * 3) >> 1;
    wr2 = wr / 2;
    hr = (view_h * 3) >> 1;
    hr2 = hr / 2;
    z = proj_dist << 3;
    star_bmp = CacheLoad("stars", "shp");
    base = cur = LockMem(star_bmp);
    for (s = stars, i = 0; i != NUM_STARS; ++i, ++s) {
        if ((i & 0xF) == 0)
            cur = base;
        bmp = (Bitmap *)cur;
        cur = cur + (((Bitmap *)cur)->w * ((Bitmap *)cur)->h + 8);
        if (s->active == 0) {
            x = rand() % wr - wr2;
            y = rand() % hr - hr2;
            Vec32FromScreen(&v, x, y, z);
            MatVecMul32(&cam_matrix, &v, &s->pos);
            Vec32Add(&s->pos, &cam_pos);
            s->active = 1;
        }
        v = s->pos;
        Vec32Sub(&v, &cam_pos);
        MatTVecMul32(&v, &cam_matrix, &v2);
        if (v2.z > star_near && v2.z < 0x3FFF
                && (v2.x > 0 ? v2.x : -v2.x) < v2.z
                && (v2.y > 0 ? v2.y : -v2.y) < v2.z) {
            r.x0 = Project(v2.x, v2.z) + view_cx;
            r.y0 = view_cy - Project(v2.y, v2.z);
            r.x1 = r.x0 + bmp->w - 1;
            r.y1 = r.y0 + bmp->h - 1;
            if (ClipRect(&rc, &r, &clip_rect))
                BlitClipped(bmp, &r, &rc);
        } else
            s->active = 0;
    }
    UnlockMem(star_bmp);
    CacheRelease(star_bmp);
    star_bmp = 0;
}

/* Draw the part clip of bmp placed at r into the screen, skipping 0. */
void BlitClipped(Bitmap *bmp, Rect *r, Rect *clip)
{
    int16_t w;
    int16_t h;
    int16_t x0;
    int16_t y0;
    char *src;
    char *dst;
    int16_t srcskip;
    int16_t dstskip;
    int16_t unused;

    x0 = r->x0;
    y0 = r->y0;
    r = clip;
    w = r->x1 - r->x0 + 1;
    h = r->y1 - r->y0 + 1;
    src = bmp->data + r->x0 - x0;
    src += (r->y0 - y0) * bmp->w;
    srcskip = bmp->w - w;
    dst = screen->data + r->x0 + r->y0 * 320;
    dstskip = 320 - w;
    for (; h > 0; h--, src += srcskip, dst += dstskip) {
        for (x0 = w; x0 > 0; x0--, src++, dst++)
            if (*src)
                *dst = *src;
    }
}
