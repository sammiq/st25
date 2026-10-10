/* seg021: sprites */

#include "compat.h"
#include <stdlib.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "resource.h"
#include "sprite.h"

Sprite *sprite_array[MAX_SPRITES];
Rect dirty_rects[64];
int16_t dirty_count;
extern int16_t video_mode;              /* 0 VGA, 1 EGA, 2 Tandy */
extern Bitmap *backbuf;         /* sprites are composed here */
extern Bitmap *screen;          /* the background without sprites */
extern unsigned char *mask_buf; /* EGA priority mask, 2 pixels a byte */
unsigned char shade_table[256];

int16_t sprite_count0 = 0;
Sprite **sprite_list = sprite_array;
int16_t *sprite_count = &sprite_count0;
Rect screen_rect = { 0, 0, 319, 199 };
/* Assigns the parameters, not what they point to, so does nothing. */
void GetSpriteList(Sprite **list, int16_t *count)
{
    list = sprite_list;
    count = sprite_count;
}

void SetSpriteList(Sprite **list, int16_t *count)
{
    sprite_list = list;
    sprite_count = count;
}

void ResetSpriteList(void)
{
    sprite_list = sprite_array;
    sprite_count = &sprite_count0;
}

void MarkAllDirty(void)
{
    int16_t i;
    Sprite *s;

    if (*sprite_count == 0)
        return;
    for (i = 0; i != *sprite_count; i++) {
        s = sprite_list[i];
        s->dirty = 1;
    }
}

void ClearSprites(void)
{
    *sprite_count = 0;
}

void SpriteInit(Sprite *s)
{
    s->type = 0;
    s->name = 0;
    s->erase = 0;
    s->old.x0 = s->old.y0 = -1;
    s->old.x1 = s->old.y1 = -2;
}

void AddSprite(Sprite *s)
{
    if (*sprite_count != MAX_SPRITES) {
        SpriteInit(s);
        sprite_list[(*sprite_count)++] = s;
    } else
        Fatal("AddSprite: fatal: too many sprites.");
}

void DelSprite(Sprite *s)
{
    int16_t i;

    for (i = 0; i != *sprite_count; i++) {
        if (sprite_list[i] == s) {
            sprite_list[i] = sprite_list[--*sprite_count];
            return;
        }
    }
    Fatal("DelSprite: fatal: sprite not in list.");
}

void SpriteErase(Sprite *s)
{
    s->erase = 1;
    s->dirty = 1;
}

/* Restore the background under every changed sprite, redraw the sprites
 * touching those areas and copy them to the screen. */
void UpdateSprites(void)
{
    int16_t i;
    Sprite *s;
    Rect *r;

    if (*sprite_count == 0)
        return;
    SortSprites();
    ComputeSpriteRects();
    dirty_count = 0;
    for (i = 0; i != *sprite_count; i++) {
        s = sprite_list[i];
        if (s->visible && s->dirty) {
            if (s->has_union)
                dirty_rects[dirty_count++] = s->union_rect;
            else {
                dirty_rects[dirty_count++] = s->rect;
                dirty_rects[dirty_count++] = s->old;
            }
        } else if (s->dirty)
            dirty_rects[dirty_count++] = s->old;
    }
    for (r = dirty_rects, i = 0; i != dirty_count; ++i, ++r)
        RestoreBackground(r);
    for (i = 0; i != *sprite_count; i++) {
        s = sprite_list[i];
        if (s->erase == 0 && s->visible) {
            Rect clip;
            Rect area;
            int16_t j;
            int16_t found;

            found = 0;
            for (r = dirty_rects, j = 0; j != dirty_count; ++j, ++r) {
                if (ClipRect(&clip, &s->rect, r)) {
                    if (found)
                        UnionRect(&area, &area, &clip);
                    else {
                        area = clip;
                        found = 1;
                    }
                }
            }
            if (found)
                DrawSprite(s, &area);
        }
        s->erase = 0;
        s->dirty = 0;
        s->old = s->rect;
    }
    for (r = dirty_rects, i = 0; i != dirty_count; ++i, ++r)
        BlitToScreen(r);
}

void SortSprites(void)
{
    qsort(sprite_list, *sprite_count, sizeof(Sprite *), CompareSprites);
}

int CompareSprites(const void *pa, const void *pb)
{
    Sprite *const *a = pa;
    Sprite *const *b = pb;
    register int16_t d;

    if ((d = (*a)->z - (*b)->z) == 0)
        if ((d = (*a)->layer - (*b)->layer) == 0)
            if ((d = (*a)->y - (*b)->y) == 0)
                d = (*a)->x - (*b)->x;
    return d;
}

void ComputeSpriteRects(void)
{
    Rect r;
    Sprite *s;
    int16_t i;
    Bitmap *bmp;

    for (i = 0; i != *sprite_count; i++) {
        s = sprite_list[i];
        bmp = LockMem(s->handle);
        s->ox = r.x0 = s->x - bmp->x;
        s->oy = r.y0 = s->y - bmp->y;
        r.x1 = r.x0 + bmp->w - 1;
        r.y1 = r.y0 + bmp->h - 1;
        UnlockMem(s->handle);
        if (ClipRect(&s->rect, &screen_rect, &r)) {
            if (s->old.x0 <= s->old.x1) {
                if (ClipRect(&r, &s->rect, &s->old)) {
                    UnionRect(&s->union_rect, &s->rect, &s->old);
                    s->has_union = 1;
                } else
                    s->has_union = 0;
            } else {
                s->union_rect = s->rect;
                s->has_union = 1;
            }
            s->visible = 1;
        } else {
            s->visible = 0;
            s->has_union = 0;
        }
    }
}

/* Draw the part clip of a sprite into the back buffer. */
void DrawSprite(Sprite *s, Rect *clip)
{
    unsigned char *mask;
    int16_t unused1;
    unsigned char *src;
    unsigned char *dst;
    uint16_t w;
    uint16_t h;
    int16_t maskskip;
    int16_t unused2;
    int16_t srcskip;
    int16_t dstskip;
    int16_t unused3;
    int16_t unused4;
    int16_t prio;
    uint16_t n;
    Bitmap *bmp;

    w = clip->x1 - clip->x0 + 1;
    h = clip->y1 - clip->y0 + 1;
    if (w <= 0 || h <= 0)
        Fatal("DrawSprite: w/h <= 0");
    prio = s->z;
    dst = backbuf->data + clip->y0 * 320 + clip->x0;
    dstskip = 320 - w;
    if (s->type == SPRITE_TEXT) {
        MemHandle font_h;
        char *font;
        Rect r;

        font_h = CacheLoad("font", "fnt");
        font = LockMem(font_h);
        r = *clip;
        bmp = LockMem(s->handle);
        r.x0 = (r.x0 - s->ox) >> 3;
        r.y0 = (r.y0 - s->oy) >> 3;
        r.x1 = (r.x1 - s->ox) >> 3;
        r.y1 = (r.y1 - s->oy) >> 3;
        w = r.x1 - r.x0 + 1;
        h = r.y1 - r.y0 + 1;
        prio = s->z;
        dst = backbuf->data + s->oy * 320 + s->ox + (r.y0 << 3) * 320 + (r.x0 << 3);
        dstskip = 2560 - (w << 3);
        src = bmp->data + r.x0 + r.y0 * (bmp->w >> 3);
        srcskip = (bmp->w >> 3) - w;
        while (h--) {
            n = w;
            while (n--) {
                char *glyph;
                int16_t gy;
                int16_t gx;
                char colour;

                colour = (*src >= 0x10 && *src <= 0x1A) ? 0xB3 : s->colour;
                glyph = font + *src * 64;
                for (gy = 0; gy != 8; gy++) {
                    for (gx = 0; gx != 8; gx++) {
                        if (*glyph == 0)
                            *dst = shade_table[*dst];
                        else if (*glyph == 'x')
                            *dst = colour;
                        else
                            *dst = *glyph;
                        glyph++;
                        dst++;
                    }
                    dst += 312;
                }
                src++;
                dst -= 2552;
            }
            src += srcskip;
            dst += dstskip;
        }
        UnlockMem(s->handle);
        UnlockMem(font_h);
        CacheRelease(font_h);
    } else if (s->type == SPRITE_SHADOW) {
        bmp = LockMem(s->handle);
        src = bmp->data + (clip->x0 - s->ox) + (clip->y0 - s->oy) * bmp->w;
        srcskip = bmp->w - w;
        while (h--) {
            n = w;
            while (n--) {
                if (*src)
                    *dst = *src;
                else
                    *dst = shade_table[*dst];
                src++;
                dst++;
            }
            src += srcskip;
            dst += dstskip;
        }
        UnlockMem(s->handle);
    } else if (s->type == SPRITE_RECT)
        ;
    else {
        bmp = LockMem(s->handle);
        src = bmp->data + (clip->x0 - s->ox) + (clip->y0 - s->oy) * bmp->w;
        srcskip = bmp->w - w;
        mask = mask_buf + clip->y0 * 160 + (clip->x0 >> 1);
        maskskip = 160 - (((clip->x1 + 1) >> 1) - (clip->x0 >> 1));
        /* A mask nibble above the sprite's priority hides the pixel. */
        for (; h != 0; h--, src += srcskip, dst += dstskip, mask += 160) {
            uint16_t x = clip->x0;

            for (n = w; n != 0; n--, x++, src++, dst++) {
                unsigned char m = mask[(x >> 1) - (clip->x0 >> 1)];

                m = (x & 1) ? m & 0x0F : m >> 4;
                if (m <= prio && *src)
                    *dst = *src;
            }
        }
        UnlockMem(s->handle);
    }
}

/* Copy r of the background into the back buffer. */
void RestoreBackground(Rect *r)
{
    uint16_t w;
    uint16_t h;
    int16_t unused;
    unsigned char *dst;
    unsigned char *src;

    dst = backbuf->data + r->y0 * 320 + r->x0;
    src = screen->data + r->y0 * 320 + r->x0;
    w = r->x1 - r->x0 + 1;
    h = r->y1 - r->y0 + 1;
    while (h--) {
        MemCpy(dst, src, w);
        dst += 320;
        src += 320;
    }
}

void BlitToScreen(Rect *r)
{
    uint16_t w;
    uint16_t h;
    unsigned char *dst;
    unsigned char *src;

    dst = plat_vram() + r->y0 * 320 + r->x0;
    src = backbuf->data + r->y0 * 320 + r->x0;
    w = r->x1 - r->x0 + 1;
    h = r->y1 - r->y0 + 1;
    while (h--) {
        MemCpy(dst, src, w);
        dst += 320;
        src += 320;
    }
}

/* The topmost sprite with a non-zero pixel at (x, y), ignoring the mouse
 * pointer. */
Sprite *SpriteAt(int16_t x, int16_t y)
{
    Sprite *s;
    int16_t i;
    int16_t dx;
    int16_t dy;
    Bitmap *bmp;

    for (i = *sprite_count - 1; i >= 0; i--) {
        s = sprite_list[i];
        if (s != &mouse_sprite && s->type != SPRITE_RECT
                && x >= s->rect.x0 && x <= s->rect.x1
                && y >= s->rect.y0 && y <= s->rect.y1) {
            bmp = LockMem(s->handle);
            dx = x - s->ox;
            dy = y - s->oy;
            if (s->type == SPRITE_SHADOW || s->type == SPRITE_TEXT
                    || ((unsigned char *)bmp->data)[dy * bmp->w + dx]) {
                UnlockMem(s->handle);
                return s;
            }
            UnlockMem(s->handle);
        }
    }
    return 0;
}

void SpriteSetPos(Sprite *s, int16_t x, int16_t y, uint16_t z)
{
    s->x = x;
    s->y = y;
    s->z = z;
    s->dirty = 1;
}

void SpriteSetBitmap(Sprite *s, MemHandle h)
{
    s->handle = h;
    s->dirty = 1;
}

/* Add s as an invisible rectangle, using h as an empty bitmap header. */
void SpriteAddRect(Sprite *s, MemHandle h, Rect r)
{
    Bitmap *bmp;
    int16_t x0 = r.x0, y0 = r.y0, x1 = r.x1, y1 = r.y1;

    SpriteSetPos(s, x0, y0, 0);
    SpriteSetBitmap(s, h);
    bmp = LockMem(h);
    bmp->x = 0;
    bmp->y = 0;
    bmp->w = x1 - x0 + 1;
    bmp->h = y1 - y0 + 1;
    UnlockMem(h);
    AddSprite(s);
    s->type = SPRITE_RECT;
}

void RedrawSpritesIn(Rect *r)
{
    int16_t i;
    Sprite *s;
    Rect clip;

    if (*sprite_count == 0)
        return;
    for (i = 0; i != *sprite_count; i++) {
        s = sprite_list[i];
        if (s->visible && ClipRect(&clip, &s->rect, r))
            DrawSprite(s, &clip);
    }
}
