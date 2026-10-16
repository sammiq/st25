/* platform.h - the boundary between the game (16-bit heritage, no host
 * headers) and the host.  Plain C types only: the implementation behind it
 * (platform/) is the only code that includes SDL.
 *
 * It stands in for what the original talked to directly: the VGA (mode 13h
 * memory and DAC), the BIOS (keyboard, tick count), the mouse driver, the
 * game port, DOS files, and the sound hardware.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

/* ---- life cycle ---- */

/* Parse host options (data directory, scale, ...) out of argv, leaving the
 * game's own arguments.  Returns the new argc. */
int plat_init(int argc, char **argv);
void plat_quit(void);

/* Handle host events (window, input, audio refill) and show the frame if it
 * is time.  The game's polling loops call this through ReadClock. */
void plat_pump(void);

/* ---- video: VGA mode 13h, 320x200, 256 colours ---- */

#define VGA_W 320
#define VGA_H 200

/* The A000 segment: VGA_W * VGA_H bytes of 8 bit pixels. */
uint8_t *plat_vram(void);

/* Set DAC entries; rgb holds 6 bit components (0-63), 3 bytes per entry. */
void plat_set_palette(int first, int count, const uint8_t *rgb);

/* ---- keyboard, as the BIOS and the keyboard interrupt saw it ---- */

int plat_key_ready(void);           /* a key is waiting */
int plat_key_read(void);            /* BIOS key: scan code << 8 | ASCII */
int plat_key_shift(void);           /* BIOS shift flags (INT 16h/02h) */
int plat_key_wait(void);            /* block for a key */

/* Raw scan codes (set 1, break = bit 7), what the INT 9 handler saw.  The
 * host calls this for every key event; the game defines it. */
void KbdScan(int code, int up);

/* ---- mouse ---- */

int plat_mouse_init(void);          /* 1 if there is a mouse */
void plat_mouse_read(int *x, int *y, int *buttons);    /* 320x200, buttons 1 left 2 right */
void plat_mouse_set(int x, int y);

/* ---- joystick ---- */

int plat_joy_present(void);
void plat_joy_read(int *x, int *y, int *buttons);      /* x, y in -128..127, buttons 1 and 2 */

/* ---- time ---- */

uint32_t plat_ticks(void);          /* BIOS ticks: 18.2065 per second since midnight */
uint32_t plat_msec(void);           /* milliseconds, monotonic */
void plat_delay(int ms);

/* ---- files ---- */

/* The game's files are named the DOS way: backslashes or slashes, any case,
 * relative to the data directory (where DATA.001 is).  These resolve the
 * name against what is really on disk. */
int plat_open(const char *path, int flags);     /* as open(2): fd, or -1 */
int32_t plat_filelength(int fd);
int plat_access(const char *path, int mode);
void *plat_fopen(const char *path, const char *mode);   /* a FILE * */
int plat_scan_dir(const char *dir, void (*cb)(const char *name));  /* names in dir */

/* The folder of the Amiga version's music as .mod files (--amiga-music,
 * STARTREK_AMIGA_MUSIC, else "amiga" in the player's data folder if there is
 * one), or NULL: then the music is the FM one. */
const char *plat_amiga_music_dir(void);
void plat_set_amiga_music_dir(const char *dir);

#endif
