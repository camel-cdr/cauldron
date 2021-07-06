PREFIX = /usr/local

HEADERS = arena-allocator.h arg.h random.h random-xmacros.h random-xoroshiro128-jump.h stretchy-buffer.h test.h

all:

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/include/cauldron"
	cd cauldron && cp $(HEADERS) "$(DESTDIR)$(PREFIX)/include/cauldron"

uninstall:
	-cd "$(DESTDIR)$(PREFIX)/include/cauldron" && rm $(HEADERS)
	-rmdir "$(DESTDIR)$(PREFIX)/include/cauldron"

clean:
	make -C tools/random/ clean
	make -C tools/random/permute/ clean

TIDY=clang-tidy -checks='cert-*,clang-analyzer-*,linuxkernel-*,misc-*, \
                         performance-*,portability-*,readability-*, \
                         -readability-braces-around-statements, \
                         -readability-isolate-declaration, \
                         -readability-magic-numbers, \
                         -readability-uppercase-literal-suffix, \
                         -cert-dcl37-c,-cert-dcl51-cpp' \
                -header-filter='.*' --extra-arg-before=-I. \
                --extra-arg=-std=c89 --extra-arg=-Dinline=""

tidy:
	${TIDY} cauldron/arg.h --extra-arg=-DARG_EXAMPLE
	${TIDY} --extra-arg=-std=gnu99 cauldron/bench.h --extra-arg=-DBENCH_EXAMPLE
	${TIDY} cauldron/test.h --extra-arg=-DTEST_EXAMPLE
	${TIDY} test/arena-allocator.c
	${TIDY} test/random/dist_normal.c
	${TIDY} test/random/jump.c
	${TIDY} test/random/shuf.c
	${TIDY} test/stretchy-buffer/test.c
