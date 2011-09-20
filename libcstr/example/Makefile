#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

BINARIES=strings.bin

all: build
	@echo "Available targets: build install uninstall clean"

build: $(BINARIES)
	@echo "Examples built"

%.bin: %.c ../src/libcstr.h Makefile
	gcc -o $@ $< -Wall -g -O0 -lcstr -I../src

install: build
	@echo "Examples installed"

uninstall:
	@echo "Examples uninstalled"

clean:
	@rm -vf *.bin
	@echo "Examples cleaned"

.PHONY: all build install uninstall clean
