/* qsort.c - the C runtime's qsort as the original linked it (Microsoft C 6.00,
 * MLIBCR qsort.asm), so that elements the comparison calls equal end up in the
 * same order as in the original.  glibc's qsort orders ties differently, and
 * the game sorts sprites, 3D objects and walk nodes with ties.
 *
 * It first looks for an already sorted array (one pass), then runs a quicksort
 * on the first element as pivot, with an explicit stack: the smaller part is
 * always done first.
 */

#include <stddef.h>
#include <string.h>

typedef int (*CompareFn)(const void *, const void *);

static void swap_elems(char *a, char *b, size_t w)
{
    char t[64];

    while (w) {
        size_t n = w < sizeof(t) ? w : sizeof(t);

        memcpy(t, a, n);
        memcpy(a, b, n);
        memcpy(b, t, n);
        a += n;
        b += n;
        w -= n;
    }
}

void ms_qsort(void *vbase, size_t num, size_t w, CompareFn cmp)
{
    char *base = vbase;
    char *p;
    size_t n;
    struct { char *lo, *hi; } stack[48];
    int k;

    if (w == 0 || num < 2)
        return;
    for (p = base, n = num - 1; n; n--, p += w)
        if (cmp(p + w, p) < 0)
            goto sort;
    return;

sort:
    k = 0;
    stack[0].lo = base;
    stack[0].hi = base + (num - 1) * w;
    k = 1;
    while (k) {
        char *lo = stack[k - 1].lo, *hi = stack[k - 1].hi;
        char *i = lo, *j = hi + w;
        char *a = lo;               /* last element found below the pivot */
        char *b = hi;               /* last element found above it */
        int c;

        if (lo >= hi) {
            k--;
            continue;
        }
        for (;;) {
            for (;;) {              /* up from the pivot to an element above it */
                i += w;
                if (i == hi)
                    break;
                c = cmp(i, lo);
                if (c > 0)
                    break;
                if (c < 0)
                    a = i;
            }
            for (;;) {              /* down from the end to an element below it */
                j -= w;
                c = cmp(lo, j);
                if (c > 0)
                    break;
                if (c == 0) {
                    if (j == lo)
                        break;
                    continue;
                }
                b = j;
            }
            if (j <= i)
                break;
            swap_elems(i, j, w);
            b = j;
            a = i;
        }
        swap_elems(lo, j, w);
        /* [lo, a] and [b, hi] are left: the smaller one is done first */
        if ((size_t)(hi - b) < (size_t)(a - lo)) {
            stack[k - 1].lo = lo;
            stack[k - 1].hi = a;
            stack[k].lo = b;
            stack[k].hi = hi;
        } else {
            stack[k - 1].lo = b;
            stack[k - 1].hi = hi;
            stack[k].lo = lo;
            stack[k].hi = a;
        }
        k++;
    }
}
