/* stretchy-buffer.h -- generic dynamic array
 *
 * Based on Sean Barrett's streachy-buffers and klib's kvec.
 *
 * Note that any arguments passed to a sb_* function macros is potentially
 * evaluated multiple times except for arguments that have the name v in the
 * code bellow.
 *
 * Example:
 * 	Sb(int) fib;
 * 	sb_init(fib);
 * 	sb_push(fib, 1);
 * 	sb_push(fib, 1);
 *
 * 	for (int i = 2; i < 32; ++i)
 * 		sb_push(fib, fib.at[i-1] + fib.at[i-2]);
 *
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef STREACHY_BUFFER_H_INCLUDED

#define Sb(T) struct { size_t _len, _cap; T *at; }

#define sb_len(a) ((const size_t)(a)._len)
#define sb_cap(a) ((const size_t)(a)._cap)
#define sb_last(a) ((a).at[(a)._len - 1])

#define sb_setlen(a,n) ((a)._len = (n), sb_setcap((a), (a)._len))
#define sb_setcap(a,n) \
	((a)._cap < (n) ? \
		((a)._cap = ((n) > ((a)._cap <<= 1) ? (n) : (a)._cap), \
		 (a).at = realloc((a).at, (a)._cap * sizeof *(a).at)) : 0)
#define sb_reserve(a,n) sb_setcap((a), (a)._cap + (n))

#define sb_init(a) sb_initcap((a), 8)
#define sb_initcap(a,n) ((a)._len = 0, (a)._cap = (n) + 8, \
                         (a).at = malloc((a)._cap * sizeof *(a).at))
#define sb_initlen(a,n) ((a)._len = (n), (a)._cap = (n) + 8, \
                         (a).at = malloc((a)._cap * sizeof *(a).at))

#define sb_push(a,v) (sb_setlen((a), (a)._len + 1), (a).at[(a)._len - 1] = (v))
#define sb_addn(a,n) sb_setlen((a), (a)._len + (n))

#define sb_pop(a) sb_popn((a), 1)
#define sb_popn(a,n) ((a)._len = ((a)._len >= (n) ? (a)._len - (n) : 0))

#define sb_free(a) free((a).at)
#define sb_shrink(a) ((a)._cap = (a)._len, \
                      (a).at = realloc((a).at, (a)._cap * sizeof *(a).at))

#define sb_rm(a,i) sb_rmn((a), (i), 1)
#define sb_rmn(a,i,n) memmove((a).at + (i), (a).at + (i) + (n), \
                               (((a)._len -= (n)) - (i)) * sizeof *(a).at)

#define sb_ins(a,i,v) (sb_insn((a), (i), 1), (a).at[i] = (v))
#define sb_insn(a,i,n) \
		(sb_addn((a), (n)), \
		 memmove((a).at + (i) + (n), (a).at + i, \
		         (sb_len(a) - (n) - (i)) * sizeof *(a).at))

#define STREACHY_BUFFER_H_INCLUDED
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
