// Emil Hedemalm
// 2013-03-17
#include "Physics.h"

#ifndef abs
#define abs(p) ((p < 0) ? (-p) : (p))
#endif

/// Define Zero as a minimal value in order for the floating points to function properly when comparing with "0"
 //const float ZERO = 0.00000000001f;

bool SpheresColliding(const Vector3f & position, const Vector3f & position2, float radiiSum)
{
	Vector3f distance = position - position2;
	if (abs(distance[0]) > radiiSum)
		return false;
	if (abs(distance[1]) > radiiSum)
		return false;
	if (abs(distance[2]) > radiiSum)
		return false;
	float distanceLengthNotRooted = distance[0] * distance[0] + distance[1] * distance[1] + distance[2] * distance[2];
	float radiusPowerOf2 = radiiSum * radiiSum;
	if (distanceLengthNotRooted > radiusPowerOf2 * 1.05f)
		return false;
	return true;
}
