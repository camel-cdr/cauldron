#include <stdio.h>

typedef struct {
	float x, y;
} Vec2;

typedef struct {
	float x, y, z;
} Vec3;

#define PRINT_CUSTOM \
	Print (*vec2)(Vec2 v); \
	Print (*vec3)(Vec3 v)

/******************************************************************************/
/* print.h */

typedef struct Print Print;
struct Print
{
	int precision;
	Print (*s)(const char *str);
	Print (*d)(int x);
	Print (*f)(float x);
	PRINT_CUSTOM;
} print;

Print print_s(const char *str) { fputs(str, stdout); return print; }
Print print_d(int x) { printf("%d", x); return print; }
Print print_f(float x) { printf("%.*f", print.precision, x); return print; }

void print_init(void)
{
	print.precision = 5;
	print.s = print_s;
	print.d = print_d;
	print.f = print_f;
}

/******************************************************************************/

Print
print_vec2(Vec2 v)
{
	print.s("{ ").f(v.x).s(". ").f(v.y).s(" }");
	return print;
}

Print
print_vec3(Vec3 v)
{
	print.s("{ ").f(v.x).s(". ").f(v.y).s(", ").f(v.z).s(" }");
	return print;
}

void print_init_custom(void)
{
	print.vec2 = print_vec2;
	print.vec3 = print_vec3;
}

int
main(void)
{
	print_init();
	print_init_custom();

	Vec2 a = { 1.0f/3.0f, -69.69f };
	Vec3 b = { 3.141592f, 2.718281828f, 420.69f };
	int x = 42;

	print.s("x = ").d(x).s("\n");
	print.s("a = ").vec2(a).s("\n");
	print.precision = 10;
	print.s("b = ").vec3(b).s("\n");
}

