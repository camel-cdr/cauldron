.POSIX:

CFLAGS=-I../ -lm

all: tcc streachy-buffer

tcc:
	which tcc || ( echo "tcc not found, please install tcc" && exit 1 )

streachy-buffer:
	tcc ${CFLAGS} -run stretchy-buffer/tests.c

.PHONY: tcc all random
