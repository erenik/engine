/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#include "Plane.h"

Plane::Plane()
{
	Set3Points(Vector3f(-0.5f,0,-0.5f), Vector3f(0.5f,0,-0.5f),Vector3f(0.5f,0,0.5f));
};

// Creates a plane, setting its 3 reference points in counter clockwise order.
Plane::Plane(Vector3f point1, Vector3f point2, Vector3f point3)
{
	Set3Points(point1, point2, point3);
}

/// Copy constructor
Plane::Plane(const Plane &plane)
{
	Set3Points(plane.point1, plane.point2, plane.point3);
}
/** Product with Matrix
	Postcondition: Returns the plane multiplied by the given matrix.
*/
Plane Plane::operator * (const Matrix4f matrix) const {
	Plane p;
	p.Set3Points(matrix * this->point1, matrix * this->point2, matrix * this->point3);
	return p;
}

/// Applies the given transform
Plane Plane::Transform(Matrix4f transformationMatrix){
	point1 = transformationMatrix * point1;
	point2 = transformationMatrix * point2;
	point3 = transformationMatrix * point3;
	Set3Points(point1, point2, point3);
	return Plane(*this);
}

/*
Assuming three points p0, p1, and p2 the coefficients A, B, C and D can be computed as follows:

Compute vectors v = p1 – p0, and u = p2 – p0;
	-	Compute n = v x u (cross product)
	-	Normalize n
	-	Assuming n = (xn,yn,zn) is the normalized normal vector then
		-	A = xn
		-	B = yn
		-	C = zn
To compute the value of D we just use the equation above, hence -D = Ax + By + Cz. Replacing (x,y,z) for a point in the plane (for instance p0), we get D = – n . p0 (dot product).
*/
void Plane::Set3Points(Vector3f p1, Vector3f p2, Vector3f p3){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	position = p1;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}

float Plane::Distance(Vector3f point) const {
	return (normal.DotProduct(point) + D);
}
