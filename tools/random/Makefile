.POSIX:
CC = c99
CFLAGS=-I../../

BIN = dist rng bench ziggurat-constants

all: $(BIN)

testu01: testu01.c
	$(CC) $(CFLAGS) -O2 -o $@ -ltestu01 testu01.c

ziggurat-constants: ziggurat-constants.c
	$(CC) $(CFLAGS) -O2 -o $@ -lm ziggurat-constants.c

PractRand:
	wget https://downloads.sourceforge.net/project/pracrand/PractRand-pre0.95.zip
	unzip PractRand-pre0.95.zip -d tmp
	rm PractRand-pre0.95.zip
	clang++ $(CFLAGS) -Ofast \
	       tmp/tools/RNG_test.cpp tmp/src/*.cpp \
	       tmp/src/RNGs/*.cpp tmp/src/RNGs/other/*.cpp \
	       -Itmp/include -pthread -std=gnu++11 -o $@
	rm -rf tmp

dist: dist.c
	$(CC) $(CFLAGS) -Ofast -o $@ -lm dist.c

rng: rng.c extra.h
	$(CC) $(CFLAGS) -march=native -Ofast -o $@ -lm rng.c

bench: bench.c extra.h
	gcc $(CFLAGS) -march=native -Ofast -o $@ -lm bench.c

clean:
	rm -f $(BIN) testu01 PractRand

