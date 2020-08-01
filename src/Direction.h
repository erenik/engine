/// Emil Hedemalm
/// 2014-03-05
/// Directions and their vectors.

#ifndef DIRECTION_H
#define DIRECTION_H

#include "MathLib.h"
#include "String/AEString.h"

enum Direction {
	NO_DIRECTION = -1,
	FORWARD = 0,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	MAX_DIRECTIONS
};

Direction GetDirection(String byString);
/// Returns unit-vectors.
Vector3f GetVector(Direction forDirection);

#endif