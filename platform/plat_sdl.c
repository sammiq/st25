/* plat_sdl.c - the host on SDL2: window, keyboard, mouse, joystick, timing.
 *
 * Only this layer includes SDL.  What the game saw as VGA memory and the DAC,
 * the BIOS keyboard and the mouse driver is kept here and drawn each frame.
 *
 * Test hooks (environment):
 *   STARTREK_SHOT=file.ppm   write the screen there and quit after
 *   STARTREK_SHOT_MS=n       n milliseconds (default 3000)
 *   STARTREK_INPUT=script    scripted events: "ms:key:0x1C0D,ms:click:x:y,..."
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "platform.h"

void plat_set_data_dir(const char *dir);
void plat_audio_init(void);
void plat_audio_quit(void);

/* ---- state ---- */

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static uint8_t vram[65536];
static uint8_t palette[768];            /* 6 bit components */
static uint32_t colours[256];           /* what the texture takes */
static int pal_dirty = 1;
static uint32_t last_present;

static int opt_scale = 3;
static int opt_fullscreen;
static int opt_square_pixels;
static int quitting;

/* ---- keyboard ---- */

#define KEYQ 64
static int keyq[KEYQ];
static int key_head, key_tail;
static int shift_state;                 /* BIOS flags */

/* SDL scancode -> XT scan code */
static int xt_scan(SDL_Scancode sc)
{
    switch (sc) {
    case SDL_SCANCODE_ESCAPE: return 0x01;
    case SDL_SCANCODE_1: return 0x02; case SDL_SCANCODE_2: return 0x03; case SDL_SCANCODE_3: return 0x04;
    case SDL_SCANCODE_4: return 0x05; case SDL_SCANCODE_5: return 0x06; case SDL_SCANCODE_6: return 0x07;
    case SDL_SCANCODE_7: return 0x08; case SDL_SCANCODE_8: return 0x09; case SDL_SCANCODE_9: return 0x0A;
    case SDL_SCANCODE_0: return 0x0B; case SDL_SCANCODE_MINUS: return 0x0C; case SDL_SCANCODE_EQUALS: return 0x0D;
    case SDL_SCANCODE_BACKSPACE: return 0x0E; case SDL_SCANCODE_TAB: return 0x0F;
    case SDL_SCANCODE_Q: return 0x10; case SDL_SCANCODE_W: return 0x11; case SDL_SCANCODE_E: return 0x12;
    case SDL_SCANCODE_R: return 0x13; case SDL_SCANCODE_T: return 0x14; case SDL_SCANCODE_Y: return 0x15;
    case SDL_SCANCODE_U: return 0x16; case SDL_SCANCODE_I: return 0x17; case SDL_SCANCODE_O: return 0x18;
    case SDL_SCANCODE_P: return 0x19; case SDL_SCANCODE_LEFTBRACKET: return 0x1A;
    case SDL_SCANCODE_RIGHTBRACKET: return 0x1B; case SDL_SCANCODE_RETURN: return 0x1C;
    case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: return 0x1D;
    case SDL_SCANCODE_A: return 0x1E; case SDL_SCANCODE_S: return 0x1F; case SDL_SCANCODE_D: return 0x20;
    case SDL_SCANCODE_F: return 0x21; case SDL_SCANCODE_G: return 0x22; case SDL_SCANCODE_H: return 0x23;
    case SDL_SCANCODE_J: return 0x24; case SDL_SCANCODE_K: return 0x25; case SDL_SCANCODE_L: return 0x26;
    case SDL_SCANCODE_SEMICOLON: return 0x27; case SDL_SCANCODE_APOSTROPHE: return 0x28;
    case SDL_SCANCODE_GRAVE: return 0x29; case SDL_SCANCODE_LSHIFT: return 0x2A;
    case SDL_SCANCODE_BACKSLASH: return 0x2B;
    case SDL_SCANCODE_Z: return 0x2C; case SDL_SCANCODE_X: return 0x2D; case SDL_SCANCODE_C: return 0x2E;
    case SDL_SCANCODE_V: return 0x2F; case SDL_SCANCODE_B: return 0x30; case SDL_SCANCODE_N: return 0x31;
    case SDL_SCANCODE_M: return 0x32; case SDL_SCANCODE_COMMA: return 0x33; case SDL_SCANCODE_PERIOD: return 0x34;
    case SDL_SCANCODE_SLASH: return 0x35; case SDL_SCANCODE_RSHIFT: return 0x36;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37; case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: return 0x38;
    case SDL_SCANCODE_SPACE: return 0x39; case SDL_SCANCODE_CAPSLOCK: return 0x3A;
    case SDL_SCANCODE_F1: return 0x3B; case SDL_SCANCODE_F2: return 0x3C; case SDL_SCANCODE_F3: return 0x3D;
    case SDL_SCANCODE_F4: return 0x3E; case SDL_SCANCODE_F5: return 0x3F; case SDL_SCANCODE_F6: return 0x40;
    case SDL_SCANCODE_F7: return 0x41; case SDL_SCANCODE_F8: return 0x42; case SDL_SCANCODE_F9: return 0x43;
    case SDL_SCANCODE_F10: return 0x44;
    case SDL_SCANCODE_HOME: case SDL_SCANCODE_KP_7: return 0x47;
    case SDL_SCANCODE_UP: case SDL_SCANCODE_KP_8: return 0x48;
    case SDL_SCANCODE_PAGEUP: case SDL_SCANCODE_KP_9: return 0x49;
    case SDL_SCANCODE_KP_MINUS: return 0x4A;
    case SDL_SCANCODE_LEFT: case SDL_SCANCODE_KP_4: return 0x4B;
    case SDL_SCANCODE_KP_5: return 0x4C;
    case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_KP_6: return 0x4D;
    case SDL_SCANCODE_KP_PLUS: return 0x4E;
    case SDL_SCANCODE_END: case SDL_SCANCODE_KP_1: return 0x4F;
    case SDL_SCANCODE_DOWN: case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_PAGEDOWN: case SDL_SCANCODE_KP_3: return 0x51;
    case SDL_SCANCODE_INSERT: case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_DELETE: case SDL_SCANCODE_KP_PERIOD: return 0x53;
    case SDL_SCANCODE_F11: return 0x57; case SDL_SCANCODE_F12: return 0x58;
    case SDL_SCANCODE_KP_ENTER: return 0x1C;
    case SDL_SCANCODE_KP_DIVIDE: return 0x35;
    default: return 0;
    }
}

/* The ASCII half of a BIOS key for a printable key (US layout). */
static int ascii_of(SDL_Keycode sym, int shift, int caps, int ctrl)
{
    static const char shifted[] = "~!@#$%^&*()_+{}|:\"<>?";
    static const char plain[]   = "`1234567890-=[]\\;',./";
    int i;

    if (sym >= 'a' && sym <= 'z') {
        if (ctrl)
            return sym & 0x1F;
        return (shift ^ caps) ? sym - 32 : sym;
    }
    if (sym < 0x20 || sym > 0x7E)
        return 0;
    if (shift)
        for (i = 0; plain[i]; i++)
            if (plain[i] == sym)
                return shifted[i];
    return sym;
}

static void key_push(int code)
{
    int next = (key_head + 1) % KEYQ;

    if (next != key_tail) {
        keyq[key_head] = code;
        key_head = next;
    }
}

static void shift_bit(int bit, int down)
{
    if (down)
        shift_state |= bit;
    else
        shift_state &= ~bit;
}

static void handle_key(const SDL_KeyboardEvent *k)
{
    int down = k->type == SDL_KEYDOWN;
    int sc = xt_scan(k->keysym.scancode);
    SDL_Keycode sym = k->keysym.sym;
    int shift, ctrl, alt, caps, ascii, code;

    switch (k->keysym.scancode) {
    case SDL_SCANCODE_RSHIFT: shift_bit(1, down); break;
    case SDL_SCANCODE_LSHIFT: shift_bit(2, down); break;
    case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: shift_bit(4, down); break;
    case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: shift_bit(8, down); break;
    default: break;
    }
    if (sc)
        KbdScan(sc, !down);
    if (!down || !sc)
        return;
    shift = (shift_state & 3) != 0;
    ctrl = (shift_state & 4) != 0;
    alt = (shift_state & 8) != 0;
    caps = (SDL_GetModState() & KMOD_CAPS) != 0;
    switch (k->keysym.scancode) {
    case SDL_SCANCODE_LSHIFT: case SDL_SCANCODE_RSHIFT: case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL:
    case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: case SDL_SCANCODE_CAPSLOCK:
        return;
    case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: ascii = ctrl ? 0x0A : 0x0D; break;
    case SDL_SCANCODE_ESCAPE: ascii = 0x1B; break;
    case SDL_SCANCODE_BACKSPACE: ascii = ctrl ? 0x7F : 0x08; break;
    case SDL_SCANCODE_TAB: ascii = shift ? 0 : 0x09; if (shift) sc = 0x0F; break;
    case SDL_SCANCODE_SPACE: ascii = ' '; break;
    case SDL_SCANCODE_HOME: case SDL_SCANCODE_UP: case SDL_SCANCODE_PAGEUP: case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_END: case SDL_SCANCODE_DOWN: case SDL_SCANCODE_PAGEDOWN:
    case SDL_SCANCODE_INSERT: case SDL_SCANCODE_DELETE:
    case SDL_SCANCODE_KP_1: case SDL_SCANCODE_KP_2: case SDL_SCANCODE_KP_3: case SDL_SCANCODE_KP_4:
    case SDL_SCANCODE_KP_5: case SDL_SCANCODE_KP_6: case SDL_SCANCODE_KP_7: case SDL_SCANCODE_KP_8:
    case SDL_SCANCODE_KP_9: case SDL_SCANCODE_KP_0: case SDL_SCANCODE_KP_PERIOD:
        ascii = 0;
        break;
    default:
        if (sc >= 0x3B && sc <= 0x44)       /* F1-F10 */
            ascii = 0;
        else if (alt)
            ascii = 0;
        else
            ascii = ascii_of(sym, shift, caps, ctrl);
        break;
    }
    code = sc << 8 | ascii;
    if (sc >= 0x3B && sc <= 0x44) {         /* shifted function keys have their own codes */
        if (alt) code = (sc + 0x2D) << 8;
        else if (ctrl) code = (sc + 0x23) << 8;
        else if (shift) code = (sc + 0x19) << 8;
    }
    if (sc == 0x4A || sc == 0x4E) {          /* keypad minus and plus */
        code = sc << 8 | (sc == 0x4A ? '-' : '+');
    }
    key_push(code);
}

int plat_key_ready(void)
{
    plat_pump();
    return key_head != key_tail;
}

int plat_key_read(void)
{
    int c;

    plat_pump();
    if (key_head == key_tail)
        return 0;
    c = keyq[key_tail];
    key_tail = (key_tail + 1) % KEYQ;
    return c;
}

int plat_key_shift(void)
{
    int f = shift_state;
    SDL_Keymod m = SDL_GetModState();

    if (m & KMOD_NUM)
        f |= 0x20;
    if (m & KMOD_CAPS)
        f |= 0x40;
    return f;
}

int plat_key_wait(void)
{
    while (!plat_key_ready())
        plat_delay(5);
    return plat_key_read();
}

/* ---- mouse ---- */

static int mouse_x = 160, mouse_y = 100, mouse_buttons;
static int have_mouse;

int plat_mouse_init(void)
{
    have_mouse = 1;
    return 1;
}

void plat_mouse_read(int *x, int *y, int *buttons)
{
    plat_pump();
    *x = mouse_x;
    *y = mouse_y;
    *buttons = mouse_buttons;
}

/* Window position -> 320x200 */
static void set_mouse_from_window(int wx, int wy)
{
    int w, h;
    float sx, sy;
    int x, y;

    SDL_GetWindowSize(window, &w, &h);
    SDL_RenderWindowToLogical(renderer, wx, wy, &sx, &sy);
    x = (int)sx;
    y = (int)(sy * VGA_H / (opt_square_pixels ? VGA_H : 240));
    mouse_x = x < 0 ? 0 : x > VGA_W - 1 ? VGA_W - 1 : x;
    mouse_y = y < 0 ? 0 : y > VGA_H - 1 ? VGA_H - 1 : y;
}

void plat_mouse_set(int x, int y)
{
    int lw = VGA_W, lh = opt_square_pixels ? VGA_H : 240;
    int wx, wy;

    mouse_x = x;
    mouse_y = y;
    if (!window || !(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS))
        return;
    /* logical -> window coordinates */
    SDL_RenderLogicalToWindow(renderer, (float)x, (float)y * lh / VGA_H, &wx, &wy);
    (void)lw;
    SDL_WarpMouseInWindow(window, wx, wy);
}

/* ---- joystick ---- */

static SDL_Joystick *joy;

int plat_joy_present(void)
{
    return joy != NULL;
}

void plat_joy_read(int *x, int *y, int *buttons)
{
    plat_pump();
    if (!joy) {
        *x = *y = *buttons = 0;
        return;
    }
    *x = SDL_JoystickGetAxis(joy, 0) / 256;
    *y = SDL_JoystickGetAxis(joy, 1) / 256;
    *buttons = (SDL_JoystickGetButton(joy, 0) ? 1 : 0) | (SDL_JoystickGetButton(joy, 1) ? 2 : 0);
}

/* ---- time ---- */

uint32_t plat_msec(void)
{
    return SDL_GetTicks();
}

static uint32_t spin_ms;
static int spin_count;

uint32_t plat_ticks(void)
{
    uint32_t ms = SDL_GetTicks();

    /* the game polls this in loops: don't burn the CPU while nothing happens */
    if (ms == spin_ms) {
        if (++spin_count >= 100) {
            spin_count = 0;
            SDL_Delay(1);
        }
    } else {
        spin_ms = ms;
        spin_count = 0;
    }
    return (uint32_t)((uint64_t)ms * 18207 / 1000000);
}

void plat_delay(int ms)
{
    SDL_Delay(ms);
}

/* ---- video ---- */

uint8_t *plat_vram(void)
{
    return vram;
}

void plat_set_palette(int first, int count, const uint8_t *rgb)
{
    int i;

    if (first < 0 || count < 0 || first + count > 256)
        return;
    memcpy(palette + first * 3, rgb, count * 3);
    for (i = first; i < first + count; i++) {
        const uint8_t *c = palette + i * 3;
        uint8_t r = c[0] & 63, g = c[1] & 63, b = c[2] & 63;

        colours[i] = 0xFF000000u | (uint32_t)(r << 2 | r >> 4) << 16 | (uint32_t)(g << 2 | g >> 4) << 8
                   | (b << 2 | b >> 4);
    }
    pal_dirty = 1;
}

static void present(void)
{
    uint32_t *pix;
    int pitch, y, x;

    if (!texture)
        return;
    if (SDL_LockTexture(texture, NULL, (void **)&pix, &pitch) != 0)
        return;
    for (y = 0; y < VGA_H; y++) {
        uint32_t *row = (uint32_t *)((uint8_t *)pix + y * pitch);
        const uint8_t *src = vram + y * VGA_W;

        for (x = 0; x < VGA_W; x++)
            row[x] = colours[src[x]];
    }
    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    pal_dirty = 0;
}

/* ---- test hooks ---- */

static const char *shot_file;
static uint32_t shot_ms = 3000;
static uint32_t start_ms;
static uint32_t shot_list[32];          /* STARTREK_SHOTS=ms,ms,...: several shots, file.N.ppm */
static int nshots, shot_next;

typedef struct Script {
    uint32_t at;
    int type;                       /* 0 key, 1 click, 2 move, 3 release */
    int a, b;
} Script;

static Script script[256];
static int nscript, script_pos;

static void write_shot(const char *name)
{
    FILE *f = fopen(name, "wb");
    int i;

    if (!f)
        return;
    fprintf(f, "P6\n%d %d\n255\n", VGA_W, VGA_H);
    for (i = 0; i < VGA_W * VGA_H; i++) {
        uint32_t c = colours[vram[i]];
        uint8_t rgb[3] = { c >> 16, c >> 8, c };

        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

static void parse_script(const char *s)
{
    while (*s && nscript < 256) {
        Script *e = &script[nscript];
        char kind[16];
        int n = 0;

        e->at = strtoul(s, (char **)&s, 10);
        if (*s != ':')
            break;
        s++;
        while (*s && *s != ':' && *s != ',' && n < 15)
            kind[n++] = *s++;
        kind[n] = 0;
        e->a = e->b = 0;
        if (*s == ':') {
            e->a = strtol(s + 1, (char **)&s, 0);
            if (*s == ':')
                e->b = strtol(s + 1, (char **)&s, 0);
        }
        e->type = !strcmp(kind, "key") ? 0 : !strcmp(kind, "click") ? 1 : !strcmp(kind, "move") ? 2 : 3;
        nscript++;
        if (*s == ',')
            s++;
    }
}

static void run_script(uint32_t now)
{
    while (script_pos < nscript && script[script_pos].at <= now - start_ms) {
        Script *e = &script[script_pos++];

        switch (e->type) {
        case 0: key_push(e->a); break;
        case 1: mouse_x = e->a; mouse_y = e->b; mouse_buttons = 1; break;
        case 3: mouse_buttons = 0; break;
        case 2: mouse_x = e->a; mouse_y = e->b; break;
        }
    }
}

/* ---- events ---- */

void plat_pump(void)
{
    SDL_Event e;
    uint32_t now = SDL_GetTicks();

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            plat_quit();
            exit(0);
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN
                    && (e.key.keysym.mod & KMOD_ALT)) {
                SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
                                        ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                break;
            }
            handle_key(&e.key);
            break;
        case SDL_MOUSEMOTION:
            set_mouse_from_window(e.motion.x, e.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int bit = e.button.button == SDL_BUTTON_LEFT ? 1 : e.button.button == SDL_BUTTON_RIGHT ? 2 : 0;

            set_mouse_from_window(e.button.x, e.button.y);
            if (e.type == SDL_MOUSEBUTTONDOWN)
                mouse_buttons |= bit;
            else
                mouse_buttons &= ~bit;
            break;
        }
        default:
            break;
        }
    }
    run_script(now);
    if (shot_file && nshots) {
        if (shot_next < nshots && now - start_ms >= shot_list[shot_next]) {
            char name[1100];

            present();
            snprintf(name, sizeof(name), "%s.%d.ppm", shot_file, shot_next);
            write_shot(name);
            shot_next++;
        }
        if (shot_next == nshots) {
            plat_quit();
            exit(0);
        }
    } else if (shot_file && now - start_ms >= shot_ms) {
        present();
        write_shot(shot_file);
        plat_quit();
        exit(0);
    }
    if (now - last_present >= 16 || (pal_dirty && now - last_present >= 8)) {
        last_present = now;
        present();
    }
}

/* ---- life cycle ---- */

int plat_init(int argc, char **argv)
{
    const char *dir = getenv("STARTREK_DATA");
    int i, out = 1;
    int lh;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--data") && i + 1 < argc)
            dir = argv[++i];
        else if (!strcmp(argv[i], "--scale") && i + 1 < argc)
            opt_scale = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--fullscreen"))
            opt_fullscreen = 1;
        else if (!strcmp(argv[i], "--square"))
            opt_square_pixels = 1;
        else
            argv[out++] = argv[i];
    }
    if (dir)
        plat_set_data_dir(dir);
    shot_file = getenv("STARTREK_SHOT");
    if (getenv("STARTREK_SHOT_MS"))
        shot_ms = atoi(getenv("STARTREK_SHOT_MS"));
    if (getenv("STARTREK_INPUT"))
        parse_script(getenv("STARTREK_INPUT"));
    if (getenv("STARTREK_SHOTS")) {
        const char *p = getenv("STARTREK_SHOTS");

        while (*p && nshots < 32) {
            shot_list[nshots++] = strtoul(p, (char **)&p, 10);
            if (*p == ',')
                p++;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }
    lh = opt_square_pixels ? VGA_H : 240;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    window = SDL_CreateWindow("Star Trek: 25th Anniversary", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              VGA_W * opt_scale, lh * opt_scale,
                              SDL_WINDOW_RESIZABLE | (opt_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_RenderSetLogicalSize(renderer, VGA_W, lh);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VGA_W, VGA_H);
    SDL_ShowCursor(SDL_DISABLE);
    if (SDL_NumJoysticks() > 0)
        joy = SDL_JoystickOpen(0);
    start_ms = SDL_GetTicks();
    plat_audio_init();
    return out;
}

void plat_quit(void)
{
    plat_audio_quit();
    if (joy)
        SDL_JoystickClose(joy);
    joy = NULL;
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    texture = NULL;
    renderer = NULL;
    window = NULL;
    SDL_Quit();
}
