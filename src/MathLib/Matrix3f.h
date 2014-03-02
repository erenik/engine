// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Matrix3f.h
// CLASS PROVIDED: Matrix3f  (a three-dimensional Matrix class using floats)

#ifndef MATRIX3F_H
#define MATRIX3F_H

#include <cmath>
#include "Vector3f.h"
#include "Matrix4f.h"

class Matrix4f;

/** A three-dimensional matrix class using floats.
*/
class Matrix3f {
    friend class Matrix4d;
    friend class Matrix4f;
public:
	/** Postcondition: Initializes an identity-matrix. */
	Matrix3f();
	/// Assuming x being the first vertical vector, y being the second vertical vector and z being the mostrighward column.
	Matrix3f(float x1,float x2,float x3,float y1,float y2,float y3,float z1,float z2,float z3);
	/** Initializes a 3D matrix using given array of arrays (matrix).
		Precondition: The elements are ordered x first, meaning the array of arrays is declared and used: arr[x][y].
		Postcondition: Initializes a 3D matrix using given array of arrays (matrix).
	*/
	Matrix3f(float elements[3][3]);
	/** Initializes a 3D matrix using given array of data.
		Precondition: The array is at least 9 elements long.
		Postcondition: Initializes a 3D matrix using given array. The elements are read in column-wise. Example below:
		[ 0] [ 3] [ 6]
		[ 1] [ 4] [ 7]
		[ 2] [ 5] [ 8]  */
	Matrix3f(float elements[9]);
	/** Initializes a 3D matrix using given array of vectors.
		Precondition: The vectors have to be at least 3 in amount and are read in as the matrix's 3 columns.
		Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 3 columns.
	*/
	Matrix3f(Vector3f * vectors);
	/** Initializes a 3D matrix using given vectors.
		Precondition: The vectors have to be at least 3 in amount and are read in as the matrix's 3 columns.
		Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 3 columns.
	*/
	Matrix3f(Vector3f vector, Vector3f vector2, Vector3f vector3);

	/** Conversion constructor
		Postcondition: Copies the upper-left co-ordinates to this matrix as if it was a submatrix of the Matrix4f.
	*/
	Matrix3f(Matrix4f &base);
	/** Copy constructor */
	Matrix3f(const Matrix3f& base);

	// Initialization functions
	/** Reloads the identity matrix. */
	void LoadIdentity();

	/** Initializes a rotation matrix around the X-axis. */
	void InitRotationMatrixX(float radians);
	/** Initializes a rotation matrix around the Y-axis. */
	void InitRotationMatrixY(float radians);
	/** Initializes a rotation matrix around the Z-axis. */
	void InitRotationMatrixZ(float radians);
	/** Initializes a rotation matrix using provided vector parameters. */
	void InitRotationMatrix(float angle, float x, float y, float z);
	/** Initializes a rotation matrix using provided vector. */
	void InitRotationMatrix(float angle, Vector3f vector);

	// Transposition and invertion
	/** Returns a transposed version of the matrix, flipping rows and columns. */
	Matrix3f TransposedCopy() const;
	/** Transposes this matrix, flipping rows and columns. */
	void Transpose();

	/// Utility function to find the determinant of the matrix
	float getDeterminant() const;
	/** Returns an inverted version of the matrix. */
	Matrix3f InvertedCopy() const;
	/** Inverts the matrix. */
	void Invert();

	// Content access
	/** Fills an array to the pointed location with a copy of the matrix's current contents. Writes the data column-wise. */
	void getContents(float * arr);

	const float * getPointer() const { return element; };

	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix3f product(const Matrix3f matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector3f product(const Vector3f vector) const;
	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix3f operator * (const Matrix3f matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector3f operator * (const Vector3f vector) const;
	/** Product with Matrix-assignment
		Postcondition: The matrices have been multiplied and assigned to this matrix.
	*/
	void operator *= (const Matrix3f matrix);

	/** Operator overloading for the array-access parameter. Returns specified column as as shown below:
	Col	  0    1    2
		[ 0] [ 3] [ 6]
		[ 1] [ 4] [ 7]
		[ 2] [ 5] [ 8]  */
	Vector3f operator[](const unsigned int index);

private:
	/** Array of the matrix elements. */
	float element[9];
};

#endif
