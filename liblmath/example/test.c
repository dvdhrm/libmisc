#include <stdio.h>
#include <stdlib.h>
#include "include/liblmath.h"

int main()
{
	lm_m4 m, i, d;

	lm_m4_identity(m);
	m[0][3] = 1;
	m[1][3] = 1.5;
	m[2][3] = -5;
	m[0][1] = 2;
	m[1][1] = 0.44;
	m[2][0] = -5.12;

	lm_m4_invert_dest(i, m);
	lm_m4_mult(d, m, i);

	lm_m4_print("", m);
	lm_m4_print("", i);
	lm_m4_print("", d);

	return 0;
}
