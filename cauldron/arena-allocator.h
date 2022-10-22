/* arena-allocator.h
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef ARENA_ALLOCATOR_H_INCLUDED

#include <stddef.h>

#ifndef aa_BLOCK_SIZE
#define aa_BLOCK_SIZE (16*1024)
#endif

typedef struct {
	/* blocks       current
	*   v             v
	*  {:} -> {:} -> {.} -> { } -> 0 */
	struct aa_Block *blocks;
	struct aa_Block *current;
} aa_Arena;

extern void *aa_alloc(aa_Arena *arena, size_t size);
extern void aa_dealloc(aa_Arena *arena);
extern void aa_free(aa_Arena *arena);


#define ARENA_ALLOCATOR_H_INCLUDED
#endif

/******************************************************************************/

#ifdef ARENA_ALLOCATOR_IMPLEMENT

#include <assert.h>
#include <stdlib.h>

struct aa_Block {
	struct aa_Block *next;
	unsigned char *first;
	unsigned char *ptr;
	unsigned char *end;
};

#if __STDC_VERSION__ >= 201112L
# define aa_MAX_ALIGN _Alignof(max_align_t)
#else
	union aa_MaxAlign {
		long double ld;
		long l;
		double d;
		char *p;
		int (*f)(void);
# if __STDC_VERSION__ >= 199901L
		long long ll;
# endif
	};
# define aa_MAX_ALIGN sizeof(union aa_MaxAlign)
#endif

void *
aa_alloc(aa_Arena *arena, size_t size)
{
#define aa_ALIGN_UP(x, n) (((x) + (n) - 1) & ~((n) - 1))

	struct aa_Block *it, *prev;
	size_t old = size;

	size = aa_ALIGN_UP(size, aa_MAX_ALIGN);
	it = prev = arena->current;

	/* find the first block with enough space */
	assert(it ? it->end >= it->ptr : 1);
	while (it && size > (size_t)(it->end - it->ptr))
		prev = it, it = it->next;

	if (it) {
		/* size fits in a block */
		arena->current = it;
		it->ptr += size;
	} else {
		/* needs to be allocated */
		size_t n = sizeof *it + size + aa_BLOCK_SIZE;
		size_t an = aa_ALIGN_UP(n, aa_MAX_ALIGN);
		it = malloc(an);
		if (arena->current)
			arena->current = prev->next = it;
		else
			arena->current = arena->blocks = it;
		it->next = 0;
		it->first = (unsigned char *)it + sizeof *it + (an - n);
		it->ptr = it->first + size;
		it->end = (unsigned char*)it + an;
	}

	return it->ptr - old;

#undef aa_ALIGN_UP
}

void
aa_dealloc(aa_Arena *arena)
{
	struct aa_Block *it = arena->blocks;

	/* reset block->ptr to the beginning of the block */
	while (it) {
		it->ptr = it->first;
		it = it->next;
	}

	arena->current = arena->blocks;
}

void
aa_free(aa_Arena *arena)
{
	struct aa_Block *it = arena->blocks;

	/* free all blocks */
	while (it) {
		struct aa_Block *b = it;
		it = it->next;
		free(b);
	}

	arena->current = arena->blocks = 0;
}

#undef ARENA_ALLOCATOR_IMPLEMENT
#endif

/*
 * Copyright (c) 2022 Olaf Berstein
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

