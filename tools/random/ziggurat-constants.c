#include <float.h>
#include <math.h>
#include <stdio.h>

#define M_SQRTPI_OVER_SQRT2 1.253314137315500251207882642405522627L
#define M_1_OVER_SQRT2 0.707106781186547524400844362104849039L

#define ziggurat_f(x) expl(-0.5 * x * x)
#define ziggurat_f_inv(y) sqrtl(-2 * logl(y))
#define ziggurat_f_int_x_to_inf(x) \
	-(M_SQRTPI_OVER_SQRT2 * (erf(x * M_1_OVER_SQRT2) - 1))

int
main(void)
{
	unsigned count = 0;
	printf("#define ZIGGURAT_COUNT ");
	scanf("%u", &count);

	long double min = 0, max = 10, pmin, pmax;
	long double area, R;
	do {
		int tobig = 0;

		pmin = min;
		pmax = max;

		R = 0.5 * (min + max);
		long double x = R;
		area = R * ziggurat_f(R) + ziggurat_f_int_x_to_inf(R);
		for (unsigned i = 1; i < count && !tobig; i++) {
			x = area / x + ziggurat_f(x);
			if (x > 1) tobig = 1;
			else       x = ziggurat_f_inv(x);
		}
		if (tobig)
			min = R;
		else
			max = R;
	} while (pmin < R && R < pmax);

	printf("#define ZIGGURAT_R     %.*Lg\n", DECIMAL_DIG, R);
	printf("#define ZIGGURAT_AREA  %.*Lg\n", DECIMAL_DIG, area);
}

/*
--------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
--------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Olaf Berstein
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
--------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <http://unlicense.org/>
--------------------------------------------------------------------------------
*/
