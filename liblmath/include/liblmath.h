/*
 * Linear Math Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#ifndef LM_LIBLMATH_H
#define LM_LIBLMATH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Basic floating point type
 * lm_float is used as generic floating point type all over the library. If we
 * want to convert the library to use double prevision, we only need to change
 * this type.
 * We use single-precision to be compatible with OpenGL.
 */

typedef float lm_float;

/*
 * Vectors
 * 3 and 4 dimensional vectors are supported. Most functions are pretty simple
 * and inlined. For the source see at the end of the file.
 * All vectors are binary compatible. That is, a V4 can be used if a V3 is
 * requested. However, the fourth dimension is invalid then.
 * LM_V3 and LM_V4 can be used to create vectors as lvalues.
 */

typedef lm_float lm_v3[3];
typedef lm_float lm_v4[4];

#define LM_V3(x, y, z) ((lm_v3) { (x), (y), (z) })
#define LM_V4(x, y, z, w) ((lm_v4) { (x), (y), (z), (w) })

#define LM_V3_NEG(v3) LM_V3(-(v3)[0], -(v3)[1], -(v3)[2])
#define LM_V4_NEG(v4) LM_V4(-(v4)[0], -(v4)[1], -(v4)[2], -(v4)[3])

#define LM_V3_ZERO LM_V3(0, 0, 0)
#define LM_V4_ZERO LM_V4(0, 0, 0, 0)

#define LM_V3TO4(v3, d4) LM_V4((v3)[0], (v3)[1], (v3)[2], (d4))
#define LM_V4TO3(v4) LM_V3((v4)[0], (v4)[1], (v4)[2])

static inline void lm_v3_copy(lm_v3 dest, const lm_v3 src);
static inline void lm_v3_add(lm_v3 dest, const lm_v3 addend);
static inline void lm_v3_sub(lm_v3 minuend, const lm_v3 subtrahend);
static inline void lm_v3_mult(lm_v3 dest, lm_float factor);
static inline bool lm_v3_cmp(const lm_v3 a, const lm_v3 b);
static inline lm_float lm_v3_dot(const lm_v3 a, const lm_v3 b);
static inline void lm_v3_cross(lm_v3 dest, const lm_v3 src);
static inline void lm_v3_cross_dest(lm_v3 dest, const lm_v3 a, const lm_v3 b);
extern lm_float lm_v3_length(const lm_v3 src);
static inline lm_float lm_v3_length2(const lm_v3 src);
static inline void lm_v3_norm(lm_v3 dest);
static inline void lm_v3_norm_dest(lm_v3 dest, const lm_v3 src);

static inline void lm_v4_copy(lm_v4 dest, const lm_v4 src);
static inline void lm_v4_add(lm_v4 dest, const lm_v4 addend);
static inline void lm_v4_sub(lm_v4 minuend, const lm_v4 subtrahend);
static inline void lm_v4_mult(lm_v4 dest, lm_float factor);
static inline bool lm_v4_cmp(const lm_v4 a, const lm_v4 b);
static inline lm_float lm_v4_dot(const lm_v4 a, const lm_v4 b);
extern lm_float lm_v4_length(const lm_v4 src);
static inline lm_float lm_v4_length2(const lm_v4 src);
static inline void lm_v4_norm(lm_v4 dest);
static inline void lm_v4_norm_dest(lm_v4 dest, const lm_v4 src);

/*
 * Matrices
 * 3 and 4 dimensional square matrices are supported. The memory layout is
 * row-major as common with many programming languages. The matrices are not
 * binary compatible so you cant use M4 where M3 is requested unless you know
 * what you're doing.
 * You may use all functions with column-major matrices, too. All functions work
 * the same on any matrix layout.
 */

typedef lm_float lm_m3[3][3];
typedef lm_float lm_m4[4][4];

static inline void lm_m3_copy(lm_m3 dest, lm_m3 src);
static inline void lm_m3_identity(lm_m3 dest);
static inline void lm_m3_transpose(lm_m3 dest);
static inline void lm_m3_transpose_dest(lm_m3 dest, lm_m3 src);

extern void lm_m4_print(const char *prefix, lm_m4 src);
static inline void lm_m4_copy(lm_m4 dest, lm_m4 src);
static inline void lm_m4_identity(lm_m4 dest);
static inline void lm_m4_transpose(lm_m4 dest);
static inline void lm_m4_transpose_dest(lm_m4 dest, lm_m4 src);
static inline void lm_m4_translate(lm_m4 dest, const lm_v3 src);
static inline void lm_m4_rotate(lm_m4 dest, lm_float angle, const lm_v3 axis);
static inline void lm_m4_mult(lm_m4 dest, lm_m4 le, lm_m4 ri);
static inline void lm_m4_mult_pre(lm_m4 dest, lm_m4 pre);
static inline void lm_m4_mult_post(lm_m4 dest, lm_m4 post);
static inline bool lm_m4_invert(lm_m4 dest);
extern bool lm_m4_invert_dest(lm_m4 dest, lm_m4 src);

/*
 * Below the source of most simple functions.
 * They are inlined to allow fast optimizations. Most of them are pretty simple
 * and will speed up a lot by inlining them.
 */

static inline void lm_v3_copy(lm_v3 dest, const lm_v3 src)
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

static inline void lm_v3_add(lm_v3 dest, const lm_v3 addend)
{
	dest[0] += addend[0];
	dest[1] += addend[1];
	dest[2] += addend[2];
}

static inline void lm_v3_sub(lm_v3 minuend, const lm_v3 subtrahend)
{
	lm_v3_add(minuend, LM_V3_NEG(subtrahend));
}

static inline void lm_v3_mult(lm_v3 dest, lm_float factor)
{
	dest[0] *= factor;
	dest[1] *= factor;
	dest[2] *= factor;
}

static inline bool lm_v3_cmp(const lm_v3 a, const lm_v3 b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

static inline lm_float lm_v3_dot(const lm_v3 a, const lm_v3 b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline void lm_v3_cross(lm_v3 dest, const lm_v3 src)
{
	lm_v3 tmp;

	lm_v3_cross_dest(tmp, dest, src);
	lm_v3_copy(dest, tmp);
}

static inline void lm_v3_cross_dest(lm_v3 dest, const lm_v3 a, const lm_v3 b)
{
	dest[0] = a[1] * b[2] - a[2] * b[1];
	dest[1] = a[2] * b[0] - a[0] * b[2];
	dest[2] = a[0] * b[1] - a[1] * b[0];
}

static inline lm_float lm_v3_length2(const lm_v3 src)
{
	return lm_v3_dot(src, src);
}

static inline void lm_v3_norm(lm_v3 dest)
{
	lm_v3_mult(dest, 1.0 / lm_v3_length(dest));
}

static inline void lm_v3_norm_dest(lm_v3 dest, const lm_v3 src)
{
	lm_v3_copy(dest, src);
	lm_v3_mult(dest, 1.0 / lm_v3_length(src));
}

static inline void lm_v4_copy(lm_v4 dest, const lm_v4 src)
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
}

static inline void lm_v4_add(lm_v4 dest, const lm_v4 addend)
{
	dest[0] += addend[0];
	dest[1] += addend[1];
	dest[2] += addend[2];
	dest[3] += addend[3];
}

static inline void lm_v4_sub(lm_v4 minuend, const lm_v4 subtrahend)
{
	lm_v4_add(minuend, LM_V4_NEG(subtrahend));
}

static inline void lm_v4_mult(lm_v4 dest, lm_float factor)
{
	dest[0] *= factor;
	dest[1] *= factor;
	dest[2] *= factor;
	dest[3] *= factor;
}

static inline bool lm_v4_cmp(const lm_v4 a, const lm_v4 b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static inline lm_float lm_v4_dot(const lm_v4 a, const lm_v4 b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

static inline lm_float lm_v4_length2(const lm_v4 src)
{
	return lm_v4_dot(src, src);
}

static inline void lm_v4_norm(lm_v4 dest)
{
	lm_v4_mult(dest, 1.0 / lm_v4_length(dest));
}

static inline void lm_v4_norm_dest(lm_v4 dest, const lm_v4 src)
{
	lm_v4_copy(dest, src);
	lm_v4_mult(dest, 1.0 / lm_v4_length(src));
}

static inline void lm_m3_copy(lm_m3 dest, lm_m3 src)
{
	lm_v3_copy(dest[0], src[0]);
	lm_v3_copy(dest[1], src[1]);
	lm_v3_copy(dest[2], src[2]);
}

static inline void lm_m3_identity(lm_m3 dest)
{
	lm_v3_copy(dest[0], LM_V3(1, 0, 0));
	lm_v3_copy(dest[1], LM_V3(0, 1, 0));
	lm_v3_copy(dest[2], LM_V3(0, 0, 1));
}

static inline void lm_m3_transpose(lm_m3 dest)
{
	lm_m3 tmp;

	lm_m3_transpose_dest(tmp, (void*)dest);
	lm_m3_copy(dest, tmp);
}

static inline void lm_m3_transpose_dest(lm_m3 dest, lm_m3 src)
{
	lm_v3_copy(dest[0], LM_V3(src[0][0], src[1][0], src[2][0]));
	lm_v3_copy(dest[1], LM_V3(src[0][1], src[1][1], src[2][1]));
	lm_v3_copy(dest[2], LM_V3(src[0][2], src[1][2], src[2][2]));
}

static inline void lm_m4_copy(lm_m4 dest, lm_m4 src)
{
	lm_v4_copy(dest[0], src[0]);
	lm_v4_copy(dest[1], src[1]);
	lm_v4_copy(dest[2], src[2]);
	lm_v4_copy(dest[3], src[3]);
}

static inline void lm_m4_identity(lm_m4 dest)
{
	lm_v4_copy(dest[0], LM_V4(1, 0, 0, 0));
	lm_v4_copy(dest[1], LM_V4(0, 1, 0, 0));
	lm_v4_copy(dest[2], LM_V4(0, 0, 1, 0));
	lm_v4_copy(dest[3], LM_V4(0, 0, 0, 1));
}

static inline void lm_m4_transpose(lm_m4 dest)
{
	lm_m4 tmp;

	lm_m4_transpose_dest(tmp, dest);
	lm_m4_copy(dest, tmp);
}

static inline void lm_m4_transpose_dest(lm_m4 dest, lm_m4 src)
{
	lm_v4_copy(dest[0], LM_V4(src[0][0], src[1][0], src[2][0], src[3][0]));
	lm_v4_copy(dest[1], LM_V4(src[0][1], src[1][1], src[2][1], src[3][1]));
	lm_v4_copy(dest[2], LM_V4(src[0][2], src[1][2], src[2][2], src[3][2]));
	lm_v4_copy(dest[3], LM_V4(src[0][3], src[1][3], src[2][3], src[3][3]));
}

static inline void lm_m4_translate(lm_m4 dest, const lm_v3 src)
{
	dest[0][3] += src[0];
	dest[1][3] += src[1];
	dest[2][3] += src[2];
}

static inline void lm_m4_rotate(lm_m4 dest, lm_float angle, const lm_v3 axis)
{
	assert(0);
}

static inline void lm_m4_mult(lm_m4 dest, lm_m4 le, lm_m4 ri)
{
	size_t i, j;

	for (i = 0; i < 4; ++i) {
		lm_v4_copy(dest[i], LM_V4_ZERO);
		for (j = 0; j < 4; ++j) {
			dest[i][0] += le[i][j] * ri[j][0];
			dest[i][1] += le[i][j] * ri[j][1];
			dest[i][2] += le[i][j] * ri[j][2];
			dest[i][3] += le[i][j] * ri[j][3];
		}
	}
}

static inline void lm_m4_mult_pre(lm_m4 dest, lm_m4 pre)
{
	lm_m4 tmp;

	lm_m4_mult(tmp, pre, dest);
	lm_m4_copy(dest, tmp);
}

static inline void lm_m4_mult_post(lm_m4 dest, lm_m4 post)
{
	lm_m4 tmp;

	lm_m4_mult(tmp, dest, post);
	lm_m4_copy(dest, tmp);
}

static inline bool lm_m4_invert(lm_m4 dest)
{
	lm_m4 tmp;
	bool ret;

	ret = lm_m4_invert_dest(tmp, dest);
	lm_m4_copy(dest, tmp);
	return ret;
}

#endif /* LM_LIBLMATH_H */
