/// Emil Hedemalm
/// 2014-03-05
/// Directions and their vectors.

#include "Direction.h"

#include <cassert>

Direction GetDirection(String byString)
{
	byString.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (byString == "Left")	return Direction::LEFT;
	if (byString == "Right")return Direction::RIGHT;
	if (byString == "UP")	return Direction::UP;
	if (byString == "DOWN")	return Direction::DOWN;
	if (byString == "FORWARD")	return Direction::FORWARD;
	if (byString == "BACKWARD")	return Direction::BACKWARD;
	return Direction::NO_DIRECTION;
}

/// Returns unit-vectors.
Vector3f GetVector(Direction forDirection)
{
	switch(forDirection)
	{
		case Direction::FORWARD:
			return Vector3f(0,0,-1);
		case Direction::BACKWARD:
			return Vector3f(0,0,1);
		case Direction::LEFT:
			return Vector3f(-1,0,0);
		case Direction::RIGHT:
			return Vector3f(1,0,0);
		case Direction::UP:
			return Vector3f(0,1,0);
		case Direction::DOWN:
			return Vector3f(0,-1,0);
		default:
			assert(false);
			return Vector3f();

	}
}