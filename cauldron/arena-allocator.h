/* arena-allocator.h
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef ARENA_ALLOCATOR_H_INCLUDED

#define aa_BLOCK_SIZE (16*1024)

typedef struct {
	/* blocks       current
	*   v             v
	*  {:} -> {:} -> {.} -> { } -> 0 */
	struct aa_Block *blocks;
	struct aa_Block *current;
} aa_Arena;

struct aa_Block {
	struct aa_Block *next;
	unsigned char *ptr;
	unsigned char *end;
};

union aa_Align {
#if __STDC_VERSION__ >= 199901L
	long long ll;
#endif
	long double ld;
	long l;
	double d;
	char *p;
	int (*f)(void);
};

struct aa_Header {
	struct aa_Block b;
	union aa_Align a;
};

extern void *aa_alloc(aa_Arena *arena, size_t size);
extern void aa_dealloc(aa_Arena *arena);
extern void aa_free(aa_Arena *arena);


#define ARENA_ALLOCATOR_H_INCLUDED
#endif

/******************************************************************************/

#ifdef ARENA_ALLOCATOR_IMPLEMENT

void *
aa_alloc(aa_Arena *arena, size_t size)
{
	struct aa_Block *it, *prev;

#define aa_ALIGN_UP(x, n) (((x) + (n) - 1) & ~((n) - 1))
	size = aa_ALIGN_UP(size, sizeof(union aa_Align));
	it = prev = arena->current;
#undef aa_ALIGN_UP

	/* find the first block with enough space */
	while (it && it->ptr + size > it->end)
		prev = it, it = it->next;

	if (it) {
		/* size fits in a block */
		arena->current = it;
		it->ptr += size;
	} else {
		/* needs to be allocated */
		size_t n = sizeof(struct aa_Header) + size + aa_BLOCK_SIZE;
		it = malloc(n);
		if (arena->current) {
			arena->current = prev->next = it;
		} else {
			arena->current = arena->blocks = it;
		}
		it->next = 0;
		it->ptr = (unsigned char *)((struct aa_Header *)it + 1) + size;
		it->end = (unsigned char*)it + n;
	}

	return it->ptr - size;
}

void
aa_dealloc(aa_Arena *arena)
{
	struct aa_Block *it = arena->blocks;

	/* reset block->ptr to the beginning of the block */
	while (it) {
		it->ptr = (unsigned char *)((struct aa_Header *)it + 1);
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
 * Copyright (c) 2020 Olaf Berstein
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

