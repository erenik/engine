/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#include "Triangle.h"

/// Positions isn't updated correctly with just the plane-constructor!
Triangle::Triangle(const Triangle &tri)
: Plane((Plane)tri)
{
	position = (point1 + point2 + point3) * 0.33333f;
}

void Triangle::Set3Points(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	position = (p1 + p2 + p3) * 0.33333f;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}
