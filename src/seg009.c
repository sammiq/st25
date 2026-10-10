/* seg009: the inventory */

#include "compat.h"
#include <string.h>
#include "mem.h"
#include "cache.h"
#include "sprite.h"
#include "event.h"
#include "sound.h"

#define ITEM_FIRST  0x40        /* item ids start here */
#define NUM_ITEMS   137

typedef struct Item {
    int16_t have;                   /* 00 carried */
    int16_t code;                   /* 02 */
    char name[12];              /* 04 bitmap name */
} Item;

/* As menu.h's MenuEntry. */
typedef struct InvEntry {
    char name[10];              /* 00 */
    int16_t x;                      /* 0A */
    int16_t y;                      /* 0C */
    int16_t code;                   /* 0E */
} InvEntry;

void BridgeSfx(int16_t a);              /* seg001 */
void SaveWrite(void *p, int16_t n);     /* seg019: save game I/O */
void SaveRead(void *p, int16_t n);

/* seg014 */
int16_t MenuHit(Sprite *s, int16_t n, uint16_t x, uint16_t y);
void MenuMove(Sprite *s, int16_t n, int16_t cur, int16_t dir);
void DrawButtonFrame(MemHandle h, int16_t c);

extern Item items[NUM_ITEMS];
InvEntry inv_ents[NUM_ITEMS];
extern int16_t key_accel;

/* What the captain starts with. */
int16_t start_items[6] = { 0x40, 0x41, 0x44, 0x45, 0x47, 0x7F };

void ItemGive(int16_t id)
{
    items[id - ITEM_FIRST].have = 1;
}

void ItemTake(int16_t id)
{
    items[id - ITEM_FIRST].have = 0;
}

void ItemsInit(void)
{
    int16_t i;

    for (i = 0x40; i != 0x89; i++)
        ItemTake(i);
    for (i = 6; i != 0; i--)
        ItemGive(start_items[i - 1]);
}

int16_t ItemHave(int16_t id)
{
    return items[id - ITEM_FIRST].have;
}

void ItemsSave(void)
{
    int16_t i;
    unsigned char c;

    for (i = 0; i < NUM_ITEMS; i++) {
        c = items[i].have;
        SaveWrite(&c, 1);
    }
}

void ItemsLoad(void)
{
    int16_t i;
    unsigned char c;

    for (i = 0; i < NUM_ITEMS; i++) {
        SaveRead(&c, 1);
        items[i].have = c;
    }
}

/* Show the items carried in rows of five at (x, y) and let the player
 * pick one. Returns its code, or -1. restore: put the pointer back. */
int16_t PickItem(int16_t x, int16_t y, int16_t restore)
{
    Sprite *sprites;
    char name[10];
    int16_t i;
    int16_t n;
    int16_t sel;
    int16_t running;
    int16_t count;
    Event *e;
    InvEntry *ents;
    uint16_t old_x;
    uint16_t old_y;
    int16_t old_accel;
    MemHandle h;

    sel = -1;
    old_x = mouse_x;
    old_y = mouse_y;
    old_accel = key_accel;
    key_accel = 0;
    n = 0;
    for (i = 0; i < NUM_ITEMS; i++) {
        if (items[i].have) {
            strcpy(inv_ents[n].name, items[i].name);
            inv_ents[n].x = n % 5 * 32 + x;
            inv_ents[n].y = n / 5 * 32 + y;
            inv_ents[n].code = items[i].code;
            n++;
        }
    }
    count = n;
    ents = inv_ents;
    sprites = AllocMemPtr(count * sizeof(Sprite));
    for (i = 0; i != count; i++) {
        AddSprite(&sprites[i]);
        sprites[i].type = SPRITE_SHADOW;
        SpriteSetPos(&sprites[i], ents[i].x, ents[i].y, 15);
        MemCpy(name, ents[i].name, 10);
        h = CacheLoad(name, "bmp");
        sprites[i].handle = h;
        sprites[i].layer = 8;
    }
    MenuMove(sprites, count, -1, 4);
    running = 1;
    while (running) {
        MusicTick();
        if ((e = PeekEvent()) != 0) {
            switch (e->type) {
            case EV_KEY:
                switch (((uint16_t)e->key >> 8) & 0xFF) {
                case 0x1C:
                case 0x3B:
                    goto pick;
                case 0x01:
                case 0x3C:
                    goto cancel;
                case 0x4D:
                    MenuMove(sprites, count, sel, 0);
                    break;
                case 0x4B:
                    MenuMove(sprites, count, sel, 1);
                    break;
                case 0x48:
                case 0x49:
                    MenuMove(sprites, count, sel, 2);
                    break;
                case 0x50:
                case 0x51:
                    MenuMove(sprites, count, sel, 3);
                    break;
                case 0x47:
                    MenuMove(sprites, count, sel, 4);
                    break;
                case 0x4F:
                    MenuMove(sprites, count, sel, 5);
                    break;
                }
                break;
            case EV_DOWN1:
            pick:
                running = 0;
                break;
            case EV_DOWN2:
            cancel:
                running = 0;
                if (sel != -1)
                    DrawButtonFrame(sprites[sel].handle, 0);
                sel = -1;
                break;
            case EV_TICK:
                i = MenuHit(sprites, count, mouse_x, mouse_y);
                if (i != sel) {
                    if (sel != -1) {
                        DrawButtonFrame(sprites[sel].handle, 0);
                        sprites[sel].dirty = 1;
                    }
                    if (i != -1) {
                        DrawButtonFrame(sprites[i].handle, 15);
                        sprites[i].dirty = 1;
                    }
                    sel = i;
                }
                SpriteSetPos(&mouse_sprite, mouse_x, mouse_y, 15);
                UpdateSprites();
                break;
            }
            PopEvent();
        }
    }
    BridgeSfx(16);
    if (sel >= 0)
        DrawButtonFrame(sprites[sel].handle, 0);
    for (i = 0; i != count; i++)
        SpriteErase(&sprites[i]);
    UpdateSprites();
    for (i = 0; i != count; i++) {
        CacheRelease(sprites[i].handle);
        DelSprite(&sprites[i]);
    }
    FreeMemPtr(sprites);
    if (sel >= 0)
        sel = inv_ents[sel].code;
    if (restore) {
        mouse_x = old_x;
        mouse_y = old_y;
        SpriteSetPos(&mouse_sprite, mouse_x, mouse_y, 15);
        SetPointer(mouse_x, mouse_y);
    }
    key_accel = old_accel;
    return sel;
}
