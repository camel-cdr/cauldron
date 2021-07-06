.POSIX:

CFLAGS=-I../ -lm -Wall -Wextra -Wno-unused -pedantic -Dinline="" \
       -ggdb3 -fsanitize=address,undefined,leak -ftrapv

CC_SANITIZE_BEG=x=`mktemp` && clang ${CFLAGS} -std=c89 -o $$x
CC_SANITIZE_END=&& $$x && rm -f $$x

CC_VALGRIND_BEG=x=`mktemp` && clang ${CFLAGS} -std=c89 -o $$x
CC_VALGRIND_END=&& $$x && rm -f $$x

CXX_BEG=x=`mktemp` && clang++ -xc++ ${CFLAGS} -o $$x
CXX_END=&& $$x && rm -f $$x

CC_BIG_ENDIAN_BEG=x=`mktemp` && \
                  which mips-linux-gnu-gcc qemu-mips >/dev/null && \
                  mips-linux-gnu-gcc -I../ -static -o $$x
CC_BIG_ENDIAN_END=-lm && qemu-mips $$x && rm -f $$x || true

all: arena-allocator random-target streachy-buffer-target

arena-allocator:
	./test.sh arena-allocator.c

random-target: random-shuf random-jump random-dist-normal
random-shuf:
	./test.sh c++ random/shuf.c
random-jump:
	./test.sh c++ random/jump.c
random-dist-normal:
	./test.sh c++ random/dist_normal.c
random-dist-uniform:
	./test.sh c++ random/dist_uniform.c

streachy-buffer-target:
	./test.sh stretchy-buffer/test.c
