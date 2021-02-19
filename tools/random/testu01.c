#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <TestU01.h>

struct Battery {
	void (*f)(unif01_Gen *gen);
	char *name, *desc;
} batteries[] = {
	{ bbattery_SmallCrush, "SmallCrush", "takes ~1m" },
	{ bbattery_Crush,      "Crush",      "takes ~1h" },
	{ bbattery_BigCrush,   "BigCrush",   "takes ~8h" },
};

void usage(char *argv0)
{
	size_t i;
	printf("usage: %s BATTERY\n", argv0);
	puts("Reads data stream from standart input.");
	puts("Batteries:");
	for (i = 0; i < sizeof batteries / sizeof *batteries; ++i)
		printf("\t'%s': %s\n", batteries[i].name, batteries[i].desc);
	puts("Execution time tested on an AMD Athlon 64 4000+ Processor");
	puts("with a clock speed of 2400 MHz running Linux.");
}

uint32_t next(void)
{
	uint32_t res;
	fread(&res, sizeof res, 1, stdin);
	return res;
}

int main(int argc, char **argv)
{
	char **it = argv + 1;

	if (!*it || argc <= 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	(void)freopen(0, "rb", stdin);

	unif01_Gen* gen = unif01_CreateExternGenBits("stdin", next);

	do {
		size_t i;
		for (i = 0; i < sizeof batteries / sizeof *batteries; ++i) {
			if (strcmp(*it, batteries[i].name) == 0) {
				batteries[i].f(gen);
				fflush(stdout);
				break;
			}
		}
		if (i == sizeof batteries / sizeof *batteries) {
			fprintf(stderr, "%s error: Unknown test battery '%s'\n\n",
					argv[0], *it);
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	} while (*++it);

	unif01_DeleteExternGenBits(gen);
	return EXIT_SUCCESS;
}
