#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>

#include <cauldron/random.h>
#include <cauldron/arg.h>

/*
 * Example:
 * ./rng <name> | ./dist -n 100000 n32 | ./freq.gp
 */

static char *argv0;

static uint32_t
rand32(void *arg)
{
	(void)arg;
	uint32_t x;
	fread(&x, sizeof x, 1, stdin);
	return x;
}

static uint64_t
rand64(void *arg)
{
	(void)arg;
	uint64_t x;
	fread(&x, sizeof x, 1, stdin);
	return x;
}

static float randf32(void) { return dist_uniformf(rand32(0)); }
static double randf64(void) { return dist_uniform(rand64(0)); }

static void
usage(void)
{
	printf("Usage: %s [OPTION...] DISRIBUTION\n", argv0);
	puts("Distribute random data from standard input according to DISRIBUTION.\n");

	puts("Options:");
	puts("  -p, --float-precision=NUM   print float with NUM decimal places");
	puts("  -n, -c, --count=NUM         quit after NUM outputs");
	puts("  -h, --help                  display this help and exit\n");

	puts("Distributions:");
	puts("  u32 [MAX]   uniform unsigned 32-bit integers");
	puts("  u64 [MAX]   uniform unsigned 64-bit integers");
	puts("  f32   uniform 32-bit floating point");
	puts("  f64   uniform 64-bit floating point");
	puts("  n32   normal distributed 32-bit floating point");
	puts("  n64   normal distributed 64-bit floating point");
	puts("  n32z   normal distributed 32-bit floating point");
	puts("          using the ziggurat method");
	puts("  n64z   normal distributed 64-bit floating point");
	puts("          using the ziggurat method");
}

int
main(int argc, char **argv)
{
	int precision = 6;
	size_t count = SIZE_MAX;

	argv0 = argv[0];

	freopen(0, "rb", stdin);


	ARG_BEGIN {
		if (ARG_LONG("help")) case 'h': case '?': {
			usage();
			return EXIT_SUCCESS;
		} else if (ARG_LONG("count")) case 'n': case 'c': {
			sscanf(ARG_VAL(), "%zu", &count);
		} else if (ARG_LONG("float-precision")) case 'p': {
			precision = atoi(ARG_VAL());
		} else default: {
			fprintf(stderr,
			        "%s: invalid option '%s'\n"
			        "Try '%s --help' for more information.\n",
			        argv0, *argv, argv0);
			return EXIT_FAILURE;
		}
	} ARG_END;

	if (argc < 1) {
		usage();
		return EXIT_FAILURE;
	}

	for (; argv[0]; --argc, ++argv) {
		/*  */ if (strcmp(*argv, "u32") == 0) {
			uint32_t max = UINT32_MAX;
			if ((argv)[1])
				sscanf((++argv)[0], "%"PRIu32, &max);
			while (count--)
				printf("%"PRIu32"\n", dist_uniform_u32(max, rand32, 0));
		} else if (strcmp(*argv, "u64") == 0) {
			uint64_t max = UINT64_MAX;
			if ((argv)[1])
				sscanf((++argv)[0], "%"PRIu64, &max);
			while (count--)
				printf("%"PRIu64"\n", dist_uniform_u64(max, rand64, 0));
		} else if (strcmp(*argv, "f32") == 0) {
			while (count--)
				printf("%.*g\n", precision, dist_uniformf(rand32(0)));
		} else if (strcmp(*argv, "f64") == 0) {
			while (count--)
				printf("%.*g\n", precision, dist_uniform(rand64(0)));
		} else if (strcmp(*argv, "n32") == 0) {
			while (count--)
				printf("%.*g\n", precision, dist_normalf(rand32, 0));
		} else if (strcmp(*argv, "n64") == 0) {
			while (count--)
				printf("%.*g\n", precision, dist_normal(rand64, 0));
		} else if (strcmp(*argv, "n32z") == 0) {
			DistNormalfZig zig;
			dist_normalf_zig_init(&zig);
			while (count--)
				printf("%.*g\n", precision, dist_normalf_zig(&zig, rand32, 0));
		} else if (strcmp(*argv, "n64z") == 0) {
			DistNormalZig zig;
			dist_normal_zig_init(&zig);
			while (count--)
				printf("%.*g\n", precision, dist_normal_zig(&zig, rand64, 0));
		} else {
			fprintf(stderr, "%s: invalid distribution -- '%s'\n", argv0, *argv);
			fprintf(stderr, "Try '%s --help' for more information.\n", argv0);
			return EXIT_FAILURE;
		}
	}

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
