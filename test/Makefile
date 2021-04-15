.POSIX:

CC=clang
CFLAGS=-I../ -lm -Wall -Wextra -Wno-unused -std=c89 -pedantic -Dinline="" \
       -ggdb3 -fsanitize=undefined -fsanitize=address -Os

all: random-target streachy-buffer-target
	rm -f ./a.out

random-target:
	${CC} ${CFLAGS} random/shuf.c && ./a.out
	${CC} ${CFLAGS} random/jump.c && ./a.out
	${CC} ${CFLAGS} random/dist_normal.c && ./a.out

streachy-buffer-target:
	${CC} ${CFLAGS} stretchy-buffer/test.c && ./a.out

check:
	cppcheck \
		-I../ -Dinline="" --std=c89 \
		--enable=all --suppress=missingInlcude \
		random/*.c stretchy-buffer/*.c

