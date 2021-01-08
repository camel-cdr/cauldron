#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STREACHY_BUFFER_IMPLEMENT
#include <cauldron/stretchy-buffer.h>

static int trng_write(void *ptr, size_t n);

int
main(void)
{
	puts("\nstretchy-buffer:");

	#define CMP(a,b) (a == b)
	#define RAND(x) (trng_write(&x, sizeof x), x)

	#define T char
	puts("char");
	#include "xtest.h"
	#define T short
	puts("short");
	#include "xtest.h"
	#define T int
	puts("int");
	#include "xtest.h"
	#define T long
	puts("long");
	#include "xtest.h"
	#define T long long
	puts("long long");
	#include "xtest.h"

	#undef RAND
	#define RAND(x) (trng_write(&x, sizeof x), isfinite(x) ? x : 42)

	#define T float
	puts("float");
	#include "xtest.h"
	#define T double
	puts("double");
	#include "xtest.h"
	#define T double
	puts("long double");
	#include "xtest.h"

	#undef RAND
	#define RAND(x) (trng_write(&x, sizeof x), x)

	struct S1 { char c[42]; size_t x, y; };
	#define T struct S1
	puts("struct { char c[42]; size_t a, b; };");
	#undef CMP
	#define CMP(a,b) (memcmp(a.c, b.c, sizeof a.c) == 0 && a.x == b.x && a.y == b.y)
	#include "xtest.h"

	struct S2 { union { struct { char x,y,z; } s; int u; } u; float c, d; };
	#define T struct S2
	puts("struct S2 { union { struct { char x,y,z; } s; int u; } u; float c, d; };");
	#undef CMP
	#define CMP(a,b) \
		(a.u.s.x == b.u.s.y && a.u.s.y == b.u.s.y && \
		 a.u.s.z == b.u.s.z && a.c == b.c && a.d == b.d)
	#include "xtest.h"

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
