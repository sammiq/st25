/* sound.h - music (Miles AIL 2.0 XMIDI drivers) and digitised sound */

#ifndef SOUND_H
#define SOUND_H

#include "compat.h"

#include "list.h"
#include "mem.h"
#include "cache.h"

/* AIL driver description (AIL_describe_driver). */
typedef struct drvr_desc {
    uint16_t min_API_version;   /* 00 */
    uint16_t drvr_type;         /* 02 2 = digital, 3 = XMIDI */
    char data_suffix[4];        /* 04 timbre file suffix */
    char *dev_name_table;   /* 08 */
    int16_t default_IO;             /* 0C */
    int16_t default_IRQ;            /* 0E */
    int16_t default_DMA;            /* 10 */
    int16_t default_DRQ;            /* 12 */
    int16_t service_rate;           /* 14 */
    uint16_t display_size;      /* 16 */
} drvr_desc;

#define DSP_DRVR    2
#define XMIDI_DRVR  3

/* A timbre in the global timbre library (.gtl) directory. */
typedef struct GTLEntry {
    char patch;                 /* 00 */
    char bank;                  /* 01 -1 ends the directory */
    int32_t offset;                /* 02 */
} GTLEntry;

/* Music sequence slot, kept in least recently used order (slots 1-7). */
typedef struct SeqNode {
    Node node;                  /* 00 */
    int16_t slot;                   /* 04 */
} SeqNode;

/* Digital output request (seg 2505 driver). */
typedef struct DmaReq {
    char *buf;              /* 00 */
    uint16_t len;               /* 04 */
    int16_t unk6;                   /* 06 */
    int16_t unk8;                   /* 08 */
    uint16_t rate;              /* 0A */
} DmaReq;

#define NUM_SEQ     8
#define NUM_CHAN    4
#define HALF        0x1000      /* half of the DMA buffer */

/* seg028: AIL */
void AIL_shutdown(char *msg);
uint16_t AIL_state_table_size(int16_t h);
int16_t AIL_register_sequence(int16_t h, void *data, int16_t seq, void *state, void *ctrl);
void AIL_release_sequence_handle(int16_t h, int16_t seq);
void AIL_install_timbre(int16_t h, int16_t bank, int16_t patch, void *data);
void AIL_start_sequence(int16_t h, int16_t seq);
void AIL_stop_sequence(int16_t h, int16_t seq);
int16_t AIL_sequence_status(int16_t h, int16_t seq);

extern int16_t music_on;
extern int16_t sfx_on;
extern int16_t midi_ok;
extern int16_t digi_ok;
extern int16_t midi_driver;
extern int16_t digi_driver;

void HookTimer(void);
void UnhookTimer(void);
void SoundInit(void);
void SoundStop(void);
void StopSeq(uint16_t slot);
void PlaySeq(uint16_t slot, int16_t seq);
int16_t MusicPlaying(void);
void StopVoc(void);
void StopAllSound(void);
void LoadMusic(char *name);
void PlayMusic(int16_t seq, int16_t next);
int16_t ReadVoc(char *buf, int16_t len);
void MusicTick(void);
void SetMusicOn(int16_t on);
void PlaySfxSeq(int16_t seq);
void SetSfxOn(int16_t on);
int16_t GetMusicSeq(void);
void SaveMusicState(void);
void LoadMusicState(void);
void DigiInit(void);
void DigiTick(void);
void DigiPlay(DmaReq *req);
int16_t DigiStatus(void);
void DigiStart(void);
void DigiShutdown(void);
void PlayVoc(char *name);
int16_t PlaySfx(char *name);
void StreamTick(void);
void MixFrame(void);
void MixSamples(char *dst, char *src, int16_t n);
int16_t MixVoc(char *dst, SndChan *ch, uint16_t n);

#endif
