/* test.h
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef TEST_H_INCLUDED

#define TEST_BEGIN(func) TEST_BEGIN(func, #func)

#define TEST_BEGIN_NAME(func, name) \
	void func(void) { \
		unsigned nAsserts = 0, nFailures = 0; \
		printf("Testing %s ... ", name); \
		do {

#define TEST_END \
		} while (0);  \
		if (nFailures == 0) { \
			puts("PASSED"); \
		} else { \
			printf("\t-> %u assertions, %u failures\n", \
			       nAsserts, nFailures); \
			exit(EXIT_FAILURE); \
		} \
	}

#define TEST_ASSERT(cnd) TEST_ASSERT_MSG((cnd), (#cnd))
#define TEST_ASSERT_MSG(cnd, msg) \
	do { \
		if (!(cnd)) { \
			if (nAsserts == 0) puts("FAILED"); \
			printf("\t%s:%d: %s\n", \
			       __FILE__, __LINE__, (msg)); \
			++nFailures; \
		} \
		++nAsserts; \
	} while (0)

#define TEST_H_INCLUDED
#endif

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