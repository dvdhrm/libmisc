#
# Written 2011 by David Herrmann
# Dedicated to the Public Domain
#

CFLAGS=-Wall -O0 -g -Isrc

all: build
	@echo "Available targets: build clean example"

build: src/libsfs.so

clean:
	@rm -rfv src/*.so example/*.bin

example: example/sfs_example.bin

src/libsfs.so: src/sfs.c
	gcc -o src/libsfs.so -shared -fPIC $(CFLAGS) src/sfs.c

example/sfs_example.bin: example/sfs_example.c src/libsfs.so
	gcc -o example/sfs_example.bin example/sfs_example.c $(CFLAGS) \
 src/libsfs.so

.PHONY: all build clean example
