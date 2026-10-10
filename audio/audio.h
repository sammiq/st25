/* audio.h - the emulated sound hardware.
 *
 * Music: an AdLib/Sound Blaster FM chip (OPL2, emulated with Nuked OPL3) driven
 * by an XMIDI sequencer that stands in for the Miles AIL music driver
 * (sound.h's AIL_* calls).  Digital sound: the Sound Blaster DAC playing a
 * DMA buffer in the game's memory, as the original's digital driver did.
 * Both are mixed into one stream by audio_render, which the host's audio
 * thread calls.  The game's thread and the audio thread share state under
 * plat_audio_lock.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

/* The host's side (platform/plat_audio_*.c). */
int plat_audio_open(int *rate);     /* start the stream; 1 if there is sound. Sets the sample rate */
void plat_audio_close(void);
void plat_audio_lock(void);
void plat_audio_unlock(void);
void plat_set_timer_callback(void (*cb)(void));    /* 18.2 Hz timer interrupt, run from plat_pump */

/* Fill out with frames of 16 bit stereo (interleaved) samples. */
void audio_render(int16_t *out, int frames);

/* ---- FM music (fm.c) ---- */
void fm_init(int rate);
void fm_render(int16_t *lr);                /* one stereo frame */

/* ---- Sound Blaster DAC and DMA (sb.c) ---- */
void sb_init(int rate);
int16_t sb_render(void);                    /* one mono sample */

#endif
