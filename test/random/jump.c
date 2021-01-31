#include <cauldron/random.h>
#include <cauldron/random-xoroshiro128-jump.h>
#include <cauldron/test.h>

#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
#define TEST(type, randomize, func, jump, args, n) do { \
		TEST_BEGIN(#jump); \
		size_t i; \
		type a, b; \
		randomize(&a); \
		b = a; \
		jump args; \
		for (i = 0; i < n; ++i) \
			func(&b); \
		for (i = 0; i < 10; ++i) \
			TEST_ASSERT(func(&a) == func(&b)); \
		TEST_END(); \
	} while (0)

	size_t n = 123;
#if PRNG32_PCG_AVAILABLE
	TEST(PRNG32Pcg, prng32_pcg_randomize, prng32_pcg, prng32_pcg_jump, (&a, n), n);
#endif
#if PRNG64_PCG_AVAILABLE
	TEST(PRNG64Pcg, prng64_pcg_randomize, prng64_pcg, prng64_pcg_jump, (&a, n), n);
#endif
#if PRNG64_XORSHIFT_AVAILABLE
	TEST(PRNG64Xoroshiro128, prng64_xoroshiro128_randomize,
	     prng64_xoroshiro128ss, prng64_xoroshiro128_jump,
	     (&a, prng64_xoroshiro128Jump2Pow[8]), 1ull << 8);
#endif
	/* The other xorshift prngs jump to far to test */

	return EXIT_SUCCESS;
}
