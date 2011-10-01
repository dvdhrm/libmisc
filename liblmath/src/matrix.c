/*
 * Linear Math
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <errno.h>
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

void lm_stack_init(struct lm_stack *stack)
{
	lm_m4_identity(stack->tip);
	stack->stack = NULL;
	stack->cache = NULL;
}

void lm_stack_destroy(struct lm_stack *stack)
{
	struct lm_stack_entry *t;

	while (stack->stack) {
		t = stack->stack;
		stack->stack = t->next;
		free(t);
	}

	while (stack->cache) {
		t = stack->cache;
		stack->cache = t->next;
		free(t);
	}
}

int lm_stack_push(struct lm_stack *stack)
{
	struct lm_stack_entry *new;

	new = stack->cache;
	if (new) {
		stack->cache = new->next;
	} else {
		new = malloc(sizeof(*new));
		if (!new)
			return -ENOMEM;
	}

	lm_m4_copy(new->matrix, stack->tip);
	new->next = stack->stack;
	stack->stack = new;

	return 0;
}

void lm_stack_pop(struct lm_stack *stack)
{
	struct lm_stack_entry *old;

	assert(stack->stack);

	old = stack->stack;
	stack->stack = old->next;

	old->next = stack->cache;
	stack->cache = old;

	lm_m4_copy(stack->tip, old->matrix);
}
