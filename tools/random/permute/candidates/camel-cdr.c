#include <stdint.h>

uint64_t
hash(uint64_t idx, uint64_t mask, uint64_t seed)
{
	idx ^= seed;
	/* splittable64 */
	idx ^= (idx & mask) >> 30; idx *= 0xBF58476D1CE4E5B9;
	idx ^= (idx & mask) >> 27; idx *= 0x94D049BB133111EB;
	idx ^= (idx & mask) >> 31;
	idx *= 0xBF58476D1CE4E5B9;

	idx ^= seed >> 32;
	idx &= mask;
	idx *= 0xED5AD4BB;

	idx ^= seed >> 48;
	///* hash16_xm3 */
	idx ^= (idx & mask) >> 7; idx *= 0x2993;
	idx ^= (idx & mask) >> 5; idx *= 0xE877;
	idx ^= (idx & mask) >> 9; idx *= 0x0235;
	idx ^= (idx & mask) >> 10;

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
	return idx;
}

