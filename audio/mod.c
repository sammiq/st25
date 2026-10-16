/* mod.c - the Amiga version's music, in place of the FM music: its songs as
 * ProTracker modules (made by tools/amiga_music.py),
 * played by micromod (audio/micromod/, BSD licence).
 *
 * Songs are numbered 1-20 as the Amiga game numbers them; seg015 maps the
 * DOS sequences onto them.  A song plays once and then reports MOD_DONE, as
 * an XMIDI sequence does, so the game decides whether it repeats.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "platform.h"
#include "micromod/micromod.h"

#define BUF_FRAMES 256

/* The sample bank of each song: the Amiga game loads trek<n><bank>.sng. */
static const char banks[] = "?daaaabbbaabbbbbbbbbc";

static const char *dir;
static int out_rate;
static signed char *song_data;
static int cur_song;
static volatile int state;              /* MOD_STOPPED, MOD_PLAYING or MOD_DONE */
static short buf[2 * BUF_FRAMES];
static int buf_pos, buf_len, ending;

int mod_init(int rate)
{
    char path[1100];
    FILE *f;

    out_rate = rate;
    dir = plat_amiga_music_dir();
    if (!dir)
        return 0;
    snprintf(path, sizeof(path), "%s/trek1d.mod", dir);
    if ((f = fopen(path, "rb")) == NULL) {
        fprintf(stderr, "No Amiga music in %s (tools/amiga_music.py makes it): playing the FM music\n", dir);
        dir = NULL;
        return 0;
    }
    fclose(f);
    return 1;
}

static signed char *load(int song)
{
    char path[1100];
    FILE *f;
    long len;
    signed char *data;

    snprintf(path, sizeof(path), "%s/trek%d%c.mod", dir, song, banks[song]);
    if ((f = fopen(path, "rb")) == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    data = len >= 1084 ? malloc(len) : NULL;
    if (data && (fread(data, 1, len, f) != (size_t)len || micromod_calculate_mod_file_len(data) < 0
                 || micromod_calculate_mod_file_len(data) > len)) {
        free(data);
        data = NULL;
    }
    fclose(f);
    if (!data)
        fprintf(stderr, "%s: not a module\n", path);
    return data;
}

/* Start song from the beginning.  Returns 0 if it isn't there. */
int mod_play(int song)
{
    signed char *data = NULL, *old = NULL;

    if (!dir || song < 1 || song >= (int)sizeof(banks) - 1)
        return 0;
    if (song != cur_song && (data = load(song)) == NULL)
        return 0;
    plat_audio_lock();
    if (data) {
        old = song_data;
        song_data = data;
        cur_song = song;
        micromod_initialise(song_data, out_rate);
    } else
        micromod_set_position(0);
    buf_pos = buf_len = ending = 0;
    state = MOD_PLAYING;
    plat_audio_unlock();
    free(old);
    return 1;
}

void mod_stop(void)
{
    plat_audio_lock();
    state = MOD_STOPPED;
    plat_audio_unlock();
}

int mod_status(void)
{
    return state;
}

/* One stereo frame.  micromod pans the four Amiga channels hard left and
 * right; a quarter of each side goes to the other, for headphones. */
void mod_render(int16_t *lr)
{
    int l, r;

    lr[0] = lr[1] = 0;
    if (state != MOD_PLAYING)
        return;
    if (buf_pos == buf_len) {
        if (ending) {
            state = MOD_DONE;
            return;
        }
        memset(buf, 0, sizeof(buf));
        buf_len = micromod_get_audio(buf, BUF_FRAMES);
        ending = buf_len < BUF_FRAMES;
        buf_pos = 0;
        if (buf_len == 0) {
            state = MOD_DONE;
            return;
        }
    }
    l = buf[2 * buf_pos];
    r = buf[2 * buf_pos + 1];
    buf_pos++;
    lr[0] = (int16_t)((l * 3 + r) / 4);
    lr[1] = (int16_t)((r * 3 + l) / 4);
}
