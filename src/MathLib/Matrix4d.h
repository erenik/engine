// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Matrix4d.h
// CLASS PROVIDED: Matrix4d  (a four-dimensional Matrix class using doubles)

#ifndef Matrix4d_H
#define Matrix4d_H

#include <cmath>
#include "Vector3d.h"
#include "Vector4d.h"
#include "Matrix3d.h"
#include "Matrix3f.h"
#include "Matrix4f.h"

class Matrix3f;
class Matrix3d;
class Matrix4f;

/** A four-dimensional matrix class using doubles.
*/
class Matrix4d {

	friend class Matrix4f;

public:
	/** Initializes an identity-matrix. */
	Matrix4d();
	/** Initializes a 4D matrix using given array of arrays (matrix).
		Precondition: The elements are ordered x first, meaning the array of arrays is declared and used: arr[x][y].
		Postcondition: Initializes a 4D matrix using given array of arrays (matrix).
	*/
	Matrix4d(double elements[4][4]);
	/** Initializes a 3D matrix using given array of data.
		Precondition: The array is at least 16 elements long.
		Postcondition: Initializes a 4D matrix using given array. The elements are read in column-wise. Example below:
		[ 0] [ 4] [ 8] [12]
		[ 1] [ 5] [ 9] [13]
		[ 2] [ 6] [10] [14]
		[ 3] [ 7] [11] [15] */
	Matrix4d(double elements[16]);
	/** Initializes a 3D matrix using given array of vectors.
		Precondition: The vectors have to be at least 4 in amount and are read in as the matrix's 4 columns.
		Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 4 columns.
	*/
	Matrix4d(Vector4d * vectors);
	/** Conversion constructor
		Postcondition: Copies the upper-left co-ordinates to this matrix as if it was a submatrix of the Matrix4d.
	*/
	Matrix4d(Matrix3d &base);
	/** Copy constructor */
	Matrix4d(const Matrix4d& base);
	/** Copy constructor */
	Matrix4d(const Matrix4f& base);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Matrix4d& vec);


	// Initialization functions
	/** Reloads the identity matrix. Returns a copy of itself afterwards. */
	Matrix4d LoadIdentity();

	/// Conversion
	Matrix3f GetMatrix3f();

	/// Returns target column of the matrix.
	Vector4d GetColumn(int columnIndex);

	/** Initializes a rotation matrix around the X-axis. */
	Matrix4d InitRotationMatrixX(double radians);
	/** Initializes and returns a rotation matrix around the X-axis. */
	static Matrix4d GetRotationMatrixX(double radians);
	/** Initializes a rotation matrix around the Y-axis. */
	Matrix4d InitRotationMatrixY(double radians);
	/** Initializes and returns a rotation matrix around the Y-axis. */
	static Matrix4d GetRotationMatrixY(double radians);
	/** Initializes a rotation matrix around the Z-axis. */
	Matrix4d InitRotationMatrixZ(double radians);
	/** Initializes and returns a rotation matrix around the Z-axis. */
	static Matrix4d GetRotationMatrixZ(double radians);
	/** Initializes a rotation matrix using provided vector parameters. */
	Matrix4d InitRotationMatrix(double angle, double x, double y, double z);
	/** Initializes a rotation matrix using provided vector. */
	Matrix4d InitRotationMatrix(double angle, Vector3d vector);
	/** Initializes a translation matrix using provided vector. */
	static Matrix4d InitTranslationMatrix(Vector3f translation);

	/** Initializes a perspective projection matrix.
	The function can be called as it is, using the default values left -1, right 1, bottom -1, top 1, near -1 and far -10.
	*/
	void InitProjectionMatrix(double left = -1, double right = 1, double bottom = -1, double top = 1, double near = -1, double far = -10);
	/** Initializes an orthographic projection matrix.
	The function can be called as it is, using the default values left -1, right 1, bottom -1, top 1, near -1 and far -10.
	*/
	void InitOrthoProjectionMatrix(double left = -1, double right = 1, double bottom = -1, double top = 1, double near = -1, double far = -10);

	// Transposition and invertion
	/** Returns a transposed version of the matrix, flipping rows and columns. */
	Matrix4d TransposedCopy() const;
	/** Transposes this matrix, flipping rows and columns. */
	void Transpose();

	/// Utility function to find the determinant of the matrix
	double getDeterminant() const;
	/** Returns an inverted version of the matrix. */
	Matrix4d InvertedCopy() const;
	/** Inverts the matrix. */
	void Invert();

	// 3D-operations
	/** Applies 3D translation using given parameters. Returns a copy of the given matrix. */
	Matrix4d translate(double x, double y, double z);
	/** Applies 3D translation using given vector. */
	Matrix4d translate(Vector3d vec);
	/// Builds a translation matrix.
	static Matrix4d Translation(double x, double y, double z);
	static Matrix4d Translation(Vector3d vec);
	/** Applies 3D scaling with provided ratio to x, y and z-dimensions. */
	Matrix4d Scale(double ratio);
	/** Applies 3D scaling using the provided x, y and z-ratios. */
	Matrix4d Scale(double xRatio, double yRatio, double zRatio);
	/** Applies 3D sclaing operation using given vector. */
	Matrix4d Scale(Vector3d vec);



	// Content access
	/** Returns a pointer to the start of the matrix element array. */
	double * getPointer() {return element; };
	/** Fills an array to the pointed location with a copy of the matrix's current contents. Writes the data column-wise. */
	void getContents(double * arr);

	/** Direct multiplication
		Postcondition: Multiplies the provided matrix into the left one and returns a copy of the current one.
	*/
	Matrix4d multiply(const Matrix4d matrix);
	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix4d product(const Matrix4d matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector4d product(const Vector4d vector) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector4f product(const Vector4f vector) const;
	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix4d operator * (const Matrix4d matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector4d operator * (const Vector4d vector) const;
	/** Product with Matrix-assignment
		Postcondition: The matrices have been multiplied and assigned to this matrix.
	*/
	void operator *= (const Matrix4d matrix);

	/** Operator overloading for the array-access parameter. Returns specified column as as shown below:
	Col	  0    1    2    3
		[ 0] [ 4] [ 8] [12]
		[ 1] [ 5] [ 9] [13]
		[ 2] [ 6] [10] [14]
		[ 3] [ 7] [11] [15]*/
	Vector4d operator[](const unsigned int index);

private:
	/** Array of the matrix elements. */
	double element[16];
};



#endif
