/// Emil Hedemalm
/// 2015-01-14
/// For 2D operations mostly.

#include "Matrix2f.h"

Matrix2f::Matrix2f()
{
	element[0] = element[3] = 1.f;
	element[1] = element[2] = 0;
	
}
/// Initializes a rotation matrix around the Z-axis.
Matrix2f Matrix2f::InitRotationMatrixZ(float radians)
{
	Matrix2f mat;
	mat.element[0] = cos((float)radians);
	mat.element[1] = sin((float)radians);
	mat.element[2] = -sin((float)radians);
	mat.element[3] = cos((float)radians);
	return mat;
}

/// Product with a vector.
Vector2f Matrix2f::Product(Vector2f vec) const
{
	float x = vec.x * element[0] + vec.y * element[2];
	float y = vec.x * element[1] + vec.y * element[3];
	return Vector2f(x,y);
}

