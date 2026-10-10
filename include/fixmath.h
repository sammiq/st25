/* fixmath.h - fixed point trigonometry, vectors and matrices
 *
 * Fixed point values use 14 fraction bits (0x4000 = 1.0). Angles are
 * 0x400 units per full turn.
 */

#ifndef FIXMATH_H
#define FIXMATH_H

#include "compat.h"

#define FIX_ONE 0x4000

typedef struct Vec {
    int16_t e[3];
} Vec;

typedef struct Matrix {
    int16_t e[3][3];
} Matrix;

extern int16_t sin_table[0x101];    /* sin over a quarter turn */
extern int16_t cos_table[0x101];
extern int16_t tan_table[0x81];     /* tan over an eighth of a turn */
extern int16_t sin_small[33];
extern int16_t cos_small[33];
extern int32_t ang_small[33];

/* seg027 (assembly) */
void MatMul(Matrix *a, Matrix *b, Matrix *out);
int16_t FixDiv(int32_t a, int32_t b);                 /* (a << 14) / b */
int16_t FixMul(int16_t a, int16_t b);                   /* (a * b) >> 14 */
int32_t FixMul2(int32_t a, int16_t b, int32_t c, int16_t d); /* (a * b + c * d) >> 14 */
int16_t FixSqrt(int32_t x);
int16_t FixLength(int16_t x, int16_t y, int16_t z);

void InitTrig(void);
int16_t FixSin(int16_t a);
int16_t FixCos(int16_t a);
int16_t FixTan(int16_t a);
int16_t FixACos(int16_t v);
int16_t FixAtan2(int32_t a, int32_t b);
int16_t FixASin(int16_t v);
void MatIdentity(Matrix *m);
Matrix *MatRotX(int16_t a, Matrix *out);
Matrix *MatRotY(int16_t a, Matrix *out);
Matrix *MatRotZ(int16_t a, Matrix *out);
Matrix *MatRotXPre(int16_t a, Matrix *out);
Matrix *MatRotYPre(int16_t a, Matrix *out);
Matrix *MatRotZPre(int16_t a, Matrix *out);
void MatTranspose(Matrix *src, Matrix *dst);
int16_t VecCross(int16_t *a, int16_t *b, int16_t *out);
int16_t VecLen32(int32_t *v);
int16_t VecNorm32(int32_t *v, int16_t *out);
int16_t VecLen(int16_t *v);
int16_t VecNorm(int16_t *v, int16_t *out);
int16_t MatFromVecs(int32_t *dir, int16_t *up, int16_t *alt, Matrix *m);
void MatFromDir(int16_t *v, Matrix *m);
Matrix *MatRotXSC(int16_t s, int16_t c, Matrix *out);
Matrix *MatRotYSC(int16_t s, int16_t c, Matrix *out);
Matrix *MatRotZSC(int16_t s, int16_t c, Matrix *out);
void MatOrtho(Matrix *m);
void VecSub(int16_t *a, int16_t *b);

#endif
