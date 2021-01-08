.POSIX:

CFLAGS=-I../ -lm

all: streachy-buffer
	rm ./a.out

streachy-buffer:
	gcc ${CFLAGS} stretchy-buffer/tests.c && ./a.out

.PHONY: all random
