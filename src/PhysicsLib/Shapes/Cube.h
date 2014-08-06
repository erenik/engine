/// Emil Hedemalm
/// 2014-08-06
/// Simple shapes.

#ifndef CUBE_H
#define CUBE_H

#include "MathLib.h"

struct Cube {
	Cube() { size = 1.0f; };
	Vector3f position;
	float size;
	Vector3f hitherTopLeft, hitherTopRight, hitherBottomLeft, hitherBottomRight,
		fartherTopLeft, fartherTopRight, fartherBottomLeft, fartherBottomRight;
};

#endif
