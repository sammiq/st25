/* amiga_sfx.c - the Amiga version's sampled sound effects, in place of the
 * FM effects the DOS game still plays: sfxNN.wav, made by
 * tools/amiga_music.py, each already as long as the Amiga plays it.
 *
 * Four voices, as Paula has; a new effect takes a free voice or else the
 * one started longest ago.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "platform.h"

#define NUM_EFFECTS 43
#define NUM_VOICES 4

typedef struct {
    int8_t *data;               /* signed 8-bit samples */
    uint32_t len;
    uint32_t step;              /* 16.16 source samples per output sample */
    int tried;
} Effect;

typedef struct {
    const Effect *e;
    int n;
    uint32_t pos;               /* 16.16 */
    uint32_t started;
} Voice;

static Effect effects[NUM_EFFECTS];
static Voice voices[NUM_VOICES];
static uint32_t started;
static int out_rate;

void asfx_init(int rate)
{
    out_rate = rate;
}

static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t le16(const uint8_t *p) { return p[0] | p[1] << 8; }

/* An 8-bit mono WAV: the samples, made signed. */
static int load(Effect *e, int n)
{
    const char *dir = plat_amiga_music_dir();
    char path[1100];
    uint8_t *buf = NULL, *p, *end, *fmt = NULL, *data = NULL;
    uint32_t data_len = 0, i;
    long len;
    FILE *f;

    e->tried = 1;
    if (!dir || !out_rate)
        return 0;
    snprintf(path, sizeof(path), "%s/sfx%02d.wav", dir, n);
    if ((f = fopen(path, "rb")) == NULL)
        return 0;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    if (len > 12 && (buf = malloc(len)) != NULL && fread(buf, 1, len, f) == (size_t)len
        && memcmp(buf, "RIFF", 4) == 0 && memcmp(buf + 8, "WAVE", 4) == 0) {
        for (p = buf + 12, end = buf + len; p + 8 <= end; p += 8 + ((le32(p + 4) + 1) & ~1u)) {
            if (memcmp(p, "fmt ", 4) == 0 && le32(p + 4) >= 16)
                fmt = p + 8;
            else if (memcmp(p, "data", 4) == 0) {
                data = p + 8;
                data_len = le32(p + 4);
                if (data_len > (uint32_t)(end - data))
                    data_len = end - data;
            }
        }
    }
    fclose(f);
    if (!fmt || !data || le16(fmt) != 1 || le16(fmt + 2) != 1 || le16(fmt + 14) != 8 || !data_len) {
        fprintf(stderr, "%s: not an 8-bit mono WAV\n", path);
        free(buf);
        return 0;
    }
    e->data = malloc(data_len);
    for (i = 0; i < data_len; i++)
        e->data[i] = (int8_t)(data[i] - 128);
    e->len = data_len;
    e->step = (uint32_t)(((uint64_t)le32(fmt + 4) << 16) / out_rate);
    free(buf);
    return 1;
}

/* Start effect n.  Returns its voice, or -1 if the effect isn't there. */
int asfx_play(int n)
{
    Effect *e;
    int i, v = 0;

    if (n < 0 || n >= NUM_EFFECTS)
        return -1;
    e = &effects[n];
    if (!e->data && (e->tried || !load(e, n)))
        return -1;
    plat_audio_lock();
    for (i = 0; i < NUM_VOICES; i++) {
        if (!voices[i].e) {
            v = i;
            break;
        }
        if (voices[i].started < voices[v].started)
            v = i;
    }
    voices[v].e = e;
    voices[v].n = n;
    voices[v].pos = 0;
    voices[v].started = ++started;
    plat_audio_unlock();
    return v;
}

/* Whether voice v is still playing effect n. */
int asfx_playing(int v, int n)
{
    return v >= 0 && v < NUM_VOICES && voices[v].e && voices[v].n == n;
}

/* The voice playing effect n, or -1. */
int asfx_find(int n)
{
    int i;

    for (i = 0; i < NUM_VOICES; i++)
        if (voices[i].e && voices[i].n == n)
            return i;
    return -1;
}

/* Stop every voice but keep (-1: all of them). */
void asfx_stop_all(int keep)
{
    int i;

    plat_audio_lock();
    for (i = 0; i < NUM_VOICES; i++)
        if (i != keep)
            voices[i].e = NULL;
    plat_audio_unlock();
}

void asfx_stop(int v)
{
    if (v < 0 || v >= NUM_VOICES)
        return;
    plat_audio_lock();
    voices[v].e = NULL;
    plat_audio_unlock();
}

/* One mono sample of the voices (each full-volume voice peaks near 16000). */
int asfx_render(void)
{
    int i, sum = 0;

    for (i = 0; i < NUM_VOICES; i++) {
        Voice *v = &voices[i];
        uint32_t j, f;
        int a, b;

        if (!v->e)
            continue;
        j = v->pos >> 16;
        if (j >= v->e->len) {
            v->e = NULL;
            continue;
        }
        f = v->pos & 0xFFFF;
        a = v->e->data[j];
        b = j + 1 < v->e->len ? v->e->data[j + 1] : a;
        sum += (a * 0x10000 + (b - a) * (int)f) >> 9;     /* linear, * 128 */
        v->pos += v->e->step;
    }
    return sum;
}
