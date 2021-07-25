#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>

#include "extra.h"

/*
 * Example:
 * ./rng <name> | ./testu01 SmallCrush
 * ./rng <name> | ./RNG_test stdin
 */

#define BUFSIZE (1024*1024)
void *buffer;
void *bufferend;

#define RANDOM_X16(type, func, rand) \
	MAKE_RNG(func, type, rand, 16)
#define RANDOM_X32(type, func, rand) \
	MAKE_RNG(func, type, rand, 32)
#define RANDOM_X64(type, func, rand) \
	MAKE_RNG(func, type, rand, 64)

#define MAKE_RNG(func, type, rand, n) \
	static void \
	run_##func(void) \
	{ \
		type rng; \
		rand(&rng); \
		while (1) { \
			uint##n##_t *p = buffer; \
			while ((void*)p < bufferend) \
				*p++ = func(&rng); \
			fwrite(buffer, 1, BUFSIZE, stdout); \
		} \
	}

#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef MAKE_RNG


static void
run_trng_write(void)
{
	while (1) {
		trng_write(buffer, BUFSIZE);
		fwrite(buffer, 1, BUFSIZE, stdout);
	}
}

static struct {
	char *name;
	void (*rng)(void);
} rngs[] = {
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
#define RANDOM_X16(type, func, rand) { #func, run_##func },
#define RANDOM_X32(type, func, rand)
#define RANDOM_X64(type, func, rand)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
#define RANDOM_X16(type, func, rand)
#define RANDOM_X32(type, func, rand) { #func, run_##func },
#define RANDOM_X64(type, func, rand)
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
#undef RANDOM_X16
#undef RANDOM_X32
#undef RANDOM_X64
#define RANDOM_X16(type, func, rand)
#define RANDOM_X32(type, func, rand)
#define RANDOM_X64(type, func, rand) { #func, run_##func },
#include <cauldron/random-xmacros.h>
#include "extra-xmacros.h"
};

static void list(void)
{
	size_t i;
	for (i = 0; i < sizeof rngs / sizeof *rngs; ++i)
		puts(rngs[i].name);
}

int
main(int argc, char **argv)
{
	size_t i;
	char *name = argv[1];

	(void)argc;

	buffer = malloc(BUFSIZE);
	bufferend = (void*)((char*)buffer + BUFSIZE);

	if (!name) {
		list();
		return EXIT_FAILURE;
	}

	for (i = 0; i < sizeof rngs / sizeof *rngs; ++ i) {
		if (strcmp(name, rngs[i].name) == 0) {
			rngs[i].rng();
			return EXIT_SUCCESS;
		}
	}
	list();
	return EXIT_FAILURE;
}
