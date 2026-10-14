/* seg003: data file location, resource files (DATA.DIR/RUN/001) and patches */

#include "compat.h"
#include <fcntl.h>
#include <unistd.h>
#include "mem.h"
#include "resource.h"

/* Globals this file owns (declared in the headers). */
char voc_path[32];
MemHandle dir_handle;
int16_t data_fd;
uint16_t dir_size;
MemHandle run_handle;
uint16_t dir_count;

int16_t data_not_open = -1;
int16_t dir_last = 0;
int16_t voc_on_cd = 0;

/* Locate the speech directory.  The original looked for ./voc/speech.mrk
 * (installed) and then for the CD; the data directory (see plat_init)
 * holds the voc directory next to DATA.001 now. */
void FindCD(void)
{
    strcpy(voc_path, "voc/");
    voc_on_cd = 0;
}

/* The game's compression: flag bytes, each covering eight items from the
 * low bit up.  A set bit is a literal byte, a clear bit a 16 bit back
 * reference (12 bit distance, length - 3 in the low 4 bits).  Distance
 * 0 or 1 repeats the last byte.  A flag byte of FF is eight literals. */
static void UnpackItem(int literal, const unsigned char **ps, unsigned char **pd)
{
    const unsigned char *src = *ps;
    unsigned char *dst = *pd;

    if (literal)
        *dst++ = *src++;
    else {
        unsigned code = src[0] | (src[1] << 8);
        unsigned count = (code & 0x0F) + 3;
        unsigned dist = code >> 4;

        src += 2;
        if (dist < 2)
            memset(dst, dst[-1], count), dst += count;
        else
            for (; count != 0; count--, dst++)
                *dst = dst[-(int)dist];
    }
    *ps = src;
    *pd = dst;
}

/* Unpack from src while at least 16 bytes of input remain; returns the
 * number of bytes consumed and advances *pdst.  Only whole flag groups are
 * taken, so the rest is unpacked once more data has arrived. */
int16_t UnpackChunk(unsigned char *src, unsigned char **pdst, uint16_t len)
{
    const unsigned char *s = src;
    const unsigned char *end;
    unsigned char *dst = *pdst;
    unsigned flags;
    int i;

    if (len <= 16)
        return 0;
    end = src + len - 16;
    do {
        flags = *s++;
        for (i = 0; i < 8; i++)
            UnpackItem(flags & (1 << i), &s, &dst);
    } while (s < end);
    *pdst = dst;
    return (int16_t)(s - src);
}

/* Unpack the rest of the input. */
void Unpack(unsigned char *src, unsigned char *dst, uint16_t len)
{
    const unsigned char *s = src;
    const unsigned char *end = src + len;
    unsigned flags;
    int i;

    while (s < end) {
        flags = *s++;
        for (i = 0; i < 8; i++) {
            UnpackItem(flags & (1 << i), &s, &dst);
            if (s >= end)
                break;
        }
    }
}

void UnpackAll(unsigned char *src, unsigned char *dst, uint16_t len)
{
    uint16_t n;

    n = UnpackChunk(src, &dst, len);
    Unpack(src + n, dst, len - n);
}

void ReadFile(int16_t fd, void *buf, uint16_t len)
{
    uint16_t got;

    if (ReadChunk(fd, buf, len, &got))
        Fatal("error reading in _lread");
}

/* Read len bytes; *pread is the number read.  Returns 0, or non zero on
 * a read error. */
uint16_t ReadChunk(int16_t fd, void *vbuf, uint16_t len, uint16_t *pread)
{
    char *buf = vbuf;
    ssize_t got;

    *pread = 0;
    while (len != 0) {
        got = read(fd, buf, len);
        if (got < 0)
            return 1;
        if (got == 0)
            break;
        len -= got;
        buf += got;
        *pread += got;
    }
    return 0;
}

void OpenDataFiles(void)
{
    int16_t fd;
    char *dir;
    char *run;
    uint16_t len;

    if (data_not_open == -1) {
        if ((fd = open("data.dir", O_RDONLY | O_BINARY)) == -1)
            Fatal("Can't open data.dir.");
        dir_size = filelength(fd);
        dir_count = dir_size / sizeof(DirEntry);
        dir_handle = AllocMemAbort(dir_size, 1);
        dir = LockMem(dir_handle);
        ReadFile(fd, dir, dir_size);
        UnlockMem(dir_handle);
        close(fd);
        if ((fd = open("data.run", O_RDONLY | O_BINARY)) == -1)
            Fatal("Can't open data.run.");
        len = filelength(fd);
        run_handle = AllocMemAbort(len, 1);
        run = LockMem(run_handle);
        ReadFile(fd, run, len);
        UnlockMem(run_handle);
        close(fd);
        if ((data_fd = open("data.001", O_RDONLY | O_BINARY)) == -1)
            Fatal("Can't open data.001.");
        data_not_open = 0;
        ScanPatches();
    }
}

/* Binary search DATA.DIR. Numbered files (NAME0, NAME1, ...) share one
 * entry that points into DATA.RUN. With mark == 0 the file is flagged as
 * replaced by a patch instead. */
FileInfo FindFile(char *name, char *ext, int16_t mark)
{
    char fname[10];
    char fext[4];
    int16_t mid;
    int16_t r;
    int16_t lo;
    int16_t hi;
    int16_t len;
    DirEntry *dir;
    FileInfo fi;
    unsigned char *run;
    int16_t i;

    dir = LockMem(dir_handle);
    CopyName(fname, name);
    strupr(fname);
    strncpy(fext, ext, 3);
    fext[3] = 0;
    strupr(fext);
    ext = fext;
    len = strlen(fname);
    len = min(len, 8);
    lo = 0;
    hi = dir_count - 1;
    mid = dir_last;
    while (lo <= hi) {
        if ((r = _fstrncmp(ext, dir[mid].ext, 3)) == 0) {
            int16_t n;
            int16_t idx;

            n = _fstrlen(dir[mid].name);
            n = min(n, 8);
            idx = fname[n - 1] - dir[mid].name[n - 1];
            if (n == len && _fstrncmp(fname, dir[mid].name, n - 1) == 0
                    && (dir[mid].hi & 0x80) && idx < (dir[mid].hi & 0x7F) && idx >= 0) {
                uint32_t pos;
                uint32_t *p;
                uint16_t *q;
                int16_t flag;

                dir_last = mid;
                MemCpy(&fi, &dir[mid], 11);
                fi.offset = dir[mid].offset;
                fi.name[n - 1] = fname[n - 1];
                UnlockMem(dir_handle);
                run = LockMem(run_handle);
                run = run + (fi.offset & 0xFFFF);
                p = (uint32_t *)run;
                pos = *p & 0x7FFFFE;
                flag = *p & 1;
                q = (uint16_t *)(run + 3);
                for (i = 0; i < idx; i++) {
                    flag = *q & 1;
                    pos += *q++ & 0xFFFE;
                }
                if (!mark) {
                    if (i == 0)
                        *p |= 1;
                    else
                        q[-1] |= 1;
                    flag = 1;
                }
                if (flag)
                    break;
                fi.offset = pos + 4;
                lseek(data_fd, pos, SEEK_SET);
                ReadChunk(data_fd, (char *)&fi.size, 4, &i);
                UnlockMem(run_handle);
                return fi;
            }
            r = _fstrncmp(fname, dir[mid].name, 8);
        }
        if (r == 0) {
            int32_t b;

            dir_last = mid;
            if (!mark)
                dir[mid].offset |= 1;
            if (dir[mid].offset & 1)
                break;
            MemCpy(&fi, &dir[mid], 11);
            b = fi.offset = ((uint32_t)dir[mid].hi << 16) + dir[mid].offset + 4;
            b = (b - 4) & 0x7FFFFFFEL;
            lseek(data_fd, b, SEEK_SET);
            ReadChunk(data_fd, (char *)&fi.size, 4, &i);
            UnlockMem(dir_handle);
            return fi;
        } else if (r < 0)
            hi = mid - 1;
        else
            lo = mid + 1;
        mid = (lo + hi) >> 1;
    }
    UnlockMem(dir_handle);
    fi.offset = -1L;
    return fi;
}

int16_t LoadPatch(char *name, char *ext, void *buf)
{
    char fname[10];
    char path[64];
    int16_t fd;
    uint16_t size;
    uint16_t got;
    int16_t unused;

    CopyName(fname, name);
    sprintf(path, "patches/%s.%s", fname, ext);
    if ((fd = open(path, O_RDONLY | O_BINARY)) == -1)
        Fatal("Not found patches/%s.%s", fname, ext);
    if (ReadChunk(fd, (char *)&size, 2, &got))
        Fatal("error reading patches/%s.%s", fname, ext);
    if (ReadChunk(fd, buf, (uint16_t)filelength(fd) - 2, &got))
        Fatal("error reading patches/%s.%s", fname, ext);
    close(fd);
    return 1;
}

int16_t LoadPatchPacked(char *name, char *ext, void *vdst)
{
    unsigned char *dst = vdst;
    char fname[10];
    char path[64];
    int16_t fd;
    uint16_t size;
    uint16_t left;
    uint16_t have;
    uint16_t got;
    unsigned char buf[512];
    uint16_t n;

    CopyName(fname, name);
    sprintf(path, "patches/%s.%s", fname, ext);
    if ((fd = open(path, O_RDONLY | O_BINARY)) == -1)
        Fatal("Not found patches/%s.%s", fname, ext);
    if (ReadChunk(fd, (char *)&size, 2, &got))
        Fatal("error reading patches/%s.%s", fname, ext);
    left = filelength(fd) - 2;
    have = 0;
    while (left != 0) {
        if (ReadChunk(fd, buf + have, min(sizeof(buf) - have, left), &got))
            Fatal("error reading %s.%s", fname, ext);
        n = UnpackChunk(buf, &dst, have + got);
        left -= got;
        have += got - n;
        MemCpy(buf, buf + n, have);
    }
    Unpack(buf, dst, have);
    close(fd);
    return 1;
}

int32_t GetPatchSize(char *name, char *ext)
{
    char fname[10];
    char path[64];
    int16_t fd;
    uint16_t size;
    uint16_t got;

    CopyName(fname, name);
    sprintf(path, "patches/%s.%s", fname, ext);
    if ((fd = open(path, O_RDONLY | O_BINARY)) == -1)
        Fatal("Not found patches/%s.%s", fname, ext);
    ReadChunk(fd, (char *)&size, 2, &got);
    close(fd);
    return size;
}

int16_t GetPatchSizes(char *name, char *ext, uint16_t *psize, uint16_t *ppacked)
{
    char fname[10];
    char path[64];
    int16_t fd;
    uint16_t got;

    CopyName(fname, name);
    sprintf(path, "patches/%s.%s", fname, ext);
    if ((fd = open(path, O_RDONLY | O_BINARY)) == -1)
        Fatal("Not found patches/%s.%s", fname, ext);
    ReadChunk(fd, psize, 2, &got);
    *ppacked = filelength(fd) - 2;
    close(fd);
    return 1;
}

void LoadFile(char *name, char *ext, void *buf)
{
    char fname[10];
    FileInfo fi;
    uint16_t got;
    int16_t unused;

    CopyName(fname, name);
    fi = FindFile(fname, ext, 1);
    if (fi.offset & 1) {
        if (LoadPatch(name, ext, buf))
            return;
        else
            Fatal("File not found: %s %s\n", name, ext);
    }
    lseek(data_fd, fi.offset, SEEK_SET);
    if (ReadChunk(data_fd, buf, fi.packed, &got))
        Fatal("error reading %s.%s", fname, ext);
}

void LoadFilePacked(char *name, char *ext, void *vdst)
{
    unsigned char *dst = vdst;
    char fname[10];
    FileInfo fi;
    uint16_t left;
    uint16_t have;
    uint16_t got;
    unsigned char buf[512];
    uint16_t n;

    CopyName(fname, name);
    fi = FindFile(fname, ext, 1);
    if (fi.offset & 1) {
        if (LoadPatchPacked(name, ext, dst))
            return;
        else
            Fatal("File not found: %s %s\n", name, ext);
    }
    lseek(data_fd, fi.offset, SEEK_SET);
    left = fi.packed;
    have = 0;
    while (left != 0) {
        if (ReadChunk(data_fd, buf + have, min(sizeof(buf) - have, left), &got))
            Fatal("error reading %s.%s", fname, ext);
        n = UnpackChunk(buf, &dst, have + got);
        left -= got;
        have += got - n;
        MemCpy(buf, buf + n, have);
    }
    Unpack(buf, dst, have);
}

int32_t GetFileSize(char *name, char *ext)
{
    int32_t size;
    FileInfo fi;
    char fname[10];

    CopyName(fname, name);
    fi = FindFile(fname, ext, 1);
    if (fi.offset & 1)
        return GetPatchSize(name, ext);
    return fi.size;
}

void GetFileSizes(char *name, char *ext, uint16_t *psize, uint16_t *ppacked)
{
    char fname[10];
    FileInfo fi;

    CopyName(fname, name);
    fi = FindFile(fname, ext, 1);
    if (fi.offset & 1) {
        GetPatchSizes(name, ext, psize, ppacked);
        return;
    }
    *psize = fi.size;
    *ppacked = fi.packed;
}

/* Copy a name up to the first space. */
void CopyName(char *dst, char *src)
{
    while ((*dst = *src) != 0 && *src != ' ') {
        dst++;
        src++;
    }
    *dst = 0;
}

static void MarkPatch(const char *file)
{
    char name[10];
    char ext[4];
    const char *p = file;
    char *q;

    q = name;
    while (*p != 0 && *p != '.' && q < name + 9)
        *q++ = *p++;
    *q = 0;
    q = ext;
    if (*p != 0) {
        p++;
        while (*p != 0 && q < ext + 3)
            *q++ = *p++;
    }
    *q = 0;
    FindFile(name, ext, 0);
}

/* Flag every file in patches/ as replaced. */
void ScanPatches(void)
{
    plat_scan_dir("patches", MarkPatch);
}
