/* arg.h -- A posix complient argument parser based on plan9's arg(3)
 * New versions available at https://github.com/camel-cdr/cauldron */

#ifndef ARG_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

static int
ARG_LONG_func(char **lp, const char *r)
{
	char *l = *lp;
	for (; *l == *r && *l; l++, r++);
	if (*l == *r || (*l == '=' && !*r)) {
		*lp = l;
		return 1;
	}
	return 0;
}

#define ARG_BEGIN do { \
	for (--argc, ++argv; \
	     argv[0] && argv[0][0] == '-'; \
	     --argc, ++argv) { \
		int isFlag = 1; \
		if (argv[0][1] == '-' && argv[0][2] == 0 && (++argv, 1)) \
			break; \
		ARG_BEGIN_REP: \
		switch ((++argv[0])[0]) { \
		case '-': \
			isFlag = 0; \
			if (argv[0][-1] == '-') \
				++argv[0];

#define ARG_LONG(name) ARG_LONG_func(&(argv[0]), name)

#define ARG_VAL() \
		(isFlag ? (argv[0][1] ? ++argv[0] : *(--argc, ++argv)) : \
		          (argv[0][0] == '=' ? ++argv[0] : *(--argc, ++argv)))

#define ARG_FLAG() if (isFlag && argv[0][1]) goto ARG_BEGIN_REP

#define ARG_END } } } while(0)

#define ARG_H_INCLUDED
#endif

/* Example */
#ifdef ARG_EXAMPLE
int main(int argc, char **argv)
{
	char *argv0 = argv[0];
	int a = 0, b = 0, c = 0, reverse;
	char *input = "default", *output = "default";
	int readstdin = 0;

	ARG_BEGIN {
		if (0) {
			case 'a': a = 1; ARG_FLAG(); break;
			case 'b': b = 1; ARG_FLAG(); break;
			case 'c': c = 1; ARG_FLAG(); break;
			case '\0': readstdin = 1; break;
		} else if (ARG_LONG("reverse")) case 'r': {
			reverse = 1;
			ARG_FLAG();
		} else if (ARG_LONG("input")) case 'i': {
			input = ARG_VAL();
		} else if (ARG_LONG("output")) case 'o': {
			output = ARG_VAL();
			break;
		} else if (ARG_LONG("help")) case 'h': case '?': {
			puts("help");
			return EXIT_SUCCESS;
		} else { default:
			fprintf(stderr,
			        "%s: invalid option '%s'\n"
			        "Try '%s --help' for more information.\n",
			        argv0, *argv, argv0);
			return EXIT_FAILURE;
		}
	} ARG_END;

	printf("a = %s\n", a ? "true" : "false");
	printf("b = %s\n", b ? "true" : "false");
	printf("c = %s\n", c ? "true" : "false");
	printf("reverse = %s\n", reverse ? "true" : "false");
	printf("readstdin = %s\n", readstdin ? "true" : "false");
	printf("input = %s\n", input);
	printf("output = %s\n", output);

	puts("\nargv:");
	while (*argv)
		puts(*argv++);
}
#endif /* ARG_EXAMPLE */

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
