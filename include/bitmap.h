/* bitmap.h - bitmaps and the bitmap scaler */

#ifndef BITMAP_H
#define BITMAP_H

#include "compat.h"

#include "mem.h"

typedef struct BitmapHdr {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
} BitmapHdr;

typedef struct Bitmap {
    int16_t x;                  /* 00 */
    int16_t y;                  /* 02 */
    uint16_t w;             /* 04 */
    uint16_t h;             /* 06 */
    char data[1];           /* 08 w * h pixels */
} Bitmap;

#define SCALE_MAX_W 0x400   /* widest scaled row */

extern uint16_t scale_idx[SCALE_MAX_W]; /* row scaler made by BuildScaler */
extern char scale_row[];    /* one scaled row */

MemHandle ScaleBitmap(MemHandle handle, int16_t scale);
int16_t BuildScaler(uint16_t *idx, int16_t dw, int16_t sw);
void RunScaler(void *dst, void *src, uint16_t *idx, int16_t count);

#endif
