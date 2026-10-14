/* plat_audio_null.c - no sound device: the game runs without sound. */
#include "audio.h"

int plat_audio_open(int *rate) { return 0; }
void plat_audio_close(void) {}
void plat_audio_lock(void) {}
void plat_audio_unlock(void) {}
void plat_audio_init(void) {}
void plat_audio_quit(void) {}
