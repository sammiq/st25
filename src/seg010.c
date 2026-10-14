/* seg010: fixed point trigonometry, vectors and 3x3 matrices */

#include "compat.h"
#include <math.h>
#include "mem.h"
#include "fixmath.h"

#define PI 3.141592654

int16_t sin_table[0x101];
int16_t cos_table[0x101];
int16_t tan_table[0x81];
int16_t sin_small[33];
int16_t cos_small[33];
int32_t ang_small[33];

void InitTrig(void)
{
    int16_t i;
    double x;

    for (i = 0, x = 0.0; i != 0x101; ++i, x += PI / 512) {
        sin_table[i] = sin(x) * 16384.0;
        cos_table[i] = cos(x) * 16384.0;
    }
    for (i = 0, x = 0.0; i != 0x81; ++i, x += PI / 512)
        tan_table[i] = tan(x) * 16384.0;
    for (i = 0; i <= 32; i++) {
        x = i * 16.0 / 0x4000;
        sin_small[i] = sin(x) * 16384.0;
        cos_small[i] = cos(x) * 16384.0;
        ang_small[i] = x * 512 / PI * 65536.0;
    }
}

int16_t FixSin(int16_t a)
{
    int16_t r;

    if (a < 0) {
        a %= 0x400;
        a += 0x400;
    } else
        a &= 0x3FF;
    switch ((a >> 8) & 3) {
    case 0:
        r = sin_table[a];
        break;
    case 1:
        r = sin_table[0x100 - (a & 0xFF)];
        break;
    case 2:
        r = -sin_table[a & 0xFF];
        break;
    case 3:
        r = -sin_table[0x100 - (a & 0xFF)];
        break;
    }
    return r;
}

int16_t FixCos(int16_t a)
{
    int16_t r;

    a += 0x100;
    if (a < 0) {
        a %= 0x400;
        a += 0x400;
    } else
        a &= 0x3FF;
    switch ((a >> 8) & 3) {
    case 0:
        r = sin_table[a];
        break;
    case 1:
        r = sin_table[0x100 - (a & 0xFF)];
        break;
    case 2:
        r = -sin_table[a & 0xFF];
        break;
    case 3:
        r = -sin_table[0x100 - (a & 0xFF)];
        break;
    }
    return r;
}

int16_t FixTan(int16_t a)
{
    return tan_table[a];
}

int16_t FixACos(int16_t v)
{
    int16_t lo;
    int16_t mid;
    int16_t hi;
    int16_t sign;

    sign = 1;
    if (v < 0) {
        sign = -1;
        v = -v;
    }
    lo = 0;
    hi = 0x100;
    while (lo <= hi) {
        mid = (lo + hi) >> 1;
        if (cos_table[mid] > v)
            lo = mid + 1;
        else if (cos_table[mid] < v)
            hi = mid - 1;
        else
            break;
    }
    if (sign < 0)
        return 0x200 - mid;
    else
        return mid;
}

/* Angle of the vector (a, b). */
int16_t FixAtan2(int32_t a, int32_t b)
{
    int16_t lo;
    int16_t mid;
    int16_t hi;
    int16_t sb;
    int16_t sa;
    int16_t t;

    if (b == 0 && a == 0)
        return 0;
    sb = b > 0 ? 0 : -1;
    sa = a > 0 ? 0 : -1;
    b = b > 0 ? b : -b;
    a = a > 0 ? a : -a;
    if (b > a)
        t = FixDiv(a, b);
    else
        t = FixDiv(b, a);
    lo = 0;
    hi = 0x80;
    while (lo <= hi) {
        mid = (lo + hi) >> 1;
        if (tan_table[mid] > t)
            hi = mid - 1;
        else if (tan_table[mid] < t)
            lo = mid + 1;
        else
            break;
    }
    if (b > a) {
        if (sb == 0 && sa == 0)
            mid = mid;
        else if (sb == 0 && sa == -1)
            mid = -mid;
        else if (sb == -1 && sa == 0)
            mid = 0x200 - mid;
        else
            mid = mid - 0x200;
    } else {
        if (sb == 0 && sa == 0)
            mid = 0x100 - mid;
        else if (sb == -1 && sa == 0)
            mid = mid + 0x100;
        else if (sb == -1 && sa == -1)
            mid = -0x100 - mid;
        else
            mid = mid - 0x100;
    }
    return mid;
}

int16_t FixASin(int16_t v)
{
    int16_t lo;
    int16_t mid;
    int16_t hi;
    int16_t sign;

    sign = 1;
    if (v < 0) {
        sign = -1;
        v = -v;
    }
    lo = 0;
    hi = 0x100;
    while (lo <= hi) {
        mid = (lo + hi) >> 1;
        if (sin_table[mid] > v)
            hi = mid - 1;
        else if (sin_table[mid] < v)
            lo = mid + 1;
        else
            break;
    }
    if (sign < 0)
        return 0 - mid;
    else
        return mid;
}

void MatIdentity(Matrix *m)
{
    int16_t i;
    int16_t j;

    MemSet(m, sizeof(Matrix), 0);
    for (i = 0; i != 3; i++)
        m->e[i][i] = FIX_ONE;
}

Matrix *MatRotX(int16_t a, Matrix *out)
{
    int32_t s;
    int32_t c;
    Matrix m;
    Matrix r;

    s = FixSin(a);
    c = FixCos(a);
    MatIdentity(&m);
    m.e[1][1] = c;
    m.e[1][2] = s;
    m.e[2][1] = -s;
    m.e[2][2] = c;
    MatMul(out, &m, &r);
    *out = r;
    return out;
}

Matrix *MatRotY(int16_t a, Matrix *out)
{
    int32_t s;
    int32_t c;
    Matrix m;
    Matrix r;

    s = FixSin(a);
    c = FixCos(a);
    MatIdentity(&m);
    m.e[0][0] = c;
    m.e[0][2] = -s;
    m.e[2][0] = s;
    m.e[2][2] = c;
    MatMul(out, &m, &r);
    *out = r;
    return out;
}

Matrix *MatRotXPre(int16_t a, Matrix *out)
{
    int32_t s;
    int32_t c;
    Matrix m;
    Matrix r;

    s = FixSin(a);
    c = FixCos(a);
    MatIdentity(&m);
    m.e[1][1] = c;
    m.e[1][2] = s;
    m.e[2][1] = -s;
    m.e[2][2] = c;
    MatMul(&m, out, &r);
    *out = r;
    return out;
}

Matrix *MatRotYPre(int16_t a, Matrix *out)
{
    int32_t s;
    int32_t c;
    Matrix m;
    Matrix r;

    s = FixSin(a);
    c = FixCos(a);
    MatIdentity(&m);
    m.e[0][0] = c;
    m.e[0][2] = -s;
    m.e[2][0] = s;
    m.e[2][2] = c;
    MatMul(&m, out, &r);
    *out = r;
    return out;
}

void MatTranspose(Matrix *src, Matrix *dst)
{
    int16_t i;
    int16_t j;
    int16_t t;

    for (i = 0; i != 3; i++)
        for (j = 0; j != 3; j++) {
            t = src->e[j][i];
            dst->e[i][j] = t;
        }
}

/* out = a x b; returns 1 if the result is zero. */
int16_t VecCross(register int16_t *a, register int16_t *b, int16_t *out)
{
    out[0] = FixMul2(a[1], b[2], a[2], -b[1]);
    out[1] = FixMul2(a[2], b[0], a[0], -b[2]);
    out[2] = FixMul2(a[0], b[1], a[1], -b[0]);
    return out[0] == 0 && out[1] == 0 && out[2] == 0;
}

int16_t VecLen32(register int32_t *v)
{
    return FixLength(v[0], v[1], v[2]);
}

int16_t VecNorm32(register int32_t *v, register int16_t *out)
{
    uint16_t len;

    len = VecLen32(v);
    if (len != 0) {
        out[0] = FixDiv(v[0], len);
        out[1] = FixDiv(v[1], len);
        out[2] = FixDiv(v[2], len);
        return 0;
    } else {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        return 1;
    }
}

int16_t VecLen(register int16_t *v)
{
    return FixLength(v[0], v[1], v[2]);
}

int16_t VecNorm(register int16_t *v, register int16_t *out)
{
    uint16_t len;

    len = VecLen(v);
    if (len != 0) {
        out[0] = FixDiv(v[0], len);
        out[1] = FixDiv(v[1], len);
        out[2] = FixDiv(v[2], len);
        return 0;
    } else {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        return 1;
    }
}

/* Build an orthonormal basis looking along dir, with up (or alt if up is
 * parallel to dir). */
int16_t MatFromVecs(int32_t *dir, int16_t *up, int16_t *alt, Matrix *m)
{
    if (VecNorm32(dir, m->e[2]))
        return 0;
    if (VecCross(up, m->e[2], m->e[0])) {
        if (alt) {
            if (VecCross(alt, m->e[2], m->e[0]))
                return 0;
            else
                return 0;
        }
    }
    VecNorm(m->e[0], m->e[0]);
    VecCross(m->e[2], m->e[0], m->e[1]);
    return 1;
}

Matrix *MatRotXSC(int16_t s, int16_t c, Matrix *out)
{
    Matrix m;
    Matrix r;

    MatIdentity(&m);
    m.e[1][1] = c;
    m.e[1][2] = s;
    m.e[2][1] = -s;
    m.e[2][2] = c;
    MatMul(&m, out, &r);
    *out = r;
    return out;
}

Matrix *MatRotYSC(int16_t s, int16_t c, Matrix *out)
{
    Matrix m;
    Matrix r;

    MatIdentity(&m);
    m.e[0][0] = c;
    m.e[0][2] = -s;
    m.e[2][0] = s;
    m.e[2][2] = c;
    MatMul(&m, out, &r);
    *out = r;
    return out;
}

Matrix *MatRotZSC(int16_t s, int16_t c, Matrix *out)
{
    Matrix m;
    Matrix r;

    MatIdentity(&m);
    m.e[0][0] = c;
    m.e[0][1] = s;
    m.e[1][0] = -s;
    m.e[1][1] = c;
    MatMul(&m, out, &r);
    *out = r;
    return out;
}

/* Re-orthonormalise a rotation matrix. */
void MatOrtho(Matrix *m)
{
    Vec v;

    VecCross(m->e[1], m->e[2], m->e[0]);
    VecCross(m->e[2], m->e[0], m->e[1]);
    VecNorm(m->e[0], v.e);
    *(Vec *)m->e[0] = v;
    VecNorm(m->e[1], v.e);
    *(Vec *)m->e[1] = v;
    VecNorm(m->e[2], v.e);
    *(Vec *)m->e[2] = v;
}

void VecSub(int16_t *a, int16_t *b)
{
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
}
