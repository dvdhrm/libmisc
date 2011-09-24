/*
 * Universal Config Files Library
 * Written 2011 by David Herrmann
 * Dedicated to the Public Domain
 */

/*
 * ASCII config files
 * This lexer reads config files with ASCII compatible charsets. See ascii
 * parser for syntax rules.
 */

%option 8bit reentrant bison-bridge
%option noyywrap stack
%option nounput noinput noyy_top_state

%{
	#include <stdint.h>
	#include <stdlib.h>

	#include <libcstr.h>

	#include "parser_ascii.h"
	#include "uconf.h"
%}

%x STRING

%%

%{
	void str_append(yyscan_t scanner, const cstr *str)
	{
		if (!cstr_cat(yylval->str, str)) {
			cstr_free(yylval->str);
			YY_FATAL_ERROR("out of memory");
		}
	}
%}

%{
	yyin = yyextra;
%}

	/*
	 * Strings
	 * Strings start with " and end with " like in C. Backslashes are used
	 * to escape special characters. Currently supported special characters
	 * are: \\, \", \n, \r, \t
	 * If a backslash is followed by any character not specified here, it is
	 * simply copied as simple backslash to the output string.
	 */
"\""				{
	yy_push_state(STRING, yyscanner);
	yylval->str = cstr_new(0);
	if (!yylval->str)
		YY_FATAL_ERROR("out of memory");
}
<STRING><<EOF>>			{
	cstr_free(yylval->str);
	return QERROR;
}
<STRING>"\""			{
	yy_pop_state(yyscanner);
	return QSTRING;
}
<STRING>"\\\""			str_append(yyscanner, CSTR("\""));
<STRING>"\\n"			str_append(yyscanner, CSTR("\n"));
<STRING>"\\t"			str_append(yyscanner, CSTR("\t"));
<STRING>"\\r"			str_append(yyscanner, CSTR("\r"));
<STRING>"\\\\"			|
<STRING>"\\"			str_append(yyscanner, CSTR("\\"));
<STRING>[^"\\]+			str_append(yyscanner, CSTR_B(yyleng, yytext));

	/*
	 * Comments
	 * There are single line comments which start with a # and go until the
	 * end of line.
	 */
"#"[^\n]*

	/* whitespaces are ignored */
[ \t\n\r]+

	/* NULL constant */
NULL				return QNULL;

	/*
	 * Identifiers
	 * Identifiers are strings that are not enclosed in " and which do not
	 * allow any escape sequences. This allows to write simple strings
	 * without annoying " characters.
	 * However, identifiers use a limited character set. They may not
	 * contain any control characters. They are reported as QSTRING so the
	 * parser cannot determine whether a regular string or identifier was
	 * read.
	 * There are special identifiers, that are not parsed as string like
	 * NULL. To get them as string you need to enclose them in quotes like
	 * "NULL".
	 */
[a-zA-Z_][a-zA-Z0-9_-]*		{
	yylval->str = cstr_dup(CSTR_B(yyleng, yytext));
	if (!yylval->str)
		YY_FATAL_ERROR("out of memory");
	return QSTRING;
}

	/*
	 * Builtins
	 * Builtins are special identifiers that are handled by the parser and
	 * have special behaviour. They are not parsed into uconf_entry
	 * structures.
	 */
"!include"			return QINCLUDE;
"!ref"				return QREF;
"!anchor"			return QANCHOR;

	/* integers */
[[:digit:]]+			{
	yylval->num = strtoull(yytext, NULL, 10);
	return QINT;
}
	/* hex integers */
0[xX][[:xdigit:]]+		{
	yylval->num = strtoull(yytext, NULL, 16);
	return QINT;
}
	/* floating point */
[[:digit:]]+.[[:digit:]]+	{
	yylval->fnum = strtold(yytext, NULL);
	return QFLOAT;
}

	/*
	 * Report the rest as raw characters to the parser.
	 */
.				return *yytext;

