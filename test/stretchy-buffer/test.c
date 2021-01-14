#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cauldron/test.h>
#include <cauldron/stretchy-buffer.h>

static int trng_write(void *ptr, size_t n);

#define CMP(a,b) (a == b)
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
#define FUNC test_llong
#define NAME "Sb(long long)"
#define T long long
#include "xtest.h"

#undef RAND
#define RAND(x) (trng_write(&x, sizeof x), isfinite(x) ? x : 42)

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
#undef CMP
#define CMP(a,b) (memcmp(a.c, b.c, sizeof a.c) == 0 && a.x == b.x && a.y == b.y)
#include "xtest.h"

union S2 { struct { char x,y,z; } s; int u; };
#define FUNC test_s2
#define NAME "Sb(union { struct { char x,y,z; } s; int u; }"
#define T union S2
#undef CMP
#define CMP(a,b) (a.s.x == b.s.y && a.s.y == b.s.y && a.s.z == b.s.z)
#include "xtest.h"


int
main(void)
{
	test_char();
	test_short();
	test_int();
	test_long();
	test_llong();
	test_float();
	test_double();
	test_ldouble();
	test_s1();
	test_s2();
	return EXIT_SUCCESS;
}

#ifdef _WIN32
#include <windows.h>
#include <ntsecapi.h>
int
trng_write(void *ptr, size_t n)
{
	unsigned char *p;
	for (p = ptr; n > ULONG_MAX; n -= ULONG_MAX, p += ULONG_MAX)
		if (!RtlGenRandom(p, n))
			return 0;
	RtlGenRandom(p, n);
	return 1;
}
#elif defined(__OpenBSD__) || defined(__CloudABI__) || defined(__wasi__)
#include <stdlib.h>
int
trng_write(void *ptr, size_t n)
{
	arc4random_buf(ptr, n);
	return 1;
}
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
int
trng_write(void *ptr, size_t n)
{
	static int urandomFd = -1;
	unsigned char *p;
	ssize_t r;

	for (p = ptr, r = 0; n > 0; n -= (size_t)r, p += r) {
		#ifdef SYS_getrandom
		if ((r = syscall(SYS_getrandom, p, n, 0)) > 0)
			continue;
		#endif
		if (urandomFd < 0)
			if ((urandomFd = open("/dev/urandom", O_RDONLY)) < 0)
				return 0;
		if ((r = read(urandomFd, p, n)) < 0)
			return 0;
	}
	return 1;
}
#else
#error "platform not supported"
#endif
