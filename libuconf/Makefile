#
# Universal Config Files Library
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

# Object files
OBJS=src/entry.o src/file.o src/parser.o
OBJS+=src/lexer_ascii.o src/parser_ascii.o
HEADERS=include/libuconf.h

# Prefix for install/uninstall
PREFIX=/usr

# CFlags
CFLAGS=-O0 -fPIC -Wall -g -Iinclude

SRCS=$(addsuffix .c, $(basename $(OBJS)))

all:
	@echo "Targets: build clean install uninstall example"

build: libuconf.so

src/lexer_ascii.c: src/lexer_ascii.l src/parser_ascii.h
	flex -o $@ --header-file=src/lexer_ascii.h $<

src/parser_ascii.c: src/parser_ascii.y
	bison -o $@ --defines=src/parser_ascii.h $<

src/%.o: src/%.c
	gcc -o $@ $< -c $(CFLAGS)

libuconf.so: $(OBJS)
	gcc -o $@ $(OBJS) -shared -lcstr

clean:
	@rm -rfv src/*.o src/parser_*.c src/parser_*.h src/lexer_*.c \
 src/lexer_*.h libuconf.so example/*.bin

.PHONY: all build clean install uninstall example

src/parser.c: src/lexer_ascii.h src/lexer_ascii.h
src/lexer_ascii.h: src/lexer_ascii.c
src/parser_ascii.h: src/parser_ascii.c
$(OBJS): include/libuconf.h include/uconf.h Makefile
$(SRCS): Makefile

install: build
	@install -v -t $(PREFIX)/include $(HEADERS)
	@install -v -t $(PREFIX)/lib libuconf.so

uninstall:
	@rm -fv $(addprefix $(PREFIX)/include/, $(notdir $(HEADERS)))
	@rm -fv $(PREFIX)/lib/libuconf.so

example: build
	gcc -o example/print_config.bin example/print_config.c $(CFLAGS) libuconf.so
