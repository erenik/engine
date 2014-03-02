// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Matrix3d.h
// CLASS PROVIDED: Matrix3d  (a three-dimensional Matrix class using doubles)

#ifndef Matrix3d_H
#define Matrix3d_H

#include <cmath>
#include "Vector3d.h"
#include "Matrix4d.h"

class Matrix4d;

/** A three-dimensional matrix class using doubles.
*/
class Matrix3d {

public:	
	/** Postcondition: Initializes an identity-matrix. */
	Matrix3d();		
	/** Initializes a 3D matrix using given array of arrays (matrix).
		Precondition: The elements are ordered x first, meaning the array of arrays is declared and used: arr[x][y].
		Postcondition: Initializes a 3D matrix using given array of arrays (matrix). 
	*/
	Matrix3d(double elements[3][3]);
	/** Initializes a 3D matrix using given array of data.
		Precondition: The array is at least 9 elements long.
		Postcondition: Initializes a 3D matrix using given array. The elements are read in column-wise. Example below:
		[ 0] [ 3] [ 6] 	
		[ 1] [ 4] [ 7] 
		[ 2] [ 5] [ 8]  */
	Matrix3d(double elements[9]);
	/** Initializes a 3D matrix using given array of vectors. 
		Precondition: The vectors have to be at least 3 in amount and are read in as the matrix's 3 columns.
		Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 3 columns.
	*/
	Matrix3d(Vector3d * vectors);
	/** Conversion constructor
		Postcondition: Copies the upper-left co-ordinates to this matrix as if it was a submatrix of the Matrix4d.
	*/
	Matrix3d(Matrix4d &base);
	/** Copy constructor */
	Matrix3d(const Matrix3d& base);

	// Initialization functions
	/** Reloads the identity matrix. */
	void LoadIdentity();	

	/** Initializes a rotation matrix around the X-axis. */
	void InitRotationMatrixX(double radians);
	/** Initializes a rotation matrix around the Y-axis. */
	void InitRotationMatrixY(double radians);
	/** Initializes a rotation matrix around the Z-axis. */
	void InitRotationMatrixZ(double radians);
	/** Initializes a rotation matrix using provided vector parameters. */
	void InitRotationMatrix(double angle, double x, double y, double z);
	/** Initializes a rotation matrix using provided vector. */
	void InitRotationMatrix(double angle, Vector3d vector);
	
	// Transposition and invertion
	/** Returns a transposed version of the matrix, flipping rows and columns. */
	Matrix3d TransposedCopy() const;
	/** Transposes this matrix, flipping rows and columns. */
	void Transpose();

	/// Utility function to find the determinant of the matrix
	double getDeterminant() const;
	/** Returns an inverted version of the matrix. */
	Matrix3d InvertedCopy() const;
	/** Inverts the matrix. */
	void Invert();

	// Content access
	/** Fills an array to the pointed location with a copy of the matrix's current contents. Writes the data column-wise. */
	void getContents(double * arr); 

	/** Product with Matrix 
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix3d product(const Matrix3d matrix) const;
	/** Product with Vector 
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector3d product(const Vector3d vector) const;
	/** Product with Matrix 
		Postcondition: Returns the product of the matrices without directly modifying them.
	*/
	Matrix3d operator * (const Matrix3d matrix) const;
	/** Product with Vector 
		Postcondition: Returns the product of the matrix and vector without directly modifying them.
	*/
	Vector3d operator * (const Vector3d vector) const;
	/** Product with Matrix-assignment
		Postcondition: The matrices have been multiplied and assigned to this matrix.
	*/
	void operator *= (const Matrix3d matrix);

	/** Operator overloading for the array-access parameter. Returns specified column as as shown below: 
	Col	  0    1    2  
		[ 0] [ 3] [ 6] 	
		[ 1] [ 4] [ 7] 
		[ 2] [ 5] [ 8]  */
	Vector3d operator[](const unsigned int index);

private:
	/** Array of the matrix elements. */
	double element[9];
};

#endif