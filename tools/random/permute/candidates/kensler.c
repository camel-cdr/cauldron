#include <stdint.h>

uint64_t
hash(uint64_t idx, uint64_t mask, uint64_t seed)
{
	seed &= UINT32_MAX;

	/* From Andrew Kensler: "Correlated Multi-Jittered Sampling" */
	idx ^= seed; idx *= 0xE170893D;
	idx ^= seed >> 16;
	idx ^= (idx & mask) >> 4;
	idx ^= seed >> 8; idx *= 0x0929EB3F;
	idx ^= seed >> 23;
	idx ^= (idx & mask) >> 1; idx *= 1 | seed >> 27;
	idx *= 0x6935FA69;
	idx ^= (idx & mask) >> 11; idx *= 0x74DCB303;
	idx ^= (idx & mask) >> 2; idx *= 0x9E501CC3;
	idx ^= (idx & mask) >> 2; idx *= 0xC860A3DF;
	idx &= mask;
	idx ^= idx >> 5;
	return (idx ^ seed) & mask;
}
