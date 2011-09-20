#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

PREFIX=/usr

LIBNAME=libcstr.so
HEADERNAME=libcstr.h
HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

all:
	@echo "Available targets: build install uninstall clean"

build: $(LIBNAME)
	@echo "Source built"

$(LIBNAME): $(SOURCES) $(HEADERS)
	gcc -shared -Wall -fPIC -o $@ $(SOURCES)

install: build
	@install -v -t $(PREFIX)/include $(HEADERNAME)
	@install -v -t $(PREFIX)/lib $(LIBNAME)
	@echo "Source installed"

uninstall:
	@rm -fv $(addprefix $(PREFIX)/include/, $(HEADERNAME))
	@rm -fv $(addprefix $(PREFIX)/lib/, $(LIBNAME))
	@echo "Source uninstalled"

clean:
	@rm -vf *.so *.o
	@echo "Source cleaned"

.PHONY: all build install uninstall clean
