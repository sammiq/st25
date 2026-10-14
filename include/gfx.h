/* gfx.h - video modes, palettes, fades, rectangles and the 8x8 font */

#ifndef GFX_H
#define GFX_H

#include "compat.h"

#include "mem.h"
#include "bitmap.h"

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef struct Rect {
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
} Rect;
#endif

/* video_mode */
#define VIDEO_VGA       0       /* mode 13h */
#define VIDEO_EGA       1       /* mode 0Eh, two pixels per remapped byte */
#define VIDEO_TANDY     2       /* Tandy 1000 320x200x16 */

extern int16_t video_mode;                  /* -1 until InitVideo */
extern unsigned char cur_palette[768];
extern unsigned char shade_table[256];
extern unsigned char remap_table[256];
extern int16_t fade_level;                  /* 0 black .. 100 full */

void InitVideo(void);
void TextMode(void);
void SetPaletteRange(unsigned char *pal, int16_t first, int16_t count);
void LoadPalette(char *name);
void CopyRect(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h);
void CopyRectNZ(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h);
void CopyRectRemap(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h,
                   int16_t remap);
int16_t PointInRect(Rect *r, int16_t x, int16_t y);
int16_t ClipRect(Rect *out, Rect *a, Rect *b);
void UnionRect(Rect *out, Rect *a, Rect *b);
char *WrapLine(char *src, char *dst, int16_t width);
void SetGlyph(int16_t c);
void BlitRows(char *src, int16_t sskip, char *dst, int16_t dskip, int16_t w, int16_t h);
int16_t DrawString(char *s, int16_t x, int16_t y, Bitmap *bmp);
void DrawText(char *s, int16_t len, int16_t x, int16_t y, Bitmap *bmp);
void LoadFont(void);
void FreeFont(void);
void FadeIn(void);
void FadeOut(void);
void SetFade(unsigned char *pal, int16_t level);
void FadeUp(void);
void FadeDown(void);

#endif
