// Emil Hedemalm
// Aeonic Games Math

#ifndef AE_MATH_H
#define AE_MATH_H

// PI o.o
const float PI = 3.1415926535897f;
#define DEGREES_TO_RADIANS(d) (d * 0.01745329251f)
#define RADIANS_TO_DEGREES(r) (r * 57.2957795f)

/// Define Zero as a minimal value in order for the floating points to function properly when comparing with "0"
const float ZERO = 0.0000000001f;

#include <cmath>

#ifndef AbsoluteValue
#define AbsoluteValue(one)    ((one < 0) ? (-(one)) : (one))
#endif

#ifndef ClampFloat
#define ClampFloat(a, min, max) if(a < min) a = min; else if(a > max) a = max;
#endif

#ifndef RoundFloat
#define RoundFloat(value) (floor((value)+0.5f))
#endif

#ifndef RoundInt
#define RoundInt(x) ((int)floor((x)+0.5f))
#endif

/*
#ifndef max
#define max(one, two) (((one) > (two)) ? (one) : (two))
#endif
*/
/*
#ifndef min
#define min(one, two) (((one) < (two)) ? (one) : (two))
#endif


#ifndef abs
#define abs(one)    ((one < 0) ? (-one) : (one))
#endif
*/

#endif
