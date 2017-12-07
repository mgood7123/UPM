# really crappy Makefile, which will do for now

PREFIX ?= /usr/local

all: strace-4.6/Makefile okapi
	cd readelf-mini && make
	cd strace-4.6 && make
	mv -f strace-4.6/cde .
	mv -f strace-4.6/cde-exec .

install: all
	install cde cde-exec $(PREFIX)/bin

strace-4.6/Makefile:
	cd strace-4.6 && ./configure

clean:
	cd readelf-mini && make clean
	cd strace-4.6 && make clean
	rm -f cde cde-exec okapi

okapi: strace-4.6/okapi.c strace-4.6/okapi.h
	gcc -Wall -g -O2 -D_GNU_SOURCE -DOKAPI_STANDALONE strace-4.6/okapi.c -o okapi
