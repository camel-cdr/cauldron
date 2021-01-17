.POSIX:

CFLAGS=-I../ -lm -Wall -Wextra

all: streachy-buffer
	rm -f ./a.out

streachy-buffer:
	${CC} ${CFLAGS} stretchy-buffer/test.c && ./a.out

.PHONY: all random
