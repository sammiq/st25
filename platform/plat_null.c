/* plat_null.c - a host with no display, input or sound: the game runs and
 * everything it draws is kept in memory (see STARTREK_SHOT in plat_shot). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform.h"

void plat_set_data_dir(const char *dir);

static uint8_t vram[65536];
static uint8_t palette[768];

/* Test hooks: STARTREK_SHOT=file.ppm writes the screen to the file and
 * quits after STARTREK_SHOT_MS milliseconds (default 3000). */
static const char *shot_file;
static uint32_t shot_ms = 3000;
static uint32_t start_ms;

static void write_shot(void)
{
    FILE *f = fopen(shot_file, "wb");
    int i;

    if (!f)
        return;
    fprintf(f, "P6\n%d %d\n255\n", VGA_W, VGA_H);
    for (i = 0; i < VGA_W * VGA_H; i++) {
        const uint8_t *c = palette + vram[i] * 3;
        uint8_t rgb[3] = { c[0] << 2 | c[0] >> 4, c[1] << 2 | c[1] >> 4, c[2] << 2 | c[2] >> 4 };

        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

uint32_t plat_msec(void);

int plat_init(int argc, char **argv)
{
    const char *dir = getenv("STARTREK_DATA");

    if (dir)
        plat_set_data_dir(dir);
    shot_file = getenv("STARTREK_SHOT");
    if (getenv("STARTREK_SHOT_MS"))
        shot_ms = atoi(getenv("STARTREK_SHOT_MS"));
    start_ms = plat_msec();
    return argc;
}

void plat_quit(void)
{
}

void audio_poll(void);

void plat_pump(void)
{
    audio_poll();
    if (shot_file && plat_msec() - start_ms >= shot_ms) {
        write_shot();
        exit(0);
    }
}

uint8_t *plat_vram(void)
{
    return vram;
}

void plat_set_palette(int first, int count, const uint8_t *rgb)
{
    if (first < 0 || first + count > 256)
        return;
    memcpy(palette + first * 3, rgb, count * 3);
}

int plat_key_ready(void) { return 0; }
int plat_key_read(void) { return 0; }
int plat_key_shift(void) { return 0; }
int plat_key_wait(void) { return 0; }

static int mx = 160, my = 100;

int plat_mouse_init(void) { return 1; }
void plat_mouse_read(int *x, int *y, int *buttons) { *x = mx; *y = my; *buttons = 0; }
void plat_mouse_set(int x, int y) { mx = x; my = y; }

int plat_joy_present(void) { return 0; }
void plat_joy_read(int *x, int *y, int *buttons) { *x = *y = *buttons = 0; }

uint32_t plat_msec(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

uint32_t plat_ticks(void)
{
    return (uint32_t)((uint64_t)plat_msec() * 18207 / 1000000);
}

void plat_delay(int ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };

    nanosleep(&ts, NULL);
}
