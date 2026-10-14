/* plat_audio_sdl.c - the audio stream on an SDL audio device, 16 bit stereo.
 * SDL calls back on its own thread: plat_audio_lock keeps the game's thread
 * out of the emulation while it changes sound state. */

#include <SDL.h>
#include <stdint.h>

#include "audio.h"

static SDL_AudioDeviceID dev;

static void callback(void *user, Uint8 *stream, int len)
{
    audio_render((int16_t *)stream, len / 4);
}

int plat_audio_open(int *rate)
{
    SDL_AudioSpec want, have;

    if (dev)
        return 1;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        return 0;
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = callback;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!dev)
        return 0;
    *rate = have.freq;
    SDL_PauseAudioDevice(dev, 0);
    return 1;
}

void plat_audio_close(void)
{
    if (dev) {
        SDL_CloseAudioDevice(dev);
        dev = 0;
    }
}

void plat_audio_lock(void)
{
    if (dev)
        SDL_LockAudioDevice(dev);
}

void plat_audio_unlock(void)
{
    if (dev)
        SDL_UnlockAudioDevice(dev);
}

void plat_audio_init(void) {}
void plat_audio_quit(void) { plat_audio_close(); }
