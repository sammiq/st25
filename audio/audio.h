/* audio.h - the emulated sound hardware.
 *
 * Music: an AdLib/Sound Blaster FM chip (OPL2, emulated with Nuked OPL3) driven
 * by an XMIDI sequencer that stands in for the Miles AIL music driver
 * (sound.h's AIL_* calls).  Digital sound: the Sound Blaster DAC playing a
 * DMA buffer in the game's memory, as the original's digital driver did.
 * Both are mixed into one stream by audio_render, which the host's audio
 * thread calls.  The game's thread and the audio thread share state under
 * plat_audio_lock.
 *
 * Optionally the Amiga version's music (ProTracker modules) and sampled
 * effects replace the FM music and effects; they are mixed into the same
 * stream.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include "sbdrv.h"

/* The host's side (platform/plat_audio_*.c). */
int plat_audio_open(int *rate);     /* start the stream; 1 if there is sound. Sets the sample rate */
void plat_audio_close(void);
void plat_audio_lock(void);
void plat_audio_unlock(void);

/* The 18.2 Hz timer interrupt: cb runs from plat_pump (audio.c). */
void plat_set_timer_callback(void (*cb)(void));
void audio_poll(void);              /* the host's event pump calls this */

/* Fill out with frames of 16 bit stereo (interleaved) samples. */
void audio_render(int16_t *out, int frames);

/* ---- FM music (fm.c) ---- */
void fm_init(int rate);
void fm_render(int16_t *lr);                /* one stereo frame, running the sequencer */

/* ---- the Amiga version's music, optionally instead of FM (mod.c) ---- */
enum { MOD_STOPPED, MOD_PLAYING, MOD_DONE };    /* as AIL_sequence_status */
int mod_init(int rate);                     /* 1 if the Amiga music is there */
int mod_play(int song);                     /* song 1-20 from the start; 0 if missing */
void mod_stop(void);
int mod_status(void);
void mod_render(int16_t *lr);               /* one stereo frame */

/* ---- the Amiga version's sampled effects, instead of FM ones (amiga_sfx.c) ---- */
void asfx_init(int rate);
int asfx_play(int n);                       /* effect 0-42: its voice, or -1 if missing */
int asfx_playing(int v, int n);             /* voice v is still playing effect n */
int asfx_find(int n);                       /* the voice playing effect n, or -1 */
void asfx_stop(int v);
void asfx_stop_all(int keep);               /* all voices but keep (-1: all) */
int asfx_render(void);                      /* one mono sample */

/* ---- Sound Blaster DAC and DMA (sb.c) ---- */
void sb_init(int rate);
int16_t sb_render(void);                    /* one mono sample */

#endif
