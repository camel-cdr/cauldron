.POSIX:

CFLAGS+=-I../../ -I/home/camel/git/cauldron

BIN = dist rng bench ziggurat-constants

all: $(BIN)

testu01: testu01.c
	$(CC) $(CFLAGS) -O2 -o $@ -ltestu01 $<

ziggurat-constants: ziggurat-constants.c
	$(CC) $(CFLAGS) -O2 -o $@ -lm $<

PractRand:
	wget https://downloads.sourceforge.net/project/pracrand/PractRand-pre0.95.zip
	unzip PractRand-pre0.95.zip -d tmp
	rm PractRand-pre0.95.zip
	g++ $(CFLAGS) -O2 tmp/tools/RNG_test.cpp tmp/src/*.cpp \
	       tmp/src/RNGs/*.cpp tmp/src/RNGs/other/*.cpp \
	       -Itmp/include -pthread -std=gnu++11 -o $@
	rm -rf tmp

dist: dist.c
	$(CC) $(CFLAGS) -Ofast -o $@ -lm $<

rng: rng.c
	$(CC) $(CFLAGS) -Ofast -o $@ -lm $<

bench: bench.c
	$(CC) $(CFLAGS) -Ofast -o $@ -lm $<

clean:
	rm -f $(BIN) testu01 PractRand

.PHONY: all options clean
