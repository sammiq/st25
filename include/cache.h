/* cache.h - resource cache, sound files and the streaming buffer */

#ifndef CACHE_H
#define CACHE_H

#include "compat.h"

#include "list.h"
#include "mem.h"

#define CACHE_ENTRIES 256

/* A cached file: the packed data as read from DATA.001 and/or the unpacked
 * data. Entries are kept in most recently used order. */
typedef struct CacheEntry {
    Node node;              /* 00 */
    char name[10];          /* 04 */
    char ext[4];            /* 0E names starting with '$' are generated */
    int16_t lock;               /* 12 */
    int16_t hash;               /* 14 */
    MemHandle packed;       /* 16 */
    uint16_t packed_size;       /* 18 */
    MemHandle data;         /* 1A */
    uint16_t size;              /* 1C */
} CacheEntry;

/* A sound file loaded whole into memory. */
typedef struct SfxFile {
    MemHandle handle;       /* 00 */
    char name[10];          /* 02 */
} SfxFile;

/* A sound effect channel. Starts like a MemRef so MapMemRef can map the
 * read position; pos/len walk the VOC blocks of the loaded file. */
typedef struct SndChan {
    MemHandle handle;       /* 00 */
    int16_t busy;               /* 02 */
    uint32_t pos;      /* 04 offset in the file */
    int32_t len;               /* 08 bytes left in the current block */
    uint32_t loop_pos; /* 0C VOC repeat start */
    uint32_t loops;    /* 10 VOC repeat count */
} SndChan;

/* A location inside a handle's block. */
typedef struct MemRef {
    MemHandle handle;       /* 00 */
    int16_t unk;                /* 02 */
    uint32_t offset;   /* 04 */
} MemRef;


extern CacheEntry cache_entries[CACHE_ENTRIES];
extern List cache_list;
extern int32_t stream_fill_pos;
extern int32_t stream_play_pos;
extern int16_t stream_active;
extern MemHandle stream_handle;
extern SndChan snd_chan[4];
extern SfxFile sfx_files[8];

void CacheInit(void);
MemHandle CacheGet(char *name, char *ext, int16_t ems, int16_t load, int16_t unpack);
void CacheStore(char *name, char *ext, MemHandle h);
void ReleaseCache(MemHandle h);
int16_t CachePurge(int16_t ems);
MemHandle LoadBitmap(char *name);
void ReleaseBitmap(MemHandle h);
MemHandle getanyfile(char *name, char *ext);
void FreeAnyFile(MemHandle h);
void CachePreload(char *name, char *ext);
MemHandle CacheLoad(char *name, char *ext);
void CacheRelease(MemHandle h);
struct Obj3D;
void LoadR3S(struct Obj3D *o, char *name, int16_t n);
void ReleaseR3S(MemHandle h);
MemHandle LoadSpriteBitmap(char *name, int16_t scale);
void FreeSpriteBitmap(MemHandle h, int16_t scale);
int16_t NameHash(char *s);
void StreamInit(void);
void StreamFill(void);
MemHandle LoadSfxFile(char *name, char *ext);
void *MapHandle(MemHandle h);
void *MapMemRef(MemRef *r);
void *MapStream(void);
void CacheNop(void);
void FreeAllSfxFiles(void);
void FreeSfxFiles(void);
void SfxNop(MemHandle h);
void LoadSfx(char *name);

#endif
