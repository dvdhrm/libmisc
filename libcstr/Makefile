#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

PREFIX=/usr
DIRS=src man3 example
MAKE=make --no-print-directory

all: build
	@echo "Available targets: build install uninstall clean"

build:
	@for i in $(DIRS) ; do cd $$i && $(MAKE) build PREFIX="$(PREFIX)" ; cd .. ; done

install:
	@for i in $(DIRS) ; do cd $$i && $(MAKE) install PREFIX="$(PREFIX)" ; cd .. ; done

uninstall:
	@for i in $(DIRS) ; do cd $$i && $(MAKE) uninstall PREFIX="$(PREFIX)" ; cd .. ; done

clean:
	@for i in $(DIRS) ; do cd $$i && $(MAKE) clean PREFIX="$(PREFIX)" ; cd .. ; done

.PHONY: all build install uninstall clean
