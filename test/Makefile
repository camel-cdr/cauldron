.POSIX:

CC=clang
CFLAGS=-I../ -lm -Wall -Wextra -Wno-unused -std=c89 -pedantic -Dinline=""

all: streachy-buffer random
	rm -f ./a.out

streachy-buffer:
	${CC} ${CFLAGS} stretchy-buffer/test.c && ./a.out

random:
	${CC} ${CFLAGS} random/shuf.c && ./a.out
	${CC} ${CFLAGS} random/jump.c && ./a.out
	${CC} ${CFLAGS} random/dist_normal.c && ./a.out

.PHONY: all random
