#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cauldron/random.h>

#include "msws.h"

/*
 * Example:
 * ./rng <name> | ./testu01 SmallCrush
 * ./rng <name> | ./RNG_test stdin
 */

#define BUFSIZE (1024*1024)
void *buffer;
void *bufferend;

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

#undef MAKE_RNG

#define MAKE_RNG(func, type, n) \
	static void \
	run_##func(void) \
	{ \
		type rng; \
		trng_write(&rng, sizeof rng); \
		while (1) { \
			uint##n##_t *p = buffer; \
			while ((void*)p < bufferend) \
				*p++ = func(&rng); \
			fwrite(buffer, 1, BUFSIZE, stdout); \
		} \
	}

MAKE_RNG(msws32_64bit, MsWs32_64bit, 32);
MAKE_RNG(msws64_128bit, MsWs64_128bit, 64);
MAKE_RNG(msws64_2x64bit, MsWs64_2x64bit, 64);

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
#define MAKE_RNG(func, type, rand, n) \
	{ #func, run_##func },
#include <cauldron/random-xmacros.h>
#undef MAKE_RNG
	{ "msws32_64bit", run_msws32_64bit },
	{ "msws64_128bit", run_msws64_128bit },
	{ "msws64_2x64bit", run_msws64_2x64bit },
	{ "trng_write", run_trng_write },
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

/*
--------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
--------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Olaf Berstein
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
--------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <http://unlicense.org/>
--------------------------------------------------------------------------------
*/
