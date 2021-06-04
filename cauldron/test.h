/* test.h -- minimal unit testing
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef TEST_H_INCLUDED

static unsigned test__nasserts, test__nfailures;

#define TEST_BEGIN(name) \
	test__nasserts = test__nfailures = 0; \
	printf("Testing %s ... ", name);

#define TEST_END() \
	if (test__nfailures == 0) { \
		puts("PASSED"); \
	} else { \
		printf("\t-> %u assertions, %u failures\n", \
			test__nasserts, test__nfailures); \
		exit(EXIT_FAILURE); \
	}

#define TEST_ASSERT(cnd) TEST_ASSERT_MSG((cnd), (#cnd))
#define TEST_ASSERT_MSG(cnd, msg) \
	do { \
		if (!(cnd)) { \
			if (test__nfailures++ == 0) puts("FAILED"); \
			printf("\t%s:%d:\n", __FILE__, __LINE__); \
			printf msg; \
			putchar('\n'); \
		} \
		++test__nasserts; \
	} while (0)

#define TEST_H_INCLUDED
#endif

/*
 * Example:
 */

#ifdef TEST_EXAMPLE

#include <stdio.h>
#include <stdlib.h>


int
main(void)
{
	TEST_BEGIN("Testing a");
	TEST_ASSERT(4 == 4);
#if 0
	TEST_ASSERT(3.141592 == 2.718281828);
	TEST_ASSERT(42 == 69);
#endif
	TEST_ASSERT(3 == 1+2);
	TEST_END();

	TEST_BEGIN("Testing b");
	TEST_ASSERT("test"[3] == 't');
	TEST_END();

	return 0;
}

#endif /* TEST_EXAMPLE */

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

