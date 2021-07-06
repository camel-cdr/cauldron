/* stretchy-buffer.h -- generic dynamic array
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 *
 * Inspired by Sean Barrett's stretchy-buffers and klib's kvec.
 *
 * Note that any arguments passed to a sb_* function macros is potentially
 * evaluated multiple times except for arguments that have the name v in the
 * code bellow.
 */

#ifndef STRETCHY_BUFFER_H_INCLUDED

/* can be zero initialized */
#define Sb(T) struct { T *at; size_t _len, _cap; }

#define sb_len(a) ((size_t const)(a)._len)
#define sb_cap(a) ((size_t const)(a)._cap)

#define sb_initcap(a,n) ((a)._len = 0, (a)._cap = (n), \
                         (a).at = malloc((a)._cap * sizeof *(a).at))
#define sb_initlen(a,n) ((a)._len = (a)._cap = (n), \
                         (a).at = malloc((a)._cap * sizeof *(a).at))

#define sb_cpy(dest, src) \
		((dest)._len = (src)._len, (dest)._cap = (src)._cap, \
		 (dest).at = malloc((dest)._cap * sizeof *(dest).at), \
		 memcpy((dest).at, (src).at, (dest)._cap * sizeof *(dest).at))

#define sb_setlen(a,n) ((a)._len = (n), sb_setcap((a), (a)._len))
#define sb_setcap(a,n) ((a)._cap < (n) ? \
		((a)._cap <<= 1, (((n) > (a)._cap) ? (a)._cap = (n) : 0), \
		(a).at = realloc((a).at, (a)._cap * sizeof *(a).at)) : 0)
#define sb_reserve(a,n) sb_setcap((a), (a)._cap + (n))

#define sb_push(a,v) (sb_setlen((a), (a)._len + 1), (a).at[(a)._len - 1] = (v))
#define sb_addn(a,n) sb_setlen((a), (a)._len + (n))

#define sb_free(a) (free((a).at), (a).at = 0, (a)._len = (a)._cap = 0)
#define sb_shrink(a) ((((a)._cap = (a)._len) == 0) ? sb_free(a), 0 : \
                      ((a).at = realloc((a).at, (a)._cap * sizeof *(a).at), 0))

/* n <= sb_len  && n > 0*/
#define sb_popn(a,n) (assert((n) <= (a)._len && (n) > 0), (a)._len -= (n))
#define sb_pop(a) (sb_popn((a), 1))

/* n + i <= sb_len && i >= 0 && n > 0 */
#define sb_rmn(a,i,n) (assert((n) + (i) <= (a)._len && (i) >= 0 && (n) > 0), \
                       memmove((a).at + (i), (a).at + (i) + (n), \
                               (((a)._len -= (n)) - (i)) * sizeof *(a).at))
#define sb_rm(a,i) sb_rmn((a), (i), 1)

/* faster rm, that doesn't preserve order */
/* n + i <= sb_len && i >= 0 && n > 0*/
#define sb_rmn_unstable(a,i,n) \
		(assert((n) + (i) <= (a)._len && (i) >= 0 && (n) > 0), \
		 memmove((a).at + (i), \
			 (a).at + ((a)._len -= (n)) - 1, \
			 (n) * sizeof *(a).at))
#define sb_rm_unstable(a,i) ((a).at[i] = (a).at[(a)._len -= 1])

/* 0 <= i <= sb_len */
#define sb_insn(a,i,n) (assert(0 <= (i) && (i) <= (a)._len), \
                        (sb_addn((a), (n)), \
                         memmove((a).at + (i) + (n), (a).at + (i), \
                         ((a)._len - (n) - (i)) * sizeof *(a).at)))
#define sb_ins(a,i,v) (sb_insn((a), (i), 1), (a).at[i] = (v))

#define STRETCHY_BUFFER_H_INCLUDED
#endif

/*
 * Example:
 */

#ifdef STRETCHY_BUFFER_EXAMPLE

#include <stdlib.h>
#include <string.h>

int
main(void)
{
	/* You could also typedef Sb(int) to be able to pass it to functions */
	int i;
	Sb(int) fib = { 0 };
	sb_push(fib, 1);
	sb_push(fib, 1);

	for (i = 2; i < 32; ++i)
		sb_push(fib, fib.at[i-1] + fib.at[i-2]);

	return 0;
}

#endif /* STRETCHY_BUFFER_EXAMPLE */


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

