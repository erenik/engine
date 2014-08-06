#ifndef PHYSICS_H
#define PHYSICS_H

/// Constans and enumerations for Physics.

#include "../MathLib.h"

/// Meters per millisecond per millisecond.. if that makes sense :P
const float DEFAULT_GRAVITY = 9.82f;
// const float DEFAULT_GRAVITY = 0.0000982f;

/// Checks if the target spheres are colliding, returns true if so, and false if not.
bool SpheresColliding(Vector3f position, Vector3f position2, float radiiSum);

#endif
