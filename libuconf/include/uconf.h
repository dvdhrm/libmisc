/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * Helper functions/macros for uconf applications. This is private to uconf and
 * should not be installed system-wide nor used by other applications than
 * uconf.
 */

#ifndef UCONF_UCONF_H
#define UCONF_UCONF_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libcstr.h>

#include "libuconf.h"

/*
 * (func) should be an expression that returns 0 on success, otherwise errno is
 * set.
 * This reruns (func) if it fails with EINTR until it returns an other error
 * code. If the function succeeded, errno is set to 0, otherwise errno is left
 * unchanged.
 */
#define UCONF_NO_EINTR(func) \
	do { \
		if (!(func)) { \
			errno = 0; \
			break; \
		} \
	} while (errno == EINTR)

/* argument that is passed to yyparse() */
struct yyarg {
	cstr *path;
	size_t depth;
	void *scanner;
	int err;
	struct uconf_entry root;
	struct uconf_entry *curr;
	FILE *fout;
};

extern int parse_file(struct uconf_file *file, struct yyarg *arg);

#endif /* UCONF_UCONF_H */
