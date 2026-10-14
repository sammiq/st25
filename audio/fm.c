/* fm.c - the music driver: an XMIDI sequencer playing through an emulated
 * OPL2 (AdLib / Sound Blaster FM).  It stands in for the Miles AIL driver the
 * original loaded from its .ADV files, with the calls the game made (sound.h).
 *
 * XMIDI is MIDI with notes carrying their own duration and delays written as
 * sums of bytes below 0x80, in ticks of 1/120 s at the default tempo (60 per
 * quarter note).  Instruments are the global timbre library (GTL.AD): two
 * operator FM patches, melodic in bank 0 and percussion in bank 127 (one
 * patch per drum note, played at a fixed pitch).
 */

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "audio.h"
#include "nuked/opl3.h"

#define NUM_SEQ     16
#define NUM_VOICES  9
#define MAX_NOTES   48
#define MAX_LOOPS   4

/* sequence states (AIL's) */
#define SEQ_FREE    0
#define SEQ_PLAYING 1
#define SEQ_DONE    2
#define SEQ_STOPPED 3

/* ---- timbres ---- */

typedef struct Timbre {
    int8_t transpose;
    uint8_t mod[5];             /* AVEKM, KSL/TL, AR/DR, SL/RR, waveform */
    uint8_t fb_c;               /* feedback and connection */
    uint8_t car[5];
    uint8_t valid;
} Timbre;

static Timbre timbres[2][128];  /* [0] bank 0, [1] bank 127 */

/* ---- state ---- */

typedef struct Chan {
    uint8_t bank, program;
    uint8_t volume, expression;
    int bend;                   /* 0..16383 */
    int bend_range;             /* semitones */
    int sustain;
    int modwheel;               /* 64 and over turns vibrato on */
    int rpn_hi, rpn_lo;
} Chan;

typedef struct Note {
    int chan, note;
    int left;                   /* ticks to go */
    int voice;
    int held;                   /* over, but the sustain pedal is down */
    int vel;                    /* velocity through the driver's curve */
} Note;

typedef struct Loop {
    const uint8_t *pos;
    int count;                  /* 0 = for ever */
} Loop;

typedef struct Seq {
    int state;
    const uint8_t *evnt, *evnt_end, *pos;
    uint32_t delay;
    uint32_t tempo;             /* microseconds per quarter note */
    Chan ch[16];
    Note notes[MAX_NOTES];
    int nnotes;
    Loop loops[MAX_LOOPS];
    int nloops;
} Seq;

typedef struct Voice {
    int seq, chan, note;        /* who plays it; seq -1 when free */
    uint32_t age;
    uint8_t b0;                 /* what B0 holds (key on, block, fnum high) */
    uint8_t on;                 /* the note is still held (else the voice is in its release) */
} Voice;

static opl3_chip chip;
static int out_rate = 48000;
static Seq seqs[NUM_SEQ];
static Voice voices[NUM_VOICES];
static uint32_t voice_clock;
static double tick_acc;                 /* output samples into the current tick */

static const uint8_t op_off[NUM_VOICES][2] = {
    { 0, 3 }, { 1, 4 }, { 2, 5 }, { 8, 11 }, { 9, 12 }, { 10, 13 }, { 16, 19 }, { 17, 20 }, { 18, 21 }
};

static void opl(int reg, int val)
{
    OPL3_WriteReg(&chip, reg, val);
}

int fm_samples_to_tick(void);
void fm_tick(void);

void fm_init(int rate)
{
    int i;

    out_rate = rate;
    OPL3_Reset(&chip, rate);
    memset(voices, 0, sizeof(voices));
    for (i = 0; i < NUM_VOICES; i++)
        voices[i].seq = -1;
    for (i = 0; i < NUM_SEQ; i++)
        seqs[i].state = SEQ_FREE;
    opl(0x01, 0x20);                    /* waveform select on */
    opl(0x08, 0x00);
    opl(0xBD, 0x00);                    /* normal tremolo and vibrato depth, no rhythm mode (the driver leaves it) */
    for (i = 0; i < NUM_VOICES; i++)
        opl(0xB0 + i, 0);
    tick_acc = 0;
}

static int tick_len;                    /* output samples per sequencer tick, 0 when idle */

void fm_render(int16_t *lr)
{
    if (--tick_len <= 0) {              /* sequence tempi are looked up once per tick */
        int n = fm_samples_to_tick();

        if (n) {
            fm_tick();
            tick_len = n;
        } else
            tick_len = 1;
    }
    OPL3_GenerateResampled(&chip, lr);
}

/* ---- voices ---- */

static const Timbre *find_timbre(const Chan *c, int chan, int note)
{
    const Timbre *t;

    if (chan == 9) {                    /* percussion: bank 127, a patch per note */
        t = &timbres[1][note & 127];
        return t->valid ? t : NULL;
    }
    t = &timbres[c->bank == 127 ? 1 : 0][c->program & 127];
    if (t->valid)
        return t;
    t = &timbres[0][c->program & 127];
    return t->valid ? t : NULL;
}

/* The driver's velocity curve: velocity / 8 indexes a table of 82..127, so
 * the softest note is still two thirds of full level. */
static int vel_curve(int vel)
{
    return 82 + 3 * ((vel & 127) >> 3);
}

/* Channel volume and expression, then velocity, each as x * y / 128 rounded
 * up by one unless it is zero (0..127). */
static int eff_volume(const Chan *c, int vel)
{
    int v = (c->volume * c->expression) >> 7;

    if (v)
        v++;
    v = (v * vel) >> 7;
    if (v)
        v++;
    return v > 127 ? 127 : v;
}

/* An operator's KSL/TL byte at volume v: the level (63 - TL) scales with v. */
static int scale_tl(int ksl_tl, int v)
{
    int level = 63 - (ksl_tl & 0x3F);

    return (ksl_tl & 0xC0) | (~(level * v / 127) & 0x3F);
}

static void set_pitch(int v, double semis, int keyon)
{
    double freq = 440.0 * pow(2.0, (semis - 69.0) / 12.0);
    int block, fnum = 0;

    for (block = 0; block < 8; block++) {
        fnum = (int)(freq * (double)(1 << (20 - block)) / 49716.0 + 0.5);
        if (fnum < 1024)
            break;
    }
    if (block == 8) {
        block = 7;
        fnum = 1023;
    }
    voices[v].b0 = (keyon ? 0x20 : 0) | (block << 2) | (fnum >> 8);
    opl(0xA0 + v, fnum & 0xFF);
    opl(0xB0 + v, voices[v].b0);
}

static void key_off(int v)
{
    voices[v].on = 0;
    voices[v].b0 &= ~0x20;
    opl(0xB0 + v, voices[v].b0);
}

static double note_semis(const Seq *s, int chan, int note, const Timbre *t)
{
    const Chan *c = &s->ch[chan];
    double n = chan == 9 ? t->transpose : note + t->transpose;

    return n + (double)(c->bend - 8192) / 8192.0 * c->bend_range;
}

static void program_voice(int v, const Timbre *t, int vol, int vib)
{
    int additive = t->fb_c & 1;
    int m = op_off[v][0], c = op_off[v][1];

    voices[v].b0 = 0;
    opl(0xB0 + v, 0);
    opl(0x20 + m, t->mod[0] | vib);
    opl(0x40 + m, additive ? scale_tl(t->mod[1], vol) : t->mod[1]);
    opl(0x60 + m, t->mod[2]);
    opl(0x80 + m, t->mod[3]);
    opl(0xE0 + m, t->mod[4] & 3);
    opl(0x20 + c, t->car[0] | vib);
    opl(0x40 + c, scale_tl(t->car[1], vol));
    opl(0x60 + c, t->car[2]);
    opl(0x80 + c, t->car[3]);
    opl(0xE0 + c, t->car[4] & 3);
    opl(0xC0 + v, t->fb_c & 0x0F);
}

/* A free voice; else one whose note is over (the longest dead), else the
 * oldest note, which is cut off. */
static int pick_voice(void)
{
    int i, best = -1;

    for (i = 0; i < NUM_VOICES; i++)
        if (voices[i].seq < 0)
            return i;
    for (i = 0; i < NUM_VOICES; i++)
        if (!voices[i].on && (best < 0 || voices[i].age < voices[best].age))
            best = i;
    if (best >= 0)
        return best;
    best = 0;
    for (i = 1; i < NUM_VOICES; i++)
        if (voices[i].age < voices[best].age)
            best = i;
    return best;
}

/* Give up note n of sequence s: key the voice off. */
static void end_note(Seq *s, int n)
{
    Note *nt = &s->notes[n];
    int v = nt->voice;

    if (voices[v].seq >= 0 && voices[v].chan == nt->chan && voices[v].note == nt->note)
        key_off(v);
    *nt = s->notes[--s->nnotes];
}

static void drop_voices_of(int si)
{
    int v;

    for (v = 0; v < NUM_VOICES; v++)
        if (voices[v].seq == si) {
            key_off(v);
            voices[v].seq = -1;
        }
}

static void note_on(Seq *s, int si, int chan, int note, int vel, int duration)
{
    Chan *c = &s->ch[chan];
    const Timbre *t = find_timbre(c, chan, note);
    int v, vol, i;

    if (!t || s->nnotes == MAX_NOTES)
        return;
    if (vel == 0)
        return;
    v = pick_voice();
    if (voices[v].seq >= 0) {           /* stolen: its note is over */
        Seq *o = &seqs[voices[v].seq];

        for (i = 0; i < o->nnotes; i++)
            if (o->notes[i].voice == v) {
                o->notes[i] = o->notes[--o->nnotes];
                break;
            }
    }
    vel = vel_curve(vel);
    vol = eff_volume(c, vel);
    program_voice(v, t, vol, c->modwheel >= 64 ? 0x40 : 0);
    voices[v].seq = si;
    voices[v].chan = chan;
    voices[v].note = note;
    voices[v].age = ++voice_clock;
    voices[v].on = 1;
    set_pitch(v, note_semis(s, chan, note, t), 1);
    s->notes[s->nnotes].chan = chan;
    s->notes[s->nnotes].note = note;
    s->notes[s->nnotes].left = duration > 0 ? duration : 1;
    s->notes[s->nnotes].voice = v;
    s->notes[s->nnotes].held = 0;
    s->notes[s->nnotes].vel = vel;
    s->nnotes++;
}

/* The pitch of the notes sounding on a channel changed (pitch bend). */
static void rebend(Seq *s, int chan)
{
    int i;

    for (i = 0; i < s->nnotes; i++) {
        Note *nt = &s->notes[i];
        const Timbre *t;

        if (nt->chan != chan)
            continue;
        t = find_timbre(&s->ch[chan], chan, nt->note);
        if (t && voices[nt->voice].seq >= 0)
            set_pitch(nt->voice, note_semis(s, chan, nt->note, t), voices[nt->voice].b0 & 0x20);
    }
}

/* ---- the XMIDI event stream ---- */

static uint32_t read_interval(Seq *s)
{
    uint32_t n = 0;

    while (s->pos < s->evnt_end && *s->pos < 0x80)
        n += *s->pos++;
    return n;
}

static uint32_t read_vlq(Seq *s)
{
    uint32_t n = 0;
    int i;

    for (i = 0; i < 4 && s->pos < s->evnt_end; i++) {
        uint8_t b = *s->pos++;

        n = (n << 7) | (b & 0x7F);
        if (!(b & 0x80))
            break;
    }
    return n;
}

static void finish(Seq *s, int si)
{
    int i;

    s->state = SEQ_DONE;
    for (i = s->nnotes - 1; i >= 0; i--)
        end_note(s, i);
    drop_voices_of(si);
}

/* Channel volume or expression changed: the notes sounding on it follow. */
static void revolume(Seq *s, int chan)
{
    int i;

    for (i = 0; i < s->nnotes; i++) {
        Note *nt = &s->notes[i];
        const Timbre *t;
        int v, m, c, vol;

        if (nt->chan != chan || voices[nt->voice].seq < 0)
            continue;
        t = find_timbre(&s->ch[chan], chan, nt->note);
        if (!t)
            continue;
        v = nt->voice;
        m = op_off[v][0];
        c = op_off[v][1];
        vol = eff_volume(&s->ch[chan], nt->vel);
        if (t->fb_c & 1)
            opl(0x40 + m, scale_tl(t->mod[1], vol));
        opl(0x40 + c, scale_tl(t->car[1], vol));
    }
}

static void controller(Seq *s, int si, int chan, int cc, int val)
{
    Chan *c = &s->ch[chan];
    int i;

    switch (cc) {
    case 1:
        c->modwheel = val;
        for (i = 0; i < s->nnotes; i++) {
            Note *nt = &s->notes[i];
            const Timbre *t = find_timbre(c, chan, nt->note);

            if (nt->chan == chan && t && voices[nt->voice].seq >= 0) {
                opl(0x20 + op_off[nt->voice][0], t->mod[0] | (val >= 64 ? 0x40 : 0));
                opl(0x20 + op_off[nt->voice][1], t->car[0] | (val >= 64 ? 0x40 : 0));
            }
        }
        break;
    case 7: c->volume = val; revolume(s, chan); break;
    case 11: c->expression = val; revolume(s, chan); break;
    case 64:
        c->sustain = val >= 64;
        if (!c->sustain)
            for (i = s->nnotes - 1; i >= 0; i--)
                if (s->notes[i].chan == chan && s->notes[i].held)
                    end_note(s, i);
        break;
    case 101: c->rpn_hi = val; break;
    case 100: c->rpn_lo = val; break;
    case 6:
        if (c->rpn_hi == 0 && c->rpn_lo == 0) {
            c->bend_range = val ? val : 2;
            rebend(s, chan);
        }
        break;
    case 114: c->bank = val; break;                 /* patch bank select */
    case 116:                                       /* for loop */
        if (s->nloops < MAX_LOOPS) {
            s->loops[s->nloops].pos = s->pos;
            s->loops[s->nloops].count = val;
            s->nloops++;
        }
        break;
    case 117:                                       /* next */
        if (val >= 64 && s->nloops) {
            Loop *l = &s->loops[s->nloops - 1];

            if (l->count == 0 || --l->count > 0)
                s->pos = l->pos;
            else
                s->nloops--;
        }
        break;
    case 123:                                       /* all notes off */
        for (i = s->nnotes - 1; i >= 0; i--)
            if (s->notes[i].chan == chan)
                end_note(s, i);
        break;
    default:
        break;                                      /* locks, protects, callbacks, pan... */
    }
}

/* Run one event.  Sets the sequence done at the end of the track. */
static void run_event(Seq *s, int si)
{
    uint8_t st = *s->pos++;
    int chan = st & 15, a, b;

    switch (st & 0xF0) {
    case 0x80:
        s->pos += 2;
        break;
    case 0x90: {
        uint32_t dur;

        if (s->pos + 2 > s->evnt_end) {
            finish(s, si);
            return;
        }
        a = *s->pos++;
        b = *s->pos++;
        dur = read_vlq(s);
        note_on(s, si, chan, a, b, dur);
        break;
    }
    case 0xA0:
        s->pos += 2;
        break;
    case 0xB0:
        a = *s->pos++;
        b = *s->pos++;
        controller(s, si, chan, a, b);
        break;
    case 0xC0:
        s->ch[chan].program = *s->pos++;
        break;
    case 0xD0:
        s->pos++;
        break;
    case 0xE0:
        a = *s->pos++;
        b = *s->pos++;
        s->ch[chan].bend = (b << 7) | a;
        rebend(s, chan);
        break;
    case 0xF0:
        if (st == 0xFF) {
            int type = *s->pos++;
            uint32_t len = read_vlq(s);

            if (type == 0x2F) {
                finish(s, si);
                return;
            }
            /* tempo events (type 0x51) are ignored: the timer stays at 120 Hz */
            s->pos += len;
        } else {
            uint32_t len = read_vlq(s);

            s->pos += len;
        }
        break;
    default:
        finish(s, si);                              /* not a status byte: give up */
        return;
    }
    if (s->pos >= s->evnt_end)
        finish(s, si);
}

static void seq_tick(Seq *s, int si)
{
    int i;

    for (i = s->nnotes - 1; i >= 0; i--) {
        Note *nt = &s->notes[i];

        if (--nt->left <= 0) {
            if (s->ch[nt->chan].sustain)
                nt->held = 1, nt->left = 1 << 30;
            else
                end_note(s, i);
        }
    }
    if (s->delay)
        s->delay--;
    while (s->state == SEQ_PLAYING && s->delay == 0) {
        run_event(s, si);
        if (s->state == SEQ_PLAYING)
            s->delay = read_interval(s);
    }
}

static void seq_reset(Seq *s)
{
    int i;

    for (i = 0; i < 16; i++) {
        s->ch[i].bank = 0;
        s->ch[i].program = 0;
        s->ch[i].volume = 127;
        s->ch[i].expression = 127;
        s->ch[i].bend = 8192;
        s->ch[i].bend_range = 2;
        s->ch[i].sustain = 0;
        s->ch[i].modwheel = 0;
        s->ch[i].rpn_hi = s->ch[i].rpn_lo = 127;
    }
    s->nnotes = 0;
    s->nloops = 0;
    s->tempo = 500000;
    s->pos = s->evnt;
    s->delay = read_interval(s);
}

/* ---- called from the audio thread ---- */

/* Advance the music by frames output samples; returns after each tick so
 * that the caller can generate the samples in between. */
int fm_samples_to_tick(void)
{
    int i, active = 0;
    double per_tick;

    for (i = 0; i < NUM_SEQ; i++)
        if (seqs[i].state == SEQ_PLAYING) {
            active = 1;
            break;
        }
    if (!active)
        return 0;
    /* the shortest tick of any sequence keeps them all in step */
    per_tick = 1e18;
    for (i = 0; i < NUM_SEQ; i++)
        if (seqs[i].state == SEQ_PLAYING) {
            double p = (double)out_rate * seqs[i].tempo / 60e6;

            if (p < per_tick)
                per_tick = p;
        }
    return (int)(per_tick + 0.5) ? (int)(per_tick + 0.5) : 1;
}

void fm_tick(void)
{
    int i;

    for (i = 0; i < NUM_SEQ; i++) {
        Seq *s = &seqs[i];

        if (s->state != SEQ_PLAYING)
            continue;
        seq_tick(s, i);
    }
}

/* ---- the AIL calls, from the game's thread ---- */

/* A chunk of an IFF file at p: id, big endian length. */
static uint32_t be32(const uint8_t *p)
{
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

/* The nth XMIDI sequence of an .xmi file: its event stream. */
static int find_sequence(const uint8_t *d, int seq, const uint8_t **evnt, const uint8_t **end)
{
    const uint8_t *p = d;
    uint32_t len;
    int n = 0;

    if (memcmp(p, "FORM", 4) != 0 || memcmp(p + 8, "XDIR", 4) != 0)
        return 0;
    p += 8 + ((be32(p + 4) + 1) & ~1u);             /* past the XDIR form */
    if (memcmp(p, "CAT ", 4) != 0 || memcmp(p + 8, "XMID", 4) != 0)
        return 0;
    len = be32(p + 4);
    {
        const uint8_t *cat_end = p + 8 + len;

        p += 12;
        while (p < cat_end && memcmp(p, "FORM", 4) == 0) {
            uint32_t flen = be32(p + 4);
            const uint8_t *form_end = p + 8 + flen;

            if (n == seq) {
                const uint8_t *q = p + 12;          /* after FORM len XMID */

                while (q + 8 <= form_end) {
                    uint32_t clen = be32(q + 4);

                    if (memcmp(q, "EVNT", 4) == 0) {
                        *evnt = q + 8;
                        *end = q + 8 + clen;
                        return 1;
                    }
                    q += 8 + ((clen + 1) & ~1u);
                }
                return 0;
            }
            n++;
            p = form_end + (flen & 1);
        }
    }
    return 0;
}

void AIL_shutdown(char *msg)
{
    int i;

    plat_audio_lock();
    for (i = 0; i < NUM_SEQ; i++)
        seqs[i].state = SEQ_FREE;
    for (i = 0; i < NUM_VOICES; i++) {
        key_off(i);
        voices[i].seq = -1;
    }
    plat_audio_unlock();
}

/* Timbre data: the length word (14) and then the transpose and operators. */
void AIL_install_timbre(int16_t h, int16_t bank, int16_t patch, void *data)
{
    const uint8_t *p = data;
    Timbre *t;

    if ((bank != 0 && bank != 127) || patch < 0 || patch > 127)
        return;
    t = &timbres[bank == 127][patch];
    plat_audio_lock();
    t->transpose = (int8_t)p[2];
    memcpy(t->mod, p + 3, 5);
    t->fb_c = p[8];
    memcpy(t->car, p + 9, 5);
    t->valid = 1;
    plat_audio_unlock();
}

/* Returns the sequence's handle, -1 if there is none or no room. */
int16_t AIL_register_sequence(int16_t h, void *data, int16_t seq, void *state, void *ctrl)
{
    const uint8_t *evnt, *end;
    int i;

    if (!find_sequence(data, seq, &evnt, &end))
        return -1;
    plat_audio_lock();
    for (i = 0; i < NUM_SEQ; i++)
        if (seqs[i].state == SEQ_FREE) {
            seqs[i].evnt = evnt;
            seqs[i].evnt_end = end;
            seq_reset(&seqs[i]);
            seqs[i].state = SEQ_STOPPED;
            plat_audio_unlock();
            return i;
        }
    plat_audio_unlock();
    return -1;
}

void AIL_release_sequence_handle(int16_t h, int16_t seq)
{
    if (seq < 0 || seq >= NUM_SEQ)
        return;
    plat_audio_lock();
    finish(&seqs[seq], seq);
    seqs[seq].state = SEQ_FREE;
    plat_audio_unlock();
}

/* Start from the beginning. */
void AIL_start_sequence(int16_t h, int16_t seq)
{
    Seq *s;

    if (seq < 0 || seq >= NUM_SEQ || seqs[seq].state == SEQ_FREE)
        return;
    s = &seqs[seq];
    plat_audio_lock();
    finish(s, seq);
    seq_reset(s);
    s->state = SEQ_PLAYING;
    plat_audio_unlock();
}

void AIL_stop_sequence(int16_t h, int16_t seq)
{
    Seq *s;

    if (seq < 0 || seq >= NUM_SEQ || seqs[seq].state == SEQ_FREE)
        return;
    s = &seqs[seq];
    plat_audio_lock();
    finish(s, seq);
    s->state = SEQ_STOPPED;
    plat_audio_unlock();
}

int16_t AIL_sequence_status(int16_t h, int16_t seq)
{
    if (seq < 0 || seq >= NUM_SEQ)
        return SEQ_FREE;
    return seqs[seq].state;
}
