/* audio_drv.c - what the game's sound code (seg015) calls into: the AIL
 * driver set-up calls that need nothing here (the drivers are emulated in
 * fm.c and sb.c) and the original's digital driver (sub_35345 ... sub_358B3),
 * which drove the Sound Blaster and is now sb.c.  The calls that play and
 * edit music are in fm.c. */

#include "compat.h"
#include "sound.h"
#include "audio.h"

/* music driver: nothing to load, detect or set up */
int16_t AIL_register_driver(void *driver) { return 0; }
drvr_desc *AIL_describe_driver(int16_t h) { static drvr_desc d; return &d; }
int16_t AIL_detect_device(int16_t h, uint16_t IO, uint16_t IRQ, uint16_t DMA, uint16_t DRQ) { return 1; }
void AIL_init_driver(int16_t h, uint16_t IO, uint16_t IRQ, uint16_t DMA, uint16_t DRQ) {}
uint16_t AIL_state_table_size(int16_t h) { return 16; }
uint16_t AIL_default_timbre_cache_size(int16_t h) { return 0; }
void AIL_define_timbre_cache(int16_t h, void *cache, uint16_t size) {}
void sub_3B584(int16_t h, char *s) {}

/* the digital driver */
int16_t sub_35345(int16_t a, int16_t b) { return 0; }
void sub_35373(int16_t a, int16_t b) {}
void sub_35388(char *msg) {}
void sub_3571B(DmaReq *req) { sb_play(req->buf, req->len, req->rate); }    /* start a block */
uint16_t sub_3577A(void) { return sb_caps(); }
void sub_357C0(void) { sb_stop(); }
void sub_357EB(DmaReq *req) { sb_play(req->buf, req->len, req->rate); }    /* next block, no auto-init */
int16_t sub_357FE(void) { return sb_busy() ? 2 : 1; }                      /* 2: still playing */
int16_t *sub_35828(void) { return sb_status(); }
void sub_3585E(int16_t on) { sb_set_auto(on); }
int16_t sub_3586D(void) { return (int16_t)sb_remaining(); }
void sub_35873(char *buf, uint16_t len) { sb_silence(buf, len); }
int16_t sub_3588D(char *buf, uint16_t len) { return sb_check_buffer(buf, len); }
int16_t sub_358B3(void) { return sb_present(); }
