#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <TestU01.h>

struct Battery {
	void (*f)(unif01_Gen *gen);
	char *name, *desc;
} batteries[] = {
	{ bbattery_SmallCrush, "SmallCrush", "takes ~1m" },
	{ bbattery_Crush,      "Crush",      "takes ~1h" },
	{ bbattery_BigCrush,   "BigCrush",   "takes ~8h" },
};

void usage(char *argv0)
{
	size_t i;
	printf("usage: %s BATTERY\n", argv0);
	puts("Reads data stream from standart input.");
	puts("Batteries:");
	for (i = 0; i < sizeof batteries / sizeof *batteries; ++i)
		printf("\t'%s': %s\n", batteries[i].name, batteries[i].desc);
	puts("Execution time tested on an AMD Athlon 64 4000+ Processor");
	puts("with a clock speed of 2400 MHz running Linux.");
}

uint32_t next(void)
{
	uint32_t res;
	fread(&res, sizeof res, 1, stdin);
	return res;
}

int main(int argc, char **argv)
{
	char **it = argv + 1;

	if (!*it || argc <= 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	freopen(0, "rb", stdin);

	unif01_Gen* gen = unif01_CreateExternGenBits("stdin", next);

	do {
		size_t i;
		for (i = 0; i < sizeof batteries / sizeof *batteries; ++i) {
			if (strcmp(*it, batteries[i].name) == 0) {
				batteries[i].f(gen);
				fflush(stdout);
				break;
			}
		}
		if (i == sizeof batteries / sizeof *batteries) {
			fprintf(stderr, "%s error: Unknown test battery '%s'\n\n",
					argv[0], *it);
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	} while (*++it);

	unif01_DeleteExternGenBits(gen);
	return EXIT_SUCCESS;
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
