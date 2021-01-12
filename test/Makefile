.POSIX:

CFLAGS=-I../ -lm

all: streachy-buffer
	rm -f ./a.out

streachy-buffer:
	gcc ${CFLAGS} stretchy-buffer/test.c && ./a.out

.PHONY: all random
