/* sbdrv.h - the game's view of the digital sound driver: what the original's
 * assembly module (sub_35345 ... sub_358B3) did to the Sound Blaster. */

#ifndef SBDRV_H
#define SBDRV_H

#include <stdint.h>

int sb_present(void);
unsigned sb_caps(void);                         /* bit 0x200: auto-init DMA */
int sb_check_buffer(void *buf, unsigned len);
void sb_silence(void *buf, unsigned len);
void sb_set_auto(int on);
void sb_play(void *buf, unsigned len, unsigned rate);
void sb_stop(void);
int sb_remaining(void);
int16_t *sb_status(void);

#endif
