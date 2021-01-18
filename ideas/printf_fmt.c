/******************************************************************************/
/* printf.h */

#include <stdio.h>

void
printf_end(const char **fmt)
{
	*fmt += printf("%s", *fmt) + 1;
}

void printf_s(const char **fmt, const char *str) { printf_end(fmt); fputs(str, stdout); }
void printf_d(const char **fmt, int x) { printf_end(fmt); printf("%d", x); }
void printf_f(const char **fmt, float x) { printf_end(fmt); printf("%f", x); }

/******************************************************************************/

typedef struct {
	float x, y;
} Vec2;

typedef struct {
	float x, y, z;
} Vec3;


void
printf_vec2(const char **fmt, Vec2 v)
{
	printf_end(fmt);
	const char *ifmt = "{\0, \0}";
	printf_f(&ifmt, v.x);
	printf_f(&ifmt, v.y);
	printf_end(&ifmt);
}

void
printf_vec3(const char **fmt, Vec3 v)
{
	printf_end(fmt);
	const char *ifmt = "{\0, \0, \0}";
	printf_f(&ifmt, v.x);
	printf_f(&ifmt, v.y);
	printf_f(&ifmt, v.z);
	printf_end(&ifmt);
}

int
main(void)
{

	{
		float a = 3.141592, b = 2.718281828;
		const char *fmt = "a + b = \0 + \0 = \0\n";
		printf_f(&fmt, a);
		printf_f(&fmt, b);
		printf_f(&fmt, a + b);
		printf_end(&fmt);
	}

	{
		Vec2 a = { 1.0f/3.0f, -69.69f };
		Vec3 b = { 3.141592f, 2.718281828f, 420.69f };
		int x = 42;

		const char *fmt = "x = \0\na = \0\nb = \0\n";
		printf_d(&fmt, x);
		//printf_d(&fmt, x, 16, "0123456789ABCDEF");
		printf_vec2(&fmt, a);
		printf_vec3(&fmt, b);
		printf_end(&fmt);
	}
}

