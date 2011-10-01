/*
 * Linear Math
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "liblmath.h"

lm_float lm_v3_length(const lm_v3 src)
{
	return sqrt(lm_v3_length2(src));
}

lm_float lm_v4_length(const lm_v4 src)
{
	return sqrt(lm_v4_length2(src));
}
