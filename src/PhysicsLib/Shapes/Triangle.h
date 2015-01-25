/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Plane.h"

class Triangle : public Plane 
{
public:
	Triangle() : Plane() {
	};
	/// Positions isn't updated correctly with just the inherited plane-constructor!
	Triangle(const Triangle &tri);
	/// Sets the three points that define define the plane in counter clockwise order.
	void Set3Points(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3);
};

#endif
