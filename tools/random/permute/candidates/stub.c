#include <stdint.h>

uint64_t
hash(uint64_t i, uint64_t mask, uint64_t seed)
{
	/* From Andrew Kensler: "Correlated Multi-Jittered Sampling" */
	return (i ^ seed) & mask;
}
