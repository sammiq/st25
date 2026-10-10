/* resource.h - DATA.DIR / DATA.RUN / DATA.001 resource files and patches */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "compat.h"

#include "mem.h"

/* DATA.DIR entry, sorted by extension then name. */
typedef struct DirEntry {
    char name[8];           /* 00 */
    char ext[3];            /* 08 */
    uint16_t offset;        /* 0B low word of the offset in DATA.001, or of
                               the DATA.RUN entry; bit 0: replaced by a patch */
    unsigned char hi;       /* 0D bits 0-6: offset bits 16-22, or for numbered
                               files (bit 7) the number of files */
} DirEntry;

/* A located file. In DATA.001 each file starts with its unpacked and packed
 * sizes, which are read into size/packed. */
typedef struct FileInfo {
    char name[12];          /* 00 */
    uint32_t offset;   /* 0C -1 if not found */
    uint16_t size;          /* 10 */
    uint16_t packed;        /* 12 */
} FileInfo;

extern int16_t voc_on_cd;       /* 1 when speech is read from the CD */
extern char *exit_msg;      /* printed by PrintExitMsg */
extern int16_t data_not_open;
extern int16_t dir_last;        /* DATA.DIR index of the last lookup */
extern char voc_path[];     /* directory holding the speech files */
extern MemHandle dir_handle;
extern uint16_t dir_size;
extern uint16_t dir_count;
extern MemHandle run_handle;
extern int16_t data_fd;
extern char file_buf[];

void FindCD(void);
void PrintExitMsg(void);
void SetExitMsg(char *msg);
int16_t UnpackChunk(unsigned char *src, unsigned char **pdst, uint16_t len);
void Unpack(unsigned char *src, unsigned char *dst, uint16_t len);
void UnpackAll(unsigned char *src, unsigned char *dst, uint16_t len);
void ReadFile(int16_t fd, void *buf, uint16_t len);
uint16_t ReadChunk(int16_t fd, void *buf, uint16_t len, uint16_t *pread);
void OpenDataFiles(void);
FileInfo FindFile(char *name, char *ext, int16_t mark);
int16_t LoadPatch(char *name, char *ext, void *buf);
int16_t LoadPatchPacked(char *name, char *ext, void *dst);
int32_t GetPatchSize(char *name, char *ext);
int16_t GetPatchSizes(char *name, char *ext, uint16_t *psize, uint16_t *ppacked);
void LoadFile(char *name, char *ext, void *buf);
void LoadFilePacked(char *name, char *ext, void *dst);
int32_t GetFileSize(char *name, char *ext);
void GetFileSizes(char *name, char *ext, uint16_t *psize, uint16_t *ppacked);
void CopyName(char *dst, char *src);
void ScanPatches(void);

#endif
