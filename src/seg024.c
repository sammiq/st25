/* seg024: bitmap scaling with generated code */

#include "compat.h"
#include <stdlib.h>
#include "mem.h"
#include "bitmap.h"

/* Globals this file owns (declared in the headers). */
uint16_t scale_idx[SCALE_MAX_W];
int16_t scale_count;
char scale_row[0x140];

/* Scale a bitmap by scale/256, returning a new bitmap. */
MemHandle ScaleBitmap(MemHandle handle, int16_t scale)
{
    int16_t unused1[12];
    char *srcrow;
    char *dstrow;
    int32_t unused2;
    int16_t srcw;
    int16_t srch;
    int16_t w;
    int16_t h;
    int16_t err;
    int16_t inc1;
    int16_t inc2;
    int16_t y;
    char drawn;
    Bitmap *src;
    MemHandle nh;
    Bitmap *dst;

    src = LockMem(handle);
    w = ((int32_t)src->w * scale) >> 8;
    h = ((int32_t)src->h * scale) >> 8;
    UnlockMem(handle);
    w = max(1, w);
    h = max(1, h);
    nh = AllocMemAbort(w * h + 8, 0);
    src = LockMem(handle);
    dst = LockMem(nh);
    dst->x = ((int32_t)src->x * scale) >> 8;
    dst->y = ((int32_t)src->y * scale) >> 8;
    dst->w = w;
    dst->h = h;
    srcw = src->w;
    srch = src->h;
    srcrow = src->data;
    dstrow = dst->data;
    scale_count = BuildScaler(scale_idx, w, srcw);
    --srch;
    --h;
    if (scale <= 0x100) {
        /* shrinking: skip source rows */
        err = 0;
        inc1 = h * 2;
        inc2 = (h - srch) * 2;
        for (y = 0; y <= srch; y++) {
            if (err < 0)
                err += inc1;
            else {
                err += inc2;
                RunScaler(scale_row, srcrow, scale_idx, scale_count);
                MemCpy(dstrow, scale_row, w);
                dstrow += w;
            }
            srcrow += src->w;
        }
    } else {
        /* enlarging: repeat source rows */
        err = srch * 2 - h;
        inc1 = srch * 2;
        inc2 = (srch - h) * 2;
        drawn = 0;
        srcw = src->w;
        for (y = 0; y <= h; y++) {
            if (!drawn) {
                RunScaler(scale_row, srcrow, scale_idx, scale_count);
                drawn = 1;
            }
            MemCpy(dstrow, scale_row, w);
            dstrow += w;
            if (err < 0)
                err += inc1;
            else {
                err += inc2;
                srcrow += srcw;
                drawn = 0;
            }
        }
    }
    UnlockMem(handle);
    UnlockMem(nh);
    return nh;
}

/* Work out how to scale a row of sw pixels to dw: idx[i] is the source
 * pixel of destination pixel i (a Bresenham step).  The original generated
 * machine code for this; returns the number of destination pixels. */
int16_t BuildScaler(uint16_t *idx, int16_t dw, int16_t sw)
{
    int16_t err;
    int16_t inc1;
    int16_t inc2;
    int16_t count;
    int16_t i;
    int16_t n;

    n = 0;
    if (dw > SCALE_MAX_W)
        Fatal("too many inst %d.\n", dw);
    if (sw >= dw) {
        err = dw * 2 - sw;
        inc1 = dw * 2;
        inc2 = (dw - sw) * 2;
        for (i = 0; i < sw; i++) {
            if (err < 0)
                err += inc1;
            else {
                err += inc2;
                if (n < SCALE_MAX_W)
                    idx[n++] = i;
            }
        }
    } else {
        int16_t src = 0;

        err = (sw - 1) * 2 - (dw - 1);
        inc1 = (sw - 1) * 2;
        inc2 = ((sw - 1) - (dw - 1)) * 2;
        for (count = dw; count--; ) {
            idx[n++] = src;
            if (err < 0)
                err += inc1;
            else {
                err += inc2;
                ++src;
            }
        }
    }
    return n;
}

/* Scale a row with the table BuildScaler made. */
void RunScaler(void *dst, void *src, uint16_t *idx, int16_t count)
{
    unsigned char *d = dst;
    unsigned char *s = src;
    int16_t i;

    for (i = 0; i < count; i++)
        d[i] = s[idx[i]];
}
