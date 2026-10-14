/* The room scripts' environment for tests/test_rooms.py: the room state, the
 * file image, and a game call that records itself. */
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include "rooms.h"

RoomState room;
uint8_t stray_mem[0x400];
uint8_t *rdf_image;

static int calls;
static int seed;
int16_t log_words[512][6];
int log_n;

#ifndef TABLE
#error TABLE
#endif
extern const RoomScript TABLE[];

static int svc_result(int n)
{
    return (int)(((uint32_t)n * 2654435761u + 12345u + (uint32_t)seed * 977u) >> 7) & 3;
}

static jmp_buf out;

int32_t ScriptService(int16_t cmd, int16_t a1, int16_t a2, int16_t a3, int16_t a4, uint16_t a5)
{
    if (log_n < 512) {
        log_words[log_n][0] = cmd;
        log_words[log_n][1] = a1;
        log_words[log_n][2] = a2;
        log_words[log_n][3] = a3;
        log_words[log_n][4] = a4;
        log_words[log_n][5] = a5;
    }
    log_n++;
    if (cmd == RC_GOTOROOM)             /* the room is left: it does not come back */
        longjmp(out, 1);
    return svc_result(log_n);
}

int nscripts(void)
{
    int n = 0;

    while (TABLE[n].fn)
        n++;
    return n;
}

int script_offset(int i)
{
    return TABLE[i].offset;
}

void run_script(int i, uint8_t *image, uint8_t *room_bytes, int sd)
{
    seed = sd;
    log_n = 0;
    rdf_image = image;
    memcpy(&room, room_bytes, sizeof(room));
    if (setjmp(out) == 0)
        TABLE[i].fn();
    memcpy(room_bytes, &room, sizeof(room));
}
