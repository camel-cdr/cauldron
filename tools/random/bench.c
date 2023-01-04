#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/bench.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "extra.h"

#define COUNT (1024*512*2ull)
#define SAMPLES (64*2)

#define BENCH_RNG(name, type, init, next, ftype, max) \
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

static void
bench_rng_16(void)
{
	puts("16-bit PRNGs:");
#define RANDOM_X16(type, func, rnd) \
	BENCH_RNG(#func, type, rnd(&rng), func(&rng), float, UINT16_MAX)
#define RANDOM_X32(type, func, rnd)
#define RANDOM_X64(type, func, rnd)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');
}

static void
bench_rng_32(void)
{
	puts("32-bit PRNGs");
#define RANDOM_X16(type, func, rnd)
#define RANDOM_X32(type, func, rnd) \
	BENCH_RNG(#func, type, rnd(&rng), func(&rng), float, UINT32_MAX)
#define RANDOM_X64(type, func, rnd)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');
}

static void
bench_rng_64(void)
{
	puts("64-bit PRNGs");
#define RANDOM_X16(type, func, rnd)
#define RANDOM_X32(type, func, rnd)
#define RANDOM_X64(type, func, rnd) \
	BENCH_RNG(#func, type, rnd(&rng), func(&rng), double, UINT64_MAX)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
	bench_done();
	putchar('\n');
}


#define BENCH_NORM_IMPL(name, type, init, next, ftype) \
	do { \
		type rng; \
		init(&rng); \
		BENCH(name, 8, SAMPLES) { \
			ftype x, y; \
			size_t i, c; \
			for (i = c = 0; i < COUNT; ++i) { \
				x = next; \
				y = next; \
				if (x*x + y*y <= 1) \
					++c; \
			} \
			BENCH_VOLATILE(c); \
		} \
	} while (0);

#define NORM_NEXT prng64_romu_duo_jr
#define BENCH_NORM(name, next) BENCH_NORM_IMPL( \
		name, PRNG64RomuDuo, prng64_romu_duo_randomize, next, double)

#define NORMF_NEXT prng32_romu_trio
#define BENCH_NORMF(name, next) BENCH_NORM_IMPL( \
		name, PRNG32RomuTrio, prng32_romu_trio_randomize, next, float)



static void
bench_normal(void)
{
	DistNormalZig zig;
	dist_normal_zig_init(&zig);

	puts("normal distribution using prng64_romu_duo_jr");
	BENCH_NORM("dist_normalf_fast", dist_normalf_fast(NORM_NEXT(&rng)));
	BENCH_NORM("dist_normal", dist_normal(NORM_NEXT, &rng));
	BENCH_NORM("dist_normal_zig", dist_normal_zig(&zig, NORM_NEXT, &rng));

	bench_done();
	putchar('\n');
}

static void
bench_normalf(void)
{
	DistNormalfZig zig;
	dist_normalf_zig_init(&zig);

	puts("normal distribution using prng32_romu_trio");
	BENCH_NORMF("dist_normalf", dist_normalf(NORMF_NEXT, &rng));
	BENCH_NORMF("dist_normalf_zig", dist_normalf_zig(&zig, NORMF_NEXT, &rng));

	bench_done();
	putchar('\n');
}

int
main(void)
{
	size_t i;
	puts("Note: Execution times between categories aren't comparable!\n");

	bench_rng_16();
	bench_rng_32();
	bench_rng_64();
	bench_normal();
	bench_normalf();

	bench_free();
	return 0;
}

