#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

PREFIX=/usr
MANDIR=$(PREFIX)/share/man

PAGES=$(wildcard *.3)

all:
	@echo "Available targets: install uninstall"

build:
	@echo "Manpages built"

install:
	@install -v -t $(MANDIR)/man3 $(PAGES)
	@echo "Manpages installed"

uninstall:
	@rm -fv $(addprefix $(MANDIR)/man3/, $(PAGES))
	@echo "Manpages uninstalled"

clean:
	@echo "Manpages cleaned"

.PHONY: all build install uninstall clean
