/// Emil Hedemalm
/// 2015-01-14
/// For 2D operations mostly.

#include "Vector2f.h"

class Matrix2f 
{
public:
	Matrix2f();
	/// Initializes a rotation matrix around the Z-axis.
	static Matrix2f InitRotationMatrixZ(float radians);
	/// Product with a vector.
	Vector2f Product(Vector2f vec) const;
private:
	/** Array of the matrix elements. Indiced as below.
		[0] [2] 
		[1] [3] */
	float element[4];
};


