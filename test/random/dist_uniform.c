#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/test.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define NUM_RUNS 8
#define RUN_LENGTH_MASK 1023
#define RUN_TIME_OUT (1024*1024*8)

#define TEST_FULL_RANGE(name, T, init, next, inc) \
	TEST_BEGIN((name " full range")); \
	for (i = 0; i < NUM_RUNS; ++i) { \
		T beg, cur, end; \
		init; \
		for (i = 0; i < RUN_TIME_OUT && cur < end; ++i) \
			if (next == cur) \
				inc; \
		TEST_ASSERT(i < RUN_TIME_OUT); \
	} \
	TEST_END()

static PRNG32RomuQuad prng32;
static PRNG64RomuQuad prng64;

static void
random_rangef(float *beg, float *cur, float *end)
{
	union { uint32_t i; float f; } u;
	uint32_t n;
	float e;
	do u.i = prng32_romu_quad(&prng32); while (!isfinite(u.f));
	*beg = *cur = e = u.f;
	n = prng32_romu_quad(&prng32) & RUN_LENGTH_MASK;
	while (n--) e = nextafterf(e, FLT_MAX);
	*end = e;
}

static void
random_range(double *beg, double *cur, double *end)
{
	union { uint64_t i; double f; } u;
	uint64_t n;
	double e;
	do u.i = prng64_romu_quad(&prng64); while (!isfinite(u.f));
	*beg = *cur = e = u.f;
	n = prng32_romu_quad(&prng32) & RUN_LENGTH_MASK;
	while (n--) e = nextafter(e, DBL_MAX);
	*end = e;
}

int
main(void)
{
	size_t i;
	prng32_romu_quad_randomize(&prng32);
	prng64_romu_quad_randomize(&prng64);

	TEST_FULL_RANGE(
		"dist_uniform_u32", uint32_t,
		(beg = cur = 0,
		 end = prng32_romu_quad(&prng32) & RUN_LENGTH_MASK),
		dist_uniform_u32(end, prng32_romu_quad, &prng32), ++cur);

	TEST_FULL_RANGE(
		"dist_uniform_u64", uint64_t,
		(beg = cur = 0,
		 end = prng64_romu_quad(&prng64) & RUN_LENGTH_MASK),
		dist_uniform_u64(end, prng64_romu_quad, &prng64), ++cur);

	TEST_FULL_RANGE(
		"dist_uniformf", float,
		random_rangef(&beg, &cur, &end),
		dist_uniformf_dense(beg, end, prng32_romu_quad, &prng32),
		cur = nextafterf(cur, FLT_MAX));

	TEST_FULL_RANGE(
		"dist_uniform", double,
		random_range(&beg, &cur, &end),
		dist_uniform_dense(beg, end, prng64_romu_quad, &prng64),
		cur = nextafter(cur, DBL_MAX));

	return 0;
}
