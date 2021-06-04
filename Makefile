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

check:
	cd tools/random && make check
	cd tools/bithacks && make check
	cd test && make check
