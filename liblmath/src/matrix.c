/*
 * Linear Math
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liblmath.h"

static inline void swapf(lm_float *a, lm_float *b)
{
	lm_float tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void lm_m4_print(const char *prefix, lm_m4 src)
{
	size_t i;

	for (i = 0; i < 4; ++i)
		printf("%s%+#12.4f %+#12.4f %+#12.4f %+#12.4f\n", prefix,
				src[i][0], src[i][1], src[i][2], src[i][3]);
	printf("\n");
}

bool lm_m4_invert_dest(lm_m4 dest, lm_m4 src)
{
	lm_m4 mat;
	size_t i, j, k, index;
	lm_float value;

	/* copy source so we can swap columns in it */
	lm_m4_copy(mat, src);
	lm_m4_identity(dest);

	for (i = 0; i < 4; ++i) {
		index = i;
		value = mat[i][i];

		for (j = i + 1; j < 4; ++j) {
			if (fabs(value) < fabs(mat[i][j])) {
				index = j;
				value = mat[i][j];
			}
		}

		/* return identity for singular matrices */
		if (fabs(value) <= FLT_EPSILON) {
			lm_m4_identity(dest);
			return false;
		}

		/* swap col to required pos */
		if (i != index) {
			for (j = 0; j < 4; ++j) {
				swapf(&dest[j][i], &dest[j][index]);
				swapf(&mat[j][i], &mat[j][index]);
			}
		}

		for (j = 0; j < 4; j++) {
			mat[j][i] *= 1.0 / value;
			dest[j][i] *= 1.0 / value;
		}

		for (j = 0; j < 4; j++) {
			if (j != i) {
				value = mat[i][j];
				for (k = 0; k < 4; ++k) {
					mat[k][j] -= mat[k][i] * value;
					dest[k][j] -= dest[k][i] * value;
				}
			}
		}
	}

	return true;
}
