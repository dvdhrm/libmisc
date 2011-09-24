/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * Parser
 * Implements the generic parser interface. The parsing is done by several
 * backends so this only dispatches the work.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uconf.h"
#include "libcstr.h"
#include "libuconf.h"

#include "parser_ascii.h"
#include "lexer_ascii.h"

extern int yyparse(void*);

static cstr *get_dir(const cstr *str)
{
	size_t len, last = 0;

	if (!CSTR_LEN(str))
		return cstr_dup(str);

	for (len = 0; len < CSTR_LEN(str); ++len) {
		if (str->buf[len] == '/')
			last = len;
	}

	/* if result is root directory, we need to copy the trailing '/' */
	if (!last && str->buf[0] == '/') {
		return cstr_dup(CSTR("/"));
	}

	return cstr_dup(CSTR_B(last, CSTR_VOID(str)));
}

int parse_file(struct uconf_file *file, struct yyarg *arg)
{
	int ret = 0;
	FILE *f;
	yyscan_t scanner = NULL;
	cstr *path = NULL;
	struct uconf_entry *pre;

	assert(arg->curr);

	f = fdopen(uconf_file_get_fd(file), "r");
	if (!f)
		return -errno;

	if (!arg->scanner) {
		if (yylex_init(&scanner)) {
			ret = -EFAULT;
			goto err;
		}
		yyset_in(f, scanner);
		arg->scanner = scanner;
	}

	if (!arg->path) {
		path = get_dir(uconf_file_get_name(file));
		if (!path) {
			ret = -ENOMEM;
			goto err;
		}
		arg->path = path;
	}

	if (!arg->fout)
		arg->fout = stderr;

	pre = arg->curr;
	arg->err = 0;

	ret = yyparse(arg);
	if (ret || arg->err) {
		ret = arg->err;
		if (!ret)
			ret = -EINVAL;
		if (arg->fout)
			fprintf(arg->fout, "Error: Invalid file format\n");
	} else {
		assert(arg->curr == pre);
	}

err:
	cstr_free(path);
	if (scanner)
		yylex_destroy(scanner);
	fclose(f);

	return ret;
}

int uconf_parse(struct uconf_entry *entry, struct uconf_file *file)
{
	struct yyarg arg;

	memset(&arg, 0, sizeof(arg));
	arg.curr = entry;

	return parse_file(file, &arg);
}

int uconf_write(const struct uconf_entry *entry, struct uconf_file *file)
{
	return -EINVAL;
}
