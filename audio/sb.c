/* sb.c - the Sound Blaster DAC with its DMA channel: an 8 bit unsigned mono
 * stream read from a buffer in the game's memory at the programmed sample
 * rate.  In auto-init mode the buffer plays round and round, which is how the
 * game used it: it keeps mixing sound into the half that has just played.
 *
 * The game polls how many bytes are left until the end of the buffer (the
 * DMA controller's count), so the position is an atomic counter the audio
 * thread advances.
 */

#include <stdint.h>
#include <string.h>

#include "audio.h"
#include "sbdrv.h"

static const uint8_t *dma_buf;
static int dma_len;
static int dma_rate = 22050;
static int dma_auto;
static volatile int dma_pos;            /* byte the DAC is on */
static volatile int dma_on;
static uint32_t frac;                   /* position between samples, 16 bit fraction */
static uint32_t step;                   /* source samples per output sample, 16.16 */
static int out_rate = 48000;
static int16_t last, next;
static int16_t status_word;

void sb_init(int rate)
{
    out_rate = rate;
    dma_on = 0;
}

int sb_present(void)
{
    return 1;
}

unsigned sb_caps(void)
{
    return 0x200;                       /* auto-init DMA */
}

/* DMA buffers must not cross a physical 64K page: nothing to check here. */
int sb_check_buffer(void *buf, unsigned len)
{
    return 1;
}

void sb_silence(void *buf, unsigned len)
{
    memset(buf, 0x80, len);
}

void sb_set_auto(int on)
{
    plat_audio_lock();
    dma_auto = on;
    plat_audio_unlock();
}

void sb_play(void *buf, unsigned len, unsigned rate)
{
    plat_audio_lock();
    dma_buf = buf;
    dma_len = len;
    dma_rate = rate ? rate : 22050;
    step = (uint32_t)(((uint64_t)dma_rate << 16) / out_rate);
    dma_pos = 0;
    frac = 0;
    dma_on = 1;
    last = next = fetch();
    plat_audio_unlock();
}

void sb_stop(void)
{
    plat_audio_lock();
    dma_on = 0;
    plat_audio_unlock();
}

/* Bytes left until the end of the buffer (the DMA count). */
int sb_remaining(void)
{
    return dma_on ? dma_len - dma_pos : 0;
}

int16_t *sb_status(void)
{
    return &status_word;
}

static int16_t fetch(void)
{
    int16_t s = (int16_t)(((int)dma_buf[dma_pos] - 128) << 8);

    if (++dma_pos >= dma_len) {
        if (dma_auto)
            dma_pos = 0;
        else
            dma_on = 0;
    }
    return s;
}

/* One mono sample at the output rate, linearly interpolated between the DAC's. */
int16_t sb_render(void)
{
    int32_t v;

    if (!dma_on)
        return 0;
    v = last + (((int32_t)next - last) * (int32_t)(frac >> 1) >> 15);
    frac += step;
    while (frac >= 0x10000) {
        frac -= 0x10000;
        last = next;
        if (!dma_on)
            break;
        next = fetch();
    }
    return (int16_t)v;
}
