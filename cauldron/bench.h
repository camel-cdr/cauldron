/* arg.h -- A POSIX compliant argument parser based on plan9's arg(3)
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef BENCH_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>

#if !defined(__pnacl__) && !defined(__EMSCRIPTEN__) && \
    (defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER))

/* artificial use of all of memory */
# define BENCH_CLOBBER() asm volatile("":::"memory")
/* artificial dependency of x on all of memory and all of memory on x */
# define BENCH_VOLATILE(x) asm volatile("" : "+g"(x) : "g"(x) : "memory")
# define BENCH_VOLATILE_REG(x) asm volatile("" : "+r"(x) : "r"(x) : "memory")
# define BENCH_VOLATILE_MEM(x) asm volatile("" : "+m"(x) : "m"(x) : "memory")

#else

# if defined(_MSC_VER)
#  pragma optimize("", off)
static inline void bench__use_ptr(void const volatile *x) {}
/* artificial use of all of memory */
# define BENCH_CLOBBER() _ReadWriteBarrier()
#  pragma optimize("", on)
# else
static void bench_use_ptr(char const volatile *x) {}
/* artificial use of all of memory */
# define BENCH_CLOBBER()
# endif

/* artificial use of all of memory dependent on x */
# define BENCH_CLOBBER_WITH(x) (bench__use_ptr(&(x)), BENCH_CLOBBER())
# define BENCH_CLOBBER_WITH_REG(x) (bench__use_ptr(&(x)), BENCH_CLOBBER())
# define BENCH_CLOBBER_WITH_MEM(x) (bench__use_ptr(&(x)), BENCH_CLOBBER())

#endif


typedef struct {
	size_t count;
	double min, mean, M2;
	char const *title;
} BenchRecord;

typedef struct {
	size_t count, cap;
	BenchRecord *records;
	/* temporaries */
	size_t i;
	clock_t t;
} Bench;

static Bench benchInternal;

#define BENCH(title, warmup, samples) \
	for (bench_append(title), \
	     benchInternal.i = (warmup) + (samples); \
	     (benchInternal.t = clock()), benchInternal.i--; \
	     benchInternal.i < (samples) ? \
	     bench_update((clock()-benchInternal.t)*1./CLOCKS_PER_SEC),0 : 0)

static inline void
bench_append(char const *title)
{
	Bench *b = &benchInternal;
	BenchRecord *r;
	if (b->count >= b->cap) {
		b->cap = (b->cap << 1) + 1;
		b->records = (BenchRecord *)
		              realloc(b->records, b->cap * sizeof *b->records);
	}
	r = &b->records[b->count++];
	r->mean = r->M2 = r->count = 0;
	r->min = DBL_MAX;
	r->title = title;
}

static inline void
bench_update(double time)
{
	BenchRecord *r = &benchInternal.records[benchInternal.count-1];
	double const delta = time - r->mean;
	r->mean += delta / ++r->count;
	r->M2 += delta * (time - r->mean);
	if (time < r->min)
		r->min = time;
}

static inline int
bench_record_cmp(void const *lhs, void const *rhs)
{
	BenchRecord const *l = (BenchRecord const *)lhs;
	BenchRecord const *r = (BenchRecord const *)rhs;
	return l->mean > r->mean ? 1 : -1;
}

static inline void
bench_done(void)
{
	size_t i, j, maxlen;
	double minmean = DBL_MAX;
	Bench *b = &benchInternal;
	qsort(b->records, b->count, sizeof *b->records, bench_record_cmp);

	for (maxlen = i = 0; i < b->count; ++i) {
		size_t l = strlen(b->records[i].title);
		if (l > maxlen)
			maxlen = l;
		if (b->records[i].mean < minmean)
			minmean = b->records[i].mean;
	}

	for (i = 0; i < b->count; ++i) {
		int l = printf("%s:", b->records[i].title) - 4;

		for (j = 0; j < maxlen-l; ++j)
			putchar(' ');

		printf("mean: %.9e,   stddev: %.2e,   min: %.9e \n",
		       b->records[i].mean / minmean,
		       sqrt(b->records[i].M2 / b->records[i].count) / minmean,
		       b->records[i].min);
	}
	b->count = 0;
}

static inline void
bench_free(void)
{
	free(benchInternal.records);
}


static inline uint64_t
bench_hash64(uint64_t x)
{
	x ^= x >> 30;
	x *= UINT64_C(0xBF58476D1CE4E5B9);
	x ^= x >> 27;
	x *= UINT64_C(0x94D049BB133111EB);
	x ^= x >> 31;
	return x;
}

#define BENCH_H_INCLUDED
#endif

/*
 * Example:
 */

#ifdef BENCH_EXAMPLE

#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	size_t i;
	BENCH("sum", 8, 64) {
		unsigned int sum = 0;
		for (i = 0; i < 1024*16u; ++i) {
			sum += i;
			BENCH_VOLATILE_REG(sum);
		}
	}
	BENCH("product", 8, 64) {
		unsigned char sum = 0;
		for (i = 0; i < 1024*16u; ++i) {
			sum *= i;
			BENCH_VOLATILE_REG(sum);
		}
	}
	bench_done();
	return 0;
}

#endif /* BENCH_EXAMPLE */

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

