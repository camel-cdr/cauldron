.POSIX:

CFLAGS=-I../ -lm -Wall -Wextra -Wno-unused -pedantic -Dinline="" \
       -ggdb3 -fsanitize=undefined -fsanitize=address

CC_BEG=x=`mktemp` && clang ${CFLAGS} -std=c89 -o $$x
CC_END=&& $$x && rm -f $$x

CXX_BEG=x=`mktemp` && clang++ -xc++ ${CFLAGS} -o $$x
CXX_END=&& $$x && rm -f $$x

CC_BIG_ENDIAN_BEG=x=`mktemp` && \
                  which mips-linux-gnu-gcc qemu-mips >/dev/null && \
                  mips-linux-gnu-gcc -I../ -static -o $$x
CC_BIG_ENDIAN_END=-lm && qemu-mips $$x && rm -f $$x

all: random-target streachy-buffer-target arena-allocator

random-target: random-shuf random-jump random-dist-normal

random-shuf:
	${CC_BEG} random/shuf.c ${CC_END}
	${CXX_BEG} random/shuf.c ${CXX_END}
	${CC_BIG_ENDIAN_BEG} random/shuf.c ${CC_BIG_ENDIAN_END}
	rm -f ./a.out

random-jump:
	${CC_BEG} random/jump.c ${CC_END}
	${CXX_BEG} random/jump.c ${CXX_END}
	${CC_BIG_ENDIAN_BEG} random/jump.c ${CC_BIG_ENDIAN_END}
	rm -f ./a.out

random-dist-normal:
	${CC_BEG} random/dist_normal.c ${CC_END}
	${CXX_BEG} random/dist_normal.c ${CXX_END}
	${CC_BIG_ENDIAN_BEG} random/dist_normal.c ${CC_BIG_ENDIAN_END}
	rm -f ./a.out


streachy-buffer-target:
	${CC_BEG} stretchy-buffer/test.c ${CC_END}
	${CC_BIG_ENDIAN_BEG} stretchy-buffer/test.c ${CC_BIG_ENDIAN_END}
	rm -f ./a.out

arena-allocator:
	${CC_BEG} arena-allocator.c ${CC_END}
	${CC_BIG_ENDIAN_BEG} arena-allocator.c ${CC_BIG_ENDIAN_END}
	rm -f ./a.out

check:
	cppcheck \
		-I../ -Dinline="" --std=c89 \
		--enable=all --suppress=missingInlcude \
		random/*.c stretchy-buffer/*.c *.c

