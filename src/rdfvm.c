/* rdfvm.c - interpreter for the room scripts (see rdfvm.h) */

#include "compat.h"
#include <setjmp.h>
#include "rdfvm.h"

#define STACK_WORDS     512
#define SENTINEL        0xFFFF
#define MAX_STEPS       2000000

enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };

typedef struct Vm {
    unsigned char *img;
    uint32_t img_size;
    unsigned char *room;
    uint16_t room_size;
    RdfService service;
    uint16_t r[8];
    unsigned cf, zf, sf, of, pf;
    uint16_t ip;
    uint16_t stk[STACK_WORDS + 8];      /* a few words of padding: calls read 6 words */
    jmp_buf fail;
} Vm;

/* A decoded instruction. */
typedef struct Insn {
    uint16_t len;
    unsigned char op;
    unsigned cs;            /* CS override prefix */
    unsigned char modrm;
    unsigned char mod, reg, rm;
    int16_t disp;
    int32_t imm;
} Insn;

static const char *fail_msg = "";
static uint16_t fail_ip;

const char *RdfError(uint16_t *ip)
{
    *ip = fail_ip;
    return fail_msg;
}

/* The script did something the real machine would not have survived. */
static void vm_fail(Vm *vm, const char *what, uint16_t ip)
{
    fail_msg = what;
    fail_ip = ip;
    longjmp(vm->fail, 1);
}

/* ---- decoding ---- */

static int fetch8(const Vm *vm, uint16_t *p, uint8_t *v)
{
    if (*p >= vm->img_size)
        return 0;
    *v = vm->img[(*p)++];
    return 1;
}

static int fetch16(const Vm *vm, uint16_t *p, uint16_t *v)
{
    uint8_t lo, hi;

    if (!fetch8(vm, p, &lo) || !fetch8(vm, p, &hi))
        return 0;
    *v = lo | (hi << 8);
    return 1;
}

/* The ModRM byte and displacement of the instruction at p. */
static int decode_modrm(const Vm *vm, uint16_t *p, Insn *in)
{
    uint8_t b;
    uint16_t w;

    if (!fetch8(vm, p, &b))
        return 0;
    in->modrm = b;
    in->mod = b >> 6;
    in->reg = (b >> 3) & 7;
    in->rm = b & 7;
    in->disp = 0;
    if (in->mod == 0 && in->rm == 6) {
        if (!fetch16(vm, p, &w))
            return 0;
        in->disp = (int16_t)w;
    } else if (in->mod == 1) {
        if (!fetch8(vm, p, &b))
            return 0;
        in->disp = (int8_t)b;
    } else if (in->mod == 2) {
        if (!fetch16(vm, p, &w))
            return 0;
        in->disp = (int16_t)w;
    }
    return 1;
}

/* Which opcodes take a ModRM byte, and what immediate follows:
 * imm: 0 none, 1 byte, 2 word. */
static int op_shape(const Insn *in, unsigned char op, int *modrm, int *imm)
{
    *modrm = 0;
    *imm = 0;
    if (op < 0x40) {
        switch (op & 7) {
        case 0: case 1: case 2: case 3: *modrm = 1; return 1;
        case 4: *imm = 1; return 1;
        case 5: *imm = 2; return 1;
        }
        return 0;
    }
    if (op >= 0x40 && op <= 0x5F)
        return 1;
    switch (op) {
    case 0x68: *imm = 2; return 1;
    case 0x6A: *imm = 1; return 1;
    case 0x80: *modrm = 1; *imm = 1; return 1;
    case 0x81: *modrm = 1; *imm = 2; return 1;
    case 0x83: *modrm = 1; *imm = 1; return 1;
    case 0x84: case 0x85: case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8D:
        *modrm = 1; return 1;
    case 0x90: return 1;
    case 0xA0: case 0xA1: case 0xA2: case 0xA3: *imm = 2; return 1;
    case 0xA8: *imm = 1; return 1;
    case 0xA9: *imm = 2; return 1;
    case 0xC3: case 0xCB: return 1;
    case 0xC6: *modrm = 1; *imm = 1; return 1;
    case 0xC7: *modrm = 1; *imm = 2; return 1;
    case 0xD0: case 0xD1: case 0xD2: case 0xD3: *modrm = 1; return 1;
    case 0xC0: case 0xC1: *modrm = 1; *imm = 1; return 1;
    case 0xE8: case 0xE9: *imm = 2; return 1;
    case 0xEB: *imm = 1; return 1;
    case 0xF6: case 0xF7: case 0xFE: case 0xFF: *modrm = 1; return 1;
    }
    if (op >= 0xB0 && op <= 0xB7) { *imm = 1; return 1; }
    if (op >= 0xB8 && op <= 0xBF) { *imm = 2; return 1; }
    if (op >= 0x70 && op <= 0x7F) { *imm = 1; return 1; }
    return 0;
}

/* Decode the instruction at ip.  Returns 0 if it isn't one the scripts
 * use (or runs off the image). */
static int decode(const Vm *vm, uint16_t ip, Insn *in)
{
    uint16_t p = ip;
    uint8_t b;
    int modrm, imm;

    memset(in, 0, sizeof(*in));
    if (!fetch8(vm, &p, &b))
        return 0;
    if (b == 0x2E) {
        in->cs = 1;
        if (!fetch8(vm, &p, &b))
            return 0;
    }
    in->op = b;
    if (!op_shape(in, b, &modrm, &imm))
        return 0;
    if (modrm && !decode_modrm(vm, &p, in))
        return 0;
    if (modrm) {
        /* only the forms the scripts use */
        switch (b) {
        case 0xFE: if (in->reg > 1) return 0; break;
        case 0xFF: if (in->reg != 0 && in->reg != 1 && in->reg != 3 && in->reg != 4 && in->reg != 6) return 0; break;
        case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xC0: case 0xC1:
            if (in->reg < 4) return 0;
            break;
        case 0xF6: case 0xF7:
            if (in->reg > 1) return 0;
            imm = b == 0xF6 ? 1 : 2;       /* test r/m, imm */
            break;
        }
    }
    if (imm == 1) {
        uint8_t v;

        if (!fetch8(vm, &p, &v))
            return 0;
        in->imm = (int8_t)v;
        if (b == 0x80 || b == 0xC6 || b == 0xF6 || b == 0xA8 || b == 0xC0 || b == 0xC1
                || (b < 0x40 && (b & 7) == 4) || (b >= 0xB0 && b <= 0xB7))
            in->imm = v;
    } else if (imm == 2) {
        uint16_t v;

        if (!fetch16(vm, &p, &v))
            return 0;
        in->imm = v;
    }
    in->len = p - ip;
    return 1;
}

/* ---- operands ---- */

static uint16_t base_reg(const Vm *vm, const Insn *in)
{
    static const signed char b1[8] = { R_BX, R_BX, R_BP, R_BP, R_SI, R_DI, R_BP, R_BX };
    static const signed char b2[8] = { R_SI, R_DI, R_SI, R_DI, -1, -1, -1, -1 };
    uint16_t ea = 0;

    if (in->mod == 0 && in->rm == 6)
        return in->disp;
    ea = vm->r[b1[in->rm]];
    if (b2[in->rm] >= 0)
        ea += vm->r[(int)b2[in->rm]];
    return ea + in->disp;
}

/* The bytes a memory operand names: CS-relative is the .rdf image, else the
 * room state in the data segment. */
static unsigned char *mem(Vm *vm, const Insn *in, uint16_t off, int size, uint16_t ip)
{
    int is_cs = in->cs;

    if (!is_cs && in->op != 0xA0 && in->op != 0xA1 && in->op != 0xA2 && in->op != 0xA3
            && (in->rm == 2 || in->rm == 3 || (in->rm == 6 && in->mod != 0)))
        vm_fail(vm, "stack segment operand", ip);
    if (is_cs) {
        if (off + size > vm->img_size)
            vm_fail(vm, "access beyond the file", ip);
        return vm->img + off;
    }
    if (off < RDF_ROOM_BASE || off + size > RDF_ROOM_BASE + vm->room_size)
        vm_fail(vm, "data access outside the room state", ip);
    return vm->room + (off - RDF_ROOM_BASE);
}

static uint16_t get16(const unsigned char *p)
{
    return p[0] | (p[1] << 8);
}

static void put16(unsigned char *p, uint16_t v)
{
    p[0] = v;
    p[1] = v >> 8;
}

/* 8 bit register n: al cl dl bl ah ch dh bh */
static uint8_t get_r8(const Vm *vm, int n)
{
    return n < 4 ? vm->r[n] & 0xFF : vm->r[n - 4] >> 8;
}

static void set_r8(Vm *vm, int n, uint8_t v)
{
    if (n < 4)
        vm->r[n] = (vm->r[n] & 0xFF00) | v;
    else
        vm->r[n - 4] = (vm->r[n - 4] & 0x00FF) | (v << 8);
}

/* The r/m operand, read and written. */
static uint32_t get_rm(Vm *vm, const Insn *in, int wide, uint16_t ip)
{
    if (in->mod == 3)
        return wide ? vm->r[in->rm] : get_r8(vm, in->rm);
    {
        unsigned char *p = mem(vm, in, base_reg(vm, in), wide ? 2 : 1, ip);

        return wide ? get16(p) : p[0];
    }
}

static void set_rm(Vm *vm, const Insn *in, int wide, uint32_t v, uint16_t ip)
{
    if (in->mod == 3) {
        if (wide)
            vm->r[in->rm] = v;
        else
            set_r8(vm, in->rm, v);
        return;
    }
    {
        unsigned char *p = mem(vm, in, base_reg(vm, in), wide ? 2 : 1, ip);

        if (wide)
            put16(p, v);
        else
            p[0] = v;
    }
}

/* ---- arithmetic ---- */

static void set_szp(Vm *vm, uint32_t r, int wide)
{
    uint32_t mask = wide ? 0xFFFF : 0xFF;
    uint8_t b = r & 0xFF;

    r &= mask;
    vm->zf = r == 0;
    vm->sf = (r >> (wide ? 15 : 7)) & 1;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    vm->pf = !(b & 1);
}

/* op: 0 add 1 or 2 adc 3 sbb 4 and 5 sub 6 xor 7 cmp.  Returns the result. */
static uint32_t alu(Vm *vm, int op, uint32_t a, uint32_t b, int wide)
{
    uint32_t mask = wide ? 0xFFFF : 0xFF;
    uint32_t top = wide ? 0x8000 : 0x80;
    uint32_t r;

    a &= mask;
    b &= mask;
    switch (op) {
    case 0: case 2:
        r = a + b + (op == 2 ? vm->cf : 0);
        vm->cf = r > mask;
        vm->of = (~(a ^ b) & (a ^ r) & top) != 0;
        break;
    case 3: case 5: case 7:
        r = a - b - (op == 3 ? vm->cf : 0);
        vm->cf = a < b + (op == 3 ? vm->cf : 0);
        vm->of = ((a ^ b) & (a ^ r) & top) != 0;
        break;
    case 1:
        r = a | b; vm->cf = vm->of = 0;
        break;
    case 4:
        r = a & b; vm->cf = vm->of = 0;
        break;
    default:
        r = a ^ b; vm->cf = vm->of = 0;
        break;
    }
    r &= mask;
    set_szp(vm, r, wide);
    return r;
}

static int cond(const Vm *vm, int cc)
{
    int r;

    switch (cc >> 1) {
    case 0: r = vm->of; break;
    case 1: r = vm->cf; break;
    case 2: r = vm->zf; break;
    case 3: r = vm->cf || vm->zf; break;
    case 4: r = vm->sf; break;
    case 5: r = vm->pf; break;
    case 6: r = vm->sf != vm->of; break;
    default: r = vm->zf || vm->sf != vm->of; break;
    }
    return (cc & 1) ? !r : r;
}

static void push(Vm *vm, uint16_t v, uint16_t ip)
{
    if (vm->r[R_SP] < 2)
        vm_fail(vm, "stack overflow", ip);
    vm->r[R_SP] -= 2;
    vm->stk[vm->r[R_SP] / 2] = v;
}

static uint16_t pop(Vm *vm, uint16_t ip)
{
    uint16_t v;

    if (vm->r[R_SP] >= STACK_WORDS * 2)
        vm_fail(vm, "stack underflow", ip);
    v = vm->stk[vm->r[R_SP] / 2];
    vm->r[R_SP] += 2;
    return v;
}

/* ---- execution ---- */

int rdf_trace;                          /* set to print every instruction (debugging) */

static void step(Vm *vm, int *done)
{
    Insn in;
    uint16_t ip = vm->ip;
    unsigned char op;
    int wide;
    uint32_t a, b, r;

    if (!decode(vm, ip, &in))
        vm_fail(vm, "unsupported instruction", ip);
    op = in.op;
    vm->ip = ip + in.len;
    if (rdf_trace)
        fprintf(stderr, "%04X op %02X  ax %04X bx %04X cx %04X dx %04X si %04X sp %04X zf %d cf %d sf %d\n", ip, op,
                vm->r[R_AX], vm->r[R_BX], vm->r[R_CX], vm->r[R_DX], vm->r[R_SI], vm->r[R_SP], vm->zf, vm->cf, vm->sf);

    if (op < 0x40 && (op & 7) < 6) {
        int aop = op >> 3;

        wide = op & 1;
        switch (op & 7) {
        case 0: case 1:                 /* E, G */
            a = get_rm(vm, &in, wide, ip);
            b = wide ? vm->r[in.reg] : get_r8(vm, in.reg);
            r = alu(vm, aop, a, b, wide);
            if (aop != 7)
                set_rm(vm, &in, wide, r, ip);
            break;
        case 2: case 3:                 /* G, E */
            a = wide ? vm->r[in.reg] : get_r8(vm, in.reg);
            b = get_rm(vm, &in, wide, ip);
            r = alu(vm, aop, a, b, wide);
            if (aop != 7) {
                if (wide)
                    vm->r[in.reg] = r;
                else
                    set_r8(vm, in.reg, r);
            }
            break;
        case 4:
            r = alu(vm, aop, get_r8(vm, 0), in.imm, 0);
            if (aop != 7)
                set_r8(vm, 0, r);
            break;
        default:
            r = alu(vm, aop, vm->r[R_AX], in.imm, 1);
            if (aop != 7)
                vm->r[R_AX] = r;
            break;
        }
        return;
    }
    if (op >= 0x40 && op <= 0x4F) {         /* inc/dec r16 */
        unsigned cf = vm->cf;
        int n = op & 7;

        vm->r[n] = alu(vm, op < 0x48 ? 0 : 5, vm->r[n], 1, 1);
        vm->cf = cf;
        return;
    }
    if (op >= 0x50 && op <= 0x57) {
        push(vm, vm->r[op & 7], ip);
        return;
    }
    if (op >= 0x58 && op <= 0x5F) {
        vm->r[op & 7] = pop(vm, ip);
        return;
    }
    if (op >= 0x70 && op <= 0x7F) {
        if (cond(vm, op & 15))
            vm->ip += (int8_t)in.imm;
        return;
    }
    if (op >= 0xB0 && op <= 0xB7) {
        set_r8(vm, op & 7, in.imm);
        return;
    }
    if (op >= 0xB8 && op <= 0xBF) {
        vm->r[op & 7] = in.imm;
        return;
    }
    switch (op) {
    case 0x68: case 0x6A:
        push(vm, (uint16_t)in.imm, ip);
        break;
    case 0x80: case 0x81: case 0x83:
        wide = op != 0x80;
        a = get_rm(vm, &in, wide, ip);
        r = alu(vm, in.reg, a, in.imm, wide);
        if (in.reg != 7)
            set_rm(vm, &in, wide, r, ip);
        break;
    case 0x84: case 0x85:
        wide = op & 1;
        alu(vm, 4, get_rm(vm, &in, wide, ip), wide ? vm->r[in.reg] : get_r8(vm, in.reg), wide);
        break;
    case 0x88: case 0x89:
        wide = op & 1;
        set_rm(vm, &in, wide, wide ? vm->r[in.reg] : get_r8(vm, in.reg), ip);
        break;
    case 0x8A: case 0x8B:
        wide = op & 1;
        r = get_rm(vm, &in, wide, ip);
        if (wide)
            vm->r[in.reg] = r;
        else
            set_r8(vm, in.reg, r);
        break;
    case 0x8D:
        vm->r[in.reg] = base_reg(vm, &in);
        break;
    case 0x90:
        break;
    case 0xA0: case 0xA1: {
        unsigned char *p = mem(vm, &in, in.imm, op & 1 ? 2 : 1, ip);

        if (op & 1)
            vm->r[R_AX] = get16(p);
        else
            set_r8(vm, 0, p[0]);
        break;
    }
    case 0xA2: case 0xA3: {
        unsigned char *p = mem(vm, &in, in.imm, op & 1 ? 2 : 1, ip);

        if (op & 1)
            put16(p, vm->r[R_AX]);
        else
            p[0] = vm->r[R_AX];
        break;
    }
    case 0xA8:
        alu(vm, 4, get_r8(vm, 0), in.imm, 0);
        break;
    case 0xA9:
        alu(vm, 4, vm->r[R_AX], in.imm, 1);
        break;
    case 0xC3:
        vm->ip = pop(vm, ip);
        break;
    case 0xCB:
        vm->ip = pop(vm, ip);
        if (vm->ip == SENTINEL && pop(vm, ip) == SENTINEL)
            *done = 1;
        else
            vm_fail(vm, "far return to somewhere else", ip);
        break;
    case 0xC6: case 0xC7:
        wide = op & 1;
        set_rm(vm, &in, wide, in.imm, ip);
        break;
    case 0xC0: case 0xC1: case 0xD0: case 0xD1: case 0xD2: case 0xD3: {
        int count = op >= 0xD2 ? (vm->r[R_CX] & 0x1F) : op >= 0xD0 ? 1 : in.imm & 0x1F;

        wide = op & 1;
        if (count == 0)
            break;
        a = get_rm(vm, &in, wide, ip);
        for (; count; count--) {
            uint32_t top = wide ? 0x8000 : 0x80;

            switch (in.reg) {
            case 4: case 6:
                vm->cf = (a & top) != 0;
                a = (a << 1) & (wide ? 0xFFFF : 0xFF);
                break;
            case 5:
                vm->cf = a & 1;
                a >>= 1;
                break;
            case 7:
                vm->cf = a & 1;
                a = (a >> 1) | (a & top);
                break;
            default:
                vm_fail(vm, "unsupported shift", ip);
            }
        }
        set_szp(vm, a, wide);
        set_rm(vm, &in, wide, a, ip);
        break;
    }
    case 0xE8:
        push(vm, vm->ip, ip);
        vm->ip += (int16_t)in.imm;
        break;
    case 0xE9:
        vm->ip += (int16_t)in.imm;
        break;
    case 0xEB:
        vm->ip += (int8_t)in.imm;
        break;
    case 0xF6: case 0xF7:
        wide = op & 1;
        alu(vm, 4, get_rm(vm, &in, wide, ip), in.imm, wide);
        break;
    case 0xFE: case 0xFF: {
        unsigned cf = vm->cf;

        wide = op & 1;
        switch (in.reg) {
        case 0: case 1:
            r = alu(vm, in.reg == 0 ? 0 : 5, get_rm(vm, &in, wide, ip), 1, wide);
            vm->cf = cf;
            set_rm(vm, &in, wide, r, ip);
            break;
        case 3:
            if (!wide || in.mod == 3)
                vm_fail(vm, "bad far call", ip);
            {
                unsigned char *p = mem(vm, &in, base_reg(vm, &in), 4, ip);

                if (!in.cs || base_reg(vm, &in) != 0)
                    vm_fail(vm, "far call that isn't the game call", ip);
                (void)p;
                r = vm->service((int16_t *)&vm->stk[vm->r[R_SP] / 2]);
                vm->r[R_AX] = r & 0xFFFF;
                vm->r[R_DX] = r >> 16;
            }
            break;
        case 4:                                 /* switch: jmp cs:[si+table] */
            vm->ip = get_rm(vm, &in, 1, ip);
            break;
        case 6:
            push(vm, get_rm(vm, &in, 1, ip), ip);
            break;
        default:
            vm_fail(vm, "unsupported group 5 instruction", ip);
        }
        break;
    }
    default:
        vm_fail(vm, "unsupported instruction", ip);
    }
}

int RdfRun(unsigned char *image, uint32_t size, uint16_t entry, void *room, uint16_t room_size,
           RdfService service)
{
    Vm vm;
    int done = 0;
    long steps = 0;

    memset(&vm, 0, sizeof(vm));
    memset(vm.stk, 0xA5, sizeof(vm.stk));        /* stale stack words read the same every time */
    vm.img = image;
    vm.img_size = size;
    vm.room = room;
    vm.room_size = room_size;
    vm.service = service;
    if (setjmp(vm.fail))
        return 1;
    vm.r[R_SI] = vm.r[R_BX] = RDF_ROOM_BASE;
    vm.r[R_SP] = STACK_WORDS * 2;
    push(&vm, SENTINEL, entry);         /* the caller's segment... */
    push(&vm, SENTINEL, entry);         /* ...and offset */
    vm.ip = entry;
    while (!done) {
        if (++steps > MAX_STEPS)
            vm_fail(&vm, "runaway script", vm.ip);
        step(&vm, &done);
    }
    return 0;
}

/* Static check: follow every path from entry. */
int RdfVerify(unsigned char *image, uint32_t size, uint16_t entry)
{
    Vm vm;
    unsigned char *seen = calloc(size + 1, 1);
    uint16_t *work = malloc(sizeof(uint16_t) * (size + 1));
    int nwork = 0, bad = 0;

    memset(&vm, 0, sizeof(vm));
    vm.img = image;
    vm.img_size = size;
    work[nwork++] = entry;
    while (nwork && !bad) {
        uint16_t ip = work[--nwork];

        while (ip < size && !seen[ip]) {
            Insn in;
            unsigned char op;

            seen[ip] = 1;
            if (!decode(&vm, ip, &in)) {
                bad = ip ? ip : 1;
                break;
            }
            op = in.op;
            ip += in.len;
            if (op >= 0x70 && op <= 0x7F)
                work[nwork++] = ip + (int8_t)in.imm;
            else if (op == 0xE8)
                work[nwork++] = ip + (int16_t)in.imm;
            else if (op == 0xE9)
                ip += (int16_t)in.imm;
            else if (op == 0xEB)
                ip += (int8_t)in.imm;
            else if (op == 0xC3 || op == 0xCB || (op == 0xFF && in.reg == 4))
                break;
        }
    }
    free(seen);
    free(work);
    return bad;
}
