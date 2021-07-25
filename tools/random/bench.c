#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/bench.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <wmmintrin.h>

#include "extra.h"

#define COUNT (1024*512)
#define SAMPLES (64)

#define MAKE_RNG(name, type, init, next, ftype, max) \
	do { \
		type rng; \
		init; \
		BENCH(name, 8, SAMPLES) { \
			ftype x, y; \
			size_t i, c; \
			double pi; \
			for (i = c = 0; i < COUNT; ++i) { \
				x = next * (ftype)1.0 / max; \
				y = next * (ftype)1.0 / max; \
				if (x*x + y*y <= (ftype)1.0) \
					++c; \
			} \
			pi = (double)c / COUNT * 4.0; \
			BENCH_VOLATILE(pi); \
		} \
	} while (0);


int
main(void)
{
	size_t i;
	puts("Note: Execution times between categories aren't comparable!\n");

	puts("16-bit");
#define RANDOM_X16(type, func, rnd) \
	MAKE_RNG(#func, type, rnd(&rng), func(&rng), float, UINT16_MAX)
#define RANDOM_X32(type, func, rnd)
#define RANDOM_X64(type, func, rnd)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');

	puts("32-bit");
#define RANDOM_X16(type, func, rnd)
#define RANDOM_X32(type, func, rnd) \
	MAKE_RNG(#func, type, rnd(&rng), func(&rng), float, UINT32_MAX)
#define RANDOM_X64(type, func, rnd)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');

	puts("64-bit");
#define RANDOM_X16(type, func, rnd)
#define RANDOM_X32(type, func, rnd)
#define RANDOM_X64(type, func, rnd) \
	MAKE_RNG(#func, type, rnd(&rng), func(&rng), double, UINT64_MAX)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');

	bench_free();
	return 0;
}

