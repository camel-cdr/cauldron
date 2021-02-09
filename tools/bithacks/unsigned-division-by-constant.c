#include <stdio.h>

int
main(void)
{
	unsigned long long div, c1, c2, c, q, r, delta;
	unsigned a, p, nbits;

	printf("divisor: ");
	scanf("%llu", &div);
	printf("with of type (in bits <= 64): ");
	scanf("%u", &nbits);

	c2 = (1ull << (nbits - 1));
	c1 = c2 - 1;

	a = 0;
	p = nbits - 1;
	q = c1 / div;
	r = c1 - q * div;

	do {
		c = (++p == nbits) ? 1 : 2 * c;
		if (r + 1 >= div - r) {
			if (q >= c1)
				a = 1;
			q = 2 * q + 1;
			r = 2 * r + 1 - div;
		} else {
			if (q >= c2)
				a = 1;
			q = 2 * q;
			r = 2 * r + 1;
		}
		delta = div - 1 - r;
	} while (p < 2 * nbits && c < delta);
	++q;

	if (a) {
		printf("(uint%u_t)(((uint%u_t)x * %llu + %llu) >> %u)\n",
			nbits, nbits * 2, q, q, p);
	} else {
		printf("(uint%u_t)(((uint%u_t)x * %llu) >> %u)\n",
			nbits, nbits * 2, q, p);
	}

	return 0;
}
