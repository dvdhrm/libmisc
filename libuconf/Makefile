#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

# to be built
LIBNAME=libuconf
C_SRC=entry.c file.c parser.c lexer_ascii.c parser_ascii.c
C_INC=libuconf.h uconf.h
LIBS=cstr

# to be installed
INC_I=libuconf.h

DEPENDS=../inc.makefile
include ../inc.makefile

#
# Project specific rules
# Build rules for this project only. This includes some examples and other
# stuff like special dependencies.
#

CLEAN+=src/lexer_ascii.h src/lexer_ascii.c
CLEAN+=src/parser_ascii.h src/parser_ascii.c

src/parser.c: src/lexer_ascii.h src/parser_ascii.h
src/lexer_ascii.h: src/lexer_ascii.c
src/parser_ascii.h: src/parser_ascii.c

src/lexer_ascii.c: src/lexer_ascii.l src/parser_ascii.h
	flex -o $@ --header-file=src/lexer_ascii.h $<

src/parser_ascii.c: src/parser_ascii.y
	bison -o $@ --defines=src/parser_ascii.h $<
