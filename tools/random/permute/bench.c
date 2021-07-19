#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/time.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/arg.h>


static void
die(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

#define N (1uLL << 30)

int
main(int argc, char **argv)
{

	char *argv0 = argv[0];

	for (char **it = argv+1; *it; ++it) {
		uint64_t (*hash)(uint64_t i, uint64_t mask, uint64_t seed) = 0;

		void *handle;
		if (!(handle = dlopen(*it, RTLD_NOW)))
			die("%s: couldn't load shared object file '%s'\n", argv0, *it);
		if (!(hash = dlsym(handle, "hash")))
			die("%s: couldn't find the symbol 'hash' in '%s'\n", argv0, *it);

		uint64_t seed = trng_u64(0);
		uint64_t sum = 0;

		struct timespec beg, end;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &beg);
		for (uint64_t i = 0; i < N; ++i)
			sum += hash(i, N-1, seed);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

		printf("%s took: %f ns/hash\n", *it, ((end.tv_sec - beg.tv_sec) * 1000000000 + (end.tv_nsec - beg.tv_nsec)) * 1.0 / N);
	}


	return 0;
}
