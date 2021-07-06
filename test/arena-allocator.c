#include <stdio.h>

#define ARENA_ALLOCATOR_IMPLEMENT
#include <cauldron/arena-allocator.h>
#include <cauldron/test.h>

int
main(void)
{
	int i;
	aa_Arena a = { 0 };

	TEST_BEGIN("arena-allocator");

#define TEST_X(T, v) \
	do { \
		T *x = aa_alloc(&a, sizeof *x); \
		TEST_ASSERT(x); \
		*x = (v); \
		TEST_ASSERT(*x == (v)); \
	} while (0)

	for (i = 0; i < 3; ++i) {
		TEST_X(char, 'u');
		TEST_X(char, 'w');
		TEST_X(char, 'u');
		TEST_X(long, 31415926L);
		TEST_X(short, 420);
		TEST_X(double, 420.69);
		aa_dealloc(&a);
	}

	TEST_END();

	aa_free(&a);
}

