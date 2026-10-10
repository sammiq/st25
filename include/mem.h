/* mem.h - memory manager: heap blocks behind handles
 *
 * The original had a near heap, a far pool of paragraphs that was compacted
 * when fragmented (hence the handles: a block could move between LockMem
 * calls), and EMS.  Blocks do not move any more, but the handle interface is
 * kept: callers still lock to get at the data.  The pool is a budget on the
 * total allocated; when it is used up the resource cache is purged.
 */

#ifndef MEM_H
#define MEM_H

#include "compat.h"

/* Handle table slot: the address of a block, or the free list link. */
typedef union MemSlot {
    void *ptr;
    union MemSlot *next;
} MemSlot;

typedef MemSlot *MemHandle;

#define MEM_MAX_HANDLES 512

extern MemSlot mem__handle_tbl[MEM_MAX_HANDLES];
extern MemSlot *mem_handle_free_list;   /* unused handle slots */
extern uint32_t pool_used;              /* bytes allocated through handles */
extern int16_t pool_stack_size;         /* PushMem depth */
extern int16_t pool_lock_count;

void *AllocMemPtr(int16_t size);
void FreeMemPtr(void *p);
void InitPMem(void);
void *PushMem(uint32_t size);
void PopMem(void);
void InitMemHandles(void);
void InitMem(void);
MemHandle AllocMemHandle(void *p);
void FreeMemHandle(MemHandle h);
MemHandle AllocMem(uint32_t size, int16_t ems);
MemHandle AllocMemAbort(uint32_t size, int16_t ems);
void *LockMem(MemHandle h);
void UnlockMem(MemHandle h);
void FreeMem(MemHandle h);
void *MemSet(void *dst, uint16_t n, int16_t c);
void MemCpy(void *dst, const void *src, uint16_t n);
void MemCpyNZ(void *dst, const void *src, uint16_t n);

#endif
