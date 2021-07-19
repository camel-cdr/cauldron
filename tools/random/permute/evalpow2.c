#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/time.h>

#define RANDOM_H_IMPLEMENTATION
#include <cauldron/random.h>
#include <cauldron/arg.h>


/* Based on https://github.com/skeeto/hash-prospector */


#define SQRT_OF_PI_OVER_TWO 0.79788456080286535589

/* Measures how each input bit affects each output bit.
 * This measures both bias and avalanche. */
static double
estimate_bias(uint64_t (*hash)(uint64_t i, uint64_t mask, uint64_t seed),
              int bits, int quality, int seedEvalRange, double *avrHashTime)
{
	/* We treat the index and the seed together as the input. */
	uint64_t bins[128][64] = {{0}};
	int64_t const n = UINT64_C(1) << quality;
	uint64_t const mask = (bits == 64) ?  UINT64_MAX : (UINT64_C(1) << bits) - 1;
	uint64_t ns = 0;
	double mean = 0;

	#pragma omp parallel
	{
		PRNG64RomuQuad rng;
		prng64_romu_quad_randomize(&rng);

		struct timespec beg, end;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &beg);

		#pragma omp for reduction(+:bins[:128][:64])
		for (int64_t i = 0; i < n; ++i) {
			uint64_t seed = prng64_romu_quad(&rng);
			uint64_t x = prng64_romu_quad(&rng) & mask;
			uint64_t h0 = hash(x, mask, seed);

			/* evaluate seed changes */
			for (int j = 0; j < seedEvalRange; ++j) {
				uint64_t bit = UINT64_C(1) << j;
				uint64_t h1 = hash(x, mask, seed ^ bit);
				uint64_t set = h0 ^ h1;
				for (int k = 0; k < bits; ++k)
					bins[j][k] += (set >> k) & 1;
			}

			/* evaluate index changes */
			for (int j = 0; j < bits; ++j) {
				uint64_t bit = UINT64_C(1) << j;
				uint64_t h1 = hash(x ^ bit, mask, seed);
				uint64_t set = h0 ^ h1;
				for (int k = 0; k < bits; ++k)
					bins[j + seedEvalRange][k] += (set >> k) & 1;
			}
		}

		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

	#pragma omp atomic
		ns += (end.tv_sec - beg.tv_sec) * 1000000000 + (end.tv_nsec - beg.tv_nsec);
	}

	*avrHashTime = (double)ns / (n * (1 + seedEvalRange + bits));

	for (int j = 0; j < bits + seedEvalRange; ++j) {
		for (int k = 0; k < bits; ++k) {
			double diff = (bins[j][k] - n/2.0) / n;
			mean += fabs(diff);
		}
	}

	return mean * 1000.0 / ((bits + seedEvalRange) * bits);
}

static void
die(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

static void
usage(char *argv0)
{
	printf("usage: %s [OPTION...] \n", argv0);
	puts("Evaluates the bias of a hash function that is invertible for a");
	puts("given power-of-two sized domain.\n");

	puts("One of the following option is required:");
	puts("  -b, --best          output the theoretically best bias possible");
	puts("  -l, --load=lib.so   the following function prototype is loaded from lib.so:");
	puts("                      uint64_t hash(uint64_t i, uint64_t mask, uint64_t seed)");

	puts("Seed bias: (default: -f)");
	puts("  -0, --eval-none      don't evaluate seed bias");
	puts("  -c, --eval-current   evaluate seed bias only up to the current power-of-two");
	puts("  -f, --eval-full      evaluate full seed bias ");

	puts("Other options:");
	puts("  -n, --num-bits=N     number of bits used by the hash (default: 32)");
	puts("  -o, --output=FILE    write the biases into a csv FILE");
	puts("  -q, --quality=N      evaluation 2^N hashes per power-of-two");
	puts("                       (12-30, default: 18)");
	puts("  -s, --start=N        test all powers-of-two stating from 2^N");
	puts("                       (default: 1)");
	puts("  -S, --stop=N         test all powers-of-two up to 2^N");
	puts("                       (default: --num-bits)");
	puts("  -v, --verbose        print the bias for every power-of-two tested");
	puts("  -?, -h, --help       display this help and exit");
}

int
main(int argc, char **argv)
{
	char *argv0 = argv[0];

	/* obligatory options */
	int printBest = 0;
	char *sofile = 0;

	/* options */
	FILE *output = 0;
	enum {
		SEED_EVAL_NONE,
		SEED_EVAL_CURRENT,
		SEED_EVAL_FULL
	} seedEvalType = SEED_EVAL_FULL;
	int nbits = 32;
	int quality = 18;
	int start = 1;
	int stop = 0;
	int stopset = 0;
	int verbose = 0;
	uint64_t (*hash)(uint64_t i, uint64_t mask, uint64_t seed) = 0;

	/* parse arguments */
	ARG_BEGIN {
		if (ARG_LONG("best")) case 'b': {
			printBest = 1;
			sofile = 0;
			ARG_FLAG();
		} else if (ARG_LONG("load")) case 'l': {
			sofile = ARG_VAL();
			printBest = 0;
		} else if (ARG_LONG("eval-none")) case '0': {
			seedEvalType = SEED_EVAL_NONE;
			ARG_FLAG();
		} else if (ARG_LONG("eval-current")) case 'c': {
			seedEvalType = SEED_EVAL_CURRENT;
			ARG_FLAG();
		} else if (ARG_LONG("num-bits")) case 'n': {
			nbits = atoi(ARG_VAL());
		} else if (ARG_LONG("output")) case 'o': {
			if (!(output = fopen(ARG_VAL(), "w")))
				die("%s: couldn't create file '%s'\n",
				    argv0, ARG_VAL());
		} else if (ARG_LONG("quality")) case 'q': {
			quality = atoi(ARG_VAL());
		} else if (ARG_LONG("start")) case 's': {
			start = atoi(ARG_VAL());
		} else if (ARG_LONG("stop")) case 'S': {
			stop = atoi(ARG_VAL());
			stopset = 1;
		} else if (ARG_LONG("verbose")) case 'v': {
			verbose = 1;
			ARG_FLAG();
		} else if (ARG_LONG("help")) case 'h': case '?': {
			usage(argv0);
			return EXIT_SUCCESS;
		} else default: {
			die("%s: invalid option '%s'\n"
			    "Try '%s --help' for more information.\n",
			    argv0, *argv, argv0);
		}
	} ARG_END;

	if (quality < 12 || quality > 30)
		die("%s: quality out of range, expected "
		    "12 to 30, got '%d'\n", argv0, quality);

	stop = (stopset) ? stop : nbits;

	if (stop < start || stop > nbits)
		die("%s: quality out of range, expected "
		    "1 to %d, got '%d'\n", argv0, nbits, quality);

	if (start < 1 || start > stop)
		die("%s: quality out of range, expected "
		    "1 to %d, got '%d'\n", argv0, nbits, quality);


	if (argc != 0 || argv[0] || (!sofile && !printBest)) {
		usage(argv0);
		return EXIT_FAILURE;
	}

	if (printBest) {
		double n = UINT64_C(1) << quality;
		double variance = sqrt(n*0.5*0.5);
		double foldedVariance = variance*SQRT_OF_PI_OVER_TWO;
		double bias = foldedVariance/n * 1000.0;

		double totalBias = 0;

		for (int i = start; i <= stop; ++i) {
			if (verbose)
				printf("bias[%d] = %.17g\n", i, bias);
			if (output)
				fprintf(output, "%d,%.17g\n", i, bias);
			totalBias += bias;
		}
		if (output)
			fclose(output);

		printf("\ntotal bias = %.17g\n", totalBias);
		printf("avr bias   = %.17g\n", totalBias / (stop - start + 1));

		return EXIT_SUCCESS;
	}

	/* load hash from sofile */
	if (sofile) {
		void *handle;
		if (!(handle = dlopen(sofile, RTLD_NOW)))
			die("%s: couldn't load shared object file '%s'\n", argv0, sofile);
		if (!(hash = dlsym(handle, "hash")))
			die("%s: couldn't find the symbol 'hash' in '%s'\n", argv0, sofile);
	}

	/* evaluate bias of hashes */
	{
		double totalBias = 0;
		double avrHashTime = 0;


		for (int i = start; i <= stop; ++i) {
			int range;
			switch (seedEvalType) {
			case SEED_EVAL_NONE: range = 0; break;
			case SEED_EVAL_CURRENT: range = i; break;
			case SEED_EVAL_FULL: range = nbits; break;
			}
			double t, bias = estimate_bias(
					hash, i, quality, range, &t);
			avrHashTime += t / nbits;
			if (verbose) {
				printf("bias[%d] = %.17g\n", i, bias);
			} else {
				printf("\r%d/%d", i - start, stop - start + 1);
				fflush(stdout);
			}
			if (output) {
				fprintf(output, "%d,%.17g\n", i, bias);
				fflush(output); /* so we can see the progress */
			}
			totalBias += bias;
		}
		if (output)
			fclose(output);

		printf("\ntotal bias = %.17g\n", totalBias);
		printf("avr bias   = %.17g\n", totalBias / (stop - start + 1));
		printf("speed      = %.3f ns / hash\n", avrHashTime);
	}

	return 0;
}
