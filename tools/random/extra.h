#include <stdint.h>

/*
 * Note: These PRNGs vary in quality, don't just blindly start using them!
 */

#ifndef EXTRA_H_INCLUDED

/* Middle Square Weyl Sequence RNG: Bernard Widynski
 * https://arxiv.org/abs/1704.00358 */

typedef struct { uint32_t s[2]; } PRNG16Msws;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG16Msws, prng16_msws)
static inline uint16_t
prng16_msws(void *rng)
{
	PRNG16Msws *r = rng;
	r->s[0] = r->s[0]*r->s[0] + (r->s[1] += 0x97DEF15B);
	return r->s[0] = (r->s[0] >> 16) | (r->s[0] << 16);
}

typedef struct { uint64_t s[2]; } PRNG32Msws;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32Msws, prng32_msws)
static inline uint32_t
prng32_msws(void *rng)
{
	PRNG32Msws *r = rng;
	r->s[0] = r->s[0]*r->s[0] + (r->s[1] += 0x6BC13D2FD5B92843);
	return r->s[0] = (r->s[0] >> 32) | (r->s[0] << 32);
}

#if __SIZEOF_INT128__
typedef struct { __uint128_t s[2]; } PRNG64Msws;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Msws, prng64_msws)
static inline uint64_t
prng64_msws(void *rng)
{
	PRNG64Msws *r = rng;
	r->s[0] = r->s[0]*r->s[0] + (r->s[1] += (__uint128_t)0x79A23B1C581C2693 << 64 | 0xEAB63C54A351C269);
	return r->s[0] = (r->s[0] >> 64) | (r->s[0] << 64);
}
#endif

typedef struct { uint64_t s[4]; } PRNG64Msws64_2x32;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Msws64_2x32, prng64_msws_2x32bit)
static inline uint64_t
prng64_msws_2x32bit(void *rng)
{
	PRNG64Msws64_2x32 *r = rng;
	r->s[0] = r->s[0]*r->s[0] + (r->s[2] += 0x9126B7F4D352FCB7);
	r->s[1] = r->s[1]*r->s[1] + (r->s[3] += 0x4352BDCE94BCE365);
	r->s[0] = (r->s[0] >> 32) | (r->s[0] << 32);
	r->s[1] = (r->s[1] >> 32) | (r->s[1] << 32);
	return (r->s[0] << 32) | (r->s[1] & ((UINT64_C(1) << 32) - 1));
}


/* SFC: Chris Doty-Humphrey
 * https://sourceforge.net/projects/pracrand/ */

typedef struct { uint16_t s[4]; } PRNG16Sfc;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG16Sfc, prng16_sfc)
static inline uint16_t
prng16_sfc(void *rng)
{
	PRNG16Sfc *r = rng;
	enum { BARREL_SHIFT = 6, RSHIFT = 5, LSHIFT = 3 };
	uint16_t const tmp = r->s[0] + r->s[1] + r->s[3]++;
	r->s[0] = r->s[1] ^ (r->s[1] >> RSHIFT);
	r->s[1] = r->s[2] + (r->s[2] << LSHIFT);
	r->s[2] = ((r->s[2] << BARREL_SHIFT) | (r->s[2] >> (16 - BARREL_SHIFT))) + tmp;
	return tmp;
}

typedef struct { uint32_t s[4]; } PRNG32Sfc;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32Sfc, prng32_sfc)
static inline uint32_t
prng32_sfc(void *rng)
{
	PRNG32Sfc *r = rng;
	enum { BARREL_SHIFT = 21, RSHIFT = 9, LSHIFT = 3 };
	uint32_t const tmp = r->s[0] + r->s[1] + r->s[3]++;
	r->s[0] = r->s[1] ^ (r->s[1] >> RSHIFT);
	r->s[1] = r->s[2] + (r->s[2] << LSHIFT);
	r->s[2] = ((r->s[2] << BARREL_SHIFT) | (r->s[2] >> (32 - BARREL_SHIFT))) + tmp;
	return tmp;
}

typedef struct { uint64_t s[4]; } PRNG64Sfc;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Sfc, prng64_sfc)
static inline uint64_t
prng64_sfc(void *rng)
{
	PRNG64Sfc *r = rng;
	enum { BARREL_SHIFT = 24, RSHIFT = 11, LSHIFT = 3 };
	uint64_t const tmp = r->s[0] + r->s[1] + r->s[3]++;
	r->s[0] = r->s[1] ^ (r->s[1] >> RSHIFT);
	r->s[1] = r->s[2] + (r->s[2] << LSHIFT);
	r->s[2] = ((r->s[2] << BARREL_SHIFT) | (r->s[2] >> (64 - BARREL_SHIFT))) + tmp;
	return tmp;
}


/* tylo64: Tyge Løvset's modified SFC64
 * https://github.com/numpy/numpy/issues/16313#issuecomment-641897028 */

typedef struct {uint64_t s[4];} PRNG64Tylo;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Tylo, prng64_tylo)
static inline uint64_t
prng64_tylo(void *rng)
{
	PRNG64Tylo *r = rng;
	enum { LROT = 24, RSHIFT = 11, LSHIFT = 3 };
	uint64_t const s1 = r->s[1], out = r->s[0] ^ (r->s[2] += r->s[3]);
	r->s[0] = (s1 + (s1 << LSHIFT)) ^ (s1 >> RSHIFT);
	r->s[1] = ((s1 << LROT) | (s1 >> (64 - LROT))) + out;
	return out;
}


/* JSF: Robert Jenkins
 * https://burtleburtle.net/bob/rand/smallprng.html */

typedef struct { uint32_t s[4]; } PRNG32Jfs;
static void
prng32_jfs_randomize(void *rng)
{
	PRNG32Jfs *r = rng;
	trng_write(r->s, sizeof r->s);
	r->s[0] = 0xF1EA5EED;
}
static inline uint32_t
prng32_jfs(void *rng)
{
	PRNG32Jfs *r = rng;
	uint32_t const e = r->s[0] - ((r->s[1] << 27) | (r->s[1] >> 5));
	r->s[0] = r->s[1] ^ ((r->s[2] << 17) | (r->s[2] >> 15));
	r->s[1] = r->s[2] + r->s[3];
	r->s[2] = r->s[3] + e;
	r->s[3] = e + r->s[0];
	return r->s[3];
}

typedef struct { uint64_t s[4]; } PRNG64Jfs;
static inline void
prng64_jfs_randomize(void *rng)
{
	PRNG64Jfs *r = rng;
	trng_write(r->s, sizeof r->s);
	r->s[0] = 0xF1EA5EED;
}
static inline uint64_t
prng64_jfs(void *rng)
{
	PRNG64Jfs *r = rng;
	uint64_t const e = r->s[0] - ((r->s[1] << 39) | (r->s[1] >> 25));
	r->s[0] = r->s[1] ^ ((r->s[2] << 11) | (r->s[2] >> 53));
	r->s[1] = r->s[2] + r->s[3];
	r->s[2] = r->s[3] + e;
	r->s[3] = e + r->s[0];
	return r->s[3];
}


/* xorshift128+: Sebastiano Vigna
 * https://web.archive.org/web/20150114061109/http://xorshift.di.unimi.it/ */

typedef struct { uint64_t s[2]; } PRNG64Xorshift128p;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Xorshift128p, prng64_xorshift128p)
static inline uint64_t
prng64_xorshift128p(void *rng)
{
	PRNG64Xorshift128p *r = rng;
	uint64_t s1 = r->s[0];
	uint64_t const s0 = r->s[1];
	r->s[0] = s0;
	s1 ^= s1 << 23;
	return (r->s[1] = (s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26))) + s0;
}

typedef struct { uint64_t s[1]; } PRNG64Xorshift64;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Xorshift64, prng64_xorshift64)
static inline uint64_t
prng64_xorshift64(void *rng)
{
	PRNG64Xorshift64 *r = rng;
	r->s[0] ^= r->s[0] << 13;
	r->s[0] ^= r->s[0] >> 7;
	r->s[0] ^= r->s[0] << 17;
	return r->s[0];
}



/* https://github.com/openjdk/jdk/blob/master/src/
 * java.base/share/classes/java/util/Random.java */

typedef struct { uint32_t s[1]; } PRNG32JavaUtilRandom;
CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32JavaUtilRandom, prng32_java_util_random)
static inline uint32_t
prng32_java_util_random(void *rng)
{
	PRNG32JavaUtilRandom *r = rng;
	r->s[1] = (r->s[1] * 0x5DEECE66Du + 0xBu) & ((UINT64_C(1) << 48) - 1);
	return r->s[1] >> (48 - 32);
}



#define EXTRA_H_INCLUDED
#endif

