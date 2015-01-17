/// Emil Hedemalm
/// 2014-03-05
/// Directions and their vectors.

#ifndef DIRECTION_H
#define DIRECTION_H

#include "MathLib.h"
#include "String/AEString.h"

namespace Direction
{
	enum direction {
		NONE = -1,
		FORWARD = 0, 
		BACKWARD, 
		LEFT, 
		RIGHT, 
		UP, 
		DOWN,
		DIRECTIONS
	};
	int Get(String byString);
	/// Returns unit-vectors.
	Vector3f GetVector(int forDirection);
};

#endif