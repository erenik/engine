// Emil Hedemalm
// 2013-07-28

#include "AEMath.h"
#include "Trigonometry.h"
#include <cmath>

/// Returns the angle in radians, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngler(float x, float y){
	/// First get raw degrees.
	float degrees = acos(x);
	if (y < 0)
		degrees *= -1.0f;
	return degrees;
}
/// Returns the angle in degrees, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngled(float x, float y){
	/// First get raw degrees.
	float degrees = GetAngler(x,y);
	degrees *= 360/(2* PI);
	return degrees;
}
