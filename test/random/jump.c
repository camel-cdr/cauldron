#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/random-xoroshiro128-jump.h>
#include <cauldron/test.h>

#include <stdio.h>
#include <stdlib.h>

#define MASK UINT16_MAX

int
main(void)
{
#define TEST(type, randomize, func, jump, args, n) do { \
		size_t j; \
		type a, b; \
		randomize(&a); \
		b = a; \
		jump args; \
		for (j = 0; j < n; ++j) \
			func(&b); \
		for (j = 0; j < 32; ++j) \
			TEST_ASSERT(func(&a) == func(&b)); \
	} while (0)

	size_t i;
	PRNG64RomuQuad prng64;
	prng64_romu_quad_randomize(&prng64);

	TEST_BEGIN("prng64_pcg_jump");
	for (i = 0; i < 25; ++i) {
		size_t n = prng64_romu_quad(&prng64) & MASK;
		TEST(PRNG32Pcg, prng32_pcg_randomize, prng32_pcg,
		     prng32_pcg_jump, (&a, n), n);
	}
	TEST_END();

	#if PRNG64_PCG_AVAILABLE
		TEST_BEGIN("prng64_pcg_jump");
		for (i = 0; i < 25; ++i) {
			uint64_t by[2] = { 0 };
			by[1] = prng64_romu_quad(&prng64) & MASK;
			TEST(PRNG64Pcg, prng64_pcg_randomize, prng64_pcg,
			     prng64_pcg_jump, (&a, by), by[1]);
		}
		TEST_END();
	#endif

	TEST_BEGIN("prng64_xoroshiro128_jump");
	for (i = 7; i <= 20; ++i) {
		TEST(PRNG64Xoroshiro128, prng64_xoroshiro128_randomize,
			prng64_xoroshiro128ss, prng64_xoroshiro128_jump,
			(&a, prng64_xoroshiro128Jump2Pow[i]), UINT64_C(1) << i);
	}
	TEST_END();
	/* The other xorshift prngs jump to far to test */

	return EXIT_SUCCESS;
}
