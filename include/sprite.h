/* sprite.h - sprites drawn over the background into the back buffer */

#ifndef SPRITE_H
#define SPRITE_H

#include "compat.h"

#include "mem.h"

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef struct Rect {
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
} Rect;
#endif

/* Sprite types. */
#define SPRITE_BITMAP   0       /* bitmap, masked by the EGA priority buffer */
#define SPRITE_RECT     1       /* invisible rectangle, never drawn */
#define SPRITE_SHADOW   2       /* 0 pixels shade the background */
#define SPRITE_TEXT     3       /* bitmap of characters drawn with font.fnt */

#define MAX_SPRITES 32

typedef struct Sprite {
    int16_t x;                  /* 00 */
    int16_t y;                  /* 02 */
    int16_t z;                  /* 04 priority, sort key */
    int16_t layer;              /* 06 second sort key */
    char *name;             /* 08 */
    MemHandle handle;       /* 0A bitmap */
    int16_t type;               /* 0C */
    int16_t colour;             /* 0E text colour */
    int16_t dirty;              /* 10 */
    int16_t has_union;          /* 12 union is valid */
    int16_t visible;            /* 14 */
    int16_t erase;              /* 16 */
    Rect old;               /* 18 rect when last drawn */
    Rect rect;              /* 20 clipped screen rect */
    Rect union_rect;        /* 28 old | rect */
    int16_t ox;                 /* 30 bitmap origin on screen */
    int16_t oy;                 /* 32 */
} Sprite;

extern Sprite **sprite_list;
extern int16_t *sprite_count;
extern Rect screen_rect;
extern Sprite mouse_sprite;

int16_t ClipRect(Rect *out, Rect *a, Rect *b);      /* seg007 */
void UnionRect(Rect *out, Rect *a, Rect *b);    /* seg007 */

void SetSpriteList(Sprite **list, int16_t *count);
void ResetSpriteList(void);
void MarkAllDirty(void);
void ClearSprites(void);
void SpriteInit(Sprite *s);
void AddSprite(Sprite *s);
void DelSprite(Sprite *s);
void SpriteErase(Sprite *s);
void UpdateSprites(void);
void SortSprites(void);
int CompareSprites(const void *a, const void *b);
void ComputeSpriteRects(void);
void DrawSprite(Sprite *s, Rect *clip);
void RestoreBackground(Rect *r);
void BlitToScreen(Rect *r);
Sprite *SpriteAt(int16_t x, int16_t y);
void SpriteSetPos(Sprite *s, int16_t x, int16_t y, uint16_t z);
void SpriteSetBitmap(Sprite *s, MemHandle h);
void SpriteAddRect(Sprite *s, MemHandle h, Rect r);
void RedrawSpritesIn(Rect *r);

#endif
