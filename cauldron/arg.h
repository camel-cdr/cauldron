/* arg.h -- A POSIX compliant argument parser based on plan9's arg(3)
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef ARG_H_INCLUDED

static int
ARG_LONG_func(char **argv0, char const *name)
{
	char *argIt = *argv0;
	while (*argIt == *name && *argIt)
		argIt++, name++;
	if (*argIt == *name || (*argIt == '=' && !*name)) {
		*argv0 = argIt;
		return 1;
	}
	return 0;
}

#define ARG_BEGIN do { \
	for (argv[0] && (--argc, ++argv); \
	     argv[0] && argv[0][0] == '-'; \
	     argv[0] && (--argc, ++argv)) { \
		int isFlag = 1; \
		char *arg = argv[0]; \
		if (arg[1] == '-' && arg[2] == 0 && (--argc, ++argv, 1)) \
			break; \
		ARG_BEGIN_REP: \
		switch ((++arg)[0]) { \
		case '-': \
			isFlag = 0; \
			if (arg[-1] == '-') \
				++arg;

#define ARG_LONG(name) ARG_LONG_func(&(arg), (name))

#define ARG_VAL() \
		(isFlag ? (arg[1] ? ++arg : *(--argc, ++argv)) : \
		          (arg[0] == '=' ? ++arg : *(--argc, ++argv)))

#define ARG_FLAG() if (isFlag && arg[1]) goto ARG_BEGIN_REP

#define ARG_END } } } while(0)

#define ARG_H_INCLUDED
#endif


/*
 * Example:
 */

#if defined(ARG_EXAMPLE) || defined(ARG_FUZZ)

#include <stdlib.h>
#include <stdio.h>

#ifdef ARG_FUZZ
#define main argMain
#undef stderr
#define stderr stdout
#endif


int
main(int argc, char **argv)
{
	char *argv0 = argv[0];
	int a = 0, b = 0, c = 0, reverse = 0;
	char const *input = "default", *output = "default";
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
		} else if (ARG_LONG("help")) case 'h': case '?': {
			printf("Usage: %s [OPTION...] [STRING...]\n", argv0);
			puts("Example usage of arg.h\n");
			puts("Options:");
			puts("  -a,                set a to true");
			puts("  -b,                set b to true");
			puts("  -c,                set c to true");
			puts("  -r, --reverse      set reverse to true");
			puts("  -i, --input=STR    set input string to STR");
			puts("  -o, --output=STR   set output string to STR");
			puts("  -h, --help         display this help and exit");
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

	printf("\nargc: %d", argc);
	puts("\nargv:");
	while (*argv)
		printf("  %s\n", *argv++);

	return 0;
}

#endif /* ARG_EXAMPLE */

/*
 * Fuzzing:
 */

#ifdef ARG_FUZZ

#include <stdint.h>
#include <string.h>
#define MAX_ARGC 50000

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size < 2)
		return -1;

	char *buf = malloc(size+2);
	memcpy(buf, data, size);
	buf[size] = buf[size+1] = 0;

	static char *argv[MAX_ARGC+1];
	size_t argc = 0;
	for (char *ptr = buf; *ptr && argc < MAX_ARGC;) {
		argv[argc++] = ptr;
		while (*ptr++);
	}
	argv[argc] = 0;

	main(argc, argv);
	free(buf);

	return 0;
}

#endif


/*
 * Copyright (c) 2021 Olaf Berstein
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

