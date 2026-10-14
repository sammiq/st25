/* audio_drv.c - what the game's sound code (seg015) calls into: the AIL
 * driver set-up calls that need nothing here (the drivers are emulated in
 * fm.c and sb.c) and the original's digital driver (sub_35345 ... sub_358B3),
 * which drove the Sound Blaster and is now sb.c.  The calls that play and
 * edit music are in fm.c. */

#include "compat.h"
#include "sound.h"
#include "audio.h"

/* music driver: nothing to load, detect or set up */
uint16_t AIL_state_table_size(int16_t h) { return 16; }

/* the digital driver */
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
