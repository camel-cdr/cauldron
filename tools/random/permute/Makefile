.POSIX:
CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -march=native -Ofast -fopenmp
LDLIBS  = -lm -ldl

hashes = \
    candidates/camel-cdr.so \
    candidates/kensler.so \
    candidates/kensler-splittable64.so \
    candidates/stub.so

compile: evalpow2 rng-shuffle $(hashes)

evalpow2: evalpow2.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ evalpow2.c $(LDLIBS)

rng-shuffle: rng-shuffle.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ rng-shuffle.c $(LDLIBS)

candidates/camel-cdr.so: candidates/camel-cdr.c
candidates/kensler.so: candidates/kensler.c
candidates/kensler-splittable64.so: candidates/kensler-splittable64.c
candidates/stub.so: candidates/stub.c

clean:
	rm -f evalpow2 rng-shuffle $(hashes)

.SUFFIXES: .so .c
.c.so:
	$(CC) -shared $(LDFLAGS) -fPIC $(CFLAGS) -o $@ $<
