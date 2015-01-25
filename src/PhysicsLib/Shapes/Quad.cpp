/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#include "Quad.h"

/// Copy constructor
Quad::Quad(const Quad &quad){
	Set4Points(quad.point1, quad.point2, quad.point3, quad.point4);
}

/// Returns width x height. Assumes point1 is min and point 3 is max.
int Quad::ManhattanSize()
{
	return point3[0] - point1[0] + point3[1] - point1[1];
};



/// Create a rectangular quad using min and max values.
void Quad::Set2Points(const Vector3f & min, const Vector3f & max)
{
	point1 = min;
	point3 = max;
	position = (point1 + point3) * 0.5f;
}

void Quad::Set4Points(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3, const Vector3f & p4){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	point4 = p4;
	position = (p1 + p2 + p3 + p4) * 0.25f;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}

/// Applies the given transform
Quad Quad::Transform(Matrix4f transformationMatrix){
	point1 = transformationMatrix * point1;
	point2 = transformationMatrix * point2;
	point3 = transformationMatrix * point3;
	point4 = transformationMatrix * point4;
	Set4Points(point1, point2, point3, point4);
	return Quad(*this);
}

