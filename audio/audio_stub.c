/* audio_stub.c - no sound: the entry points the game's sound code calls
 * into the music driver (the Miles AIL) and the digital driver. */

#include "compat.h"
#include "sound.h"

void AIL_startup(void) {}
void AIL_shutdown(char *msg) {}
int16_t AIL_register_driver(void *driver) { return -1; }
drvr_desc *AIL_describe_driver(int16_t h) { static drvr_desc d; return &d; }
int16_t AIL_detect_device(int16_t h, uint16_t IO, uint16_t IRQ, uint16_t DMA, uint16_t DRQ) { return 0; }
void AIL_init_driver(int16_t h, uint16_t IO, uint16_t IRQ, uint16_t DMA, uint16_t DRQ) {}
uint16_t AIL_state_table_size(int16_t h) { return 0; }
int16_t AIL_register_sequence(int16_t h, void *data, int16_t seq, void *state, void *ctrl) { return -1; }
void AIL_release_sequence_handle(int16_t h, int16_t seq) {}
uint16_t AIL_default_timbre_cache_size(int16_t h) { return 0; }
void AIL_define_timbre_cache(int16_t h, void *cache, uint16_t size) {}
void AIL_install_timbre(int16_t h, int16_t bank, int16_t patch, void *data) {}
void AIL_start_sequence(int16_t h, int16_t seq) {}
void AIL_stop_sequence(int16_t h, int16_t seq) {}
int16_t AIL_sequence_status(int16_t h, int16_t seq) { return 0; }
void sub_3B584(int16_t h, char *s) {}

/* the digital driver */
int16_t sub_35345(int16_t a, int16_t b) { return 0; }
void sub_35373(int16_t a, int16_t b) {}
void sub_35388(char *msg) {}
void sub_3571B(DmaReq *req) {}
uint16_t sub_3577A(void) { return 0; }
void sub_357C0(void) {}
void sub_357EB(DmaReq *req) {}
int16_t sub_357FE(void) { return 0; }
int16_t *sub_35828(void) { static int16_t status; return &status; }
void sub_3585E(int16_t on) {}
int16_t sub_3586D(void) { return 0; }
void sub_35873(char *buf, uint16_t len) {}
int16_t sub_3588D(char *buf, uint16_t len) { return 0; }
int16_t sub_358B3(void) { return 0; }
