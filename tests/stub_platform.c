/* Platform stubs for the oracle tests. */
#include "platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
static uint8_t vram[65536];
uint8_t *plat_vram(void) { return vram; }
int plat_joy_present(void) { return 0; }
void plat_joy_read(int *x, int *y, int *b) { *x = *y = *b = 0; }
void plat_set_palette(int f, int n, const uint8_t *r) {}
void Fatal(const char *fmt, ...) { va_list a; va_start(a, fmt); vfprintf(stderr, fmt, a); abort(); }
/* globals the code under test expects from elsewhere */
#include "sprite.h"
Rect clip_rect; int32_t proj_dist; int16_t kbd_mouse; uint32_t ticks;
int16_t CachePurge(int16_t ems) { return 0; }
