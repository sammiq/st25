/* audio.c - mixes the FM music and the Sound Blaster into the host's stream,
 * and runs the 18.2 Hz timer for the game.
 */

#include <stdint.h>
#include <string.h>

#include "audio.h"
#include "platform.h"

/* ---- the output stream ---- */

static int clip(int v)
{
    return v > 32767 ? 32767 : v < -32768 ? -32768 : v;
}

void audio_render(int16_t *out, int frames)
{
    int i;

    for (i = 0; i < frames; i++) {
        int16_t lr[2];
        int dac;

        fm_render(lr);
        dac = sb_render() * 5 / 8;          /* the DAC is far louder than the FM chip: balance them */
        out[2 * i] = (int16_t)clip(lr[0] * 3 + dac);
        out[2 * i + 1] = (int16_t)clip(lr[1] * 3 + dac);
    }
}

/* ---- the timer interrupt ---- */

static void (*timer_cb)(void);
static uint32_t timer_last;

void plat_set_timer_callback(void (*cb)(void))
{
    timer_cb = cb;
    timer_last = plat_msec();
}

/* 18.2065 Hz = every 54.9 ms.  Ticks missed while the game was busy are run
 * (a few at most) as the interrupt controller would have queued them. */
void audio_poll(void)
{
    uint32_t now;
    int n = 0;

    if (!timer_cb)
        return;
    now = plat_msec();
    while (now - timer_last >= 55 && n < 4) {
        timer_last += 55;
        timer_cb();
        n++;
    }
    if (now - timer_last >= 55)
        timer_last = now;
}
