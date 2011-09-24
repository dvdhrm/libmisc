/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * Parser for ASCII compatible configs
 * This is a parser for plain old known C-style config files. The syntax is
 * pretty straightforward and quite common.
 */

%{

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libcstr.h>

#include "libuconf.h"
#include "uconf.h"

extern int yylex(void*, void*);

static void parser_fout(struct yyarg *arg, const char *format, ...)
{
	va_list list;

	if (!arg->fout)
		return;

	va_start(list, format);
	vfprintf(arg->fout, format, list);
	va_end(list);
}

static void parser_error(struct yyarg *arg, int err)
{
	arg->err = err;
}

static inline int yyerror(struct yyarg *arg, const char *s)
{
	parser_fout(arg, "Error: %s\n", s);
	return 0;
}

#define MEMFAIL { \
		parser_error(arg, -ENOMEM); \
		parser_fout(arg, "Error: Memory allocation failed\n"); \
		YYABORT; \
	}

#define YYLEX_PARAM (arg->scanner)

static bool builtin_include(struct yyarg *arg, const cstr *path)
{
	struct uconf_file *file;
	cstr *npath = NULL;
	int ret;
	struct yyarg parg;
	bool succ = false;

	memset(&parg, 0, sizeof(parg));
	parg.depth = arg->depth + 1;
	parg.fout = arg->fout;
	parg.curr = arg->curr;

	if (parg.depth > 10) {
		parser_fout(arg, "Error: Maximum include depth 10 reached\n");
		parser_error(arg, -EINVAL);
		return NULL;
	}

	if (path->len == 0) {
		parser_fout(arg, "Error: Invalid include path\n");
		parser_error(arg, -EINVAL);
		return NULL;
	}

	if (path->buf[0] != '/') {
		npath = cstr_dup(arg->path);
		if (!npath || !cstr_cat(npath, CSTR("/")) ||
						!cstr_ccat(npath, path)) {
			parser_error(arg, -ENOMEM);
			return NULL;
		}
		path = npath;
	}

	file = uconf_file_new();
	if (!file) {
		parser_error(arg, -ENOMEM);
		goto err_mem;
	}

	ret = uconf_file_open(file, path, UCONF_FILE_READ);
	if (ret) {
		parser_fout(arg, "Error: Cannot open include file %s\n",
			path->buf);
		parser_error(arg, ret);
		goto err;
	}

	ret = parse_file(file, &parg);
	if (ret) {
		parser_error(arg, ret);
		goto err;
	}

	succ = true;
err:
	uconf_file_free(file);
err_mem:
	cstr_free(npath);
	return succ;
}

%}

%define api.pure
%parse-param { struct yyarg *arg }

%union {
	int64_t num;
	long double fnum;
	cstr *str;
	struct uconf_entry *entry;
}

%token <num> QINT
%token <fnum> QFLOAT
%token <str> QSTRING
%token QERROR QINCLUDE QNULL QREF QANCHOR

%type <num> int
%type <fnum> float
%type <entry> assignment rvalue statement list
%type <entry> anchor_rvalue
%type <str> sref anchor

%left <num> '-' '+'
%left <num> '*' '/'
%left <num> NEG

%destructor { cstr_free($$); } <str>
%destructor { uconf_entry_unref($$); } <entry>

%%

container:
	configfile

configfile:
	/* empty */
	|
	{
		if (uconf_entry_set_type(arg->curr, UCONF_ENTRY_LIST))
			MEMFAIL;
	}
	statements
	;

statements:
	statement
	{
		if ($1)
			uconf_entry_link_last(arg->curr, $1);
	}
	| statements statement
	{
		if ($2)
			uconf_entry_link_last(arg->curr, $2);
	}
	;

statement:
	assignment ';'
	{
		$$ = $1;
	}
	| builtin ';'
	{
		$$ = NULL;
	}
	| error ';'
	{
		parser_error(arg, -EINVAL);
		yyerrok;
		parser_fout(arg, "Error: Invalid statement\n");
		$$ = NULL;
	}
	;

builtin:
	include
	| anchor_assign
	;

include:
	QINCLUDE '(' QSTRING ')'
	{
		bool res;

		res = builtin_include(arg, $3);
		cstr_free($3);

		if (!res) {
			parser_fout(arg, "Error: Include failed\n");
			YYABORT;
		}
	}
	;

anchor_assign:
	anchor
	{
		cstr_free(arg->curr->anchor);
		arg->curr->anchor = $1;
	}

anchor:
	QANCHOR assign QSTRING { $$ = $3; }
	| QANCHOR '(' QSTRING ')' { $$ = $3; }
	;

assignment:
	QSTRING assign anchor_rvalue
	{
		$$ = $3;
		$$->name = $1;
	}
	;

assign:
	/* empty */
	| '='
	| ':'
	;

anchor_rvalue:
	rvalue { $$ = $1; }
	| anchor rvalue
	{
		$$ = $2;
		cstr_free($$->anchor);
		$$->anchor = $1;
	}
	| rvalue anchor
	{
		$$ = $1;
		cstr_free($$->anchor);
		$$->anchor = $2;
	}
	;

rvalue:
	QNULL
	{
		$$ = uconf_entry_new_type(UCONF_ENTRY_NULL);
		if (!$$)
			MEMFAIL;
	}
	|
	QSTRING
	{
		$$ = uconf_entry_new_type(UCONF_ENTRY_QSTR);
		if (!$$) {
			cstr_free($1);
			MEMFAIL;
		}
		$$->v.qstr = $1;
	}
	| list
	{
		$$ = $1;
	}
	| int
	{
		$$ = uconf_entry_new_type(UCONF_ENTRY_QINT);
		if (!$$)
			MEMFAIL;
		$$->v.qint = $1;
	}
	| float
	{
		$$ = uconf_entry_new_type(UCONF_ENTRY_QFLOAT);
		if (!$$)
			MEMFAIL;
		$$->v.qfloat = $1;
	}
	| sref
	{
		$$ = uconf_entry_new_type(UCONF_ENTRY_SREF);
		if (!$$) {
			cstr_free($1);
			MEMFAIL;
		}
		$$->v.sref = $1;
	}
	;

sref:
	QREF '(' QSTRING ')' { $$ = $3; }
	;

float:
	QFLOAT { $$ = $1; }
	| '-' float %prec NEG { $$ = - $2; }
	;

int:
	QINT { $$ = $1; }
	| int '+' int { $$ = $1 + $3; }
	| int '-' int { $$ = $1 - $3; }
	| int '*' int { $$ = $1 * $3; }
	| int '/' int { $$ = $1 / $3; }
	| '-' int %prec NEG { $$ = - $2; }
	| '(' int ')' { $$ = $2; }
	;

list:
	{
		struct uconf_entry *enew;

		enew = uconf_entry_new_type(UCONF_ENTRY_LIST);
		if (!enew)
			MEMFAIL;
		uconf_entry_link_last(arg->curr, enew);
		arg->curr = enew;
	}
	'{' list_body '}'
	{
		$$ = arg->curr;
		arg->curr = $$->parent;
		/*
		 * Set parent to NULL to avoid assertion errors when linking it
		 * again in the upper layers.
		 */
		uconf_entry_unlink($$);
	}
	;

list_body:
	/* empty */
	| named_list
	| anon_list
	;

named_list:
	statements
	;

anon_list:
	anon_list_valid
	| anon_list_valid ','
	;

anon_list_valid:
	anon_entry
	| anon_list_valid ',' anon_entry
	;

anon_entry:
	rvalue
	{
		uconf_entry_link_last(arg->curr, $1);
	}
	| assignment
	{
		uconf_entry_link_last(arg->curr, $1);
	}
	;
