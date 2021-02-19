.POSIX:
CC = c99
CFLAGS = -I../../
LDLIBS = -lm

all: unsigned-division-by-constant

unsigned-division-by-constant: unsigned-division-by-constant.c

check:
	cppcheck \
		-I../../ -Dinline="" --std=c89 \
		--enable=all --suppress=missingInlcude \
		*.c

clean:
	rm -f unsigned-division-by-constant
