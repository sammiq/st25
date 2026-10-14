/* seg015: music and digitised sound */

#include "compat.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "list.h"
#include "cache.h"
#include "resource.h"
#include "sound.h"
#include "audio.h"

/* Globals this file owns (declared in the headers). */
int16_t digi_driver;
int16_t midi_driver;
SndChan snd_chan[4];


/* seg 2505: the digital sound driver interface (asm/t35052) */
void sub_35388(char *msg);
void sub_3571B(DmaReq *req);
uint16_t sub_3577A(void);
void sub_357C0(void);
void sub_357EB(DmaReq *req);
int16_t sub_357FE(void);
int16_t *sub_35828(void);
void sub_3585E(int16_t on);
int16_t sub_3586D(void);
void sub_35873(char *buf, uint16_t len);
int16_t sub_3588D(char *buf, uint16_t len);
int16_t sub_358B3(void);

/* seg019: save game I/O */
void SaveWrite(void *p, int16_t n);
void SaveRead(void *p, int16_t n);

int16_t *digi_status;
int16_t digi_handle;
int16_t midi_handle;
extern int16_t sb_port;
extern int16_t sb_irq;
char *dma_buf;
void *seq_state[NUM_SEQ];
int16_t seq_handles[NUM_SEQ];
int16_t seq_ids[NUM_SEQ];
char midi_suffix[4];
uint16_t timbre_cache_size;
char music_name[10];
List seq_list;
SeqNode seq_nodes[NUM_SEQ];
int16_t music_next;
int16_t music_cur;
int16_t stream_mixed;
int16_t voc_abort;
uint32_t voc_left;
char voc_name[80];
int16_t voc_rate;
int16_t voc_pack;
extern char sfx_name[];
int16_t dma_auto;
DmaReq dma_req;
int16_t dma_half_pos;
char dma_half;
char mix_half;
int16_t digi_running;
int16_t digi_ticks;
int16_t mix_first;
int16_t dma_state;
int16_t dma_count;
extern int16_t stream_done;
int16_t stream_filling;

int16_t music_on = 1;
int16_t sfx_on = 1;
int16_t no_midi = 0;
int16_t midi_ok = 0;
int16_t digi_ok = 0;
int16_t voc_fd = -1;
char *music_data = 0;
int16_t sound_closed = 0;

/* The original's timer interrupt ran the digital sound tick (DigiTick) 18
 * times a second: the host calls it at that rate from its event pump. */
void HookTimer(void)
{
    plat_set_timer_callback(DigiTick);
}

void UnhookTimer(void)
{
    plat_set_timer_callback(0);
}

/* Start the sound system: the emulated AdLib/Sound Blaster FM for music and
 * the emulated Sound Blaster DAC for digital sound.  (The drivers the
 * original loaded from .ADV files are not needed: audio/ is their stand-in.) */
void SoundInit(void)
{
    int rate;
    int16_t i;
    uint16_t size;
    GTLEntry *e;
    MemHandle h;
    char *gtl;

    if (!plat_audio_open(&rate)) {
        no_midi = 1;
        return;
    }
    fm_init(rate);
    sb_init(rate);
    midi_ok = 1;
    digi_ok = 1;
    midi_handle = 0;
    digi_handle = 1;
    dma_buf = PushMem(0x2000);
    HookTimer();
    StreamInit();
    size = AIL_state_table_size(midi_handle);
    for (i = 0; i < NUM_SEQ; i++) {
        seq_state[i] = PushMem(size);
        seq_handles[i] = -1;
    }
    strcpy(midi_suffix, "ad");
    timbre_cache_size = 0;
    h = getanyfile("gtl", midi_suffix);
    gtl = LockMem(h);
    for (e = (GTLEntry *)gtl; e->bank != -1; e++)
        AIL_install_timbre(midi_handle, e->bank, e->patch, gtl + e->offset);
    UnlockMem(h);
    FreeAnyFile(h);
    strcpy(music_name, "");
    music_data = 0;
    ListInit(&seq_list);
    for (i = 1; i < NUM_SEQ; i++) {
        seq_nodes[i].slot = i;
        ListAdd(&seq_list, &seq_nodes[i].node);
        seq_ids[i] = -1;
    }
    music_on = 1;
    sfx_on = 1;
}

void SoundStop(void)
{
    if (digi_ok)
        UnhookTimer();
    if (midi_ok || digi_ok) {
        AIL_shutdown(0);
        sb_stop();
        plat_audio_close();
    }
}

void StopSeq(uint16_t slot)
{
    if (seq_handles[slot] != -1) {
        AIL_stop_sequence(midi_handle, seq_handles[slot]);
        AIL_release_sequence_handle(midi_handle, seq_handles[slot]);
        seq_handles[slot] = -1;
        seq_ids[slot] = -1;
    }
}

void PlaySeq(uint16_t slot, int16_t seq)
{
    StopSeq(slot);
    if (seq == -1)
        return;
    seq_handles[slot] = AIL_register_sequence(midi_handle, music_data, seq, seq_state[slot], 0);
    seq_ids[slot] = seq;
    AIL_start_sequence(midi_handle, seq_handles[slot]);
}

int16_t MusicPlaying(void)
{
    int16_t i;

    if (midi_ok == 0)
        return 0;
    for (i = 0; i < NUM_SEQ; i++)
        if (seq_handles[i] != -1 && AIL_sequence_status(midi_handle, seq_handles[i]) == 1)
            return 1;
    return 0;
}

void StopVoc(void)
{
    if (digi_ok) {
        stream_play_pos = stream_fill_pos;
        if (voc_fd != -1) {
            close(voc_fd);
            voc_fd = -1;
        }
        stream_mixed = 0;
        stream_active = 0;
    }
}

void StopAllSound(void)
{
    int16_t i;

    for (i = 0; i < NUM_SEQ; i++)
        StopSeq(i);
    StopVoc();
}

void LoadMusic(char *name)
{
    int16_t unused;
    uint16_t size;

    if (midi_ok == 0)
        return;
    StopAllSound();
    PlayMusic(-1, -1);
    if (strcmpi(name, music_name) == 0)
        return;
    strcpy(music_name, name);
    if (music_data)
        PopMem();
    size = GetFileSize(name, midi_suffix);
    music_data = PushMem(size);
    LoadFilePacked(name, midi_suffix, music_data);
}

/* Play sequence seq; next (-2 keep, -3 same as seq) plays when it ends. */
void PlayMusic(int16_t seq, int16_t next)
{
    if (midi_ok == 0 || music_on == 0)
        return;
    if (next == -2)
        ;
    else if (next == -3)
        music_next = seq;
    else
        music_next = next;
    if (seq == -2)
        return;
    music_cur = seq;
    if (music_on)
        PlaySeq(0, seq);
}

/* Read up to len bytes of sample data from the speech files listed in
 * voc_name ("file,file,..."), following the VOC blocks. Returns the number
 * of bytes read. */
int16_t ReadVoc(char *buf, int16_t len)
{
    uint16_t got;
    int16_t start;
    char path[128];
    int16_t i;

    voc_abort = 0;
    start = len;
    while (voc_abort == 0 && len != 0) {
        if (voc_fd == -1 && voc_name[0] != 0) {
            char hdr[26];
            uint16_t n;

            for (i = 0; voc_name[i] != 0 && voc_name[i] != ','; i++)
                ;
            strcpy(path, voc_path);
            strncat(path, voc_name, i);
            strcat(path, ".voc");
            if ((voc_fd = open(path, O_RDONLY | O_BINARY)) == -1)
                return 0;
            if (voc_name[i] == ',')
                strcpy(voc_name, &voc_name[i + 1]);
            else
                voc_name[0] = 0;
            ReadChunk(voc_fd, hdr, 26, &n);
            voc_abort = 0;
            voc_left = 0;
        }
        if (voc_fd == -1)
            return start - len;
        if (voc_left) {
            uint16_t n;

            n = len;
            if (n > voc_left)
                n = voc_left;
            ReadChunk(voc_fd, buf, n, &got);
            n = got;
            if (n == 0) {
                close(voc_fd);
                voc_fd = -1;
            }
            buf += n;
            len -= n;
            voc_left -= n;
        } else {
            unsigned char hdr[4];

            ReadChunk(voc_fd, hdr, 4, &got);
            if (got < 4 && got != 1 && hdr[0] != 0) {
                close(voc_fd);
                voc_fd = -1;
                break;
            }
            voc_left = ((int32_t)hdr[3] << 16) | ((int32_t)hdr[2] << 8) | hdr[1];
            switch (hdr[0]) {
            case 0:
                close(voc_fd);
                voc_fd = -1;
                break;
            case 1:
                ReadChunk(voc_fd, hdr, 2, &got);
                voc_rate = hdr[0];
                voc_pack = hdr[1];
                voc_left -= 2;
                break;
            case 2:
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            default:
                while (voc_left >= 4)
                    ReadChunk(voc_fd, hdr, 4, &got), voc_left -= 4;
                if (voc_left)
                    ReadChunk(voc_fd, hdr, voc_left, &got);
                voc_left = 0;
                break;
            }
        }
    }
    return start - len;
}

void MusicTick(void)
{
    int16_t unused;

    if (midi_ok == 0 || music_on == 0)
        return;
    if (seq_handles[0] != -1 && AIL_sequence_status(midi_handle, seq_handles[0]) == 2) {
        if (music_cur == music_next && music_cur != -1)
            AIL_start_sequence(midi_handle, seq_handles[0]);
        else
            PlayMusic(music_next, music_next);
    }
    StreamTick();
}

void SetMusicOn(int16_t on)
{
    if (midi_ok == 0 || music_on == on)
        return;
    music_on = on;
    if (music_on)
        PlayMusic(music_next, music_next);
    else
        StopSeq(0);
}

/* Play seq in slots 1-7, reusing a slot already holding it or else the
 * least recently used one. */
void PlaySfxSeq(int16_t seq)
{
    int16_t i;

    if (midi_ok == 0 || music_on == 0)
        return;
    for (i = 1; i < NUM_SEQ; i++) {
        if (seq_ids[i] == seq) {
            AIL_start_sequence(midi_handle, seq_handles[i]);
            ListRemove(&seq_list, &seq_nodes[i].node);
            ListAdd(&seq_list, &seq_nodes[i].node);
            return;
        }
    }
    i = ((SeqNode *)seq_list.head.prev)->slot;
    ListRemove(&seq_list, &seq_nodes[i].node);
    ListAdd(&seq_list, &seq_nodes[i].node);
    PlaySeq(i, seq);
}

void SetSfxOn(int16_t on)
{
    int16_t i;

    if (digi_ok == 0 || sfx_on == on)
        return;
    sfx_on = on;
    if (sfx_on == 0)
        for (i = 1; i < NUM_SEQ; i++)
            StopSeq(i);
    if (sfx_on == 0)
        FreeAllSfxFiles();
    if (sfx_on == 1 && sfx_name[0])
        PlaySfx(sfx_name);
}

int16_t GetMusicSeq(void)
{
    return seq_ids[0];
}

void SaveMusicState(void)
{
    char name[10];
    int16_t seq;

    seq = music_next;
    strcpy(name, music_name);
    SaveWrite(name, 9);
    SaveWrite(&seq, 2);
}

void LoadMusicState(void)
{
    char name[10];
    int16_t seq;

    SaveRead(name, 9);
    SaveRead(&seq, 2);
    if (name[0]) {
        LoadMusic(name);
        PlayMusic(seq, seq);
    } else
        StopAllSound();
}

/* Start digital output: auto-init DMA over the whole buffer when the
 * driver supports it, else one half at a time. */
void DigiInit(void)
{
    digi_status = sub_35828();
    if (sub_358B3() == 0) {
        sub_35388("No sound driver resident.\n");
        exit(1);
    }
    dma_auto = 0;
    dma_req.len = HALF;
    dma_req.rate = 22050;
    if (sub_3577A() & 0x200) {
        if (sub_3588D(dma_buf, 0x2000)) {
            dma_auto = 1;
            dma_half_pos = HALF;
            sub_35873(dma_buf, 0x2000);
            sub_3585E(1);
            dma_req.buf = dma_buf;
            dma_req.len = 0x2000;
            dma_req.rate = 22050;
            sub_3571B(&dma_req);
        } else {
            printf("DMA buffer crosses segment boundary\n");
            exit(1);
        }
    } else {
        sub_35873(dma_buf, 0x2000);
        dma_req.buf = dma_buf + (dma_half << 12);
        dma_req.len = HALF;
        dma_req.rate = 22050;
        sub_3571B(&dma_req);
        dma_req.buf = dma_buf + (mix_half << 12);
        DigiPlay(&dma_req);
    }
}

/* Timer tick: when a half of the buffer has played, mix the next. */
void DigiTick(void)
{
    if (digi_ok && digi_running) {
        digi_ticks++;
        if (DigiStatus() != 2) {
            if (dma_half == 0) {
                dma_half = 1;
                mix_half = 0;
            } else {
                dma_half = 0;
                mix_half = 1;
            }
            if (stream_active == 1 && snd_chan[0].busy == 0 && snd_chan[1].busy == 0
                    && snd_chan[2].busy == 0 && snd_chan[3].busy == 0)
                ;
            sub_35873(dma_buf + (mix_half << 12), HALF);
            mix_first = 0;
            MixFrame();
            dma_req.len = HALF;
            dma_req.buf = dma_buf + (mix_half << 12);
            DigiPlay(&dma_req);
        }
    }
}

void DigiPlay(DmaReq *req)
{
    if (dma_auto == 0)
        sub_357EB(req);
}

int16_t DigiStatus(void)
{
    int16_t st;
    int16_t n;

    if (dma_auto) {
        dma_state = st = 2;
        dma_count = n = sub_3586D();
        if (dma_half_pos && n < dma_half_pos) {
            dma_state = st = 1;
            dma_half_pos = 0;
        } else if (dma_half_pos == 0 && n >= HALF) {
            dma_state = st = 1;
            dma_half_pos = HALF;
        }
    } else
        st = sub_357FE();
    return st;
}

void DigiStart(void)
{
    digi_running = 1;
    dma_half = 0;
    mix_half = 1;
    mix_first = 1;
    DigiInit();
}

void DigiShutdown(void)
{
    int16_t i;

    if (digi_ok) {
        sub_357C0();
        sub_35873(dma_buf + (mix_half << 12), HALF);
        digi_running = 0;
        for (i = 0; i < NUM_CHAN; i++)
            snd_chan[i].busy = 0;
        if (voc_fd != -1) {
            close(voc_fd);
            voc_fd = -1;
        }
        stream_active = 0;
    }
}

/* Stream the speech files in name ("file,file,..."). */
void PlayVoc(char *name)
{
    if (sfx_on == 0 || digi_ok == 0)
        return;
    StopVoc();
    stream_active = 1;
    strcpy(voc_name, name);
    stream_done = 0;
    stream_filling = 1;
    StreamFill();
    stream_filling = 0;
    if (digi_running == 0)
        DigiStart();
}

/* Start sound effect name.voc on a free channel, mixing straight into the
 * part of the buffer not yet played. */
int16_t PlaySfx(char *name)
{
    int16_t n;
    uint16_t pos;
    int16_t unused1;
    int16_t unused2;
    int16_t i;
    uint16_t half;
    MemHandle h;

    if (strcmpi(name + 4, "loop") == 0)
        strcpy(sfx_name, name);
    if (sfx_on == 0 || digi_ok == 0)
        return 0;
    if (digi_running == 0)
        DigiStart();
    for (i = 0; i < NUM_CHAN; i++) {
        if (snd_chan[i].busy == 0) {
            h = LoadSfxFile(name, "voc");
            snd_chan[i].handle = h;
            if (snd_chan[i].handle == 0)
                return 0;
            snd_chan[i].pos = 0;
            snd_chan[i].len = 0;
            pos = 0x2000 - sub_3586D();
            half = pos / HALF;
            if (half == dma_half) {
                if (pos >= HALF)
                    pos -= HALF;
                n = HALF - pos;
                MixVoc(dma_buf + ((dma_half << 12) + pos), &snd_chan[i], n);
                if (snd_chan[i].pos == 0) {
                    SfxNop(snd_chan[i].handle);
                    snd_chan[i].busy = 0;
                    return 0;
                }
            }
            if (mix_first == 1) {
                n = HALF;
                MixVoc(dma_buf + (mix_half << 12), &snd_chan[i], n);
                if (snd_chan[i].pos == 0) {
                    SfxNop(snd_chan[i].handle);
                    snd_chan[i].busy = 0;
                    return 0;
                }
            }
            snd_chan[i].busy = 1;
            return 1;
        }
    }
    return 0;
}

void StreamTick(void)
{
    StreamFill();
}

/* Mix the speech stream and the sound effects into the half being filled. */
void MixFrame(void)
{
    int16_t i;
    int16_t n;
    int32_t unused;

    mix_first = 1;
    n = HALF;
    if (stream_fill_pos != stream_play_pos && stream_filling == 0) {
        stream_mixed = 1;
        MixSamples(dma_buf + (mix_half << 12), MapStream(), n);
        stream_play_pos += n;
        if (stream_play_pos >= 0x10000L)
            stream_play_pos = 0;
    } else {
        if (stream_mixed == 1)
            stream_done = 1;
        stream_mixed = 0;
    }
    for (i = 0; i < NUM_CHAN; i++) {
        if (snd_chan[i].busy) {
            n = HALF;
            MixVoc(dma_buf + (mix_half << 12), &snd_chan[i], n);
            if (snd_chan[i].pos == 0) {
                snd_chan[i].busy = 0;
                SfxNop(snd_chan[i].handle);
            }
        }
    }
}

/* Add n signed samples from src to the unsigned samples in dst, clamped. */
void MixSamples(char *dst, char *src, int16_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    int v;

    for (; n > 0; n--, d++, s++) {
        v = (int8_t)(*s + 0x80) + *d;
        *d = v < 0 ? 0 : v > 255 ? 255 : v;
    }
}

/* Mix up to n bytes of the VOC file playing on ch into dst, following its
 * blocks. Returns the bytes mixed; ch->pos is 0 at the end of the file. */
int16_t MixVoc(char *dst, SndChan *ch, uint16_t n)
{
    int16_t unused;
    int16_t total;

    total = 0;
    while (n) {
        if (ch->len) {
            char *src;
            uint16_t cnt;

            cnt = n;
            if (cnt > (uint32_t)ch->len)
                cnt = ch->len;
            src = MapMemRef((MemRef *)ch);
            MixSamples(dst, src, cnt);
            CacheNop();
            dst += cnt;
            ch->pos += cnt;
            n -= cnt;
            ch->len -= cnt;
            total += cnt;
        } else {
            int32_t unused2;
            unsigned char *p;
            char hdr[26];

            if (ch->pos == 0) {
                p = MapMemRef((MemRef *)ch);
                ch->pos += 26;
            }
            p = MapMemRef((MemRef *)ch);
            ch->len = ((int32_t)p[3] << 16) | ((uint16_t)p[2] << 8) | p[1];
            ch->pos += 4;
            switch (p[0]) {
            case 0:
                ch->len = 0;
                n = 0;
                ch->pos = 0;
                break;
            case 1:
                ch->pos += 2;
                ch->len -= 2;
                break;
            case 2:
                break;
            case 6:
                p += 4;
                ch->loops = ((int32_t)p[1] << 8) | p[0];
                ch->pos += 2;
                ch->loop_pos = ch->pos;
                ch->len = 0;
                break;
            case 7:
                ch->loops--;
                if (ch->loops)
                    ch->pos = ch->loop_pos;
                break;
            case 3:
            case 4:
            case 5:
            default:
                ch->pos += ch->len;
                ch->len = 0;
                break;
            }
            CacheNop();
        }
    }
    return total;
}
