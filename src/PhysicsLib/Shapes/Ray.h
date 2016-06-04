/// Emil Hedemalm
/// 2014-08-06
/// Ray used for intersection tests. Based on a start position and direction.

#ifndef RAY_H
#define RAY_H

#include "MathLib.h"

class AABB;
class Sphere;
class Triangle;
class Quad;
class Plane;

class Ray
{
public:
	Ray();
	Ray(ConstVec3fr start, ConstVec3fr dir);
	/// Intersect with sphere.
	bool Intersect(Sphere & sphere, float * distance);
    /// Returns true if they intersect, and the distance (along the ray) if so.
    bool Intersect(Triangle & triangle, float * distance);
	/** Calculates if the provided plane and ray intersect.
		Returns 1 if an intersection occurs, and 0 if not.
		If a collission occurs, the point, normal and distance to collission is stored in the pointers.
	*/
	bool Intersect(Quad & quad, float * distance);
	/** Calculates if the provided plane and ray intersect.
		Returns 1 if an intersection occurs, and 0 if not.
		If a collission occurs, the point, normal and distance to collission is stored in the pointers.
	*/
	bool Intersect(AABB & aabb, float * distance);
	/** Calculates if the provided plane and ray intersect.
		Returns 1 if an intersection occurs, and 0 if not.
		If a collission occurs, the point, normal and distance to collission is stored in the pointers.
	*/
	bool Intersect(const Plane& plane, Vector3f * collisionPoint = NULL, Vector3f * collissionPointNormal = NULL, double * collissionDistance = NULL);

	int collisionFilter; // Default 0
	Vector3f start; // or where it origins from
	Vector3f direction;
};

#endif
