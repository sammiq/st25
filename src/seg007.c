/* seg007: video modes, palettes, fades, rectangles and text */

#include "compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "resource.h"
#include "sprite.h"
#include "gfx.h"
#include "event.h"

/* Globals this file owns (declared in the headers). */
int16_t video_mode;
unsigned char cur_palette[768];
unsigned char remap_table[256];
int16_t fade_level;

unsigned char *cur_glyph;

MemHandle font_handle = 0;
unsigned char *font_data = 0;
int16_t cur_remap_id = -1;
/* The display is VGA mode 13h, always: the original's EGA and Tandy modes
 * are gone.  video_mode keeps its place for the code that checks it. */
void InitVideo(void)
{
    video_mode = VIDEO_VGA;
    memset(plat_vram(), 0, VGA_W * VGA_H);
}

void TextMode(void)
{
}

/* Set all 256 DAC registers from 768 bytes of 6 bit components. */
void SetPalette(unsigned char *pal)
{
    plat_set_palette(0, 256, pal);
}

void SetPaletteRange(unsigned char *pal, int16_t first, int16_t count)
{
    plat_set_palette(first, count, pal);
}

/* Load name.pal and its shading table name.lut. */
void LoadPalette(char *name)
{
    LoadFilePacked(name, "pal", cur_palette);
    LoadFilePacked(name, "lut", shade_table);
}

void CopyRect(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h)
{
    char *d;
    char *s;
    int16_t unused;
    int16_t dskip;
    int16_t sskip;

    d = dst->data;
    d += y * dst->w + x;
    s = src->data;
    s += sy * src->w + sx;
    dskip = dst->w;
    sskip = src->w;
    while (h--) {
        MemCpy(d, s, w);
        d += dskip;
        s += sskip;
    }
}

/* CopyRect skipping 0 pixels. */
void CopyRectNZ(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h)
{
    char *d;
    char *s;
    int16_t unused;
    int16_t dskip;
    int16_t sskip;

    d = dst->data;
    d += y * dst->w + x;
    s = src->data;
    s += sy * src->w + sx;
    dskip = dst->w;
    sskip = src->w;
    while (h--) {
        MemCpyNZ(d, s, w);
        d += dskip;
        s += sskip;
    }
}

/* CopyRect through the colour table cloak<remap>.lut; runs of 0 are copied
 * as 0. */
void CopyRectRemap(Bitmap *dst, int16_t x, int16_t y, Bitmap *src, int16_t sx, int16_t sy, int16_t w, int16_t h,
                   int16_t remap)
{
    char *d;
    char *s;
    int16_t unused;
    int16_t dskip;
    int16_t sskip;
    char name[10];
    int16_t i;

    if (w == 0 || h == 0)
        return;
    d = dst->data;
    d += y * dst->w + x;
    s = src->data;
    s += sy * src->w + sx;
    dskip = dst->w - w;
    sskip = src->w - w;
    if (remap != cur_remap_id) {
        sprintf(name, "cloak%d", remap);
        LoadFilePacked(name, "lut", remap_table);
        cur_remap_id = remap;
    }
    for (; h != 0; h--, s += sskip, d += dskip)
        for (i = w; i != 0; i--, s++, d++)
            *d = *s ? remap_table[(unsigned char)*s] : 0;
}

int16_t PointInRect(Rect *r, int16_t x, int16_t y)
{
    if (r->x0 > x || r->x1 < x || r->y0 > y || r->y1 < y)
        return 0;
    return 1;
}

/* out = a & b; 0 if they don't overlap. */
int16_t ClipRect(Rect *out, Rect *a, Rect *b)
{
    if (a->x0 > b->x1 || a->x1 < b->x0 || a->y0 > b->y1 || a->y1 < b->y0)
        return 0;
    else {
        out->x0 = max(a->x0, b->x0);
        out->y0 = max(a->y0, b->y0);
        out->x1 = min(a->x1, b->x1);
        out->y1 = min(a->y1, b->y1);
        return 1;
    }
}

/* out = bounding box of a and b. */
void UnionRect(Rect *out, Rect *a, Rect *b)
{
    out->x0 = min(a->x0, b->x0);
    out->y0 = min(a->y0, b->y0);
    out->x1 = max(a->x1, b->x1);
    out->y1 = max(a->y1, b->y1);
}

/* Copy the first line of src into dst, breaking at a newline or the last
 * space within width characters. Returns where the next line starts, or 0
 * at the end of the text. */
char *WrapLine(char *src, char *dst, int16_t width)
{
    int16_t i;
    int16_t brk_i;
    char *brk_src;
    char *brk_dst;

    *dst = 0;
    if (*src == 0)
        return 0;
    brk_src = brk_dst = 0;
    for (i = 0; i != width && *src; i++) {
        if (*src == '\n') {
            *dst = 0;
            return src + 1;
        }
        if (*src == ' ') {
            brk_i = i;
            brk_src = src;
            brk_dst = dst;
        }
        if (*src == '\r') {
            src++;
            i--;
            continue;
        }
        *dst++ = *src++;
    }
    if (*src == 0) {
        *dst = 0;
        return src;
    }
    if (*src == ' ') {
        *dst = 0;
        return src + 1;
    }
    if (brk_dst == 0) {
        *dst = 0;
        return src;
    }
    src = brk_src;
    dst = brk_dst;
    *dst = 0;
    return src + 1;
}

void SetGlyph(int16_t c)
{
    cur_glyph = font_data + c * 64;
}

void BlitRows(char *src, int16_t sskip, char *dst, int16_t dskip, int16_t w, int16_t h)
{
    int16_t unused;

    sskip += w;
    dskip += w;
    while (h--) {
        MemCpy(dst, src, w);
        dst += dskip;
        src += sskip;
    }
}

int16_t DrawString(char *s, int16_t x, int16_t y, Bitmap *bmp)
{
    int16_t len;

    if (s == 0)
        return 0;
    len = strlen(s);
    DrawText(s, len, x, y, bmp);
    return len;
}

/* Draw len characters of s with the 8x8 font at (x, y) in bmp. */
void DrawText(char *s, int16_t len, int16_t x, int16_t y, Bitmap *bmp)
{
    int16_t x0;
    int16_t x1;
    int16_t y0;
    int16_t y1;
    int16_t bx0;
    int16_t bx1;
    int16_t by0;
    int16_t by1;
    int16_t cx0;
    int16_t dx;
    int16_t cx1;
    int16_t unused1;
    int16_t cy0;
    int16_t cy1;
    int16_t dy;
    int16_t unused2;
    int16_t w;
    int16_t h;
    int16_t i;
    int16_t cw;
    int16_t ch;
    int16_t sskip;
    int16_t dskip;
    int16_t soff;
    int16_t doff;
    char *src;
    char *dst;

    cw = 8;
    ch = 8;
    font_data = LockMem(font_handle);
    for (i = 0; i < len; i++) {
        x0 = x;
        x1 = cw - 1 + x0;
        y0 = y;
        y1 = ch - 1 + y0;
        bx0 = 0;
        bx1 = bx0 + (bmp->w - 1);
        by0 = 0;
        by1 = by0 + (bmp->h - 1);
        SetGlyph(s[i]);
        if (x0 <= bx1 && x1 >= bx0 && y0 <= by1 && y1 >= by0) {
            cx0 = bx0 - x0;
            if (cx0 < x0 - x0)
                cx0 = x0 - x0;
            cx1 = bx1 - x0;
            if (cx1 > x1 - x0)
                cx1 = x1 - x0;
            cy0 = by0 - y0;
            if (cy0 < y0 - y0)
                cy0 = y0 - y0;
            cy1 = by1 - y0;
            if (cy1 > y1 - y0)
                cy1 = y1 - y0;
            dx = x0 - bx0;
            if (dx < bx1 - bx1)
                dx = bx1 - bx1;
            dy = y0 - by0;
            if (dy < by0 - by0)
                dy = by0 - by0;
            w = cx1 - cx0 + 1;
            h = cy1 - cy0 + 1;
            soff = cy0 * cw + cx0;
            doff = dx + (int32_t)dy * (bx1 - bx0 + 1);
            sskip = cw - w;
            dskip = bx1 - bx0 + 1 - w;
            src = cur_glyph + soff;
            dst = bmp->data + doff;
            BlitRows(src, sskip, dst, dskip, w, h);
        }
        x += cw;
    }
    UnlockMem(font_handle);
}

void LoadFont(void)
{
    font_handle = CacheLoad("font", "fnt");
}

void FreeFont(void)
{
    CacheRelease(font_handle);
}

void VideoNop(void)
{
}

void FadeIn(void)
{
    uint32_t t;

    while (fade_level <= 100) {
        t = GetTicks() + 1;
        while (GetTicks() < t)
            ;
        SetFade(cur_palette, fade_level);
        fade_level += 10;
    }
    fade_level = 100;
}

void FadeOut(void)
{
    uint32_t t;

    while (fade_level >= 0) {
        t = GetTicks() + 1;
        while (GetTicks() < t)
            ;
        SetFade(cur_palette, fade_level);
        fade_level -= 10;
    }
    fade_level = 0;
}

/* Show pal at level percent brightness. */
void SetFade(unsigned char *pal, int16_t level)
{
    uint16_t scale;
    unsigned char buf[768];
    int16_t i;

    scale = ((uint16_t)level << 8) / 100;
    for (i = 0; i < 768; i++)
        buf[i] = (unsigned char)(((uint16_t)pal[i] * scale) >> 8);
    plat_set_palette(0, 256, buf);
}

void FadeUp(void)
{
    if (fade_level < 100)
        SetFade(cur_palette, fade_level += 10);
}

void FadeDown(void)
{
    if (fade_level > 0)
        SetFade(cur_palette, fade_level -= 10);
}
