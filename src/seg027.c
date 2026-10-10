/* seg027: fixed point 3D maths (matrix and vector helpers) and joystick
 * input.  Replaces the original assembly module; checked against it with
 * tools/oracle.py (tests/test_seg027.py). */

#include "compat.h"
#include "platform.h"
#include "fixmath.h"
#include "sprite.h"
#include "obj3d.h"

extern Rect clip_rect;
extern int32_t proj_dist;
extern int16_t kbd_mouse;
extern uint32_t ticks;

/* The joystick, read as the original read the game port: axis values are
 * timer counts of the capacitor discharge, 0x400 is the centre. */
int16_t joy_center_x = 0x400;
int16_t joy_center_y = 0x600;
int16_t joy_min_x;
int16_t joy_max_x;
int16_t joy_min_y;
int16_t joy_max_y;
unsigned char joy_last_buttons;
int16_t joy_x;
int16_t joy_y;
int16_t joy_buttons;
int16_t joy_present;
int16_t joy_frac_x;
int16_t joy_frac_y;
int16_t joy_frame;

/* The high word of a 32 bit sum shifted left twice: (sum >> 14), truncated
 * to 16 bits. */
static int16_t fix14(int32_t sum)
{
    return (int16_t)(((uint32_t)sum << 2) >> 16);
}

/* out = (x, y, z) of the screen position (x, y) at depth z */
void Vec32FromScreen(Vec32 *out, int16_t x, int16_t y, int16_t z)
{
    out->z = z;
    out->y = (int16_t)((int32_t)y * z / (int16_t)proj_dist);
    out->x = (int16_t)((int32_t)x * z / (int16_t)proj_dist);
}

/* out = m * v */
void MatVecMul32(Matrix *mat, Vec32 *v, Vec32 *out)
{
    int16_t *m = mat->e[0];
    int32_t *res = &out->x;
    int i;

    for (i = 0; i < 3; i++, m += 3, res++) {
        int32_t sum = (int32_t)m[0] * (int16_t)v->x + (int32_t)m[1] * (int16_t)v->y
                    + (int32_t)m[2] * (int16_t)v->z;
        *res = fix14(sum);
    }
}

void Vec32Add(Vec32 *a, Vec32 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

void Vec32Sub(Vec32 *a, Vec32 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

/* out = transpose(m) * v */
void MatTVecMul32(Vec32 *v, Matrix *m, Vec32 *out)
{
    int32_t *res = &out->x;
    int i;

    for (i = 0; i < 3; i++, res++) {
        int32_t sum = (int32_t)m->e[0][i] * (int16_t)v->x + (int32_t)m->e[1][i] * (int16_t)v->y
                    + (int32_t)m->e[2][i] * (int16_t)v->z;
        *res = fix14(sum);
    }
}

/* a * proj_dist / z */
int16_t Project(int16_t a, int16_t z)
{
    return (int16_t)((int32_t)a * (int16_t)proj_dist / z);
}

/* Approximate length of (x, y, z): the biggest component, plus a third of
 * the middle one, plus a quarter of the smallest. */
int32_t sub_3A35E(int32_t x, int32_t y, int32_t z)
{
    int16_t a = (int16_t)x, b = (int16_t)y, c = (int16_t)z, t;

    a = (int16_t)-(a < 0 ? a : -a);         /* abs, 16 bit wrap */
    b = (int16_t)-(b < 0 ? b : -b);
    c = (int16_t)-(c < 0 ? c : -c);
    if (a < b)
        t = a, a = b, b = t;
    if (a < c)
        t = a, a = c, c = t;
    if (b < c)
        t = b, b = c, c = t;
    return (int16_t)((uint16_t)a + (uint16_t)(((uint32_t)0x5555 * (uint16_t)b) >> 16)
                     + ((uint16_t)c >> 2));
}

/* out = a * b (3x3, 14 bit fractions); out may be a or b, rows are
 * computed in order like the original did. */
void MatMul(Matrix *a, Matrix *b, Matrix *out)
{
    int r, c;

    for (r = 0; r < 3; r++)
        for (c = 0; c < 3; c++) {
            int32_t sum = (int32_t)a->e[r][0] * b->e[0][c] + (int32_t)a->e[r][1] * b->e[1][c]
                        + (int32_t)a->e[r][2] * b->e[2][c];
            out->e[r][c] = fix14(sum);
        }
}

/* (a << 14) / b, from the low words */
int16_t FixDiv(int32_t a, int32_t b)
{
    int16_t d = (int16_t)b;

    if (d == 0)
        return 0;
    return (int16_t)(((int32_t)(int16_t)a * 0x4000) / d);
}

/* (a * b) >> 14 */
int16_t FixMul(int16_t a, int16_t b)
{
    return fix14((int32_t)a * b);
}

/* (a * b + c * d) >> 14, from the low words of a and c */
int32_t FixMul2(int32_t a, int16_t b, int32_t c, int16_t d)
{
    return fix14((int32_t)(int16_t)a * b + (int32_t)(int16_t)c * d);
}

/* floor(sqrt(i)), the original's lookup table for the top byte */
static unsigned char SqrtByte(unsigned i)
{
    unsigned r = 0;

    while ((r + 1) * (r + 1) <= i)
        r++;
    return (unsigned char)r;
}

/* Square root of an unsigned 32 bit value.  A restoring square root that
 * takes the top byte from a table and two more input bits per step, done
 * on 16 bit registers exactly as the original did: the result is not always
 * the true root (it can be a few units short for big arguments). */
#define RCL(r)      do { unsigned _o = (r) >> 15; (r) = (uint16_t)(((r) << 1) | cf); cf = _o; } while (0)
#define SHL(r)      do { cf = (r) >> 15; (r) = (uint16_t)((r) << 1); } while (0)
#define STEP(wide) do { \
        cf = 1; RCL(ax); \
        if (wide) { SHL(bx); RCL(cx); } else SHL(cx); RCL(dx); \
        if (wide) { SHL(bx); RCL(cx); } else SHL(cx); RCL(dx); \
        if (dx >= ax) { dx -= ax; ax++; } else ax--; \
    } while (0)

int16_t FixSqrt(int32_t x)
{
    uint32_t n = (uint32_t)x;
    uint16_t ax, bx, cx, dx;
    unsigned cf;
    unsigned char t;
    int entry;

    if ((n >> 16) == 0) {
        t = SqrtByte((n >> 8) & 0xFF);
        cx = (n >> 8) & 0xFF;
        entry = 3;
        bx = 0;
    } else if ((n >> 24) == 0) {
        t = SqrtByte((n >> 16) & 0xFF);
        cx = (n >> 16) & 0xFF;
        entry = 2;
        bx = 0;
    } else {
        t = SqrtByte(n >> 24);
        cx = n >> 24;
        entry = 1;
        bx = 0;
    }
    bx = (uint16_t)(t << 1);
    ax = (uint16_t)(t * t);
    cx -= ax;
    ax = bx;
    dx = cx;
    if (entry == 3) {
        cx = (uint16_t)((n & 0xFF) << 8);
    } else if (entry == 2) {
        cx = (uint16_t)n;
    } else {
        bx = (uint16_t)((n & 0xFF) << 8);
        cx = (uint16_t)(n >> 8);
    }
    if (entry == 1) {
        STEP(1); STEP(1); STEP(1); STEP(1);
    }
    if (entry <= 2) {
        STEP(0); STEP(0); STEP(0); STEP(0);
    }
    STEP(0); STEP(0); STEP(0);
    /* the last half step rounds */
    SHL(cx);
    RCL(dx);
    if (cf)
        ax++;
    else if (dx > ax || (dx == ax && (int16_t)dx < 0))
        ax++;
    return (int16_t)ax;
}

int16_t FixLength(int16_t x, int16_t y, int16_t z)
{
    return FixSqrt((int32_t)x * x + (int32_t)y * y + (int32_t)z * z);
}

/* ---- joystick ---- */

/* Read both axes: 0x400 at the centre, 0x200 at either end of the travel.
 * Clears joy_present and returns the centre if there is no joystick. */
static void JoyReadAxes(int16_t *cx, int16_t *dx)
{
    int x, y, b;

    if (!plat_joy_present()) {
        *cx = joy_center_x;
        *dx = joy_center_y;
        joy_present = 0;
        return;
    }
    plat_joy_read(&x, &y, &b);
    *cx = 0x400 + x * 4;
    *dx = 0x400 + y * 4;
}

void JoyCalibrate(void)
{
    int16_t cx, dx;

    if (!joy_present)
        return;
    JoyReadAxes(&cx, &dx);
    joy_center_x = cx;
    joy_center_y = dx;
    joy_min_x = cx - 0x200;
    joy_min_y = dx - 0x200;
    joy_max_x = joy_min_x + 0x400;
    joy_max_y = joy_min_y + 0x400;
}

/* Update the pointer position (once per tick) and the buttons. */
void JoyUpdate(void)
{
    int16_t cx, dx, bx;
    int x, y, b;

    if ((int16_t)ticks != joy_frame) {
        joy_frame = (int16_t)ticks;
        do
            JoyReadAxes(&cx, &dx);
        while ((int16_t)(cx | dx) < 0);
        if (kbd_mouse) {
            /* absolute: the travel maps onto the view */
            if (cx < joy_min_x)
                joy_min_x = cx;
            if (cx > joy_max_x)
                joy_max_x = cx;
            cx -= joy_center_x;
            bx = cx < 0 ? joy_center_x - joy_min_x : joy_max_x - joy_center_x;
            cx = (int16_t)((int32_t)cx * 0x4000 / bx) + 0x4000;
            if (dx < joy_min_y)
                joy_min_y = dx;
            if (dx > joy_max_y)
                joy_max_y = dx;
            dx -= joy_center_y;
            bx = dx < 0 ? joy_center_y - joy_min_y : joy_max_y - joy_center_y;
            dx = (int16_t)((int32_t)dx * 0x4000 / bx) + 0x4000;
            joy_y = clip_rect.y1 - (int16_t)(((uint32_t)(uint16_t)((clip_rect.y1 - clip_rect.y0) << 1)
                                              * (uint16_t)dx) >> 16);
            joy_x = (int16_t)(((uint32_t)(uint16_t)((clip_rect.x1 - clip_rect.x0) << 1)
                               * (uint16_t)cx) >> 16) + clip_rect.x0;
        } else {
            /* relative: the stick's deflection moves the pointer */
            cx -= joy_center_x;
            if (cx < -32 || cx > 32) {
                uint16_t f = (uint16_t)(cx << 9) + (uint16_t)joy_frac_x;
                int carry = f < (uint16_t)(cx << 9);
                int16_t v = (int16_t)((cx >> 7) + joy_x + carry);

                joy_frac_x = f;
                joy_x = v < 0 ? 0 : v >= 320 ? 319 : v;
            }
            dx -= joy_center_y;
            if (dx < -32 || dx > 32) {
                uint16_t f = (uint16_t)(dx << 9) + (uint16_t)joy_frac_y;
                int carry = f < (uint16_t)(dx << 9);
                int16_t v = (int16_t)((dx >> 7) + joy_y + carry);

                joy_frac_y = f;
                joy_y = v < 0 ? 0 : v >= 200 ? 199 : v;
            }
        }
    }
    if (plat_joy_present()) {
        plat_joy_read(&x, &y, &b);
        joy_buttons = b & 3;
    } else
        joy_buttons = 0;
    joy_last_buttons = (unsigned char)joy_buttons;
}
