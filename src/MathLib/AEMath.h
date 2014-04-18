// Emil Hedemalm
// Aeonic Games Math

#ifndef AE_MATH_H
#define AE_MATH_H

// PI o.o
const float PI = 3.1415926535897f;

/// Define Zero as a minimal value in order for the floating points to function properly when comparing with "0"
const float ZERO = 0.0000000001f;

#include <cmath>

#ifndef AbsoluteValue
#define AbsoluteValue(one)    ((one < 0) ? (-(one)) : (one))
#endif

#ifndef Clamp
#define Clamp(a,c,b) if(a<c)a=c;else if(a>b)a=b;
#endif

#ifndef Round
#define RoundFloat(value) (floor((value)+0.5f))
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
