/* seg005: input events */

#include "compat.h"
#include "mem.h"
#include "sprite.h"
#include "sound.h"
#include "event.h"

/* Globals this file owns (declared in the headers). */
int16_t buttons;
uint32_t ticks;
int16_t input_dev;

void MsgBox(char *title, char *msg, int16_t a, int16_t b, int16_t c, int16_t d);    /* seg008: message box */
void QuitMenu(int16_t a, int16_t b);       /* seg019: quit dialog */

/* seg027: joystick */
extern int16_t joy_x;
extern int16_t joy_y;
extern int16_t joy_buttons;
extern int16_t joy_present;
void JoyCalibrate(void);
void JoyUpdate(void);

extern Rect clip_rect;
extern int16_t sound_closed;
Event *ev_write;
Event *ev_read;
Event ev_queue[EV_MAX];
unsigned char move_pending;      /* an EV_MOVE is queued */
unsigned char tick_pending;      /* an EV_TICK is queued */
unsigned char mouse_present;
uint32_t last_tick;
int16_t old_buttons;
int16_t key_code;
int16_t key_shift;
int16_t key_b1;                      /* keypad + held: left button */
int16_t key_b2;                      /* keypad - held: right button */
extern int16_t kbd_mouse;                   /* keypad arrows jump the pointer */
int16_t arrow_key;                   /* keypad arrow held */
int16_t arrow_changed;
int16_t btn_key;
int16_t btn_changed;
int16_t shift_key;

int16_t last_mx = 160;
int16_t last_my = 100;
int16_t joy_recal = 0;
int16_t key_accel = 1;                      /* held keypad arrows speed up */
uint32_t key_tick0 = 0;
int32_t key_unused = 0;
uint32_t last_key_tick = 0;

void EventInit(void)
{
    ev_read = ev_write = ev_queue;
    tick_pending = move_pending = 0;
    MouseInit();
    mouse_present = input_dev == INPUT_MOUSE;
}

Event *PeekEvent(void)
{
    PollInput();
    if (ev_read != ev_write)
        return ev_read;
    else
        return 0;
}

void PopEvent(void)
{
    if (ev_read == ev_write)
        return;
    move_pending -= ev_read->type == EV_MOVE;
    tick_pending -= ev_read->type == EV_TICK;
    ev_read = NextEvent(ev_read);
}

Event *GetEvent(Event *e)
{
    Event *p;

    if ((p = PeekEvent()) != 0) {
        MemCpy(e, p, sizeof(Event));
        PopEvent();
        return e;
    } else
        return 0;
}

/* Queue a copy of e. Moves and ticks are merged into one already queued. */
void PostEvent(Event *e)
{
    if (e->type == EV_MOVE && move_pending) {
        register Event *p;

        for (p = ev_read; p != ev_write; p = NextEvent(p)) {
            if (p->type == EV_MOVE) {
                *p = *e;
                return;
            }
        }
    } else if (e->type == EV_TICK && tick_pending)
        return;
    if (e->type == EV_MOVE)
        move_pending = 1;
    if (e->type == EV_TICK)
        tick_pending = 1;
    *ev_write = *e;
    ev_write = NextEvent(ev_write);
}

void FlushEvents(void)
{
    Event e;

    while (GetEvent(&e))
        ;
    ev_read = ev_write = ev_queue;
    tick_pending = move_pending = 0;
}

int32_t GetTicks(void)
{
    ReadClock();
    return ticks;
}

Event *NextEvent(Event *p)
{
    if (++p == &ev_queue[EV_MAX])
        p = ev_queue;
    if (ev_read == p)
        Fatal("event overflow");
    return p;
}

/* Turn the timer, pointer and keyboard state into events. */
void PollInput(void)
{
    if (sound_closed) {
        sound_closed = 0;
        MsgBox("Warning",
                  "#GENE\\GENER027#Insufficient memory to continue playing music and sound effects.",
                  300, 199, 176, 0);
    }
    if (joy_recal == 1) {
        joy_recal = 0;
        JoyCalibrate();
        MsgBox("Joystick",
                  "Move the joystick from upper-left to lower-right and then press <ENTER>.",
                  300, 199, 176, 0);
    }
    PollTimer();
    PollMouse();
    PollKeys();
}

void PollTimer(void)
{
    Event e;

    ReadClock();
    if (ticks != last_tick) {
        e.type = EV_TICK;
        last_tick = e.time = ticks;
        PostEvent(&e);
    }
}

void PollMouse(void)
{
    Event e;

    if (ReadPointer()) {
        if (mouse_x != last_mx || mouse_y != last_my) {
            e.type = EV_MOVE;
            last_mx = e.x = mouse_x;
            last_my = e.y = mouse_y;
            PostEvent(&e);
        }
        if ((buttons & 1) != (old_buttons & 1)) {
            old_buttons ^= 1;
            e.type = (buttons & 1) ? EV_DOWN1 : EV_UP1;
            e.x = mouse_x;
            e.y = mouse_y;
            PostEvent(&e);
        }
        if ((buttons & 2) != (old_buttons & 2)) {
            old_buttons ^= 2;
            e.type = (buttons & 2) ? EV_DOWN2 : EV_UP2;
            e.x = mouse_x;
            e.y = mouse_y;
            PostEvent(&e);
        }
    }
}

/* Keys; Ctrl-S sound, Ctrl-M music, Ctrl-P pause, Ctrl-Q quit and Ctrl-J
 * joystick are handled here. */
void PollKeys(void)
{
    Event e;

    while (KeyReady()) {
        ReadKey();
        if (key_code == 0x1F13)
            SetSfxOn(!sfx_on);
        else if (key_code == 0x320D)
            SetMusicOn(!music_on);
        else if (key_code == 0x1910)
            getch();
        else if (key_code == 0x1011)
            QuitMenu(300, 199);
        else if (!mouse_present && key_code == 0x240A) {
            joy_present = 1;
            joy_recal = 1;
        } else if (key_code) {
            e.type = EV_KEY;
            e.key = key_code;
            e.shift = key_shift;
            PostEvent(&e);
        }
    }
}

void MouseInit(void)
{
    if (input_dev == INPUT_MOUSE) {
        if (!plat_mouse_init())
            input_dev = INPUT_KEYS;
    } else if (input_dev == INPUT_JOY) {
        joy_present = plat_joy_present();
        JoyCalibrate();
        joy_x = 160;
        joy_y = 100;
        if (joy_present == 0)
            input_dev = INPUT_KEYS;
    }
}

/* Read the pointer position and buttons from the keypad, mouse or
 * joystick. */
int16_t ReadPointer(void)
{
    int16_t keys;

    keys = (key_b1 ? 1 : 0) | (key_b2 ? 2 : 0);
    if (kbd_mouse) {
        /* the arrows jump to the middle or the edges of the view */
        if (arrow_key || TestAndClear(&arrow_changed)) {
            int16_t dx;
            int16_t dy;
            int16_t cx;
            int16_t cy;
            int16_t hw;
            int16_t hh;

            switch (arrow_key) {
            case 0x47:
                dx = -1, dy = -1;
                break;
            case 0x48:
                dx = 0, dy = -1;
                break;
            case 0x49:
                dx = 1, dy = -1;
                break;
            case 0x4B:
                dx = -1, dy = 0;
                break;
            case 0x4D:
                dx = 1, dy = 0;
                break;
            case 0x4F:
                dx = -1, dy = 1;
                break;
            case 0x50:
                dx = 0, dy = 1;
                break;
            case 0x51:
                dx = 1, dy = 1;
                break;
            case 0x4A:
            case 0x4C:
            case 0x4E:
            default:
                dx = 0, dy = 0;
                break;
            }
            dx <<= 1, dy <<= 1;
            cx = (clip_rect.x0 + clip_rect.x1) / 2;
            cy = (clip_rect.y0 + clip_rect.y1) / 2;
            hw = (clip_rect.x1 - clip_rect.x0) / 4;
            hh = (clip_rect.y1 - clip_rect.y0) / 4;
            mouse_x = dx * hw + cx;
            mouse_y = dy * hh + cy;
            SetPointer(mouse_x, mouse_y);
            buttons = keys;
            return 1;
        }
    } else {
        /* the arrows move the pointer, faster the longer they are held */
        if (arrow_key && key_accel) {
            if (ticks != last_key_tick) {
                uint16_t step;
                uint32_t saved;

                if (TestAndClear(&arrow_changed)) {
                    key_tick0 = ticks;
                    step = 1;
                } else if (ticks <= key_tick0 + 64) {
                    step = (uint16_t)(ticks - key_tick0) / 4;
                    step = (step + 1) / 2 * step;
                } else
                    step = 128;
                switch (arrow_key) {
                case 0x47:
                    mouse_x -= step, mouse_y -= step;
                    break;
                case 0x48:
                    mouse_y -= step;
                    break;
                case 0x49:
                    mouse_x += step, mouse_y -= step;
                    break;
                case 0x4B:
                    mouse_x -= step;
                    break;
                case 0x4D:
                    mouse_x += step;
                    break;
                case 0x4F:
                    mouse_x -= step, mouse_y += step;
                    break;
                case 0x50:
                    mouse_y += step;
                    break;
                case 0x51:
                    mouse_x += step, mouse_y += step;
                    break;
                }
                if ((int16_t)mouse_x < 0)
                    mouse_x = 0;
                else if (mouse_x > 319)
                    mouse_x = 319;
                if ((int16_t)mouse_y < 0)
                    mouse_y = 0;
                else if (mouse_y > 199)
                    mouse_y = 199;
                saved = key_tick0;
                SetPointer(mouse_x, mouse_y);
                key_tick0 = saved;
                buttons = keys;
            }
            last_key_tick = ticks;
            return 1;
        }
    }
    if (mouse_present) {
        int x, y, b;

        plat_mouse_read(&x, &y, &b);
        buttons = (b & 3) | keys;
        mouse_x = x;
        mouse_y = y;
        return 1;
    }
    if (joy_present) {
        JoyUpdate();
        buttons = joy_buttons | keys;
        mouse_x = joy_x;
        mouse_y = joy_y;
        return 1;
    }
    return 0;
}

void SetPointer(int16_t x, int16_t y)
{
    key_tick0 = ticks;
    if (mouse_present) {
        plat_mouse_set(x, y);
    } else if (joy_present) {
        joy_x = x;
        joy_y = y;
    }
}

/* The BIOS tick count.  Polling it is how the game's loops pass time, so
 * this is where the host gets to run. */
void ReadClock(void)
{
    plat_pump();
    ticks = plat_ticks();
}

/* A raw key event (the original's INT 9 handler): track the keypad keys
 * used as a pointer. */
void KbdScan(int code, int up)
{
    switch (code) {
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4B:
    case 0x4D:
    case 0x4F:
    case 0x50:
    case 0x51:
        if (up) {
            if (arrow_key == code)
                arrow_key = 0, arrow_changed = 1;
        } else if (arrow_key != code)
            arrow_key = code, arrow_changed = 1;
        break;
    case 0x52:
    case 0x53:
        if (up) {
            if (btn_key == code)
                btn_key = 0, btn_changed = 1;
        } else if (arrow_key != code)
            btn_key = code, btn_changed = 1;
        break;
    case 0x2A:
    case 0x36:
        if (up) {
            if (shift_key == code)
                shift_key = 0;
        } else
            shift_key = code;
        break;
    case 0x4A:
        key_b2 = !up;
        break;
    case 0x4E:
        key_b1 = !up;
        break;
    }
}

void HookKbd(void)
{
}

void UnhookKbd(void)
{
}

int16_t TestAndClear(int16_t *p)
{
    int16_t v;

    v = *p;
    *p = 0;
    return v;
}

int16_t KeyReady(void)
{
    return plat_key_ready();
}

void ReadKey(void)
{
    key_shift = plat_key_shift();
    key_code = 0;
    while (plat_key_ready()) {
        int c = plat_key_read();

        if (c) {
            key_code = c;
            break;
        }
    }
}

/* The pointer is drawn by the game; the host's is hidden. */
void MouseShow(void)
{
}

void MouseReset(void)
{
}
