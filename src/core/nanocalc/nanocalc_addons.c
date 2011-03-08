/*
 * nanocalc addons
 * Mathematical built-in functions for nanocalc
 * Copyright (c) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensnc.sourceforge.net
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
 * to whom the Software is furnished to do so, subject to the following conditions:
 *   
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "nanocalc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* constants */
static const float one = 1.0f;
#define EPS             1e-5
#define INFI            (1.0f / (1.0f - one))



/* ============ available functions ============ */

/* if cond is true, returns t. Otherwise, returns f */
static float f_cond(float cond, float t, float f) { return fabs(cond)>EPS ? t : f; }

/* if val < lo, returns lo. if val > hi, returns hi. Otherwise, returns val */
static float f_clamp(float val, float lo, float hi) { return lo > hi ? f_clamp(val,hi,lo) : (val>lo ? (val<hi ? val : hi) : lo); }

/* returns the maximum between a and b */
static float f_max(float a, float b) { return a>b?a:b; }

/* the minimum between a and b */
static float f_min(float a, float b) { return a<b?a:b; }

/* returns the arc tangent of y/x, in the interval [-pi/2,pi/2] radians */
static float f_atan2(float y, float x) { return (fabs(y)<EPS&&fabs(x)<EPS) ? 0.0f : atan2(y,x); }

/* returns 1 if x>=0 or -1 otherwise */
static float f_sign(float x) { return x >= 0.0f ? 1.0f : -1.0f; }

/* the absolute value of x */
static float f_abs(float x) { return fabs(x); }

/* given x integer, returns a random integer between 0 and x-1, inclusive */
static float f_random(float x) { return floor(((float)rand() / ((float)(RAND_MAX)+(float)(1)))*(floor(f_max(1,x)))); }

/* round down the value */
static float f_floor(float x) { return floor(x); }

/* round up the value */
static float f_ceil(float x) { return ceil(x); }

/* round to the closest integer */
static float f_round(float x) { return floor(x + 0.5f); }

/* compute the square root of x */
static float f_sqrt(float x) { return x >= 0.0f ? sqrt(x) : 0.0f; }

/* returns e raised to the power x */
static float f_exp(float x) { return exp(x); }

/* returns the natural logarithm of x */
static float f_log(float x) { return x > 0.0f ? log(x) : -INFI; }

/* returns the common (base-10) logarithm of x */
static float f_log10(float x) { return x > 0.0f ? log10(x) : -INFI; }

/* returns the cosine of x, x in radians */
static float f_cos(float x) { return cos(x); }

/* returns the sine of x, x in radians */
static float f_sin(float x) { return sin(x); }

/* returns the tangent of x, x in radians */
static float f_tan(float x) { return fabs(cos(x))>EPS ? tan(x) : f_sign(sin(x))*f_sign(cos(x))*INFI; }

/* returns the arc sine of x, in the interval [-pi/2,pi/2] radians */
static float f_asin(float x) { return asin(f_clamp(x,-1,1)); }

/* returns the arc cosine of x, in the interval [-pi/2,pi/2] radians */
static float f_acos(float x) { return acos(f_clamp(x,-1,1)); }

/* returns the arc tangent of x, in the interval [-pi/2,pi/2] radians */
/* Notice that because of the sign ambiguity, a function cannot determine with certainty in which quadrant the angle falls only by its tangent value. You can use atan2 if you need to determine the quadrant. */
static float f_atan(float x) { return atan(x); }

/* hyperbolic sine of x */
static float f_sinh(float x) { return sinh(x); }

/* hyperbolic cosine of x */
static float f_cosh(float x) { return cosh(x); }

/* hyperbolic tangent of x */
static float f_tanh(float x) { return tanh(x); }

/* convert radians to degrees */
static float f_rad2deg(float x) { return x * 57.2957795147f; }

/* convert degrees to radians */
static float f_deg2rad(float x) { return x / 57.2957795147f; }

/* easter egg ;) */
static float f_leet() { return 1337; }

/* pi */
static float f_pi() { return 3.1415926535f; }

/* infinity */
static float f_infinity() { return INFI; }




/* ============ nanocalc addons ================ */

/* binds the mathematical functions: call this AFTER nanocalc_init() */
void nanocalc_addons_enable()
{
    nanocalc_register_bif_arity3("cond", f_cond);
    nanocalc_register_bif_arity3("clamp", f_clamp);

    nanocalc_register_bif_arity2("max", f_max);
    nanocalc_register_bif_arity2("min", f_min);
    nanocalc_register_bif_arity2("atan2", f_atan2);

    nanocalc_register_bif_arity1("sign", f_sign);
    nanocalc_register_bif_arity1("abs", f_abs);
    nanocalc_register_bif_arity1("random", f_random);
    nanocalc_register_bif_arity1("floor", f_floor);
    nanocalc_register_bif_arity1("ceil", f_ceil);
    nanocalc_register_bif_arity1("round", f_round);
    nanocalc_register_bif_arity1("sqrt", f_sqrt);
    nanocalc_register_bif_arity1("exp", f_exp);
    nanocalc_register_bif_arity1("log", f_log);
    nanocalc_register_bif_arity1("log10", f_log10);
    nanocalc_register_bif_arity1("cos", f_cos);
    nanocalc_register_bif_arity1("sin", f_sin);
    nanocalc_register_bif_arity1("tan", f_tan);
    nanocalc_register_bif_arity1("asin", f_asin);
    nanocalc_register_bif_arity1("acos", f_acos);
    nanocalc_register_bif_arity1("atan", f_atan);
    nanocalc_register_bif_arity1("cosh", f_cosh);
    nanocalc_register_bif_arity1("sinh", f_sinh);
    nanocalc_register_bif_arity1("tanh", f_tanh);
    nanocalc_register_bif_arity1("deg2rad", f_deg2rad);
    nanocalc_register_bif_arity1("rad2deg", f_rad2deg);

    nanocalc_register_bif_arity0("leet", f_leet);
    nanocalc_register_bif_arity0("pi", f_pi);
    nanocalc_register_bif_arity0("infinity", f_infinity);
}


#ifdef __cplusplus
}
#endif
