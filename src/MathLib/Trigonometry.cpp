// Emil Hedemalm
// 2013-07-28

#include "AEMath.h"
#include "Trigonometry.h"
#include <cmath>

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
