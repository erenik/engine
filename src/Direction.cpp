/// Emil Hedemalm
/// 2014-03-05
/// Directions and their vectors.

#include "Direction.h"

#include <cassert>

/// Returns unit-vectors.
Vector3f Direction::GetVector(int forDirection)
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

	}
}