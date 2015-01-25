// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Matrix4f.h
// CLASS PROVIDED: Matrix4f  (a four-dimensional Matrix class using floats)

#ifndef MATRIX4F_H
#define MATRIX4F_H

#include "List/List.h"
#include <cmath>
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix3f.h"
#include "Matrix4d.h"

class Matrix3f;
class Matrix4d;

/** A four-dimensional matrix class using floats.
*/
class Matrix4f {

	friend class Matrix4d;

public:
	/** Initializes an identity-matrix. */
	Matrix4f();
	/** Initializes a 4D matrix using given array of arrays (matrix).
		Precondition: The elements are ordered x first, meaning the array of arrays is declared and used: arr[x][y].
		Postcondition: Initializes a 4D matrix using given array of arrays (matrix).
	*/
	Matrix4f(float elements[4][4]);
	/** Initializes a 3D matrix using given array of data.
		Precondition: The array is at least 16 elements long.
		Postcondition: Initializes a 4D matrix using given array. The elements are read in column-wise. Example below:
		[ 0] [ 4] [ 8] [12]
		[ 1] [ 5] [ 9] [13]
		[ 2] [ 6] [10] [14]
		[ 3] [ 7] [11] [15] */
	Matrix4f(float elements[16]);
	/** Initializes a 3D matrix using given array of vectors.
		Precondition: The vectors have to be at least 4 in amount and are read in as the matrix's 4 columns.
		Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 4 columns.
	*/
	Matrix4f(Vector4f * vectors);
	/** Conversion constructor
		Postcondition: Copies the upper-left co-ordinates to this matrix as if it was a submatrix of the Matrix4f.
	*/
	Matrix4f(Matrix3f &base);
	/** Copy constructor */
	Matrix4f(const Matrix4f& base);
	/** Conversion constructor from the double-class. */
	Matrix4f(const Matrix4d& base);

	/// o.o Create matrices!
	static List<Matrix4f> FromFloatList(List<float> floatList, int numMatricesToExtract, bool transpose);
	

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Matrix4f& mat);

	// Initialization functions
	/** Reloads the identity matrix. */
	void LoadIdentity();

	/// Conversion
	Matrix3f GetMatrix3f();

	/// Returns target column of the matrix.
	Vector4f GetColumn(int columnIndex);

#define GetRotationMatrixX InitRotationMatrixX
#define GetRotationMatrixY InitRotationMatrixY
#define GetRotationMatrixZ InitRotationMatrixZ
	/// Initializes a rotation matrix around the X-axis.
	static Matrix4f InitRotationMatrixX(float radians);
	/** Initializes a rotation matrix around the Y-axis. */
	static Matrix4f InitRotationMatrixY(float radians);
	/// Initializes a rotation matrix around the Z-axis.
	static Matrix4f InitRotationMatrixZ(float radians);

	/** Initializes a rotation matrix using provided vector parameters. */
	void InitRotationMatrix(float angle, float x, float y, float z);
	/** Initializes a rotation matrix using provided vector. */
	void InitRotationMatrix(float angle, Vector3f vector);

	/** Returns an initialized translation-matrix using given vector. */
//	static Matrix4f Translation(Vector3f trans);
	/** Initializes a translation matrix using provided vector. */
	static Matrix4f InitTranslationMatrix(Vector3f translation);

	/// Creates a scaling matrix (XYZ)
	static Matrix4f InitScalingMatrix(Vector3f scale);

	/** Initializes a perspective projection matrix.
	The function can be called as it is, using the default values left -1, right 1, bottom -1, top 1, near -1 and far -10.
	*/
	void InitProjectionMatrix(float left = -1, float right = 1, float bottom = -1, float top = 1, float near = -1, float far = -10);
	/** Initializes an orthographic projection matrix.
	The function can be called as it is, using the default values left -1, right 1, bottom -1, top 1, near -1 and far -10.
	*/
	void InitOrthoProjectionMatrix(float left = -1, float right = 1, float bottom = -1, float top = 1, float near = -1, float far = -10);

	// Transposition and invertion
	/** Returns a transposed version of the matrix, flipping rows and columns. */
	Matrix4f TransposedCopy() const;
	/** Transposes this matrix, flipping rows and columns. */
	void Transpose();

	/// Utility function to find the determinant of the matrix
	float getDeterminant() const;
	/** Returns an inverted version of the matrix. */
	Matrix4f InvertedCopy() const;
	/** Inverts the matrix. */
	void Invert();

	// 3D-operations
	/** Applies 3D translation using given parameters. */
	void Translate(float x, float y, float z);
	/// Builds a 3D translation matrix using given vector.
	static const Matrix4f Translation(const Vector3f & vec);

	/** Applies 3D scaling with provided ratio to x, y and z-dimensions. */
	void Scale(float ratio);
	/** Applies 3D scaling using the provided x, y and z-ratios. */
	void Scale(float xRatio, float yRatio, float zRatio);
	/// Scale using a given vector
	void Scale(const Vector3f & scalingVector);
	/// Builds a 3D scaling matrix using given vector.
	static const Matrix4f Scaling(const Vector3f & scalingVector);

	/// Returns true if it has non-0 values in the scale diagonal (elements 0, 5 and 10)
	bool HasValidScale();


	// Content access
	/** Returns a pointer to the start of the matrix element array. */
	float * getPointer() {return element; };
	/** Fills an array to the pointed location with a copy of the matrix's current contents. Writes the data column-wise. */
	void getContents(float * arr);

	/** Direct multiplication
		Postcondition: Multiplies the provided matrix into the left one and returns a copy of the current one.
	*/
	Matrix4f Multiply(const Matrix4f matrix);

	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix4f Product(const Matrix4f matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector4f Product(const Vector4f & vector) const;
	/** Product with Matrix
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix4f operator * (const Matrix4f matrix) const;
	/** Product with Vector
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector4f operator * (const Vector4f & vector) const;
	/** Product with Matrix-assignment
		Postcondition: The matrices have been multiplied and assigned to this matrix.
	*/
	void operator *= (const Matrix4f matrix);

	/** Operator overloading for the array-access. 
		[ 0] [ 4] [ 8] [12]
		[ 1] [ 5] [ 9] [13]
		[ 2] [ 6] [10] [14]
		[ 3] [ 7] [11] [15]*/
	float & operator[](const unsigned int index);
	/** Operator overloading for the array-access. Returns specified column as as shown below:
	Col	  0    1    2    3
		[ 0] [ 4] [ 8] [12]
		[ 1] [ 5] [ 9] [13]
		[ 2] [ 6] [10] [14]
		[ 3] [ 7] [11] [15]*/
//	Vector4f operator[](const unsigned int index);

private:
	/** Array of the matrix elements. */
	float element[16];
};



#endif
