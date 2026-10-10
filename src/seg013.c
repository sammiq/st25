/* seg013: memory manager - heap blocks behind handles */

#include "compat.h"
#include "mem.h"

/* Globals this file owns (declared in the headers). */
MemSlot mem__handle_tbl[MEM_MAX_HANDLES];
MemSlot *mem_handle_free_list;
uint32_t pool_used;

int16_t CachePurge(int16_t ems);    /* seg016: release cached memory, 0 if nothing left */

/* Total of the blocks behind handles; the cache is purged to stay below. */
#define POOL_BUDGET (48UL << 20)

#define MAX_PUSH 16

int16_t pool_stack_size = 0;
int16_t pool_lock_count = 0;

/* Every handle block starts with its size, so that FreeMem can account. */
typedef struct BlockHdr {
    uint32_t size;
    uint32_t pad;
} BlockHdr;

static void *pushed[MAX_PUSH];

void *AllocMemPtr(int16_t size)
{
    void *p;

    if ((p = malloc(size)) == 0)
        Fatal("Near memory allocation failure, game over.");
    return p;
}

void FreeMemPtr(void *p)
{
    free(p);
}

void InitPMem(void)
{
    pool_used = 0;
}

/* Allocate permanent memory (the original took it from the top of the pool;
 * PopMem gives back the last one). */
void *PushMem(uint32_t size)
{
    void *p;

    if (pool_stack_size == MAX_PUSH || (p = calloc(1, size)) == 0)
        Fatal("Can't push %d bytes of FAR memory", (int)size);
    pushed[pool_stack_size++] = p;
    return p;
}

void PopMem(void)
{
    if (pool_stack_size == 0)
        Fatal("PopMem without PushMem.");
    free(pushed[--pool_stack_size]);
}

void InitMemHandles(void)
{
    int16_t i;

    mem_handle_free_list = 0;
    for (i = 0; i < MEM_MAX_HANDLES; i++) {
        mem__handle_tbl[i].next = mem_handle_free_list;
        mem_handle_free_list = &mem__handle_tbl[i];
    }
}

void InitMem(void)
{
    InitMemHandles();
    InitPMem();
}

MemHandle AllocMemHandle(void *p)
{
    MemHandle h;

    for (; mem_handle_free_list == 0; ) {
        if (!CachePurge(1))
            Fatal("Out of memory handles!");
    }
    h = mem_handle_free_list;
    mem_handle_free_list = mem_handle_free_list->next;
    h->ptr = p;
    return h;
}

void FreeMemHandle(MemHandle h)
{
    h->next = mem_handle_free_list;
    mem_handle_free_list = h;
}

/* Allocate a block of size bytes.  ems says the block is not wanted in low
 * memory in the original, which makes no difference now.  Returns 0 if the
 * cache has nothing left to give back. */
MemHandle AllocMem(uint32_t size, int16_t ems)
{
    BlockHdr *b;

    while (pool_used + size > POOL_BUDGET) {
        if (!CachePurge(ems != 0))
            return 0;
    }
    if ((b = malloc(sizeof(BlockHdr) + size)) == 0) {
        if (!CachePurge(ems != 0))
            return 0;
        return AllocMem(size, ems);
    }
    b->size = size;
    pool_used += size;
    return AllocMemHandle(b + 1);
}

MemHandle AllocMemAbort(uint32_t size, int16_t ems)
{
    MemHandle h;

    h = AllocMem(size, ems);
    if (h != 0)
        return h;
    Fatal("out of FAR memory");
    return 0;
}

void *LockMem(MemHandle h)
{
    ++pool_lock_count;
    return h->ptr;
}

void UnlockMem(MemHandle h)
{
    --pool_lock_count;
    if (pool_lock_count < 0)
        Fatal("LockCount < 0!");
}

void FreeMem(MemHandle h)
{
    BlockHdr *b = (BlockHdr *)h->ptr - 1;

    pool_used -= b->size;
    free(b);
    FreeMemHandle(h);
}

void *MemSet(void *dst, uint16_t n, int16_t c)
{
    memset(dst, c, n);
    return (char *)dst + n;
}

void MemCpy(void *dst, const void *src, uint16_t n)
{
    memcpy(dst, src, n);
}

/* Copy, skipping zero (transparent) bytes. */
void MemCpyNZ(void *dst, const void *src, uint16_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    while (n--) {
        if (*s)
            *d = *s;
        s++;
        d++;
    }
}
