#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

#
# Build examples
# This does currently work on linux only. Adjust this makefile if you need to
# build the examples on other systems.
#

BINARIES=print_config.bin

build: prepare $(BINARIES)

../libuconf.so:
	@cd .. && make build

libuconf.so: ../libuconf.so
	@cp -v ../libuconf.so .

libuconf.h: ../include/libuconf.h
	@cp -v ../include/libuconf.h .

%.bin: %.c libuconf.so libuconf.h Makefile
	gcc -o $@ $< -Wall -g -O0 libuconf.so

clean:
	@rm -vf *.bin libuconf.h libuconf.so

.PHONY: build prepare clean
