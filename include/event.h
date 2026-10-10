/* event.h - input events: timer ticks, mouse/joystick/keypad pointer, keys */

#ifndef EVENT_H
#define EVENT_H

#include "compat.h"

/* Event types */
#define EV_TICK     0       /* the BIOS tick count changed */
#define EV_DOWN1    1       /* left button pressed */
#define EV_MOVE     2       /* pointer moved */
#define EV_UP1      3
#define EV_DOWN2    4       /* right button pressed */
#define EV_UP2      5
#define EV_KEY      6

typedef struct Event {
    int16_t type;               /* 00 */
    int16_t key;                /* 02 BIOS scan code and character */
    int16_t shift;              /* 04 BIOS shift state */
    int16_t x;                  /* 06 */
    int16_t y;                  /* 08 */
    int32_t time;              /* 0A */
} Event;

#define EV_MAX      64

/* input_dev */
#define INPUT_KEYS  0
#define INPUT_MOUSE 1
#define INPUT_JOY   2

extern int16_t input_dev;
extern uint32_t ticks;     /* BIOS ticks, updated by ReadClock */
extern uint16_t mouse_x;
extern uint16_t mouse_y;
extern int16_t buttons;

void EventInit(void);
Event *PeekEvent(void);
void PopEvent(void);
Event *GetEvent(Event *e);
void PostEvent(Event *e);
void FlushEvents(void);
int32_t GetTicks(void);
Event *NextEvent(Event *p);
void PollInput(void);
void PollTimer(void);
void PollMouse(void);
void PollKeys(void);
void MouseInit(void);
int16_t ReadPointer(void);
void SetPointer(int16_t x, int16_t y);
void ReadClock(void);
void HookKbd(void);
void UnhookKbd(void);
int16_t TestAndClear(int16_t *p);
int16_t KeyReady(void);
void ReadKey(void);
void MouseShow(void);
void MouseReset(void);

#endif
