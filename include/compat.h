/* compat.h - the portable port's view of the original's Microsoft C 6.00
 * dialect.  Every game source file includes this first.
 *
 * The original is a 16-bit medium model program: `int` is 16 bits, `long`
 * 32, far pointers are segment:offset.  The sources use the fixed width
 * types below instead, and `far`/`near` are gone (all pointers are flat).
 * Structures are packed to 1 byte like the compiler's default, which keeps
 * the layout of everything that is made of 16 and 32 bit integers (the data
 * files and saved games) identical to the original.
 */

#ifndef COMPAT_H
#define COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#include "platform.h"

/* Audit build (compile only): make the 16 bit types 32 bit, so that the C
 * compiler's -Wsign-compare finds the comparisons that a 16 bit int made
 * unsigned (an int against an unsigned, which promotion hides at 16 bits). */
#ifdef AUDIT_SIGN
#define int16_t int
#define uint16_t unsigned int
#endif

/* The game's own structures, byte packed.  Headers of the platform layer
 * (SDL etc.) must not be included in files that include this one. */
#pragma pack(1)

#ifndef min
#define min(a, b)   (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b)   (((a) > (b)) ? (a) : (b))
#endif

/* MS C library names for the far variants of the string functions. */
#define _fmemcpy    memcpy
#define _fmemset    memset
#define _fmemcmp    memcmp
#define _fstrlen    strlen
#define _fstrcpy    strcpy
#define _fstrncmp   strncmp
#define _fstrnicmp  strncasecmp
#define _fstrncpy   strncpy
#define _fstrchr    strchr
#define _fstrstr    strstr
#define _fstrupr    strupr
#define strcmpi     strcasecmp

/* the original runtime's qsort: it orders elements that compare equal in its own way (src/qsort.c) */
void ms_qsort(void *base, size_t num, size_t width, int (*cmp)(const void *, const void *));
#define qsort       ms_qsort

/* Case-insensitive/in-place string helpers that are not in ISO C. */
char *strupr(char *s);
#define stricmp     strcasecmp
#define strnicmp    strncasecmp

#define O_BINARY    0

/* A blocking key read (the Press any key prompts). */
static inline int getch(void) { return plat_key_wait() & 0xFF; }

/* DOS style file names: see platform.h. */
#define open(path, flags)       plat_open((path), (flags))
#define access(path, mode)      plat_access((path), (mode))
#define fopen(path, mode)       ((FILE *)plat_fopen((path), (mode)))
#define filelength(fd)          plat_filelength(fd)

/* Fatal error: print the message and quit. */
void Fatal(const char *fmt, ...) __attribute__((noreturn));

#endif
