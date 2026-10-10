/* menu.h - menus of bitmap buttons loaded from .mnu files */

#ifndef MENU_H
#define MENU_H

#include "compat.h"

#include "mem.h"
#include "sprite.h"

/* An entry of a .mnu file. */
typedef struct MenuEntry {
    char name[10];          /* 00 button bitmap */
    int16_t x;                  /* 0A */
    int16_t y;                  /* 0C */
    int16_t id;                 /* 0E returned when chosen */
} MenuEntry;

/* The state of a menu replaced by one opened over it. */
typedef struct MenuSave {
    Sprite *sprites;        /* 00 */
    int16_t *ids;               /* 02 */
    int32_t disabled;          /* 04 */
    MemHandle handle;       /* 08 */
    int16_t count;              /* 0A */
    int16_t sel;                /* 0C */
    struct MenuSave *next;  /* 0E */
} MenuSave;

/* Menu directions for MenuMove */
#define MENU_NEXT   0       /* right arrow: the next button in reading order */
#define MENU_PREV   1
#define MENU_UP     2
#define MENU_DOWN   3
#define MENU_FIRST  4
#define MENU_LAST   5

/* MenuLoop results besides the button ids */
#define MENU_NONE   (-1)    /* clicked outside the buttons */
#define MENU_ESCAPE (-2)
#define MENU_TIMEOUT (-3)
#define MENU_CANCEL (-4)

extern Sprite *menu_sprites;
extern int16_t *menu_ids;
extern int32_t menu_disabled;
extern int16_t menu_sel;
extern int16_t menu_count;

int16_t MenuHit(Sprite *s, int16_t n, int16_t x, int16_t y);
void MenuMove(Sprite *s, int16_t n, int16_t cur, int16_t dir);
void DrawButtonFrame(MemHandle h, int16_t c);
void OptionsMenu(int16_t x, int16_t y);
int16_t ActionPanel(void);
void LoadMenu(char *name, int16_t x, int16_t y);
void MenuShow(int32_t mask);
void MenuDisable(int32_t mask);
void MenuEnable(int32_t mask);
void MenuHome(void);
int16_t MenuLoop(uint32_t timeout, int16_t keys);
int16_t MenuWait(uint32_t timeout);
void MenuClose(void);

#endif
