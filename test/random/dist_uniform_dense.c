#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/test.h>

/*
 * Make sure that you don't use -ffast-math or -Ofast when compiling this,
 * because that might disable denormals, which this test for.
 */

#define NRANGES 8
#define MAX_TESTS (1024*1024*2)
#define ALPHA 100.0

#define ARRLEN(a) (sizeof (a) / sizeof *(a))

static float float_tests[][4] = {
	{ 0, 1, 2, 3 },
	{ -2, -1, 1, 2 },
	{ -5, -4, -2, -1 },
	{ 9e-45f, 4e-41f, 2e-39f, 1.1e-38f }, /* denormals */
	{ 1e-38f, 1.2e-38f, 1.4e-38f, 2e-38f }, /* denormal boundary */
	{ 1e-40f, 3e-38f, 3e-35f, 3e-33f },
};

static double double_tests[][4] = {
	{ 0, 1, 2, 3 },
	{ -2, -1, 1, 2 },
	{ -5, -4, -2, -1 },
	{ 9e-320, 9e-317, 9e-311, 1.1e-310 }, /* denormals */
	{ 9e-317, 9e-314, 9e-310, 2e-308 }, /* denormal boundary */
	{ 9e-300, 9e-294, 9e-292, 2e-288 },
};

#define MAKE_TEST(name, type, next) \
	TEST_BEGIN(name); \
	for (i = 0; i < NRANGES + ARRLEN(float_tests); ++i) { \
		double expected, ntests, stddev; \
		type tmp, r[4]; /* f1, s1, s2, f2 */ \
		size_t cnt = 0; \
\
		if (i < ARRLEN(float_tests)) { \
			for (j = 0; j < 4; ++j) \
				r[j] = type##_tests[i][j]; \
		} else while (1) { type##fallback: \
			trng_write(r, sizeof r); \
			if (!isfinite(r[0]) || !isfinite(r[1]) || \
			    !isfinite(r[2]) || !isfinite(r[3])) \
				continue; \
			/* sorting network for r */ \
			if (r[0] > r[2]) tmp = r[0], r[0] = r[2], r[2] = tmp; \
			if (r[1] > r[3]) tmp = r[1], r[1] = r[3], r[3] = tmp; \
			if (r[0] > r[1]) tmp = r[0], r[0] = r[1], r[1] = tmp; \
			if (r[2] > r[3]) tmp = r[2], r[2] = r[3], r[3] = tmp; \
			if (r[1] > r[2]) tmp = r[1], r[1] = r[2], r[2] = tmp; \
			if (r[0] != r[3]  && r[1] != r[2]) \
				break; \
		} \
\
		expected = (1.0*r[2]-r[1]) / (1.0*r[3]-r[0]); \
		ntests = ALPHA / expected; \
		if (ntests > MAX_TESTS) \
			goto type##fallback; \
\
		for (j = 0; j < ntests; ++j) { \
			type x = next; \
			if (x >= r[1] && x <= r[2]) \
				++cnt; \
		} \
\
		stddev = sqrt(ntests * expected * (1.0 - expected)); \
		/* printf("%g %g %g %g | %g %g | %g %g\n",
		         r[0], r[1], r[2], r[3],
		         expected, ntests, stddev * 5, fabs(ALPHA - cnt)); */ \
		/* fail with a probability of 5 sigma */ \
		TEST_ASSERT(5 * stddev > fabs(ALPHA - cnt)); \
	} \
	TEST_END();

int
main(void)
{
	size_t i, j;
	PRNG32RomuQuad prng32;
	PRNG64RomuDuo prng64;
	prng32_romu_quad_randomize(&prng32);
	prng64_romu_duo_randomize(&prng64);

	MAKE_TEST("dist_uniformf_dense", float,
	          dist_uniformf_dense(r[0], r[3], prng32_romu_quad, &prng32));
	MAKE_TEST("dist_uniform_dense", double,
	          dist_uniform_dense(r[0], r[3], prng64_romu_duo_jr, &prng64));

	return 0;
}
