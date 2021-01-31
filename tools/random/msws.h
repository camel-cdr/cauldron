#include <stdint.h>

#ifndef MSWS_H_INCLUDED

#ifdef UINT64_MAX
typedef struct { uint64_t x, w; } MsWs32_64bit;
static uint32_t
msws32_64bit(void *rng)
{
	MsWs32_64bit *r = rng;
	r->x = r->x*r->x + (r->w += 0xB5AD4ECEDA1CE2A9);
	return r->x = (r->x >> 32) | (r->x << 32);
}
#endif

#if (__SIZEOF_INT128__ && UINT64_MAX)
typedef struct { __uint128_t x, w; } MsWs64_128bit;
static uint64_t
msws64_128bit(void *rng)
{
	MsWs64_128bit *r = rng;
	r->x = r->x*r->x + (r->w += (__uint128_t)0x918FBA1EFF8E67E1 << 64 |
	                                         0x8367589D496E8AFD);
	return r->x = (r->x >> 64) | (r->x << 64);
}
#endif

#ifdef UINT32_MAX
typedef struct { uint64_t x1, x2, w1, w2; } MsWs64_2x64bit;
static uint64_t
msws64_2x64bit(void *rng)
{
	MsWs64_2x64bit *r = rng;
	r->x1 = r->x1*r->x1 + (r->w1 += 0x918FBA1EFF8E67E1);
	r->x2 = r->x2*r->x2 + (r->w2 += 0x8367589D496E8AFD);
	r->x1 = (r->x1 >> 32) | (r->x1 << 32);
	r->x2 = (r->x2 >> 32) | (r->x2 << 32);
	return (r->x1 << 32) | (r->x2 & ((UINT64_C(1) << 32) - 1));
}
#endif

#define MSWS_H_INCLUDED
#endif
