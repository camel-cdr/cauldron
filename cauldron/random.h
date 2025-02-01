/*
 * State of the art random number generation in C, a guided implementation by
 * Olaf Bernstein <camel-cdr@protonmail.com>.
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 *
 * Table of contents ===========================================================
 *
 * 1. Introduction
 *     1.1 Library structure
 *     1.2 API overview
 * 2. True random number generators
 * 3. Pseudorandom number generator
 *     3.1 Permuted Congruential Generators (PCGs)
 *     3.2 Romu PRNGs
 *     3.3 Xorshift PRNGs
 *     3.4 Middle Square Weyl Sequence PRNGs
 * 4. Cryptographically secure PRNGs
 *     5.1 ChaCha stream cypher
 * 5. Random distributions
 *     5.1 Uniform integer distribution
 *     5.2 Uniform real distribution
 *     5.3 Normal real distribution
 *         5.3.1 Ratio method
 *         5.3.2 Ziggurat method
 *         5.3.3 Approximation using popcount
 * 6. Shuffling
 * References
 * Licensing
 *     MIT License
 *
 * 1. Introduction =============================================================
 *
 * Random number generators (RNGs) are an essential ingredient in the software
 * landscape, yet standard libraries of most languages only implement outdated
 * algorithms or don't implement them at all.
 * The following is the description and implementation of a C library which
 * implements the most important features for dealing with randomness.
 *
 * 1.1 Library structure -------------------------------------------------------
 *
 * Before we get into the code, let's quickly glance over the library structure:
 *     - We use tex math notation for more complex mathematical expressions.
 *       If you have trouble reading them just try using online tex engines
 *       like http://atomurl.net/math/.
 *     - The code from each chapter should be independent and could be
 *       easily copy-pasted into any other project.
 *     - We use the stb style for header only libraries, that is we only supply
 *       the implementation of non inline functions if RANDOM_H_IMPLEMENTATION
 *       is defined. This means that you must define RANDOM_H_IMPLEMENTATION
 *       in EXACTLY ONE C file before you include this header file:
 *           #define RANDOM_H_IMPLEMENTATION
 *           #include "random.h"
 *     - We'll also cast all conversions from void* manually, to obtain C++
 *       compatibility
 */
#if !defined(RANDOM_H_INCLUDED) || defined(RANDOM_H_IMPLEMENTATION)

#ifdef RANDOM_H_IMPLEMENTATION
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <assert.h>
# include <limits.h>
# include <math.h>
# ifdef __cplusplus
#  include <string.h>
# endif
#endif

#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <stdint.h>
/*
 *     - We assume that 32 and 64-bit integers are available and that the
 *       floating point representation uses at least 7 bits for the exponent
 *       (This simplifies the assumptions for dist_normal(f)_zig later).
 *     - Everything else that isn't portable is guarded by ..._AVAILABLE macros.
 */
#if !(UINT32_MAX && UINT64_MAX && INT_MAX <= UINT32_MAX && \
    (32 - FLT_MANT_DIG) >= 7 && (64 - DBL_MANT_DIG) >= 7)
# error random.h: platform not supported
#endif

/*
 * 1.2 API overview ------------------------------------------------------------
 *
 * True random number generator:
 *     void trng_close(void)
 *     int trng_write(void *ptr, size_t n);
 *     int trng_write_notallzero(void *ptr, size_t n);
 *
 *     // compatibility functions to allow TRNG to interface with the
 *     // distributions
 *     uint32_t trng_u32(void *);
 *     uint64_t trng_u64(void *);
 *
 * Pseudorandom number generators:
 *     Every PRNGs implements a common generic interface for initialization
 *     and number generation,
 *         void NAME_randomize(void *rng); // randomly initializes rng
 *         uintXX_t NAME(void *rng); // returns next random number
 *     a generator specific one for explicit initialization (seeding)
 *         void NAME_init(TYPE *rng, [...]); // initialize rng with custom data
 *     and optionally a jump function.
 *         void NAME_jump(TYPE *rng, [...]); // skip multiple calls to the rng
 *
 *     The generators NAME is prefixed with the classification e.g.:
 *         void prng32_pcg_randomize(void *rng);
 *         uint32_t prng32_pcg(void *rng);
 *         void prng32_pcg_init(PRNG32 *rng, uint64_t seed, uint64_t stream);
 *         void prng32_pcg_jump(PRNG32 *rng, uint64_t by);
 *
 *     Supported are:
 *       prng64_NAME       | Jump Support   prng64_NAME        | Jump Support
 *       --------------------------------   ---------------------------------
 *       pcg               | arbitrary      pcg                | arbitrary
 *       romu_trio         | ---            romu_duo_jr        | ---
 *       romu_quad         | ---            romu_duo           | ---
 *       xoroshiro64(s/ss) | fixed          romu_trio          | ---
 *       xoshiro128(s/ss)  | fixed          romu_quad          | ---
 *                                          xoroshiro128(p/ss) | fixed
 *       csprng32_NAME | Jump Support       xoshiro128(p/ss)   | fixed
 *       ----------------------------
 *       chacha        | ---
 *
 * Random distributions:
 *
 *     // random integer inside [0,r)
 *     uint32_t dist_uniform_u32(uint32_t r, uint32_t (*)(void*), void *);
 *     uint64_t dist_uniform_u64(uint64_t r, uint64_t (*)(void*), void *);
 *
 *     // random floating-point from the output of an RNG output inside [0,1)
 *     float dist_uniformf(uint32_t x);
 *     double dist_uniform(uint64_t x);
 *
 *     // random floating-point in range [a,b] including all representable
 *     // values
 *     float dist_uniformf_dense(float a,b, uint64_t (*)(void*), void *);
 *     double dist_uniform_dense(double a,b, uint64_t (*)(void*), void *);
 *
 *     // random sample from the standard normal distribution
 *     float dist_normalf(uint32_t (*)(void*), void *);
 *     double dist_normal(uint64_t (*)(void*), void *);
 *
 *     // a faster dist_normal(f) implementation using a lookup table
 *     void dist_normalf_zig_init(DistNormalfZig *);
 *     void dist_normal_zig_init(DistNormalZig *);
 *     float dist_normalf_zig(DistNormalfZig *, uint32_t (*)(void*), void *);
 *     double dist_normal_zig(DistNormalZig *, uint64_t (*)(void*), void *);
 *
 * Shuffling:
 *
 *     // Shuffle an array with 'nel' elements of size 'size'
 *     void shuf_arr(void *base, uint64_t nel, uint64_t size,
 *                   uint64_t (*)(void*), void *);
 *
 *     Iterator that randomly traverses each element of an array exactly once:
 *
 *         // faster but with not that random
 *         void shuf_weyl_init(ShufWeyl *, size_t mod, size_t seed[2]);
 *         void shuf_weyl_randomize(ShufWeyl *, size_t mod);
 *         size_t shuf_weyl(ShufWeyl *rng);
 *
 *         // a bit slower but more random
 *         void shuf_lcg_init(ShufLcg *, size_t mod, size_t seed[3]);
 *         void shuf_lcg_randomize(ShufLcg *, size_t mod);
 *         size_t shuf_lcg(ShufLcg *rng);
 *
 *
 * 2. True random number generators ============================================
 *
 * We'll begin by implementing the backbone of every proper usage of random
 * number generation, true random number generators (TRNGs).
 * The output of TRNGs should be indistinguishable from "true randomness".
 * In contrast to pseudorandom number generators (PRNGs), which solely rely
 * on a arithmetics, TRNGs sample from physical sources of randomness.
 * They utilize a combination of cryptographically secure PRNGs (CSPRNGs) and
 * high-quality entropy sources, which are very platform-specific.
 * Luckily most operating systems have a build in TRNG we can use:
 *     - RtlGenRandom() on windows
 *     - arc4random() on OpenBSD, CloudABI and WebAssembly
 *     - getrandom system call on FreeBSD and modern Linux kernels
 *     - /dev/urandom on other UNIX-like operating systems
 *
 * If none is available, which frequently happens in embedded systems,
 * TRNG_NOT_AVAILABLE will be defined. In such cases, one can use a CSPRNG in
 * combination with hardware entropy, although if you need secure cryptography
 * please consult an expert.
 */

extern void trng_close(void);
extern int trng_write(void *ptr, size_t n);

#ifdef _WIN32
# ifdef RANDOM_H_IMPLEMENTATION
#  include <windows.h>
#  include <ntsecapi.h>

void
trng_close(void) {}

/* RtlGenRandom takes an unsigned long as the buffer length, which might be
 * smaller than a size_t and in extension n. This requires us to repeatedly call
 * RtlGenRandom until we've written n random bytes. */

int
trng_write(void *ptr, size_t n)
{
	unsigned char *p = (unsigned char*)ptr;
	#if SIZE_MAX > ULONG_MAX
	for (; n > ULONG_MAX; n -= ULONG_MAX, p += ULONG_MAX) {
		if (!RtlGenRandom(p, ULONG_MAX))
			return 0;
	}
	#endif
	return RtlGenRandom(p, n);
}

# endif /* RANDOM_H_IMPLEMENTATION */
#elif defined(__OpenBSD__) || defined(__CloudABI__) || defined(__wasi__)
# ifdef RANDOM_H_IMPLEMENTATION
#  include <stdlib.h>

void
trng_close(void) {}

int
trng_write(void *ptr, size_t n)
{
	arc4random_buf(ptr, n);
	return 1;
}

# endif /* RANDOM_H_IMPLEMENTATION */
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
# ifdef RANDOM_H_IMPLEMENTATION
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/syscall.h>
#  include <sys/types.h>

/* getrandom is only available on some UNIX-like OS, so we'll try calling the
 * getrandom syscall. We'll otherwise read from "/dev/urandom" as a fallback,
 * which is available in almost every UNIX-like OS. */

static int urandomFd = -1;

void
trng_close(void)
{
	if (urandomFd >= 0)
		close(urandomFd);
}

int
trng_write(void *ptr, size_t n)
{
	unsigned char *p;
	ssize_t r;

	/* getrandom or read() aren't guaranteed to write the entirety of the
	 * requested bytes, which means that we need to read one chunk at a
	 * time. */

	for (p = (unsigned char*)ptr; n > 0; n -= (size_t)r, p += r) {
		/* if available use getrandom */
		#ifdef SYS_getrandom
		if ((r = syscall(SYS_getrandom, p, n, 0)) > 0)
			continue;
		#endif

		/* Otherwise, fall through and read from "/dev/urandom", make
		 * sure to open "/dev/urandom" if it isn't already. */
		if (urandomFd < 0)
			if ((urandomFd = open("/dev/urandom", O_RDONLY)) < 0)
				return 0;

		if ((r = read(urandomFd, p, n)) <= 0)
			return 0;
	}

	return 1;
}

# endif /* RANDOM_H_IMPLEMENTATION */
#else
# define TRNG_NOT_AVAILABLE 1
void trng_close(void) { assert(0 && "random.h: trng not available "); }
int trng_write(void *ptr, size_t n) { trng_close(); return 0; }
#endif

#if !TRNG_NOT_AVAILABLE
/* trng_u32/64 are used to be api compatible with the PRNGs */

static inline uint32_t
trng_u32(void *ptr)
{
	uint32_t x;
	(void)ptr;
	trng_write(&x, sizeof x);
	return x;
}

static inline uint64_t
trng_u64(void *ptr)
{
	uint64_t x;
	(void)ptr;
	trng_write(&x, sizeof x);
	return x;
}

/* We implement the trng_write_notallzero helper funciton,
 * because many PRNGs must be initialized with at least one bit set. */

extern int trng_write_notallzero(void *ptr, size_t n);

# ifdef RANDOM_H_IMPLEMENTATION
int
trng_write_notallzero(void *ptr, size_t n)
{
	unsigned char *p;
	size_t i;
	/* Stop after 128 tries. */
	for (i = 0; i < 128; ++i) {
		if (!trng_write(ptr, n))
			return 0;
		/* Return if any bit is set. */
		for (p = (unsigned char*)ptr; *p; ++p)
			if (*p != 0)
				return 1;
	}
	return 0;
}
# endif /* RANDOM_H_IMPLEMENTATION */

/* To reduce the loc we also define a simple macro that defines an *_randomize
 * function for a given PRNG */
#define CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(Type, name) \
	static inline void \
	name##_randomize(void *rng) \
	{ \
		Type *r = (Type*)rng; \
		trng_write_notallzero(r->s, sizeof(r->s)); \
	}

#else

/* We don't want to define these functions if no trng is available */
#define CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(Type, name)

#endif/* TRNG_NOT_AVAILABLE */

/*
 * 3. Pseudorandom number generator ============================================
 *
 * Pseudorandom number generators (PRNGs) deterministically generate numbers
 * whose properties approximate those of "true random numbers".
 * They work by utilizing initial "true random numbers" to arithmetically
 * generate new random-looking (pseudorandom) numbers, which makes that PRNGs,
 * in contrast to TRNGs, deterministic. The same initialization (seeding) will
 * result in the generation of the same sequence of numbers. This can be quite
 * practical for uses in simulations, games and many other use cases.
 * Furthermore, PRNGs are way faster and more memory efficient than TRNGs.
 *
 * But before we get ahead of ourselves, let's consider the following PRNG you
 * can trivially compute using mental arithmetics <1>:
 *     1. Select an initial random number (seed) between 1 and 58.
 *        (This is accomplished by mentally collecting entropy from the
 *         environment, e.g. counting a group of objects of unknown quantity)
 *     2. Multiply the least significant digit by 6, add the most significant
 *        digit and use the result as the next seed/state.
 *     3. The second digit of the state is your generated pseudorandom number.
 *     4. Goto 2.
 *                            Sequence generated by 42:
 *   42|16|37|45|34|27|44|28|50|05|03|18|49|58|53|23|20|02|12|13|19|55|35|33|21
 *    2| 6| 7| 5| 4| 7| 4| 8| 0| 5| 3| 8| 9| 8| 3| 3| 0| 2| 2| 3| 9| 5| 5| 3| 1
 *
 * While these numbers certainly look pretty random for humans, they won't be
 * sufficient for simulation purposes and are easily predictable.
 * Not to mention, that the generated sequence repeats after only 58
 * generated numbers. Note that this is not a uniform PRNG, because the digits
 * 9 and 0 are slightly less likely to be generated, because 58 doesn't divide
 * evenly into 10. We call, how many numbers a PRNG can generate before it
 * repeats, the period.
 *
 * The period must be big enough to generate no overlapping sequences.
 * For n sequences of length L given a generator of period P, we can approximate
 * the probability with the formula p <= L*(n^2)/P. <2>
 * For most purposes, you want a period of at least 2^{64} and for parallel
 * applications, anything in the ballpark of 2^{128} should be sufficient.
 *
 * Other than that the quality of the generated randomness is equally essential.
 * Sadly no mathematical proof of randomness exists, therefore we need to use
 * empirical statistic analysis test suites like TestU01 <3> and PractRand <4>.
 *
 * There are many different PRNGs with various periods, qualities, speeds and
 * memory footprints of which we need to strike a balance.
 * We'll be implementing three families of generators (and discus a forth one)
 * which pass all tests of PractRand and TestU01 (xoshiro/xoroshiro's + variants
 * fail linear testing) and vary in security, speed and portability.
 * This is a crude overview of the featured generators, more detail is available
 * in the subsequent chapters:
 *
 * Permuted Congruential Generator (PCG) family <5>:
 *     Advantages:
 *         - Very high quality
 *         - Jump ahead support
 *         - Independent streams
 *         - Hard to reverse (though by no means cryptographic <6>)
 *     Disadvantages:
 *         - Not as fast as the other PRNGs
 *         - 32/64-bit variant requires 64/128-bit arithmetic
 *             -> not as portable
 *             -> generating 64-bit numbers using a 32-bit PCG is even slower
 *
 * Rotate-multiply (Romu) family <7>:
 *     Advantages:
 *         - Very high quality
 *         - Very very fast
 *     Disadvantages:
 *         - The period varies in length (although the probabilities of small
 *           periods are known and very very low)
 *         - New and not yet extensively tested/reviewed in the scientific
 *           literature (as of 2021)
 *
 * xoshiro/xoroshiro family <8>:
 *     Advantages:
 *         - Very high quality (except the + variety)
 *         - Very fast
 *         - Jump ahead support
 *     Disadvantages:
 *         - The + variety fails linear tests
 *         - Some issues escaping zero land
 *           (when only a few bits of the state are set)
 *
 * Middle Square Weyl Sequence (MSWS) generator <10>:
 *     Advantages:
 *         - Very high quality
 *         - Independent streams
 *         - Easy to memorize implementation
 *     Disadvantages:
 *         - Not as fast as the other PRNGs
 *         - 32/64-bit variant requires 64/128-bit arithmetic
 *             -> not as portable
 *             -> generating 64-bit numbers using a 32-bit PCG is even slower
 *
 * For a performance comparison check out the benchmark at tools/random/bench.c.
 * The tools to test the quality of the PRNGs are also available in
 * tools/random (e.g. ./rng prng64_romu_quad | ./PractRand stdin64).
 */

/*
 * 3.1 Permuted Congruential Generators (PCGs) ---------------------------------
 *
 * Permuted congruential generators (PCGs) <5> improve upon the statistical
 * properties of linear congruential generators (LCGs) by using a state twice as
 * large as the output and applying a transformation/permutation, which
 * results in excellent statistical properties. Sadly this sacrifices either
 * performance or portability, as you require a type twice as large as the
 * output (generating 64-bit integers require a 128-bit integer type).
 * The LCG nature of PCGs allows for arbitrary jump ahead and the application of
 * different streams.
 *
 * LCGs are one of the oldest and best-known PRNGs and have the format:
 *     state := (a * state + c) \mod m
 * The following conditions must be satisfied by 'a' and 'c', to obtain the
 * maximal period length 'm':
 *     - 'c' is coprime to 'm'
 *     - a-1 is divisible by all prime factors of 'm'
 *     - a-1 is divisible by 4 if 'm' is divisible by 4
 * If we use the two to the power of bits in our state variable as the modulo,
 * unsigned integer overflow will automatically apply the effect of modulo and
 * lets us omit the expensive instruction. Now it's trivial to find suitable
 * values for 'c' because every smaller odd number is coprime to a power-of-two.
 * We'll use this to implement different streams for our PCG generators.
 *
 * Values for 'a', with good statistical properties, can be determined using the
 * spectral test though the specifics are out of scope for this document.
 *
 * We can derive a jump ahead equation by simplifying multiple applications of
 * the LCG.
 * Two applications
 *     state := (a * (a * state + c) + c) \mod m
 * can be simplified to
 *     state := (a^2 * state + c * (a + 1)) \mod m.
 * To jump ahead n applications
 *     state := (a*(a*(...(a * state + c)...)+c)+c) \mod m
 * simplifies to
 *     state := (a^n * state + c * (\sum_{i=0}^{n-1} a^i)) \mod m.
 * Further simpifying, using the formula
 *     \sum_{i=0}^{n-1} a^i = (a^n - 1)/(a - 1),
 * yields the final simplification
 *     state := (a^n * state + c * (a^n - 1)/(a - 1)) \mod m.
 *
 * Vanilla LCGs perform quite poorly at small state sizes, but rapidly improve
 * with bigger state sizes. The problem with power-of-2 LCGs is that the
 * lower-order bits have more regularities. To overcome this problem PCGs
 * apply a variable rotate permutation to the generated output.
 */

#define PRNG32_PCG_MULT 6364136223846793005u

typedef struct { uint64_t state, stream; } PRNG32Pcg;

static inline void
prng32_pcg_init(PRNG32Pcg *rng, uint64_t seed, uint64_t stream)
{
	rng->state = seed;
	rng->stream = stream | 1u;
}

#if !TRNG_NOT_AVAILABLE
static inline void
prng32_pcg_randomize(void *rng)
{
	PRNG32Pcg *r = (PRNG32Pcg*)rng;
	trng_write(&r->state, sizeof r->state);
	trng_write(&r->stream, sizeof r->stream);
	r->stream |= 1u;
}
#endif

static inline uint32_t
prng32_pcg(void *rng)
{
	/* Period: 2^{64} with 2^{63} streams
	 * BigCrush: Passes
	 * PractRand: Passes (>32T) */
	PRNG32Pcg *r = (PRNG32Pcg*)rng;
	uint32_t const perm = ((r->state >> 18) ^ r->state) >> 27;
	uint32_t const rot = r->state >> 59;
	r->state = r->state * PRNG32_PCG_MULT + r->stream;
	return (perm >> rot) | (perm << (-rot & 31u));
}

extern void prng32_pcg_jump(PRNG32Pcg *rng, uint64_t by);

#ifdef RANDOM_H_IMPLEMENTATION
void
prng32_pcg_jump(PRNG32Pcg *rng, uint64_t by)
{
	uint64_t curmult = PRNG32_PCG_MULT, curplus = rng->stream;
	uint64_t actmult = 1,               actplus = 0;
	/* We derived the formula
	 *     state := (a^n * seed + c * (a^n - 1)/(a - 1)) \mod m
	 * above, but we can't compute this without arbitrary precision
	 * arithmetic. Fortunately, there is an O(log(i)) jump ahead algorithm
	 * from Forrest B. Brown <9>, which we've implemented here. */
	while (by > 0) {
		if (by & 1u) {
			actmult *= curmult;
			actplus = actplus * curmult + curplus;
		}
		curplus = (curmult + 1u) * curplus;
		curmult *= curmult;
		by >>= 1;
	}
	rng->state = actmult * rng->state + actplus;
}
#endif /* RANDOM_H_IMPLEMENTATION */

#define PRNG64_PCG_AVAILABLE (__SIZEOF_INT128__)
#if PRNG64_PCG_AVAILABLE

# define PRNG64_PCG_MULT \
	(((__uint128_t)2549297995355413924u << 64) | \
	               4865540595714422341u)

typedef struct { __uint128_t state, stream; } PRNG64Pcg;

static inline void
prng64_pcg_init(PRNG64Pcg *rng,
                uint64_t const seed[2],
                uint64_t const stream[2])
{
	rng->state = ((__uint128_t)seed[0] << 64) | seed[1];
	rng->stream = ((__uint128_t)stream[0] << 64) | 1u | stream[1];
}

#if !TRNG_NOT_AVAILABLE
static inline void
prng64_pcg_randomize(void *rng)
{
	PRNG64Pcg *r = (PRNG64Pcg*)rng;
	trng_write(&r->state, sizeof r->state);
	trng_write(&r->stream, sizeof r->stream);
	r->stream |= 1;
}
#endif

static inline uint64_t
prng64_pcg(void *rng)
{
	/* Period: 2^{128} with 2^{127} streams
	 * BigCrush: Passes
	 * PractRand: Passes (>32T) */
	PRNG64Pcg *r = (PRNG64Pcg*)rng;
	uint64_t const xorshifted =
			((uint64_t)(r->state >> 64)) ^ (uint64_t)r->state;
	uint64_t const rot = r->state >> 122;
	r->state = r->state * PRNG64_PCG_MULT + r->stream;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 63u));
}

extern void prng64_pcg_jump(PRNG64Pcg *rng, uint64_t const by[2]);

# ifdef RANDOM_H_IMPLEMENTATION
void
prng64_pcg_jump(PRNG64Pcg *rng, uint64_t const by[2])
{
	__uint128_t curmult = PRNG64_PCG_MULT, curplus = rng->stream;
	__uint128_t actmult = 1,               actplus = 0;
	__uint128_t by128 = ((__uint128_t)by[0]) << 64 | by[1];
	while (by128 > 0) {
		if (by128 & 1u) {
			actmult *= curmult;
			actplus = actplus * curmult + curplus;
		}
		curplus = (curmult + 1u) * curplus;
		curmult *= curmult;
		by128 >>= 1;
	}
	rng->state = actmult * rng->state + actplus;
}
# endif /* RANDOM_H_IMPLEMENTATION */

#endif /* PRNG64_PCG_AVAILABLE */

/*
 * 3.2 Romu PRNGs --------------------------------------------------------------
 *
 * Rotate-multiply (Romu) PRNGs <7> combine the linear operation of
 * multiplication with the nonlinear rotations.
 *
 * The rotation constants were devised empirically by testing scaled-down
 * generators with less state against PractRand. The best of these constants
 * were then scale up for use in the full-size generators.
 * The multipliers are handcrafted, making sure that they have an irregular
 * bit-pattern. A similar technique is employed by B. Widynski's Middle Square
 * Weyl Sequence PRNG to obtain several independent streams. <10>
 * The duo, trio and quad variants each use 2, 3 and 4 state variables, on
 * which the operations above and extra additions and subtractions, are
 * applied. The nature of these operations dictates that the state can't have
 * every bit set to zero, which mustn't be forgotten when initializing a
 * generator.
 *
 * One can estimate the capacity of the full-sized generators, by running a
 * scaled-down version through statistical testing until it fails. This
 * theoretically means that the generators won't fail until they have
 * generated more than their capacity specifies using a particular seed.
 *
 * The incredible speed of the Romu generators is due to it being optimized
 * with instruction-level parallelism (ILP) in mind. ILP is used in most modern
 * processors and allows the execution of multiple instructions in each clock
 * cycle whenever possible. Most x86 CPUs can execute up to four instructions
 * in each clock cycle. Always leaving one instruction slot per cycle free to
 * be used by the surrounding application code, allows the inlined Romu
 * generators to introduce almost no delay for the application.
 *
 * As mentioned above Romu PRNGs are nonlinear, which means that they don't
 * have a single cycle that goes through the while period, but rather multiple
 * ones with various periods. One must be exceedingly cautious with nonlinear
 * generators, because of the likely hood to encounter short cycles. The
 * probabilities of such are fortunately known and very low. The upper bound
 * probability of periods shorter than 2^k is calculated, given a state
 * size of 2^s-1, with 2^{k-s+7}.
 *
 * We can't follow our already established formula to calculate the probability
 * of overlapping sequences. Fortunately, this was also addressed in the Romu
 * paper. For n sequences of length 2^L given s bits of state in the generator,
 * we can approximate the probability with the formula
 *     p <= 2^{6.5+l-s}(s-l+1)(n-1)n.
 *
 * Recommendations:
 *     - duo_jr for all-out speed
 *     - quad as the default
 */

#define PRNG_ROMU_ROTL(x,k) (((x) << (k)) | ((x) >> (8 * sizeof(x) - (k))))


typedef struct { uint32_t s[3]; } PRNG32RomuTrio; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32RomuTrio, prng32_romu_trio)

static inline uint32_t
prng32_romu_trio(void *rng)
{
	/* Capacity: >2^{53}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG32RomuTrio *r = (PRNG32RomuTrio*)rng;
	uint32_t const s0 = r->s[0], s1 = r->s[1], s2 = r->s[2];
	r->s[0] = 3323815723u * s2;
	r->s[1] = s1 - s0;
	r->s[2] = s2 - s1;
	r->s[1] = PRNG_ROMU_ROTL(r->s[1], 6);
	r->s[2] = PRNG_ROMU_ROTL(r->s[2], 22);
	return s0;
}

typedef struct { uint32_t s[4]; } PRNG32RomuQuad; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32RomuQuad, prng32_romu_quad)

static inline uint32_t
prng32_romu_quad(void *rng)
{
	/* Capacity: >2^{62}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG32RomuQuad *r = (PRNG32RomuQuad*)rng;
	uint32_t const s0 = r->s[0], s1 = r->s[1], s2 = r->s[2], s3 = r->s[3];
	r->s[0] = 3323815723u * s3;
	r->s[1] = s3 + PRNG_ROMU_ROTL(s0, 26);
	r->s[2] = s2 - s1;
	r->s[3] = s2 + s0;
	r->s[3] = PRNG_ROMU_ROTL(r->s[3], 9);
	return s1;
}

typedef struct { uint64_t s[2]; } PRNG64RomuDuo; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64RomuDuo, prng64_romu_duo)

static inline uint64_t
prng64_romu_duo_jr(void *rng)
{
	/* Capacity: >2^{51}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG64RomuDuo *r = (PRNG64RomuDuo*)rng;
	uint64_t const s0 = r->s[0];
	r->s[0] = 15241094284759029579u * r->s[1];
	r->s[1] = r->s[1] - s0;
	r->s[1] = PRNG_ROMU_ROTL(r->s[1], 27);
	return s0;
}

static inline uint64_t
prng64_romu_duo(void *rng)
{
	/* Capacity: >2^{61}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG64RomuDuo *r = (PRNG64RomuDuo*)rng;
	uint64_t const s0 = r->s[0];
	r->s[0] = 15241094284759029579u * r->s[1];
	r->s[1] = PRNG_ROMU_ROTL(r->s[1], 36) +
	          PRNG_ROMU_ROTL(r->s[1], 15) - s0;
	return s0;
}

typedef struct { uint64_t s[3]; } PRNG64RomuTrio; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64RomuTrio, prng64_romu_trio)

static inline uint64_t
prng64_romu_trio(void *rng)
{
	/* Capacity: >2^{75}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG64RomuTrio *r = (PRNG64RomuTrio*)rng;
	uint64_t const s0 = r->s[0], s1 = r->s[1], s2 = r->s[2];
	r->s[0] = 15241094284759029579u * s2;
	r->s[1] = s1 - s0;
	r->s[2] = s2 - s1;
	r->s[1] = PRNG_ROMU_ROTL(r->s[1], 12);
	r->s[2] = PRNG_ROMU_ROTL(r->s[2], 44);
	return s0;
}

typedef struct { uint64_t s[4]; } PRNG64RomuQuad; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64RomuQuad, prng64_romu_quad)

static inline uint64_t
prng64_romu_quad(void *rng)
{
	/* Capacity: >2^{90}
	 * BigCrush: Passes
	 * PractRand: Passes (>256T) */
	PRNG64RomuQuad *r = (PRNG64RomuQuad*)rng;
	uint64_t const s0 = r->s[0], s1 = r->s[1], s2 = r->s[2], s3 = r->s[3];
	r->s[0] = 15241094284759029579u * s3;
	r->s[1] = s3 + PRNG_ROMU_ROTL(s0, 52);
	r->s[2] = s2 - s1;
	r->s[3] = s2 + s0;
	r->s[3] = PRNG_ROMU_ROTL(r->s[3], 19);
	return s1;
}


#undef PRNG_ROMU_ROTL

/*
 * 3.3 Xorshift PRNGs ----------------------------------------------------------
 *
 * The Xorshift family of PRNGs combine XOR operations's with bit-shifts and are
 * a subset of linear-feedback shift registers (LFSRs). While these generators
 * possess better statistical properties than vanilla LCGs they nevertheless
 * fail many statistical tests.
 *
 * Linearity tests specifically are impossible to be passd by vanilla LFSRs,
 * hence, similar to the PCGs generators the output must be scrambled. This is
 * frequently achieved by using various additions or multiplications.
 *
 * The xoshiro/xoroshiro generators <8> featured here additionally incorporate
 * bit rotations, which improve the state diffusion, as no bit of the operand is
 * discarded. We'll be implementing two different scramblers, a faster one with
 * a weaker scramble (s/p i.e. '*'/'+') and a slower one with a stronger
 * scramble (ss/pp i.e. '**'/'++'). It's recommended to use the s/p variants for
 * generating floating-point numbers.
 */
#define PRNG_XORSHIFT_ROTL(x,k) (((x) << (k)) | ((x) >> (8 * sizeof(x) - (k))))


typedef struct { uint32_t s[2]; } PRNG32Xoroshiro64; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32Xoroshiro64, prng32_xoroshiro64)

static inline void
prng32_xoroshiro64_advance(PRNG32Xoroshiro64 *rng) /* 26-9-13 */
{
	rng->s[1] ^= rng->s[0];
	rng->s[0] = PRNG_XORSHIFT_ROTL(rng->s[0], 26) ^
	            rng->s[1] ^ (rng->s[1] << 9);
	rng->s[1] = PRNG_XORSHIFT_ROTL(rng->s[1], 13);
}
static inline uint32_t
prng32_xoroshiro64s(void *rng)
{
	/* Period: 2^{64}-1
	 * BigCrush: TODO
	 * PractRand: lower bits fail linear tests, passes all others (>128G) */
	PRNG32Xoroshiro64 *r = (PRNG32Xoroshiro64*)rng;
	uint32_t const res = r->s[0] * 0x9E3779BB;
	prng32_xoroshiro64_advance(r);
	return res;
}
static inline uint32_t
prng32_xoroshiro64ss(void *rng)
{
	/* Period: 2^{64}-1
	 * BigCrush: TODO
	 * PractRand: Passes (>128G) */
	PRNG32Xoroshiro64 *r = (PRNG32Xoroshiro64*)rng;
	uint32_t const tmp = r->s[0] * 0x9E3779BB;
	uint32_t const res = PRNG_XORSHIFT_ROTL(tmp, 5) * 5;
	prng32_xoroshiro64_advance(r);
	return res;
}

typedef struct { uint32_t s[4]; } PRNG32Xoshiro128; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG32Xoshiro128, prng32_xoshiro128)

static inline void
prng32_xoshiro128_advance(PRNG32Xoshiro128 *rng) /* 0-9-11 */
{
	uint32_t const t = rng->s[1] << 9;
	rng->s[2] ^= rng->s[0];
	rng->s[3] ^= rng->s[1];
	rng->s[1] ^= rng->s[2];
	rng->s[0] ^= rng->s[3];
	rng->s[2] ^= t;
	rng->s[3] = PRNG_XORSHIFT_ROTL(rng->s[3], 11);
}
static inline uint32_t
prng32_xoshiro128s(void *rng)
{
	/* Period: 2^{128}-1
	 * BigCrush: TODO
	 * PractRand: lower bits fail linear tests, passes all others (>128G) */
	PRNG32Xoshiro128 *r = (PRNG32Xoshiro128*)rng;
	uint32_t const res = r->s[0] + r->s[3];
	prng32_xoshiro128_advance(r);
	return res;
}
static inline uint32_t
prng32_xoshiro128ss(void *rng)
{
	/* Period: 2^{128}-1
	 * BigCrush: TODO
	 * PractRand: Passes (>128G) */
	PRNG32Xoshiro128 *r = (PRNG32Xoshiro128*)rng;
	uint32_t const tmp = r->s[1] * 5;
	uint32_t const res = PRNG_XORSHIFT_ROTL(tmp, 7) * 9;
	prng32_xoshiro128_advance(r);
	return res;
}

typedef struct { uint64_t s[2]; } PRNG64Xoroshiro128; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Xoroshiro128, prng64_xoroshiro128)

static inline void
prng64_xoroshiro128_advance(PRNG64Xoroshiro128 *rng) /* 24-16-37 */
{
	rng->s[1] ^= rng->s[0];
	rng->s[0] = PRNG_XORSHIFT_ROTL(rng->s[0], 24) ^
	            rng->s[1] ^ (rng->s[1] << 16);
	rng->s[1] = PRNG_XORSHIFT_ROTL(rng->s[1], 37);
}
static inline uint64_t
prng64_xoroshiro128p(void *rng)
{
	/* Period: 2^{128}-1
	 * BigCrush: Passes
	 * PractRand: lower bits fail linear tests, passes all others (>128G) */
	PRNG64Xoroshiro128 *r = (PRNG64Xoroshiro128*)rng;
	uint64_t const res = r->s[0] + r->s[1];
	prng64_xoroshiro128_advance(r);
	return res;
}
static inline uint64_t
prng64_xoroshiro128ss(void *rng)
{
	/* Period: 2^{128}-1
	 * BigCrush: Passes
	 * PractRand: Passes (>512G) */
	PRNG64Xoroshiro128 *r = (PRNG64Xoroshiro128*)rng;
	uint64_t const tmp = r->s[0] * 5;
	uint64_t const res = PRNG_XORSHIFT_ROTL(tmp, 7) * 9;
	prng64_xoroshiro128_advance(r);
	return res;
}

typedef struct { uint64_t s[4]; } PRNG64Xoshiro256; /* not all zero */

CAULDRON_MAKE_PRNG_NOTALLZERO_RANDOMIZE(PRNG64Xoshiro256, prng64_xoshiro256)

static inline void
prng64_xoshiro256_advance(PRNG64Xoshiro256 *rng) /* 0-17-54 */
{
	uint64_t const t = rng->s[1] << 17;
	rng->s[2] ^= rng->s[0];
	rng->s[3] ^= rng->s[1];
	rng->s[1] ^= rng->s[2];
	rng->s[0] ^= rng->s[3];
	rng->s[2] ^= t;
	rng->s[3] = PRNG_XORSHIFT_ROTL(rng->s[3], 45);
}
static inline uint64_t
prng64_xoshiro256p(void *rng)
{
	/* Period: 2^{256}-1
	 * BigCrush: Passes
	 * PractRand: lower bits fail linear tests, passes all others (>128G) */
	PRNG64Xoshiro256 *r = (PRNG64Xoshiro256*)rng;
	uint64_t const res = r->s[0] + r->s[3];
	prng64_xoshiro256_advance(r);
	return res;
}
static inline uint64_t
prng64_xoshiro256ss(void *rng)
{
	/* Period: 2^{256}-1
	 * BigCrush: Passes
	 * PractRand: Passes (>512G) */
	PRNG64Xoshiro256 *r = (PRNG64Xoshiro256*)rng;
	uint64_t const tmp = r->s[1] * 5;
	uint64_t const res = PRNG_XORSHIFT_ROTL(tmp, 7) * 9;
	prng64_xoshiro256_advance(r);
	return res;
}

/* There are also 512/1024-bit xoroshiro variant's, although 256-bits are
 * already more than enough. */

#undef PRNG_XORSHIFT_ROTL

/*
 * Every LFSR has specific jump polynomials that can be used to efficiently
 * jump ahead, we supply some precomputed ones bellow.
 *
 * For more interested readers here is how to obtain Vigna's script to generate
 * these polynomials. You should be able to generate full jump tables for all of
 * the generators, though I only got xoroshiro128 to work properly:
 *
 * Generate the full xoroshiro128 jump table:
 *     1. Download and extract "https://web.archive.org/web/20180909014435/
 *                              http://xoroshiro.di.unimi.it/xorshift-1.2.tgz"
 *     2. Install python2 and Fermat (http://home.bway.net/lewis)
 *     3. Replace "fermat" in xorshift-1.2 with the name of the Fermat
 *        executable (in my case "fer64"):
 *        $ sed -i "s/fermat/fer64/g" `find xorshift-1.2 -type f`
 *     4. Replace "#!/usr/bin/python" in jump.py with "#!/usr/bin/python2".
 *     5. $ cd xorshift-1.2/full/128roshiro64/
 *        $ j=0
 *        $ while [[ $j -le 128 ]] ; do
 *        $         echo -e -n $j "   " | head -c4;
 *        $         grep 24-16-37 prim.txt | ../common/jump.sh $j
 *        $         j=$((j+1))
 *        $ done
 */

extern uint32_t const prng32Xoroshiro128Jump2Pow64[4];
extern uint32_t const prng32Xoroshiro128Jump2Pow96[4];
extern void prng32_xoshiro128_jump(PRNG32Xoshiro128 *rng,
                                   uint32_t const jump[4]);

#ifdef RANDOM_H_IMPLEMENTATION
uint32_t const prng32Xoroshiro128Jump2Pow64[4] = /* 0-9-11 */
	{ 0x8764000B, 0xF542D2D3, 0x6FA035C3, 0x77F2DB5B };
uint32_t const prng32Xoroshiro128Jump2Pow96[4] = /* 0-9-11 */
	{ 0xB523952E, 0x0B6F099F, 0xCCF5A0EF, 0x1C580662 };

void
prng32_xoshiro128_jump(PRNG32Xoshiro128 *rng, uint32_t const jump[4])
{
	size_t i, b, j;
	uint32_t s[4] = { 0 };
	for (i = 0; i < 4; i++)
		for (b = 0; b < 32; prng32_xoshiro128_advance(rng), b++)
			if (jump[i] & UINT32_C(1) << b)
				for (j = 0; j < 4; j++)
					s[j] ^= rng->s[j];
	for (i = 0; i < 4; i++)
		rng->s[i] = s[i];
}
#endif /* RANDOM_H_IMPLEMENTATION */

extern uint64_t const prng64Xoroshiro128Jump2Pow16[2],
	prng64Xoroshiro128Jump2Pow32[2], prng64Xoroshiro128Jump2Pow48[2],
	prng64Xoroshiro128Jump2Pow64[2], prng64Xoroshiro128Jump2Pow96[2];
extern uint64_t const prng64Xoshiro256Jump2Pow32[4],
	prng64Xoshiro256Jump2Pow48[4], prng64Xoshiro256Jump2Pow64[4],
	prng64Xoshiro256Jump2Pow96[4], prng64Xoshiro256Jump2Pow128[4],
	prng64Xoshiro256Jump2Pow160[4], prng64Xoshiro256Jump2Pow192[4];

extern void prng64_xoroshiro128_jump(PRNG64Xoroshiro128 *rng,
                                     uint64_t const jump[2]);
extern void prng64_xoshiro256_jump(PRNG64Xoshiro256 *rng,
                                   uint64_t const jump[4]);

#ifdef RANDOM_H_IMPLEMENTATION
uint64_t const prng64Xoroshiro128Jump2Pow16[2] = /* 24-16-37 */
	{ 0xB82CA99A09A4E71E, 0x81E1DD96586CF985 };
uint64_t const prng64Xoroshiro128Jump2Pow32[2] = /* 24-16-37 */
	{ 0xFAD843622B252C78, 0xD4E95EEF9EDBDBC6 };
uint64_t const prng64Xoroshiro128Jump2Pow48[2] = /* 24-16-37 */
	{ 0xD769CFC9028DEB78, 0x9B19BA6B3752065A };
uint64_t const prng64Xoroshiro128Jump2Pow64[2] = /* 24-16-37 */
	{ 0xDF900294D8F554A5, 0x170865DF4B3201FC };
uint64_t const prng64Xoroshiro128Jump2Pow96[2] = /* 24-16-37 */
	{ 0xD2A98B26625EEE7B, 0xDDDF9B1090AA7AC1 };

uint64_t const prng64Xoshiro256Jump2Pow32[4] = /* 0-17-54 */
{ 0x58120D583C112F69,0x7D8D0632BD08E6AC,0x214FAFC0FBDBC208,0xE055D3520FDB9D7 };
uint64_t const prng64Xoshiro256Jump2Pow48[4] = /* 0-17-54 */
{ 0xF11FB4FAEA62C7F1,0xF825539DEE5E4763,0x474579292F705634,0x5F728BE2C97E9066 };
uint64_t const prng64Xoshiro256Jump2Pow64[4] = /* 0-17-54 */
{ 0xB13C16E8096F0754,0xB60D6C5B8C78F106,0x34FAFF184785C20A,0x12E4A2FBFC19BFF9 };
uint64_t const prng64Xoshiro256Jump2Pow96[4] = /* 0-17-54 */
{ 0x148C356C3114B7A9,0xCDB45D7DEF42C317,0xB27C05962EA56A13,0x31EEBB6C82A9615F };
uint64_t const prng64Xoshiro256Jump2Pow128[4] = /* 0-17-54 */
{ 0x180EC6D33CFD0ABA,0xD5A61266F0C9392C,0xA9582618E03FC9AA,0x39ABDC4529B1661C };
uint64_t const prng64Xoshiro256Jump2Pow160[4] = /* 0-17-54 */
{ 0xC04B4F9C5D26C200,0x69E6E6E431A2D40B,0x4823B45B89DC689C,0xF567382197055BF0 };
uint64_t const prng64Xoshiro256Jump2Pow192[4] = /* 0-17-54 */
{ 0x76E15D3EFEFDCBBF,0xC5004E441C522FB3,0x77710069854EE241,0x39109BB02ACBE635 };

void
prng64_xoroshiro128_jump(PRNG64Xoroshiro128 *rng, uint64_t const jump[2])
{
	size_t i, j, b;
	uint64_t s[2] = { 0 };
	for (i = 0; i < 2; i++)
		for (b = 0; b < 64; prng64_xoroshiro128_advance(rng), b++)
			if (jump[i] & UINT64_C(1) << b)
				for (j = 0; j < 2; j++)
					s[j] ^= rng->s[j];
	for (i = 0; i < 2; i++)
		rng->s[i] = s[i];
}

void
prng64_xoshiro256_jump(PRNG64Xoshiro256 *rng, uint64_t const jump[4])
{
	size_t i, b, j;
	uint64_t s[4] = { 0 };
	for (i = 0; i < 4; i++)
		for (b = 0; b < 64; prng64_xoshiro256_advance(rng), b++)
			if (jump[i] & UINT64_C(1) << b)
				for (j = 0; j < 4; j++)
					s[j] ^= rng->s[j];
	for (i = 0; i < 4; i++)
		rng->s[i] = s[i];
}
#endif /* RANDOM_H_IMPLEMENTATION */

/*
 * 3.4 Middle Square Weyl Sequence PRNGs ---------------------------------------
 *
 * The Middle Square Weyl Sequence (MSWS) PRNG is an improved version of the
 * first ever PRNG, the middle square method invented by John von Neumann.
 * The original had a big problem with short periods, to circumvent this issue
 * Widynski <10> proposed combining it with a Weyl sequence.
 *
 * A Weyl sequence is constructed by continuously adding a constant 'c' to our
 * state with the modulo of 'm':
 *      state := state + c \mod m
 * This will give us a full period of [0;m) for every value 'c' that is coprime
 * to 'm'.
 * The resulting sequence of numbers isn't very random, but can be used in
 * combination with the way more random middle square method to guarantee
 * full cycles.
 *
 * The MSWS generators are about as fast as the PCG generators and also have
 * excellent statistical properties. However this library doesn't implement it
 * directly, because the other PRNGs cover most use-cases already and because it
 * has a very simple structure.
 * So simple in fact, that you can memorize it compleatly. It only uses three
 * instructions and doesn't require you to memorize any fancy constants,
 * only three rules to create arbitrary constants:
 *     x *= x;                    // square x
 *     x += (w += s);             // advance the Weyl sequence and add it to x
 *     x = (x >> 32) | (x << 32); // swap upper and lower bits (rotate)
 * Where s is a randomly chosen stream, which should have an irregular bit-
 * pattern you can manually create by following three easy rules:
 *      1. The constant must be odd.
 *      2. Every 4-bytes should have unique nibbles within these 4-bytes.
 *      3. No nibble shall equal zero.
 * E.g.: uint32_t s = 0x9F32E1CA; or uint64_t s = 0xB5AD4ECEDA1CE2A9;
 *
 * It suffers from the same problem as the PCG generators, you require an
 * internal type twice as large as the output type.
 * Though it's fast enough that you could just combine two consecutive outputs
 * or distinct generators to subvert this limitation.
 *
 * Period: Full size of x
 * BigCrush: Passes
 * PractRand: Passes (TODO)
 */

/*
 * 4. Cryptographically secure PRNGs ===========================================
 *
 * Cryptographically secure PRNGs (CSPRNGs), while behaving deterministically,
 * are designed to be impossible for an outside observer to predict or reverse.
 * This is accomplished using mathematical constructs that guarantee, that there
 * is no polynomial-time algorithm that could predict the subsequent bit with a
 * better than 50% probability. Moreover, CSPRNGs should be resistant to state
 * compromises, i.e. it should be impossible to reconstruct the state even if
 * fragments of it were compromised. Many cryptographic applications of CSPRNGs
 * additionally require a constant execution time, which, if not guaranteed,
 * could leak information on the internal state.
 * The CSPRNG implementation presented bellow should satisfy the aforementioned
 * requirements, though I'm uncertain about the constant execution time.
 *
 * If you need cryptographic security, make sure to consult an expert!!!
 */

/*
 * 4.1 ChaCha stream cypher ----------------------------------------------------
 *
 * ChaCha is a family of stream cyphers by Daniel J. Bernstein and a predecessor
 * of the Salsa stream cypher.
 * It's recommended to use 20 round, but if performance is valued more than
 * security, the rounds can be reduced.
 *
 * Note that this implementation doesn't respect the endianness of seeds, as
 * they should be randomized anyway. Keep this in mind when using the code for
 * encryption purposes.
 */

/* Should be 20 for cryptographical security. */
#ifndef CSPRNG32_CHACHA_ROUNDS
# define CSPRNG32_CHACHA_ROUNDS 20
#endif

typedef struct {
	uint32_t s[16];
	int idx;
} CSPRNG32Chacha;

extern void csprng32_chacha_init(CSPRNG32Chacha *rng,
                                 uint32_t const seed[8],
                                 uint32_t const stream[2]);
extern void csprng32_chacha_randomize(void *rng);
extern uint32_t csprng32_chacha(void *rng);

#ifdef RANDOM_H_IMPLEMENTATION
void
csprng32_chacha_init(CSPRNG32Chacha *rng,
                     uint32_t const seed[8],
                     uint32_t const stream[2])
{
	rng->s[ 0] = 0x61707865; rng->s[ 1] = 0x3320646E;
	rng->s[ 2] = 0x79622D32; rng->s[ 3] = 0x6B206574;
	rng->s[ 4] = seed[0]; rng->s[ 5] = seed[1];
	rng->s[ 6] = seed[2]; rng->s[ 7] = seed[3];
	rng->s[ 8] = seed[4]; rng->s[ 9] = seed[5];
	rng->s[10] = seed[6]; rng->s[11] = seed[7];
	rng->s[12] = rng->s[13] = 0;
	rng->s[14] = stream[0];
	rng->s[15] = stream[1];
	rng->idx = 16;
}

void
csprng32_chacha_randomize(void *rng)
{
	uint32_t seed[8+2];
	trng_write(seed, sizeof seed);
	csprng32_chacha_init((CSPRNG32Chacha*)rng, seed, seed + 8);
}

/* The user should make sure not to call the generator more than 2^{64} times
 * with the same seed! */
uint32_t
csprng32_chacha(void *rng)
{
	#define CSPRNG32_CHACHA_ROTL(x,k) \
		(((x) << (k)) | ((x) >> (8 * sizeof(x) - (k))))
	#define CSPRNG32_CHACHA_QR(x, a, b, c, d) ( \
		x[a]+=x[b], x[d]^=x[a], x[d]=CSPRNG32_CHACHA_ROTL(x[d],16), \
		x[c]+=x[d], x[b]^=x[c], x[b]=CSPRNG32_CHACHA_ROTL(x[b],12), \
		x[a]+=x[b], x[d]^=x[a], x[d]=CSPRNG32_CHACHA_ROTL(x[d], 8), \
		x[c]+=x[d], x[b]^=x[c], x[b]=CSPRNG32_CHACHA_ROTL(x[b], 7))

	CSPRNG32Chacha *r = (CSPRNG32Chacha*)rng;

	if (r->idx >= 16) {
		size_t i;
		/* reset index */
		r->idx = 0;
		/* refill block cyphers */
		for (i = 0; i < CSPRNG32_CHACHA_ROUNDS; i += 2) {
			/* Odd round */
			CSPRNG32_CHACHA_QR(r->s, 0, 4,  8, 12);
			CSPRNG32_CHACHA_QR(r->s, 1, 5,  9, 13);
			CSPRNG32_CHACHA_QR(r->s, 2, 6, 10, 14);
			CSPRNG32_CHACHA_QR(r->s, 3, 7, 11, 15);
			/* Even round */
			CSPRNG32_CHACHA_QR(r->s, 0, 5, 10, 15);
			CSPRNG32_CHACHA_QR(r->s, 1, 6, 11, 12);
			CSPRNG32_CHACHA_QR(r->s, 2, 7,  8, 13);
			CSPRNG32_CHACHA_QR(r->s, 3, 4,  9, 14);
		}
		/* increment counter */
		if (++r->s[12] == 0) {
			++r->s[13];
			assert(r->s[13] != 0);
		}
	}
	return r->s[r->idx++];

	#undef CSPRNG32_CHACHA_ROTL
	#undef CSPRNG32_CHACHA_QR
}
#endif /* RANDOM_H_IMPLEMENTATION */


/*
 * 5. Random distributions =====================================================
 *
 * Just having random bits isn't that helpful for most applications.
 * We might want to generate random numbers in a specific range or under a
 * specific probability distribution.
 *
 * We implement functions to generate unbiased random numbers in ranges and
 * functions to generate normal distributions.
 */

/*
 * 5.1 Uniform integer distribution --------------------------------------------
 *
 * Let's tackle this with a quick example. Suppose we had a 4-bit RNG and want
 * to simulate dice rolls, how hard could it be?
 *
 * A widespread misconception is that one can just take the modulus:
 *     x = rand4() % 6 + 1;
 * But that doesn't add up, does it?
 *     Possible RNG output:     0|1|2|3|4|5|6|7   Histogram: 2|2|1|1|1|1
 *     Corresponding dice roll: 1|2|3|4|5|6|1|2              1|2|3|4|5|6
 *
 * Another only marginally better approach is to convert to and from floating-
 * point numbers:
 *     x = (int)(rand4() / 8.0 * 6.0) + 1;
 * This method spreads out the bias across the range, so it's less noticeable.
 *     Possible RNG output:     0|1|2|3|4|5|6|7   Histogram: 2|1|1|2|1|1
 *     Corresponding dice roll: 1|1|2|3|4|4|5|6              1|2|3|4|5|6
 * For this to work as expected, the floating-point type must have a mantissa
 * with at least as many bits as the generator generates. This requires the use
 * of long double for 64-bit numbers and specific knowledge about the platform.
 *
 * While these approaches are biased, you might still want to use them and trade
 * correctness for performance. Though many use-cases, like simulations or
 * cryptography, demand unbiased uniform integer distributions.
 *
 * The easiest way to achieve such an unbiased distribution is to just choose a
 * power-of-two range and bitmask the relevant bits. This should be used
 * whenever possible and preferably implemented using the bit and '&' operator
 * instead of the modulo operator '%', to document that this was on purpose and
 * not an accidentally unbiased case of the biased method.
 *
 * We can generalize the above approach by just dropping numbers that don't fit
 * into the range. In our dice example above we'd just call the RNG again if
 * the value is greater than 6:
 *     while ((x = rand4() + 1) > 6);
 *
 * We run into problems if the differences between the generators and our range
 * is large. A better apoach than restricting the generated size is to drop out
 * all of the biased states. This is achieved by calculating the number of
 * overrepesented digits and skipping any generated numbers that are smaller
 * than the count of those digits.
 *     uint32_t x, r = (-range) % range; // equivalent to '2^{32} \mod range'
 *     do x = rng(); while (x < r);
 *     return r % range;
 *
 * By removing the top part of the engine's range one can improve the algorithm
 * further, so that the expensive modulo operation is called less often: */

#define dist_UNIFORM_INT(type, rng, range, result) /* [0,range) */ \
	do { \
		type dist_UNIFORM_INT_x, dist_UNIFORM_INT_r; \
		do { \
			dist_UNIFORM_INT_x = (rng); \
			dist_UNIFORM_INT_r = (x) % (range); \
		} while (dist_UNIFORM_INT_x - dist_UNIFORM_INT_r  > -(range)); \
		*(result) = dist_UNIFORM_INT_r; \
	} while (0)

/* This macro can't be used inside of expressions and the following should hold
 * true about the arguments:
 *     - type should be the type returned by the rng.
 *     - rng is expected to be a call to the used rng.
 *     - range must have the same type as the return value of rng.
 *     - result is expected to be a pointer, to indicate it being modified.
 *
 * Lemire <11> proposed an almost division less algorithm, that exploits the
 * fact that modern processors can efficiently compute the upper part of a
 * multiplication. This can be used to reduces the bigger to the range of the
 * smaller factor, in a similar facion to modulo reduction:
 * E.g. uint64_t x = ((uint128_t)a * b) >> 64; reduces a to the range b in a
 * similar facion to a % b, although the mapping order is different<12>.
 */

static inline uint32_t
dist_uniform_u32(uint32_t range, /* [0,range) */
                 uint32_t (*rand32)(void*), void *rng)
{
	uint64_t m = (uint64_t)rand32(rng) * (uint64_t)range;
	uint32_t l = m;

	if (l < range) {
		uint32_t r = (-range) % range;
		while (l < r) {
			l = m = (uint64_t)rand32(rng) * (uint64_t)range;
		}
	}

	return m >> 32;
}

static inline uint64_t
dist_uniform_u64(uint64_t range, /* [0,range) */
                 uint64_t (*rand64)(void*), void *rng)
{
#if __SIZEOF_INT128__
	__uint128_t m = (__uint128_t)rand64(rng) * (__uint128_t)range;
	uint64_t l = m;

	if (l < range) {
		uint64_t r = (-range) % range;
		while (l < r) {
			l = m = (__uint128_t)rand64(rng) * (__uint128_t)range;
		}
	}

	return m >> 64;
#else /* fallback algorithm */
	uint64_t x, r;
	do {
		x = rand64(rng);
		r = x % range;
	} while (x - r > -range);
	return r;
#endif
}


/*
 * 5.2 Uniform real distribution -----------------------------------------------
 *
 * Generating uniform random floating-point numbers might seem easy at first
 * glance, just cast the output of a PRNG to the desired float type and divide
 * it by the biggest possible output of the PRNG to obtain a random float
 * between 0 and 1 that can now be scaled up.
 * Though as before this straightforward approach is biased.
 *
 * For the next part, I assume you are already acquainted with the IEEE 754
 * floating-point standard <13>, we will not cover other formats.
 *
 * To recap, here is a quick reminder on how 32-bit floats are laid out in
 * memory:
 *
 *     sign  exponent           fraction
 *     +/-  bias: -126
 *     [X]  [XXXXXXXX]  [XXXXXXXXXXXXXXXXXXXXXXX]
 *
 * The interpretation of the data is dependant on the value of the exponent:
 *
 *     If 0 < exp < 255:
 *         +/- 1.fraction * 2^{exponent - 126}
 *     If exp = 0 (demormalized):
 *         +/- 0.fraction * 2^{-126}
 *     If exp = 255:
 *         If fraction = 0:
 *             >+/- inf
 *         If fraction != 0:
 *             NaN
 *
 * Note that the 1/0.fraction part is also called the mantissa.
 *
 * The varying exponent allows floating-point numbers to represent a huge range
 * of values. What makes random float generation tricky is the fact that the
 * amount of numbers less than 1 and greater than 1 is the same. This means that
 * we can't just divide a random integer among these values without getting
 * duplicates and omissions <14>:
 *
 *    Integer: |----|----|----|----|----|----|----|----|----|----|----|----|
 *             |    |    /    |   /    /    /    /      \ __/    \ __/     |
 *    Float:   ||||-|-|-|--|--|--|----|----|----|--------|--------|--------|
 *
 * There are multiple ways to subvert this problem.
 *
 * You can use the division method described above with a divider that
 * guarantees equal spacing between generated floating-point numbers.
 * This is less biased, but can't generate all possible representable values
 * between 0 and 1 and can lose equal spacing when scaling up a specific range.
 * Nevertheless, this is a good tradeoff between speed and accuracy.
 */

static inline float
dist_uniformf(uint32_t x)
{
	return (x >> (32 - FLT_MANT_DIG)) *
	       (1.0f / (UINT32_C(1) << FLT_MANT_DIG));
}

static inline double
dist_uniform(uint64_t x)
{
	return (x >> (64 - DBL_MANT_DIG)) *
	       (1.0 / (UINT64_C(1) << DBL_MANT_DIG));
}

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * WARNING: this code does currently not work correctly for subnormals.
 * The probability of subnormals isn't quite correct, and there needs to be
 * a new special case for subnormals when both a and b are subnormals.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *
 * Another solution is to generate every representable floating-point number
 * with a probability proportional to the covered real number range. <15>
 * So obtaining a number in a floating-point subrange [s1,s2] of the output
 * range [r1,r2] has the probability of (s2-s1)/(r2-r1).
 *
 *     r1=5                   r2=25
 *        |||||||||||||||||||||     -->  (15-10)/(25-5) = 0.25
 *             |    |
 *         s1=10    s2=15
 *
 * This property is obtained by randomizing the fraction uniformly and
 * randomizing the exponent, with the probability 2^{-x} for every possible
 * exponent 'x'.
 * This can be achieved programmatically by generating random bits and
 * decrementing the maximal exponent until one of the generated bits is set,
 * thus halving the probability of the next decrement every time.
 * Finally, we need to reject all values that are outside of the desired range.
 *
 * Before we go into the implementation, let's compare the quality of generated
 * floating-point numbers between dist_uniformf and dist_uniformf_dense:
 *
 * The difference in the expected probability and the empirical result of a
 * random floating-point in [r1,r2] also being inside [s1,s2] are plotted in the
 * histograms below.
 * Here [r1,r2] was randomly choosen inside [-10^{-6},10^{-6}] and [s1,s2]
 * randomly choosen inside [r1,r2].
 * Then random floats between 0 and 1 were generated using dist_uniformf and
 * dist_uniformf_dense and rejected if they weren't inside [r1,r2].
 * From those that get through, we count the probability of them also being
 * inside [s1,s2] and plott (P(expected)-P(empirical))/P(expected),
 * with the expected probability calculated as described above.
 * The rejection process is necessary to expose the defects in dist_uniformf,
 * as they only show up in the smaller subranges.
 * With larger ranges (e.g. [r1=-0.1,r2=0.1]) the two plots are equally normal
 * distributed.
 *
 *    150 +---------------------------+     100 +---------------------------+
 *        |      +      **     +      |         |    +   +    +    +   +    |
 *        |             **   dist_uni-|         |                  dist_uni-|
 *        |             ** formf_dense|         |                  *   formf|
 *        |             **            |         |                  *        |
 *    100 |-+           **          +-|         |                  *        |
 *        |             **            |         |                  *        |
 *        |             **            |         |                 **        |
 *        |            ****           |      50 |-+               **   *  +-|
 *        |           *****           |         |                 ***  *    |
 *        |           *******         |         |                 ***  *    |
 *     50 |-+         *******       +-|         |                ****  *    |
 *        |           *******         |         |                ****  *    |
 *        |         *********         |         |                ****  *    |
 *        |         **********        |         |               ****** *    |
 *        |   ...***************...   |         |   . . *.  ...*******.*    |
 *      0 +---------------------------+       0 +---------------------------+
 *       -1    -0.5     0     0.5     1        -4   -3  -2   -1    0   1    2
 *
 * We can observe that the dist_uniform plot has a big spike at 1 and deviates
 * further in the negative direction.
 * The spike at 1 can be explained by dist_uniform not being able to generate
 * any float that lies in the range [s1,s2] when s2-s1 is too small, hence:
 *     (P(exp)-P(emp))/P(exp) = (P(exp)-0)/P(exp) = 1
 * The bigger deviation in the negative x-axis happens because some
 * probabilities are too likely when mapping integers to floating-points
 * directly:
 *
 *    Integer: |----|----|----|----|----|----|----|----|----|----|----|----|
 *             |    |    /    |   /    /    /    /      \ __/    \ __/     |
 *    Float:   ||||-|-|-|--|--|--|----|----|----|--------|--------|--------|
 *                  |   |
 *                 [     ] <-- This range would be too likely
 */

extern float dist_uniformf_dense(
		float a, float b, /* [a,b] */
		uint32_t (*rand32)(void*), void *rng);

extern double
dist_uniform_dense(
		double a, double b, /* [a,b] */
		uint64_t (*rand64)(void*), void *rng);

#ifdef RANDOM_H_IMPLEMENTATION

/* We'll begin by writing a macro that decrements the exponent until one bit is
 * set, using __builtin_ctz if it's available. */
# if __GNUC__ >= 4 || __clang_major__ >= 2 || \
     (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__clang_major__ == 1 && __clang_minor__ >= 5)
#  if UINT_MAX >= UINT32_MAX
#   define DIST_UNIFORMF_DENSE_DEC_CTZ(exp, x) ((exp) -= (uint32_t)__builtin_ctz(x))
#  elif ULONG_MAX >= UINT32_MAX
#   define DIST_UNIFORMF_DENSE_DEC_CTZ(exp, x) ((exp) -= (uint32_t)__builtin_ctzl(x))
#  endif
#  if UINT_MAX >= UINT64_MAX
#   define DIST_UNIFORM_DENSE_DEC_CTZ(exp, x) ((exp) -= (uint64_t)__builtin_ctz(x))
#  elif ULONG_MAX >= UINT64_MAX
#   define DIST_UNIFORM_DENSE_DEC_CTZ(exp, x) ((exp) -= (uint64_t)__builtin_ctzl(x))
#  elif ULLONG_MAX >= UINT64_MAX
#   define DIST_UNIFORM_DENSE_DEC_CTZ(exp, x) ((exp) -= (uint64_t)__builtin_ctzll(x))
#  endif
# endif

/* Otherwise, we'll use a lookup table and a De Bruijn sequence to calculate
 * the number of trailing zeros. <16>
 *
 * Given an initial random number 'n', we calculate m = n & -n,
 * this will only set the last set bit in 'n' inside 'm'.
 * Now we have a distinct value for every possible number of trailing zeros and
 * a perfect hash function can be constructed that maps every potential value of
 * m to the number of trailing zeros.
 *
 * This is done by multiplying the modified number by a De Bruijn constant.
 * A De Bruijn constant contains all possible binary values of n-bits in all
 * subranges of n-bits. E.g. for n = 3:
 *                               0b00011101 = 232
 *                                 000        = 0
 *                                  001       = 1
 *                                   011      = 3
 *                                    111     = 7
 *                                     110    = 6
 *                                      101   = 5
 *                                       010  = 2
 *                                        100 = 4
 *
 * 'm' is a power-of-two, as only one bit is set, hence a multiplication with
 * the De Bruijn constant shifts it by its power.
 * Thus we get a distinct De Bruijn subsequence for every possible power and the
 * index can now be determined via a lookup table. */

# ifndef DIST_UNIFORMF_DENSE_DEC_CTZ
#  define DIST_UNIFORMF_DENSE_DEC_CTZ(exp, x) do { \
		static char const deBruijn[32] = { \
			0, 1, 28,2,29,14,24,3,30,22,20,15,25,17, 4,8, \
			31,27,13,23,21,19,16,7,26,12,18, 6,11, 5,10,9 }; \
		(exp) -= deBruijn[(((x) & -(x)) * 0x077CB531u) >> 27]; \
	} while (0)
# endif

# ifndef DIST_UNIFORM_DENSE_DEC_CTZ
#  define DIST_UNIFORM_DENSE_DEC_CTZ(exp, x) do { \
		static char const deBruijn[64] = { \
			0,  1, 2,53, 3, 7,54,27, 4,38,41, 8,34,55,48,28, \
			62, 5,39,46,44,42,22, 9,24,35,59,56,49,18,29,11, \
			63,52, 6,26,37,40,33,47,61,45,43,21,23,58,17,10, \
			51,25,36,32,60,20,57,16,50,31,19,15,30,14,13,12 }; \
		(exp) -= deBruijn[(((x) & -(x)) * 0x022FDD63CC95386Du) >> 58]; \
	} while (0)
# endif

float
dist_uniformf_dense(
		float a, float b, /* [a,b] */
		uint32_t (*rand32)(void*), void *rng)
{
	#ifdef __cplusplus
		struct { float f; uint32_t i; } u;
	#else
		union { float f; uint32_t i; } u;
	#endif
	enum { SIGN_POS = 0, SIGN_RAND = 1, SIGN_NEG = 2 };
	int sign;
	uint32_t minexp, minfrac, maxexp, maxfrac;

	#define DIST_UNIFORMF_DENSE_FRAC_MASK \
			((UINT32_C(1) << (FLT_MANT_DIG - 1)) - 1)

	/* make sure a is smaller than b */
	assert(a < b);

	{
		/* calculate min and max with respect to signedness */
		float min, max;
		switch ((sign = (a < 0.0f) + (b < 0.0f))) {
		case SIGN_POS: min = a; max = b; break;
		case SIGN_RAND: min = 0; max = (b > -a) ? b : a; break;
		case SIGN_NEG: min = b; max = a; break;
		}
		/* extract the minimum and maximum exponent and fraction */
		#ifdef __cplusplus
			memcpy(&u.i, &min, sizeof u.i);
		#else
			u.f = min;
		#endif
		minexp = (u.i << 1) >> (FLT_MANT_DIG);
		minfrac = u.i & DIST_UNIFORMF_DENSE_FRAC_MASK;
		#ifdef __cplusplus
			memcpy(&u.i, &max, sizeof u.i);
		#else
			u.f = max;
		#endif
		maxexp = (u.i << 1) >> (FLT_MANT_DIG);
		maxfrac = u.i & DIST_UNIFORMF_DENSE_FRAC_MASK;
	}

	/* optimize special case where the exponents are the same */
	if (minexp == maxexp) {
		u.i = (minexp << (FLT_MANT_DIG - 1)) |
		       (dist_uniform_u32(maxfrac - minfrac + 1, rand32, rng) +
		       minfrac);

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= rand32(rng) << 31;
		else if (sign == SIGN_NEG)
			u.i |= UINT32_C(1) << 31;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif
		return u.f;
	}

	/* optimize special case where the exponents offset by one
	 * (this doesn't work for demormalized numbers) */
	if (minexp + 1 == maxexp && minexp > 0) {
		uint32_t const invminfrac = DIST_UNIFORMF_DENSE_FRAC_MASK - minfrac;
		uint32_t const range = invminfrac + maxfrac + 1;
		uint32_t frac, exp, x;
		size_t i = 0;

		while (1) {
			/* make sure three bits are always set, two for the
			 * rejection and one for the sign. */
			if (i <= 3) {
				x = rand32(rng);
				i = 32;
			}

			/* use maxexp if the first bit is set and minexp
			 * if the second bit is set, otherwise repeat. */
			if (x & 1) {
				i -= 1, x >>= 1;
				exp = maxexp;
				frac = dist_uniform_u32(range, rand32, rng);
				if (frac <= maxfrac)
					break;
			} else if (x & 2) {
				i -= 2, x >>= 2;
				exp = minexp;
				frac = dist_uniform_u32(range, rand32, rng);
				if (frac <= invminfrac) {
					frac = DIST_UNIFORMF_DENSE_FRAC_MASK - frac;
					break;
				}
			} else {
				i -= 2, x >>= 2;
			}
		}

		/* combine exp with a fraction */
		u.i = (exp << (FLT_MANT_DIG - 1)) | frac;

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= x << 31;
		else if (sign == SIGN_NEG)
			u.i |= UINT32_C(1) << 31;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif
		return u.f;
	}

	while (1) {
		uint32_t exp, x;

		/* decrement exp until at least one bit is set */
		exp = maxexp;
		while ((x = rand32(rng)) == 0)
			exp -= 32;

		/* decrement exp by the number of trailing zeros */
		DIST_UNIFORMF_DENSE_DEC_CTZ(exp, x);

		/* clamp exp to 0 */
		if ((exp < minexp) || (exp > maxexp))
			exp = 0;

		/* combine exp with a random fraction */
		x = rand32(rng);
		u.i = (exp << (FLT_MANT_DIG - 1)) | (x >> (33 - FLT_MANT_DIG));

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= x << 31;
		else if (sign == SIGN_NEG)
			u.i |= UINT32_C(1) << 31;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif

		/* accept if in range */
		if (u.f >= a && u.f <= b)
			return u.f;
	}
	#undef DIST_UNIFORMF_DENSE_FRAC_MASK
}

double
dist_uniform_dense(
		double a, double b, /* [a,b] */
		uint64_t (*rand64)(void*), void *rng)
{
	#ifndef __cplusplus
		union { double f; uint64_t i; } u;
	#else
		struct { double f; uint64_t i; } u;
	#endif
	enum { SIGN_POS = 0, SIGN_RAND = 1, SIGN_NEG = 2 };
	int sign;
	uint64_t minexp, minfrac, maxexp, maxfrac;

	#define DIST_UNIFORM_DENSE_FRAC_MASK \
			((UINT64_C(1) << (DBL_MANT_DIG - 1)) - 1)

	/* make sure a is smaller than b */
	assert(a < b);

	{
		/* calculate min and max with respect to signedness */
		double min, max;
		switch ((sign = (a < 0.0) + (b < 0.0))) {
		case SIGN_POS: min = a; max = b; break;
		case SIGN_RAND: min = 0; max = (b > -a) ? b : -a; break;
		case SIGN_NEG: min = b; max = a; break;
		}
		/* extract the minimum and maximum exponent and fraction */
		#ifdef __cplusplus
			memcpy(&u.i, &min, sizeof u.i);
		#else
			u.f = min;
		#endif
		minexp = (u.i << 1) >> (DBL_MANT_DIG);
		minfrac = u.i & DIST_UNIFORM_DENSE_FRAC_MASK;
		#ifdef __cplusplus
			memcpy(&u.i, &max, sizeof u.i);
		#else
			u.f = max;
		#endif
		maxexp = (u.i << 1) >> (DBL_MANT_DIG);
		maxfrac = u.i & DIST_UNIFORM_DENSE_FRAC_MASK;
	}

	/* optimize special case where the exponents are the same */
	if (minexp == maxexp) {
		u.i = (minexp << (DBL_MANT_DIG - 1)) |
		       (dist_uniform_u64(maxfrac - minfrac + 1, rand64, rng) +
		       minfrac);

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= rand64(rng) << 63;
		else if (sign == SIGN_NEG)
			u.i |= UINT64_C(1) << 63;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif
		return u.f;
	}

	/* optimize special case where the exponents offset by one
	 * (this doesn't work for demormalized numbers) */
	if (minexp + 1 == maxexp && minexp > 0) {
		uint64_t const invminfrac = DIST_UNIFORM_DENSE_FRAC_MASK - minfrac;
		uint64_t const range = invminfrac + maxfrac + 1;
		uint64_t frac, exp, x;
		size_t i = 0;

		while (1) {
			/* make sure three bits are always set, two for the
			 * rejection and one for the sign. */
			if (i <= 3) {
				x = rand64(rng);
				i = 64;
			}

			/* use maxexp if the first bit is set and minexp
			 * if the second bit is set, otherwise repeat. */
			if (x & 1) {
				i -= 1, x >>= 1;
				exp = maxexp;
				frac = dist_uniform_u64(range, rand64, rng);
				if (frac <= maxfrac)
					break;
			} else if (x & 2) {
				i -= 2, x >>= 2;
				exp = minexp;
				frac = dist_uniform_u64(range, rand64, rng);
				if (frac <= invminfrac) {
					frac = DIST_UNIFORM_DENSE_FRAC_MASK - frac;
					break;
				}
			} else {
				i -= 2, x >>= 2;
			}
		}

		/* combine exp with a fraction */
		u.i = (exp << (DBL_MANT_DIG - 1)) | frac;

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= x << 63;
		else if (sign == SIGN_NEG)
			u.i |= UINT64_C(1) << 63;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif
		return u.f;
	}

	while (1) {
		uint64_t exp, x;

		/* decrement exp until at least one bit is set */
		exp = maxexp;
		while ((x = rand64(rng)) == 0)
			exp -= 64;

		/* decrement exp by the number of trailing zeros */
		DIST_UNIFORM_DENSE_DEC_CTZ(exp, x);

		/* clamp exp to 0 */
		if ((exp < minexp) || (exp > maxexp))
			exp = 0;

		/* combine exp with a random fraction */
		x = rand64(rng);
		u.i = (exp << (DBL_MANT_DIG - 1)) | (x >> (65 - DBL_MANT_DIG));

		/* apply signedness */
		if (sign == SIGN_RAND)
			u.i |= x << 63;
		else if (sign == SIGN_NEG)
			u.i |= UINT64_C(1) << 63;

		#ifdef __cplusplus
			memcpy(&u.f, &u.i, sizeof u.f);
		#endif

		/* accept if in range */
		if (u.f >= a && u.f <= b)
			return u.f;
	}
	#undef DIST_UNIFORM_DENSE_FRAC_MASK
}

# undef DIST_UNIFORMF_DENSE_DEC_CTZ
# undef DIST_UNIFORM_DENSE_DEC_CTZ

#endif /* RANDOM_H_IMPLEMENTATION */

/*
 * 5.3 Normal real distribution ------------------------------------------------
 *
 * One of the fastest algorithms for generating normally distributed random
 * numbers is the ziggurat method <17><18>. It uses a lookup table and thus
 * requires a somewhat large amount of memory.
 * As this might not meet everybodie's needs we also implement the ratio method
 * <19><20>, which doesn't require any internal state and is a bit slower.
 *
 * The more popular polar method might be faster than the ratio method (though
 * not the ziggurat method), but generates two values at a time.
 * We'd need to discard one of these values as our function shouldn't have any
 * state, hence the ratio method is faster in our use case.
 *     polar method: 660 ns (without discarding the second number: 330ns)
 *     ratio method: 368 ns
 *     <https://github.com/ampl/gsl/blob/master/randist/gauss.c>
 */



/*
 * 5.3.1 Ratio method ..........................................................
 *
 * The ration method uses the fact that the ratio of a pair of random variables
 * (u,v) uniformaly distributed over
 *     C_f = \{(u,v): 0 <= u <= \sqrt{f(v/u)}\}
 * yields a random variable X=(v/u) with the density f. <19>
 *
 * If we apply this to the normal distribution f(x) = \exp(-(v^2)/2)
 * then u <= \sqrt{\exp(-((v/u)^2)/2)} = \exp(-(v^2)/(4u^2))
 * and v_{1,2} <= +/- 2u\sqrt{-\ln(u)} or in a more easily computable format
 * v^2 <= -4u^2\ln(u).
 *
 * To obtain (u,v) uniformaly over C_f, we'll use a acception-rejection method,
 * similar to the one used in dist_UNIFORM_INT. We generate (u,v) uniformly in
 * the bounding box B = \{ 0 < u < 1; -\sqrt(2/e) < v < \sqrt(2/e) \} of C_f
 * and reject everything that isn't inside of C_f (if v^2 > -4u^2\ln u).
 *
 * As computing the natural log is rather expensive, Leva <20> proposed a set of
 * quadratic bounding curves that drastically decrease the calls to \ln:
 *             r_2 < Q(u,v) = (u-s)^2 - b(u-s)(v+t) + a(u-t)^2 < r_1
 *                      s=0.449871  a=0.19600   t=0.386595
 *                      b=0.25472 r_1=0.27597 r_2=0.27846
 */
extern float dist_normalf(uint32_t (*rand32)(void*), void *rng);
extern double dist_normal(uint64_t (*rand64)(void*), void *rng);

#ifdef RANDOM_H_IMPLEMENTATION

float
dist_normalf(uint32_t (*rand32)(void*), void *rng)
{
	static float const s = 0.449871f, t = 0.386595f, a = 0.19600f;
	static float const b = 0.25472f, r1 = 0.27597f, r2 = 0.27846f;
	static float const m = 1.715527769921414f; /* 2*\sqrt{2/e} */
	float u, v, x, y, Q;

	do {
		/* (0,1] to avoid division by 0 */
		u = 1 - dist_uniformf(rand32(rng));

		/* v is in the iterval [-m/2, m/2), however v=-m/2 is rejected
		 * later, so it's effectivly (-m/2, m/2). */
		v = (dist_uniformf(rand32(rng)) - 0.5f) * m;

		/* Evaluate quadratic bounding curves */
		x = u - s;
		y = fabsf(v) + t;
		Q = x*x + y*(a*y - b*x);

		/* log is only evaluated 0.012 times per function call */
	} while (Q >= r1 && (Q > r2 || v*v > -4*u*u * logf(u)));

	return v / u;
}

double
dist_normal(uint64_t (*rand64)(void*), void *rng)
{
	static double const s = 0.449871, t = 0.386595, a = 0.19600;
	static double const b = 0.25472, r1 = 0.27597, r2 = 0.27846;
	static double const m = 1.715527769921414; /* 2*\sqrt{2/e} */
	double u, v, x, y, Q;

	do {
		/* (0,1] to avoid division by 0 */
		u = 1 - dist_uniform(rand64(rng));

		/* v is in the iterval [-m/2, m/2), however v=-m/2 is rejected
		 * later, so it's effectivly (-m/2, m/2). */
		v = (dist_uniform(rand64(rng)) - 0.5) * m;

		/* Evaluate quadratic bounding curves */
		x = u - s;
		y = fabs(v) + t;
		Q = x*x + y*(a*y - b*x);

		/* log is only evaluated 0.012 times per function call */
	} while (Q >= r1 && (Q > r2 || v*v > -4*u*u * log(u)));

	return v / u;
}

#endif /* RANDOM_H_IMPLEMENTATION */

/*
 * 5.3.2 Ziggurat method .......................................................
 *
 * The ziggurat method works by partitioning the normal distribution density
 *                          f(x) = \exp(-(x^2)/2)
 * into horizontal blocks of equal area, where all boxes are rectangular
 * shaped, except for the bottom one, which is trailing of to infinity. <17>
 *
 *                    +-------------------------------------+
 *                    |              \exp(-(x^2)/2) ******  |
 *                y_3 +****-------                          |
 *                    |    ***   |                          |
 *                    |      **  |                          |
 *                    | Box 3 ***|                          |
 *                y_2 +---------***----                     |
 *                    |          :**  |                     |
 *                    | Box 2    : ***|                     |
 *                y_1 +--------------***----                |
 *                    | Box 1         :*** |                |
 *                y_0 +------------------****               |
 *                    | Box 0              :******          |
 *                    +----------+----+----+----------------+
 *                                x_3  x_2  x_1=R
 *
 * To calculate the coordinates of the boxes given N boxes,
 * we need to find the corresponding constant R=x_1,
 * from which we can compute the area 'V' and the rest of the coordinates:
 *   V = R * f(R) - \sqrt(\pi) * (\sqrt(2) * \erf(R/\sqrt(2)) - \sqrt(2)) / 2
 *                        x_n = f^-1(V / x_{n-1} + f(x_{n-1}))
 * The calculation of 'R' is a bit more tricky, we need to solve the expression
 * above for x_N=0, which can be done by arithmetically narrowing down on value
 * for R (code under tools/random/ziggurat-constants.c).
 *
 * The exact algorithm is based on Doorki's implementation <18>,
 * but we don't use a lookup for the initial bound check,
 *     if (u * x[idx] < x[idx + 1])
 * because the multiplication this saves is calculated at return regardless.
 * This halves the memory footprint thus allows bigger lookup tables, which
 * saves more time in the long run.
 */

#ifndef DIST_NORMALF_ZIG_COUNT
# define DIST_NORMALF_ZIG_COUNT 128
# define DIST_NORMALF_ZIG_R     3.4426198558966522559f
# define DIST_NORMALF_ZIG_AREA  0.00991256303533646112916f
#elif DIST_NORMALF_ZIG_COUNT > 128
# error random.h: DIST_NORMALF_ZIG_COUNT must be a power of two rand <= 128
#endif

typedef struct {
	float x[DIST_NORMALF_ZIG_COUNT + 1];
} DistNormalfZig;

extern void dist_normalf_zig_init(DistNormalfZig *zig);
extern float dist_normalf_zig(DistNormalfZig const *zig,
                              uint32_t (*rand32)(void*), void *rng);

#ifndef DIST_NORMAL_ZIG_COUNT
# define DIST_NORMAL_ZIG_COUNT 256
# define DIST_NORMAL_ZIG_R     3.65415288536100716461
# define DIST_NORMAL_ZIG_AREA  0.00492867323397465524494
#elif DIST_NORMAL_ZIG_COUNT > 1024
# error random.h: DIST_NORMAL_ZIG_COUNT must be a power of two and <= 1024
#endif

typedef struct {
	double x[DIST_NORMAL_ZIG_COUNT + 1];
} DistNormalZig;

extern void dist_normal_zig_init(DistNormalZig *zig);
extern double dist_normal_zig(DistNormalZig const *zig,
                              uint64_t (*rand64)(void*), void *rng);

#ifdef RANDOM_H_IMPLEMENTATION

void
dist_normalf_zig_init(DistNormalfZig *zig)
{
	size_t i;
	float f = expf(-0.5f * DIST_NORMALF_ZIG_R * DIST_NORMALF_ZIG_R);
	zig->x[0] = (float)DIST_NORMALF_ZIG_AREA / f;
	zig->x[1] = DIST_NORMALF_ZIG_R;

	for (i = 2; i < DIST_NORMALF_ZIG_COUNT; ++i) {
		float xx = logf((float)DIST_NORMALF_ZIG_AREA /
		                     zig->x[i - 1] + f);
		zig->x[i] = sqrtf(-2 * xx);
		f = expf(xx);
	}

	zig->x[DIST_NORMALF_ZIG_COUNT] = 0;
}

float
dist_normalf_zig(DistNormalfZig const *zig, uint32_t (*rand32)(void*),
                 void *rng)
{
	float x, y, f0, f1, uf32;
	uint32_t u32, idx;
	union { uint32_t i; float f; } u;

	while (1) {
		/* To minimize calls to the rng we, use every bit for its own
		 * purposes:
		 *    - The MANT_DIG most significant bits are used to generate
		 *      a random floating-point number
		 *    - The least significant bit is used to randomly set the
		 *      sign of the return value
		 *    - The second to the (DIST_NORMALF_ZIG_COUNT+1)th
		 *      least significant bit are used to generate a index in
		 *      the range [0,DIST_NORMALF_ZIG_COUNT)
		 *
		 * Since we can't rely on dist_uniformf adhering to this order,
		 * we define a custom conversion macro: */
		#define DIST_NORMALF_ZIG_2FLT(x) \
			(((x) >> (32 - FLT_MANT_DIG)) * \
			 (1.0f / (UINT32_C(1) << FLT_MANT_DIG)))

		u32 = rand32(rng);
		idx = (u32 >> 1) & (DIST_NORMALF_ZIG_COUNT - 1);
		uf32 = DIST_NORMALF_ZIG_2FLT(u32) * zig->x[idx];

		/* Take a random box (box[idx])
		 * and get the value of a random x-coordinate inside it.
		 * If it's also inside box[idx + 1] we already know to accept
		 * this value. */
		if (uf32 < zig->x[idx + 1])
			break;

		/* If our random box is at the bottom, we can't use the lookup
		 * table and need to generate a variable for the trail of the
		 * normal distribution, as described in <21>: */
		if (idx == 0) {
			do {
				x = logf(1-DIST_NORMALF_ZIG_2FLT(rand32(rng))) *
				    1.0f / DIST_NORMALF_ZIG_R;
				y = logf(1-DIST_NORMALF_ZIG_2FLT(rand32(rng)));
			} while (-(y + y) < x * x);
			return u32 & 1u ?
				x - DIST_NORMALF_ZIG_R :
				DIST_NORMALF_ZIG_R - x;
		}

		/* Take a random x-coordinate U in between x[idx] and x[idx+1]
		 * and return x if U is inside of the normal distribution,
		 * otherwise, repeat the entire ziggurat method. */
		y = uf32 * uf32;
		f0 = expf(-0.5f * (zig->x[idx]     * zig->x[idx]     - y));
		f1 = expf(-0.5f * (zig->x[idx + 1] * zig->x[idx + 1] - y));
		if (f1 + DIST_NORMALF_ZIG_2FLT(rand32(rng)) * (f0 - f1) < 1)
			break;

		#undef DIST_NORMALF_ZIG_2FLT
	}

	#ifdef __cplusplus
		memcpy(&idx, &uf32, sizeof uf32);
		idx |= (u32 & 1) << 31;
		memcpy(&uf32, &idx, sizeof uf32);
		return uf32;
	#else
		return u.f = uf32, u.i |= (u32 & 1) << 31, u.f;
	#endif
}

void
dist_normal_zig_init(DistNormalZig *zig)
{
	size_t i;
	double f = exp(-0.5 * DIST_NORMAL_ZIG_R * DIST_NORMAL_ZIG_R);
	zig->x[0] = (double)DIST_NORMAL_ZIG_AREA / f;
	zig->x[1] = DIST_NORMAL_ZIG_R;

	for (i = 2; i < DIST_NORMAL_ZIG_COUNT; ++i) {
		double xx = log((double)DIST_NORMAL_ZIG_AREA /
		                     zig->x[i - 1] + f);
		zig->x[i] = sqrt(-2 * xx);
		f = exp(xx);
	}

	zig->x[DIST_NORMAL_ZIG_COUNT] = 0;
}

double
dist_normal_zig(DistNormalZig const *zig, uint64_t (*rand64)(void*), void *rng)
{
	double x, y, f0, f1, uf64;
	uint64_t u64, idx;
	union { uint64_t i; double f; } u;

	while (1) {
		/* To minimize calls to the rng we, use every bit for its own
		 * purposes:
		 *    - The MANT_DIG most significant bits are used to generate
		 *      a random floating-point number
		 *    - The least significant bit is used to randomly set the
		 *      sign of the return value
		 *    - The second to the (DIST_NORMAL_ZIG_COUNT+1)th
		 *      least significant bit are used to generate a index in
		 *      the range [0,DIST_NORMAL_ZIG_COUNT)
		 *
		 * Since we can't rely on dist_uniformf adhering to this order,
		 * we define a custom conversion macro: */
		#define DIST_NORMAL_ZIG_2DBL(x) \
			(((x) >> (64 - DBL_MANT_DIG)) * \
			 (1.0 / (UINT64_C(1) << DBL_MANT_DIG)))

		u64 = rand64(rng);
		idx = (u64 >> 1) & (DIST_NORMAL_ZIG_COUNT - 1);
		uf64 = DIST_NORMAL_ZIG_2DBL(u64) * zig->x[idx];

		/* Take a random box (box[idx])
		 * and get the value of a random x-coordinate inside it.
		 * If it's also inside box[idx + 1] we already know to accept
		 * this value. */
		if (uf64 < zig->x[idx + 1])
			break;

		/* If our random box is at the bottom, we can't use the lookup
		 * table and need to generate a variable for the trail of the
		 * normal distribution, as described in <21>: */
		if (idx == 0) {
			do {
				x = log(1 - DIST_NORMAL_ZIG_2DBL(rand64(rng))) *
				    1.0 / DIST_NORMAL_ZIG_R;
				y = log(1 - DIST_NORMAL_ZIG_2DBL(rand64(rng)));
			} while (-(y + y) < x * x);
			return u64 & 1u ?
				x - DIST_NORMAL_ZIG_R :
				DIST_NORMAL_ZIG_R - x;
		}

		/* Take a random x-coordinate U in between x[idx] and x[idx+1]
		 * and return x if U is inside of the normal distribution,
		 * otherwise, repeat the entire ziggurat method. */
		y = uf64 * uf64;
		f0 = exp(-0.5 * (zig->x[idx]     * zig->x[idx]     - y));
		f1 = exp(-0.5 * (zig->x[idx + 1] * zig->x[idx + 1] - y));
		if (f1 + DIST_NORMAL_ZIG_2DBL(rand64(rng)) * (f0 - f1) < 1.0)
			break;

		#undef DIST_NORMAL_ZIG_2DBL
	}

	#ifdef __cplusplus
		memcpy(&idx, &uf64, sizeof uf64);
		idx |= (u64 & 1) << 63;
		memcpy(&uf64, &idx, sizeof uf64);
		return uf64;
	#else
		return u.f = uf64, u.i |= (u64 & 1) << 63, u.f;
	#endif
}

#endif /* RANDOM_H_IMPLEMENTATION */


/*
 * 5.3.3 Approximation using a binomially distribution .........................
 *
 *
 * We can approximate a normal distribution using a binomial distribute <22>:
 * Just generate a random power-of-2 integer, and count the number of set bits.
 * This gives us a normal distributed step function, now we just need to add a
 * bit of randomness into the step, and nudge the normal distribution into the
 * correct range.
 */

# if __GNUC__ >= 4 || __clang_major__ >= 2 || \
     (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__clang_major__ == 1 && __clang_minor__ >= 5)
#  if UINT_MAX >= UINT64_MAX
#   define DIST_NORMALF_POPCOUNT64 __builtin_popcount
#  elif ULONG_MAX >= UINT64_MAX
#   define DIST_NORMALF_POPCOUNT64 __builtin_popcountl
#  elif ULLONG_MAX >= UINT64_MAX
#   define DIST_NORMALF_POPCOUNT64 __builtin_popcountll
#  else
#   define DIST_NORMALF_POPCOUNT64 dist_normalf_popcount64
#  endif
# endif

/* TODO: explain */
static inline int
dist_normalf_popcount64(uint64_t x)
{
	/* 2-bit sums */
	x -= (x >> 1) & (-(uint64_t)1/3);
	/* 4-bit sums */
	x = (x & (-(uint64_t)1/15*3)) + ((x >> 2) & (-(uint64_t)1/15*3));
	/* 8-bit sums */
	x = (x + (x >> 4)) & (-(uint64_t)1/255*15);
	/* sum 8-bit sums */
	return (x * (-(uint64_t)1/255)) >> (sizeof(uint64_t) - 1) * CHAR_BIT;
}

static inline double
dist_normalf_fast(uint64_t u)
{
	/* since we are approximating anyway, we might as well scramble u
	 * multiple times to get a better resolution */
	float x = DIST_NORMALF_POPCOUNT64(u*0x2c1b3c6dU) +
	          DIST_NORMALF_POPCOUNT64(u*0x297a2d39U) - 64;
	x += (int64_t)u * (1 / 9223372036854775808.0f);
	x *= 0.1765469659009499f; /* sqrt(1/(32 + 4/12)) */
	return x;
}

/*
 * 6. Shuffling ================================================================
 *
 * Many programs require a random selection of n data points, e.g. a random
 * permutation of an array.
 * This can be done with multiple approaches, most popularly via shuffling the
 * array. */

extern void shuf32_arr(void *base, uint32_t nel, uint32_t size,
                       uint32_t (*rand32)(void*), void *rng);
extern void shuf64_arr(void *base, uint64_t nel, uint64_t size,
                       uint64_t (*rand64)(void*), void *rng);

#ifdef RANDOM_H_IMPLEMENTATION

void
shuf32_arr(void *base, uint32_t nel, uint32_t size,
         uint32_t (*rand32)(void*), void *rng)
{
	unsigned char *b = (unsigned char*)base;
	while (nel > 1) {
		uint32_t s = size;
		unsigned char *r = b + size *
		                   dist_uniform_u32(nel, rand32, rng);
		unsigned char *l = b + size * (--nel);

		/* swap */
		for (; s; s--, ++l, ++r) {
			unsigned char tmp = *r;
			*r = *l;
			*l = tmp;
		}
	}
}

void
shuf64_arr(void *base, uint64_t nel, uint64_t size,
         uint64_t (*rand64)(void*), void *rng)
{
	unsigned char *b = (unsigned char*)base;
	while (nel > 1) {
		uint64_t s = size;
		unsigned char *r = b + size *
		                   dist_uniform_u64(nel, rand64, rng);
		unsigned char *l = b + size * (--nel);

		/* swap */
		for (; s; s--, ++l, ++r) {
			unsigned char tmp = *r;
			*r = *l;
			*l = tmp;
		}
	}
}

#endif /* RANDOM_H_IMPLEMENTATION */

/* But we might not want to or need to modify the order of the array.
 * In such cases, we can use a shuffle iterator, that returns pseudorandom
 * indices that don't repeat until the entire range is covered.
 * We can do this by exploiting the fact that many PRNGs can be constructed with
 * an arbitrary period. The only problem is to create PRNGs with random
 * constants that have a full period.
 *
 * But let's begin with the simplest of such PRNGs, the Weyl sequence, as we
 * already explained it in 3.4.
 * The quality of random numbers generated by a Weyl sequence on it's own is
 * rather low. Furthermore, it must be considered that this can only generate a
 * small subset
 *     6/(\pi^2) * m \approx 0.6 * m
 * of all m! possible permutations. <23>
 * Still some applications might be willing to take this tradeoff for
 * performance. */


/* Two values are coprime if their greatest common denominator (GCD) is one. */
static inline size_t
shuf__gcd(size_t a, size_t b)
{
	while (b) {
		size_t t = b;
		b = a % b;
		a = t;
	}
	return a;
}

typedef struct { size_t x, c, mod; } ShufWeyl;

static inline void
shuf_weyl_init(ShufWeyl *rng, size_t mod, size_t const seed[2])
{
	rng->x = seed[0];
	rng->mod = mod;
	/* m and c must be coprime */
	rng->c = seed[1];
	while (shuf__gcd(mod, rng->c) != 1)
		++rng->c;
}

static inline void
shuf_weyl_randomize(ShufWeyl *rng, size_t mod)
{
	size_t seed[2];
	trng_write(seed, sizeof seed);
	shuf_weyl_init(rng, mod, seed);
}

static inline size_t
shuf_weyl(ShufWeyl *rng)
{
	return (rng->x = (rng->x + rng->c) % rng->mod);
}

/* We can also use LCGs (see 3.1 for an in-depth explanation) for better quality
 * shuffling.
 * We'll only work with power-of-two moduli because the computation of a is
 * very expensive and requires the prime factorization of 'm'.
 * This means that we'll need to reject numbers that aren't inside the range.
 *
 * The LCG approach can generate
 *    m/2 * m/4 = m^2/8
 * of all possible m! permutations. */

typedef struct { size_t x, a, c, mod, mask; } ShufLcg;

static inline void
shuf_lcg_init(ShufLcg *rng, size_t mod, size_t const seed[3])
{
	rng->x = seed[0];
	/* a-1 must be divisible by 4 */
	rng->a = (seed[1] & ~(size_t)3) + 1;
	/* m and c must be coprime.
	 * Since m is a power-of-two every odd number will be coprime to it. */
	rng->c = seed[2] | 1;

	/* mask is one minus the next power-of-two */
	rng->mod = mod;
	rng->mask = 1;
	while (rng->mask < mod)
		rng->mask *= 2;
	rng->mask -= 1;
}

static inline void
shuf_lcg_randomize(ShufLcg *rng, size_t mod)
{
	size_t seed[3];
	trng_write(seed, sizeof seed);
	shuf_lcg_init(rng, mod, seed);
}

static inline size_t
shuf_lcg(ShufLcg *rng)
{
	do {
		rng->x = ((rng->a * rng->x) + rng->c) & rng->mask;
	} while (rng->x >= rng->mod);
	return rng->x;
}

#define RANDOM_H_INCLUDED
#endif

/*
 * References ==================================================================
 *
 * <1> "Multiply with carry", George Marsaglia (1994):
 *     URL: "https://groups.google.com/g/sci.math/c/6BIYd0cafQo/m/Ucipn_5T_TMJ"
 *
 * <2> Sebastiano Vigna (2020):
 *     "On the probability of overlap of random subsequences of pseudorandom
 *      number generators"
 *     DOI: "https://doi.org/10.1016/j.ipl.2020.105939"
 *
 * <3> "TestU01", Pierre L'Ecuyer:
 *     URL: "http://simul.iro.umontreal.ca/testu01/tu01.html"
 *
 * <4> "PractRand", Chris Doty-Humphrey:
 *     URL: "https://sourceforge.net/projects/pracrand"
 *
 * <5> Melissa E. O'Neill (2014):
 *     "PCG: A Family of Simple Fast Space-Efficient Statistically Good
 *      Algorithms for Random Number Generation":
 *     URL: "https://www.cs.hmc.edu/tr/hmc-cs-2014-0905.pdf"
 *     URL: "https://www.pcg-random.org"
 *
 * <6> Charles Bouillaguet, Florette Martinez, Julia Sauvage (2020):
 *     "Predicting the PCG Pseudo-Random Number Generator In Practice."
 *     DOI: "https://doi.org/10.13154/tosc.v2020.i3.175-196"
 *
 * <7> Mark A. Overton (2020):
 *     "Romu: Fast Nonlinear Pseudo-Random Number Generators
 *      Providing High Quality"
 *     URL: "http://www.romu-random.org/romupaper.pdf"
 *     URL: "https://www.romu-random.org"
 *
 * <8> David Blackman and Sebastiano Vigna (2019):
 *     "Scrambled Linear Pseudorandom Number Generators"
 *     URL: "http://vigna.di.unimi.it/ftp/papers/ScrambledLinear.pdf"
 *     URL: "http://prng.di.unimi.it"
 *
 * <9> Forrest B. Brown (1994):
 *     "Random number generation with arbitrary strides."
 *     URL: "https://laws.lanl.gov/vhosts/mcnp.lanl.gov/pdf_files/
 *           anl-rn-arb-stride.pdf"
 *
 * <10> Bernard Widynski (2020):
 *      "Middle Square Weyl Sequence RNG"
 *      URL: "https://arxiv.org/abs/1704.00358"
 *
 * <11> Daniel Lemire (2018):
 *      "Fast Random Integer Generation in an Interval"
 *      URL: "https://arxiv.org/abs/1805.10941"
 *
 * <12> Daniel Lemire (2018):
 *      "A fast alternative to the modulo reduction"
 *      URL: "https://lemire.me/blog/2016/06/27/
 *            a-fast-alternative-to-the-modulo-reduction/"
 *
 * <13> Wikipedia (January 2021): "IEEE 754"
 *      URL: "https://en.wikipedia.org/wiki/IEEE_754"
 *
 * <14> Frédéric Goualard (2020):
 *      "Generating Random Floating-Point Numbers by Dividing Integers:
 *       a Case Study"
 *      DOI: "https://doi.org/10.1007/978-3-030-50417-5_2"
 *
 * <15> AllenB. Downey (2007):
 *      "Generating Pseudo-random Floating-Point Values"
 *      URL: http://allendowney.com/research/rand/downey07randfloat.pdf
 *
 * <16> Charles E. Leiserson, Harald Prokop, Keith H. Randall (1998):
 *      "Usign de Bruijn Sequences to Index a 1 in a Computer Word"
 *      URL: http://supertech.csail.mit.edu/papers/debruijn.pdf
 *
 * <17> Marsaglia, George, and Wai Wan Tsang (2000):
 *      "The ziggurat method for generating random variables."
 *      Journal of statistical software 5.8: 1-7.
 *
 * <18> Doornik, Jurgen A (2005):
 *      "An improved ziggurat method to generate normal random samples."
 *      University of Oxford: 77.
 *
 * <19> A. J. Kindermann, J. F. Monahan (1976):
 *      "Computer Generation of Random Variables Using the Ratio of Uniform
 *       Deviates"
 *      DOI: https://doi.org/10.1145/355744.355750
 *
 * <20> Joseph L. Leva (1991):
 *      "A Fast Normal Random Number Generator"
 *      DOI: https://doi.org/10.1145/138351.138364
 *
 * <21> Marsaglia, George (1964):
 *      "Generating a variable for the trail of the normal distribution"
 *      Technometrics, 6, 101-102
 *
 * <22> Reddit post (December 2022):
 *      "Fast Approximate Gaussian Generator" by u/Dusty_Coder
 *      URL: https://old.reddit.com/r/algorithms/comments/
 *           yyz59u/fast_approximate_gaussian_generator/
 *
 * <23> Wikipedia (January 2021): "Coprime integers"
 *      URL: https://en.wikipedia.org/wiki/Coprime_integers
 *           #Generating_all_coprime_pairs
 *
 *
 * Other resources:
 *     - https://espadrine.github.io/blog/posts/a-primer-on-randomness.html
 *     - https://rhet.dev/wheel/rng-battle-royale-47-prngs-9-consoles
 */

/*
 * Licensing ===================================================================
 *
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

