#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <omp.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>


#include <dlfcn.h>
#include <unistd.h>


#define BUFSIZE (1uLL << 30)

PRNG64RomuQuad *prngs;

void
gen_cnt(uint64_t cnt[256], uint64_t count)
{
	uint64_t i;
	#pragma omp parallel
	{
		PRNG64RomuQuad *r = prngs + omp_get_thread_num();
		#pragma omp for reduction(+:cnt[:256])
		for (i = 0; i < count; ++i)
			++cnt[prng64_romu_quad(r) & 0xFF];
	}
}

static void
die(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

static void
usage(char *argv0)
{
	printf("usage: %s N SHARED_OBJECT_FILE \n", argv0);
	puts("Outputs 2^N bytes per seed from the lowest byte of the output from");
	puts("the following hash function, which is loaded from the SHARED_OBJECT_FILE.");
	puts("uint64_t hash(uint64_t i, uint64_t mask, uint64_t seed)");
}

int
main(int argc, char **argv)
{
	uint64_t count = 0;
	uint8_t *buf;
	uint64_t (*hash)(uint64_t i, uint64_t mask, uint64_t seed);

	if (argc != 3 || !argv[2]) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	{
		void *handle;
		int c = atoi(argv[1]);
		if (c < 1 || c > 63)
			die("%s: N out of range, expected 1 to 63, got '%d'\n",
			    argv[0], c);
		count = UINT64_C(1) << c;

		if (!(handle = dlopen(argv[2], RTLD_NOW)))
			die("%s: couldn't load shared object file '%s'\n", argv[0], argv[2]);
		if (!(hash = dlsym(handle, "hash")))
			die("%s: couldn't find the symbol 'hash' in '%s'\n", argv[0], argv[2]);
	}

	buf = malloc(BUFSIZE);
	prngs = malloc(sizeof *prngs * omp_get_max_threads());
	if (!buf || !prngs)
		die("%s: malloc failed\n", argv[0]);

	for (int i = 0; i < omp_get_max_threads(); ++i)
		prng64_romu_quad_randomize(prngs + i);

	while (1) {
		uint64_t cnt[256] = { 0 };
		uint64_t n = count;
		uint64_t mask = n-1;
		uint64_t seed = trng_u64(0);

		gen_cnt(cnt, n);
		for (int i = 1; i < 256; ++i)
			cnt[i] += cnt[i-1];
		cnt[255] = n;

		for (; n > BUFSIZE; n -= BUFSIZE) {
			#pragma omp parallel for
			for (uint64_t i = 0; i < BUFSIZE; ++i) {
				uint64_t x = hash(i, mask, seed);
				for (int j = 0; j < 256; ++j) {
					if (x < cnt[j]) {
						buf[i] = j;
						break;
					}
				}
			}
			fwrite(buf, BUFSIZE, 1, stdout);
		}

		#pragma omp parallel for
		for (uint64_t i = 0; i < n; ++i) {
			uint64_t x = hash(i, mask, seed);
			for (int j = 0; j < 256; ++j) {
				if (x < cnt[j]) {
					buf[i] = j;
					break;
				}
			}
		}
		fwrite(buf, n, 1, stdout);
	}

	return EXIT_SUCCESS;
}

