/******************************************************************************/
/* printf.h */

#include <stdio.h>

#define FMT_BEGIN(fmt) do { \
	const char *internal__fmt = fmt; \
	internal__fmt += printf("%s", internal__fmt) + 1;


#define FMT(name, args) \
	(fmt_##name args, internal__fmt += printf("%s", internal__fmt) + 1)

#define FMT_END } while (0)


void fmt_uint(unsigned x) { printf("%u", x); }
void fmt_float(float x) { printf("%f", x); }

/******************************************************************************/

#include <limits.h>

typedef struct {
	float x, y, z;
} Vec3;

void
fmt_Vec3(Vec3 v)
{
	FMT_BEGIN("{\0, \0, \0}") {
		FMT(float, (v.x));
		FMT(float, (v.y));
		FMT(float, (v.z));
	} FMT_END;
}

void fmt_uint_base(unsigned x, int base, const char *digits)
{
	char arr[sizeof x * CHAR_BIT];
	char *s = arr + sizeof arr;
	do *--s = digits[x % base]; while ((x /= base) > 0);
	while (*s)
		putchar(*s++);
}

int
main(void)
{
	{
		float a = 3.141592, b = 2.718281828;

		FMT_BEGIN("a + b = \0 + \0 = \0\n") {
			FMT(float, (a));
			FMT(float, (b));
			FMT(float, (a + b));
		} FMT_END;
	}

	{
		Vec3 a = { 1.0f/3.0f, -69.69f, 189 };
		Vec3 b = { 3.141592f, 2.718281828f, 420.69f };
		unsigned x = 0xdeadbeef;

		FMT_BEGIN("x = \0 = 0x\0 = 0b\0\na = \0\nb = \0\n") {
			FMT(uint, (x));
			FMT(uint_base, ((unsigned)x, 16, "0123456789abcdef"));
			FMT(uint_base, ((unsigned)x, 2, "01"));
			FMT(Vec3, (a));
			FMT(Vec3, (b));
		} FMT_END;
	}
}

