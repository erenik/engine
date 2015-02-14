// Emil Hedemalm
// 2013-07-28

#include "AEMath.h"
#include "Trigonometry.h"
#include <cmath>
#include <cstdlib>
#include "Globals.h"


/// Returns the angle in radians, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngler(float x, float y)
{
	if (x == y)
		return 0;
	/// First get raw degrees.
	float radians = acos(x);
	if (y < 0)
		radians *= -1.0f;
	return radians;
}
/// Returns the angle in degrees, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngled(float x, float y)
{
	/// First get raw degrees.
	float radians = GetAngler(x,y);
	float degrees = radians * 360/(2* PI);
	return degrees;
}

double * arr360 = NULL;

double SinSampled360(double d)
{
	int degree = (int)(RADIANS_TO_DEGREES(d));
	while (degree < 0)
		degree += 360;
	degree %= 360;
	return arr360[degree];
}
double CosSampled360(double d)
{
	int degree = (int)(RADIANS_TO_DEGREES(d));
	degree += 90;
	while (degree < 0)
		degree += 360;
	degree %= 360;
	return arr360[degree];
}

void InitSampled360()
{
	arr360 = new double[360];
	for (int i = 0; i < 360; ++i)
	{
		arr360[i] = sin((double)i);
	}
}
void DeallocSampled360()
{
	SAFE_DELETE_ARR(arr360);
}


