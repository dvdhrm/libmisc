#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

#
# Build examples
# This does currently work on linux only. Adjust this makefile if you need to
# build the examples on other systems.
#

BINARIES=strings.bin

build: prepare $(BINARIES)

../libcstr.so:
	@cd .. && make build

libcstr.so: ../libcstr.so
	@cp -v ../libcstr.so .

libcstr.h: ../include/libcstr.h
	@cp -v ../include/libcstr.h .

%.bin: %.c libcstr.so libcstr.h Makefile
	gcc -o $@ $< -Wall -g -O0 libcstr.so

clean:
	@rm -vf *.bin libcstr.h libcstr.so

.PHONY: build prepare clean
