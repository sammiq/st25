/* seg016: resource cache, sound files and the streaming buffer */

#include "compat.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "list.h"
#include "mem.h"
#include "bitmap.h"
#include "resource.h"
#include "cache.h"
#include "sound.h"
#include "obj3d.h"

#include <ctype.h>
void DigiShutdown(void);                       /* seg015 */

CacheEntry cache_entries[CACHE_ENTRIES];
List cache_list;
int32_t stream_fill_pos;
int32_t stream_play_pos;
int16_t stream_active;
MemHandle stream_handle;
SfxFile sfx_files[8];

void CacheInit(void)
{
    int16_t i;
    CacheEntry *e;

    ListInit(&cache_list);
    for (e = cache_entries, i = 0; i != CACHE_ENTRIES; ++i, ++e) {
        MemSet(e, sizeof(CacheEntry), 0);
        ListAdd(&cache_list, &e->node);
    }
}

/* Find a file in the cache, loading it into the least recently used free
 * entry if needed (and if load is set). With unpack the unpacked data is
 * returned and the entry stays locked until ReleaseCache. */
MemHandle CacheGet(char *name, char *ext, int16_t ems, int16_t load, int16_t unpack)
{
    int16_t i;
    int16_t hash;
    CacheEntry *e;

    hash = NameHash(name) + NameHash(ext);
    for (i = 0, e = (CacheEntry *)cache_list.tail.next; i != CACHE_ENTRIES;
            ++i, e = (CacheEntry *)e->node.next) {
        if (e->hash == hash && strncmp(e->name, name, 10) == 0 && strncmp(e->ext, ext, 4) == 0)
            goto found;
    }
    if (load == 0)
        return 0;
    for (i = 0, e = (CacheEntry *)cache_list.head.prev; i != CACHE_ENTRIES;
            ++i, e = (CacheEntry *)e->node.prev) {
        if (e->lock == 0) {
            if (e->packed)
                FreeMem(e->packed);
            if (e->data)
                FreeMem(e->data);
            GetFileSizes(name, ext, &e->size, &e->packed_size);
            e->lock++;
            e->packed = AllocMemAbort(e->packed_size, 0);
            e->lock--;
            LoadFile(name, ext, LockMem(e->packed));
            UnlockMem(e->packed);
            strncpy(e->name, name, 10);
            strncpy(e->ext, ext, 4);
            e->data = 0;
            e->hash = hash;
            goto found;
        }
    }
    Fatal("Cache overflow.");
found:
    ListRemove(&cache_list, &e->node);
    ListAdd(&cache_list, &e->node);
    if (unpack) {
        e->lock++;
        if (e->data == 0 && e->packed != 0) {
            e->data = AllocMemAbort(e->size, ems);
            UnpackAll(LockMem(e->packed), LockMem(e->data), e->packed_size);
            UnlockMem(e->packed);
            UnlockMem(e->data);
            if ((int16_t)e->size < (int16_t)((e->packed_size >> 2) + e->packed_size)) {
                FreeMem(e->packed);
                e->packed = 0;
            }
        }
        return e->data;
    } else
        return 0;
}

/* Put an existing block into the cache under name.ext (locked). */
void CacheStore(char *name, char *ext, MemHandle h)
{
    int16_t i;
    int16_t hash;
    CacheEntry *e;

    hash = NameHash(name) + NameHash(ext);
    for (i = 0, e = (CacheEntry *)cache_list.head.prev; i != CACHE_ENTRIES;
            ++i, e = (CacheEntry *)e->node.prev) {
        if (e->lock == 0) {
            if (e->packed)
                FreeMem(e->packed);
            if (e->data)
                FreeMem(e->data);
            e->lock++;
            ListRemove(&cache_list, &e->node);
            ListAdd(&cache_list, &e->node);
            e->data = h;
            e->packed = 0;
            strncpy(e->name, name, 10);
            strncpy(e->ext, ext, 4);
            e->hash = hash;
            return;
        }
    }
    Fatal("Cache overflow.");
}

void ReleaseCache(MemHandle h)
{
    int16_t i;
    CacheEntry *e;

    e = (CacheEntry *)cache_list.tail.next;
    for (i = 0; i != CACHE_ENTRIES; ++i, e = (CacheEntry *)e->node.next) {
        if (e->data == h) {
            if (--e->lock < 0)
                Fatal("ReleaseCache: too many releases. %s", e->name);
            return;
        }
    }
}

/* Free some cached memory; returns 0 if nothing could be freed. */
int16_t CachePurge(int16_t ems)
{
    CacheEntry *e;
    int16_t i;
    MemHandle h;

    /* unlocked entries: drop the packed or unpacked copy */
    e = (CacheEntry *)cache_list.head.prev;
    for (i = 0; i != CACHE_ENTRIES; ++i, e = (CacheEntry *)e->node.prev) {
        if (e->lock == 0 && e->data != 0) {
            if (e->packed != 0) {
                FreeMem(e->data);
                e->data = 0;
                return 1;
            } else if (e->ext[0] == '$') {
                FreeMem(e->data);
                ListRemove(&cache_list, &e->node);
                MemSet(e, sizeof(CacheEntry), 0);
                ListAdd(&cache_list, &e->node);
                return 1;
            }
        }
    }
    /* unlocked entries: drop the entry */
    e = (CacheEntry *)cache_list.head.prev;
    for (i = 0; i != CACHE_ENTRIES; ++i, e = (CacheEntry *)e->node.prev) {
        if (e->lock == 0 && (e->data != 0 || e->packed != 0)) {
            h = e->data ? e->data : e->packed;
            FreeMem(h);
            ListRemove(&cache_list, &e->node);
            MemSet(e, sizeof(CacheEntry), 0);
            ListAdd(&cache_list, &e->node);
            return 1;
        }
    }
    /* locked entries: drop the packed copy */
    e = (CacheEntry *)cache_list.head.prev;
    for (i = 0; i != CACHE_ENTRIES; ++i, e = (CacheEntry *)e->node.prev) {
        if (e->lock != 0 && e->packed != 0 && e->data != 0) {
            FreeMem(e->packed);
            e->packed = 0;
            return 1;
        }
    }
    return 0;
}

MemHandle LoadBitmap(char *name)
{
    return CacheLoad(name, "bmp");
}

void ReleaseBitmap(MemHandle h)
{
    CacheRelease(h);
}

/* Load and unpack a file into its own block (not cached). */
MemHandle getanyfile(char *name, char *ext)
{
    int32_t size;
    MemHandle h;

    if ((size = GetFileSize(name, ext)) > 65000L)
        Fatal("getanyfile: excessively large object requested %s.", name);
    h = AllocMemAbort(size, 0);
    LoadFilePacked(name, ext, LockMem(h));
    UnlockMem(h);
    return h;
}

void FreeAnyFile(MemHandle h)
{
    FreeMem(h);
}

void CachePreload(char *name, char *ext)
{
    CacheGet(name, ext, 0, 1, 0);
}

MemHandle CacheLoad(char *name, char *ext)
{
    return CacheGet(name, ext, 0, 1, 1);
}

void CacheRelease(MemHandle h)
{
    ReleaseCache(h);
}

void LoadR3S(Obj3D *o, char *name, int16_t n)
{
    MemHandle h;
    void *p;
    char buf[16];
    int16_t unused;

    unused = 0;
    sprintf(buf, "%s%02x", name, (n >> 1) & 0x77);
    h = CacheGet(buf, "r3s", 1, 1, 1);
    o->r3s = h;
    o->r3s_type = 0x24;
    p = LockMem(h);
    MemCpy(&o->rot, p, 18);
    UnlockMem(h);
}

void ReleaseR3S(MemHandle h)
{
    ReleaseCache(h);
}

/* Load a bitmap, scaled by scale/256. Crew "stnd"/"tele" bitmaps other
 * than the 'm' ones are built from the 'm' bitmap plus a .xor delta, with
 * the uniform colours shifted for 'k' and 'r', and cached as .$bm. */
MemHandle LoadSpriteBitmap(char *name, int16_t scale)
{
    MemHandle h;
    MemHandle scaled;
    int16_t unused;
    Bitmap *src;
    Bitmap *xr;
    Bitmap *dst;
    MemHandle hbase;
    MemHandle hxor;
    int16_t shift;
    uint16_t npix;
    char c;
    char *s;
    char *d;
    int16_t i;
    int16_t rows;
    uint16_t w;
    int16_t skip;
    int16_t unused2;

    if ((strncmp(name + 1, "stnd", 4) == 0 || strncmp(name + 1, "tele", 4) == 0)
            && (name[0] == 'm' || name[0] == 's' || name[0] == 'k' || name[0] == 'r')) {
        if (name[0] == 'm')
            h = CacheGet(name, "bmp", 0, 1, 1);
        else if ((h = CacheGet(name, "$bm", 0, 0, 1)) != 0)
            ;
        else {
            c = name[0];
            name[0] = 'm';
            hbase = CacheGet(name, "bmp", 0, 1, 1);
            name[0] = c;
            hxor = CacheGet(name, "xor", 0, 1, 1);
            src = LockMem(hbase);
            npix = src->w * src->h;
            UnlockMem(hbase);
            h = AllocMemAbort(npix + 8, 1);
            dst = LockMem(h);
            src = LockMem(hbase);
            xr = LockMem(hxor);
            *(BitmapHdr *)dst = *(BitmapHdr *)src;
            if (c == 'k')
                shift = 8;
            else if (c == 'r')
                shift = -8;
            else
                shift = 0;
            if (shift == 0)
                MemCpy(dst->data, src->data, npix);
            else {
                s = src->data;
                d = dst->data;
                /* shift the colours 0xA8-0xAF (the uniform's) by shift */
                for (; npix != 0; npix--, s++, d++) {
                    unsigned char t = (unsigned char)*s - 0xA8;

                    *d = t <= 7 ? (unsigned char)(t + 0xA8 + shift) : (unsigned char)*s;
                }
            }
            s = xr->data;
            d = &dst->data[(src->y - xr->y) * dst->w + (src->x - xr->x)];
            w = xr->w;
            rows = xr->h;
            skip = dst->w - w;
            /* xor the difference bitmap onto the copy */
            for (; rows != 0; rows--, d += skip)
                for (i = w; i != 0; i--)
                    *d++ ^= *s++;
            UnlockMem(hxor);
            ReleaseCache(hxor);
            UnlockMem(hbase);
            ReleaseCache(hbase);
            UnlockMem(h);
            CacheStore(name, "$bm", h);
        }
    } else
        h = CacheGet(name, "bmp", 1, 1, 1);
    if (scale == 0x100)
        return h;
    else {
        scaled = ScaleBitmap(h, scale);
        ReleaseCache(h);
        return scaled;
    }
}

void FreeSpriteBitmap(MemHandle h, int16_t scale)
{
    if (scale == 0x100)
        ReleaseCache(h);
    else
        FreeMem(h);
}

int16_t NameHash(char *s)
{
    int16_t sum;

    for (sum = 0; *s != 0; s++)
        sum += tolower(*s);
    return sum;
}

/* A 64K ring buffer for streamed sound. */
void StreamInit(void)
{
    stream_handle = AllocMem(0x10000L, 1);
}

/* Refill the ring buffer up to the play position, 8K at a time. */
void StreamFill(void)
{
    int32_t left;
    int32_t pos;
    int32_t chunk;
    uint16_t n;
    char *p;

    if (!stream_active)
        return;
    left = stream_play_pos - stream_fill_pos;
    if (left <= 0)
        left += 0x10000L;
    left -= 1;
    left &= 0xFFFFE000L;
    pos = stream_fill_pos;
    while (left != 0) {
        p = (char *)LockMem(stream_handle) + pos;
        chunk = left > 0x2000 ? 0x2000L : left;
        n = ReadVoc(p, chunk);
        if (n == 0) {
            stream_active = 0;
            UnlockMem(stream_handle);
            break;
        } else if (n < chunk) {
            stream_active = 0;
            memset(p + n, 0x80, 0x2000 - n);
        }
        left -= chunk;
        pos += chunk;
        if (pos >= 0x10000L)
            pos = 0;
        UnlockMem(stream_handle);
    }
    stream_fill_pos = pos;
}

/* Load voc_path/sfx/name.ext whole into memory, remembering up to 8 files. */
MemHandle LoadSfxFile(char *name, char *ext)
{
    int32_t size;
    MemHandle h;
    char path[40];
    int16_t fd;
    uint16_t got;
    int16_t i;

    for (i = 0; i < 8; i++)
        if (sfx_files[i].handle != 0 && strncasecmp(sfx_files[i].name, name, 10) == 0)
            return sfx_files[i].handle;
    sprintf(path, "%ssfx/%s.%s", voc_path, name, ext);
    if ((fd = open(path, O_RDONLY | O_BINARY)) == -1)
        return 0;
    size = filelength(fd);
    if ((h = AllocMem(size + 4, 1)) == 0) {       /* MixVoc reads a whole block header at the last byte */
        close(fd);
        return 0;
    }
    memset((char *)h->ptr + size, 0, 4);
    {
        char *p = LockMem(h);
        int32_t left = size;

        while (left > 0) {
            ReadChunk(fd, p, left > 0x4000 ? 0x4000 : (uint16_t)left, &got);
            if (got == 0)
                break;
            p += got;
            left -= got;
        }
    }
    UnlockMem(h);
    close(fd);
    for (i = 0; i < 8; i++)
        if (sfx_files[i].handle == 0)
            break;
    if (i == 8)
        FreeSfxFiles();
    for (i = 0; i < 8; i++)
        if (sfx_files[i].handle == 0) {
            sfx_files[i].handle = h;
            strncpy(sfx_files[i].name, name, 10);
            break;
        }
    return h;
}

void *MapMemRef(MemRef *r)
{
    return (char *)r->handle->ptr + r->offset;
}

void *MapStream(void)
{
    return (char *)stream_handle->ptr + stream_play_pos;
}

void CacheNop(void)
{
}

void FreeAllSfxFiles(void)
{
    int16_t i;

    DigiShutdown();
    for (i = 0; i < 8; i++)
        if (sfx_files[i].handle != 0) {
            FreeAnyFile(sfx_files[i].handle);
            sfx_files[i].handle = 0;
            sfx_files[i].name[0] = 0;
        }
}

/* Free the sound files that no channel is playing. */
void FreeSfxFiles(void)
{
    int16_t i;

    for (i = 0; i < 8; i++) {
        if (sfx_files[i].handle != 0
                && !(snd_chan[0].handle == sfx_files[i].handle && snd_chan[0].busy)
                && !(snd_chan[1].handle == sfx_files[i].handle && snd_chan[1].busy)
                && !(snd_chan[2].handle == sfx_files[i].handle && snd_chan[2].busy)
                && !(snd_chan[3].handle == sfx_files[i].handle && snd_chan[3].busy)) {
            FreeAnyFile(sfx_files[i].handle);
            sfx_files[i].handle = 0;
            sfx_files[i].name[0] = 0;
        }
    }
}

void SfxNop(MemHandle h)
{
}

void LoadSfx(char *name)
{
    MemHandle h;

    h = LoadSfxFile(name, "voc");
    SfxNop(h);
}
