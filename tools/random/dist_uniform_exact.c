#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>

/* This is some messy testing of dist_uniformf_exact, read at your own risk. */

static PRNG32RomuQuad prng32;

#define NRANGES (1)
#define MAX_TESTS (1024)
#define ALPHA (NRANGES * 10.0)

static int
fcomp(const void *lhs, const void *rhs)
{
	const float *l = lhs, *r = rhs;
	return *l > *r;
}

int
main(void)
{
	size_t i, j;
	prng32_romu_quad_randomize(&prng32);

	for (i = 0; i < NRANGES; ++i) {
		float r[4]; /* f1, s1, s2, f2 */
		rep:
		do {
			r[0] = dist_uniformf(prng32_romu_quad(&prng32));
			r[1] = dist_uniformf(prng32_romu_quad(&prng32));
			r[2] = dist_uniformf(prng32_romu_quad(&prng32));
			r[3] = dist_uniformf(prng32_romu_quad(&prng32));
		} while (!isfinite(r[0]) || !isfinite(r[1]) ||
		         !isfinite(r[2]) || !isfinite(r[3]) ||
			 r[0] == r[1] || r[0] == r[2] || r[0] == r[3] ||
			 r[1] == r[2] || r[1] == r[3] || r[2] == r[3]);

		qsort(r, 4, sizeof *r, fcomp);

		float expected = ((float)r[2]-r[1]) / ((float)r[3]-r[0]);
		float ntests = ALPHA / expected;
		if (ntests > MAX_TESTS)
			goto rep;

		size_t cnt = 0;
		for (j = 0; j < (size_t)ntests; ++j) {
			float x;
#if 0
			//do { x = dist_uniformf(prng32_romu_quad(&prng32)); } while (x < r[0] || x > r[3]);
			x = dist_uniformf(prng32_romu_quad(&prng32)) * (r[3] - r[0]) + r[0];
#else
			//do { x = dist_uniformf_exact(0, 1, prng32_romu_quad, &prng32); } while (x < r[0] || x > r[3]);
			x = dist_uniformf_exact(r[0], r[3], prng32_romu_quad, &prng32);
#endif
			if (x >= r[1] && x <= r[2])
				++cnt;
		}

		float got = (float)cnt / ntests;
		printf("%.2f\n", (expected - got) / expected);
	}
}
