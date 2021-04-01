#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/stretchy-buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "msws.h"

#ifndef RANDOM_BENCHMARK_COUNT
# define RANDOM_BENCHMARK_COUNT (1024*1024*32)
#endif

#define MAKE_RNG(name, type, init, next, ftype, max) \
	do { \
		Record r; \
		volatile double pi; \
		size_t i, c; \
		ftype x, y; \
		type rng; \
		clock_t beg; \
		init; \
		beg = clock(); \
		for (i = c = 0; i < RANDOM_BENCHMARK_COUNT; ++i) { \
			x = (ftype)next / max; \
			y = (ftype)next / max; \
			if (x*x + y*y <= (ftype)1.0) \
				++c; \
		} \
		pi = (double)c / RANDOM_BENCHMARK_COUNT * 4.0; \
		r.time = (double)(clock() - beg) / CLOCKS_PER_SEC; \
		r.rng = name; \
		sb_push(records, r); \
	} while (0)

#define MAKE_32(type, func, rnd) \
	MAKE_RNG(#func, type, rnd(&rng), func(&rng), \
	         float, UINT32_MAX)
#define MAKE_64(type, func, rnd) \
	MAKE_RNG(#func, type, rnd(&rng), func(&rng), \
	         double, UINT64_MAX)

typedef struct {
	double time;
	const char *rng;
} Record;

static int
record_cmp(const void *lhs, const void *rhs)
{
	const Record *l = lhs, *r = rhs;
	return l->time > r->time ? 1 : -1;
}

int
main(void)
{
	size_t i;
	puts("Note: Execution times between categories aren't comparable!\n");
	Sb(Record) records;

	puts("32-bit:");
	MAKE_RNG("msws32_64bit", MsWs32_64bit, trng_write(&rng, sizeof rng),
	         msws32_64bit(&rng), float, UINT32_MAX);
	MAKE_32(PRNG32Pcg, prng32_pcg, prng32_pcg_randomize);
	MAKE_32(PRNG32RomuTrio, prng32_romu_trio, prng32_romu_trio_randomize);
	MAKE_32(PRNG32RomuQuad, prng32_romu_quad, prng32_romu_quad_randomize);
	MAKE_32(PRNG32Xoroshiro64, prng32_xoroshiro64s, prng32_xoroshiro64_randomize);
	MAKE_32(PRNG32Xoroshiro64, prng32_xoroshiro64ss, prng32_xoroshiro64_randomize);
	MAKE_32(PRNG32Xoshiro128, prng32_xoshiro128s, prng32_xoshiro128_randomize);
	MAKE_32(PRNG32Xoshiro128, prng32_xoshiro128ss, prng32_xoshiro128_randomize);
	MAKE_32(CSPRNG32Chacha, csprng32_chacha, csprng32_chacha_randomize);
	qsort(records.at, sb_len(records), sizeof *records.at, record_cmp);
	for (i = 0; i < sb_len(records); ++i) {
		printf("\t%s: %fs\n", records.at[i].rng, records.at[i].time);
	}
	sb_setlen(records, 0);


	puts("\n64-bit:");
	MAKE_RNG("msws64_2x64bit", MsWs64_2x64bit, trng_write(&rng, sizeof rng),
	         msws64_2x64bit(&rng), double, UINT64_MAX);
#if __SIZEOF_INT128__
	MAKE_RNG("msws64_128bit", MsWs64_128bit, trng_write(&rng, sizeof rng),
	         msws64_128bit(&rng), double, UINT64_MAX);
#endif
#if PRNG64_PCG_AVAILABLE
	MAKE_64(PRNG64Pcg, prng64_pcg, prng64_pcg_randomize);
#endif
	MAKE_64(PRNG64RomuDuo, prng64_romu_duo_jr, prng64_romu_duo_randomize);
	MAKE_64(PRNG64RomuDuo, prng64_romu_duo, prng64_romu_duo_randomize);
	MAKE_64(PRNG64RomuTrio, prng64_romu_trio, prng64_romu_trio_randomize);
	MAKE_64(PRNG64RomuQuad, prng64_romu_quad, prng64_romu_quad_randomize);
	MAKE_64(PRNG64Xoroshiro128, prng64_xoroshiro128p, prng64_xoroshiro128_randomize);
	MAKE_64(PRNG64Xoroshiro128, prng64_xoroshiro128ss, prng64_xoroshiro128_randomize);
	MAKE_64(PRNG64Xoshiro256, prng64_xoshiro256p, prng64_xoshiro256_randomize);
	MAKE_64(PRNG64Xoshiro256, prng64_xoshiro256ss, prng64_xoshiro256_randomize);
	qsort(records.at, sb_len(records), sizeof *records.at, record_cmp);
	for (i = 0; i < sb_len(records); ++i) {
		printf("\t%s: %fs\n", records.at[i].rng, records.at[i].time);
	}
	sb_free(records);
}

