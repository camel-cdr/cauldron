#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/stretchy-buffer.h>
#include <cauldron/test.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define EQ(a,b) (a == b)
#define RAND(x) (trng_write(&x, sizeof x), x)

#define FUNC test_char
#define NAME "Sb(char)"
#define T char
#include "xtest.h"
#define FUNC test_short
#define NAME "Sb(short)"
#define T short
#include "xtest.h"
#define FUNC test_int
#define NAME "Sb(int)"
#define T int
#include "xtest.h"
#define FUNC test_long
#define NAME "Sb(long)"
#define T long
#include "xtest.h"

#undef RAND
#define RAND(x) (x = dist_uniform(trng_u64(0)))

#define FUNC test_float
#define NAME "Sb(float)"
#define T float
#include "xtest.h"
#define FUNC test_double
#define NAME "Sb(double)"
#define T double
#include "xtest.h"
#define FUNC test_ldouble
#define NAME "Sb(long double)"
#define T long double
#include "xtest.h"

#undef RAND
#define RAND(x) (trng_write(&x, sizeof x), x)

struct S1 { char c[42]; size_t x, y; };
#define FUNC test_s1
#define NAME "Sb(struct { char c[42]; size_t a, b; })"
#define T struct S1
#undef EQ
#define EQ(a,b) (memcmp(&a.c, &b.c, sizeof a.c) == 0 && a.x == b.x && a.y == b.y)
#include "xtest.h"

union S2 { struct { char x,y,z; } s; int u; };
#define FUNC test_s2
#define NAME "Sb(union { struct { char x,y,z; } s; int u; })"
#define T union S2
#undef EQ
#define EQ(a,b) (a.s.x == b.s.x && a.s.y == b.s.y && a.s.z == b.s.z)
#include "xtest.h"


int
main(void)
{
	test_char();
	test_short();
	test_int();
	test_long();
	test_float();
	test_double();
	test_ldouble();
	test_s1();
	test_s2();
	return 0;
}

