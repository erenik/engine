/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#ifndef PLANE_H
#define PLANE_H

#include "MathLib.h"

/// A 3-dimensional plane
/// Defines a plane class, used for physical calculations/checks.
class Plane 
{
public:
	// Creates a flat plane in X/Z, with a normal point in Y+
	Plane();
	// Creates a plane, setting its 3 reference points in counter clockwise order.
	Plane(Vector3f point1, Vector3f point2, Vector3f point3);
	/// Copy constructor
	Plane(const Plane &plane);

	/** Product with Matrix
		Postcondition: Returns the plane multiplied by the given matrix.
	*/
	Plane operator * (const Matrix4f matrix) const;

	/// Applies the given transform
	Plane Transform(Matrix4f transformationMatrix);
	/// Sets the three points that define define the plane in counter clockwise order.
	void Set3Points(Vector3f p1, Vector3f p2, Vector3f p3);
	/// Calculates and returns the distance to target point.
	float Distance(Vector3f point) const;

	/// Three points that define the plane. Assigned in counter clockwise order.
	Vector3f point1, point2, point3;
	/// Position in center of the three points.
	Vector3f position;
	/// Normal, calculated based on the 3 points.
	Vector3f normal;
	/// D - value of the plane, if specified as Ax + By + Cz + D = 0
	float D;
};

#endif
