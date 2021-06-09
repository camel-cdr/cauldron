#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/test.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE (1024)
#define N (128)

#define ALPHA (0.5)

int
comp_size_t(void const *lhs, void const *rhs)
{
	size_t const *l = lhs, *r = rhs;
	return *l - *r;
}

size_t
validate_shuffle(size_t *arr, size_t *sorted, size_t size)
{
	size_t i, cnt;
	/* we'd expect to get on average 1 element with the same position */
	for (i = cnt = 0; i < size; ++i)
		cnt += (arr[i] == sorted[i]);

	/* make sure no element is lost */
	qsort(arr, size, sizeof *arr, comp_size_t);
	for (i = 0; i < size; ++i) {
		TEST_ASSERT(arr[i] == sorted[i]);
	}
	return cnt;
}

int
main(void)
{
	size_t i, j, cnt, size;
	PRNG32RomuQuad prng32;
	PRNG64RomuQuad prng64;
	size_t *arr = malloc(MAX_SIZE * sizeof *arr);
	size_t *sorted = malloc(MAX_SIZE * sizeof *arr);
	prng32_romu_quad_randomize(&prng32);
	prng64_romu_quad_randomize(&prng64);

	for (i = 0; i < MAX_SIZE; ++i)
		sorted[i] = arr[i] = i;

	TEST_BEGIN("shuf32_arr");
	for (cnt = i = 0; i < N; ++i) {
		size = dist_uniform_u32(MAX_SIZE-2, prng32_romu_quad, &prng32)+2;
		shuf32_arr(arr, size, sizeof *arr, prng32_romu_quad, &prng32);
		cnt += validate_shuffle(arr, sorted, size);
	}
	TEST_ASSERT((float)cnt / N - 1.0 < ALPHA);
	TEST_END();

	TEST_BEGIN("shuf64_arr");
	for (cnt = i = 0; i < N; ++i) {
		size = dist_uniform_u64(MAX_SIZE-2, prng64_romu_quad, &prng64)+2;
		shuf64_arr(arr, size, sizeof *arr, prng64_romu_quad, &prng64);
		cnt += validate_shuffle(arr, sorted, size);
	}
	TEST_ASSERT((float)cnt / N - 1.0 < ALPHA);
	TEST_END();

	TEST_BEGIN("shuf_weyl")
	for (cnt = i = 0; i < N; ++i) {
		ShufWeyl weyl;
		size = dist_uniform_u64(MAX_SIZE-2, prng64_romu_quad, &prng64)+2;
		shuf_weyl_randomize(&weyl, size);

		for (i = 0; i < size; ++i) {
			j = shuf_weyl(&weyl);
			TEST_ASSERT(j < size);
			arr[j] = sorted[i];
		}

		cnt += validate_shuffle(arr, sorted, size);
	}
	TEST_ASSERT((float)cnt / N - 1.0 < ALPHA);
	TEST_END();

	TEST_BEGIN("shuf_lcg")
	for (cnt = i = 0; i < N; ++i) {
		ShufLcg lcg;
		size = dist_uniform_u64(MAX_SIZE-2, prng64_romu_quad, &prng64)+2;
		shuf_lcg_randomize(&lcg, size);

		for (i = 0; i < size; ++i) {
			j = shuf_lcg(&lcg);
			TEST_ASSERT(j < size);
			arr[j] = sorted[i];
		}

		cnt += validate_shuffle(arr, sorted, size);
	}
	TEST_ASSERT((float)cnt / N - 1.0 < ALPHA);
	TEST_END();

	free(sorted);
	free(arr);

	return EXIT_SUCCESS;
}
