#include "Matrix3f.h"
#include <cstring>

// ************************************************************************//
// Constructors
// ************************************************************************//
Matrix3f::Matrix3f(){
	for (int i = 0; i < 9; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[4] = 1;
	element[8] = 1;
}

Matrix3f::Matrix3f(float x1,float x2,float x3,float y1,float y2,float y3,float z1,float z2,float z3){
    element[0] = x1;
    element[1] = x2;
    element[2] = x3;
    element[3] = y1;
    element[4] = y2;
    element[5] = y3;
    element[6] = z1;
    element[7] = z2;
    element[8] = z3;
}

Matrix3f::Matrix3f( float elements[3][3]){
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			element[x * 3 + y] = elements[x][y];
		}
	}
}

Matrix3f::Matrix3f(float elements[9]){
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			const int index = x * 3 + y;
			element[index] = elements[index];
		}
	}
}

/** Initializes a 3D matrix using given vectors.
	Precondition: The vectors have to be at least 3 in amount and are read in as the matrix's 3 columns.
	Postcondition: The 3D matrix has been initialized using the provided vectors as the matrix's 3 columns.
*/
Matrix3f::Matrix3f(const Vector3f & vector, const Vector3f & vector2, const Vector3f & vector3)
{
	element[0] = vector[0];
	element[1] = vector[1];
	element[2] = vector[2];

	element[3] = vector2[0];
	element[4] = vector2[1];
	element[5] = vector2[2];

	element[6] = vector3[0];
	element[7] = vector3[1];
	element[8] = vector3[2];
}

Matrix3f::Matrix3f(Matrix4f &base){
	float data[16];
	base.getContents(data);
	element[0] = data[0];
	element[1] = data[1];
	element[2] = data[2];
	element[3] = data[4];
	element[4] = data[5];
	element[5] = data[6];
	element[6] = data[8];
	element[7] = data[9];
	element[8] = data[10];
}

Matrix3f::Matrix3f(const Matrix3f& base){
	for (int i = 0; i < 9; ++i){
		element[i] = base.element[i];
	}
}

// ************************************************************************//
// Initialization functions.
// ************************************************************************//
void Matrix3f::LoadIdentity(){
	for (int i = 0; i < 9; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[4] = 1;
	element[8] = 1;
}

void Matrix3f::InitRotationMatrixX(float radians){
	element[4] = cos((float)radians);
	element[5] = sin((float)radians);
	element[7] = -sin((float)radians);
	element[8] = cos((float)radians);
}

void Matrix3f::InitRotationMatrixY(float radians){
	element[0] = cos((float)radians);
	element[6] = sin((float)radians);
	element[2] = -sin((float)radians);
	element[8] = cos((float)radians);
}

void Matrix3f::InitRotationMatrixZ(float radians){
	element[0] = cos((float)radians);
	element[1] = sin((float)radians);
	element[3] = -sin((float)radians);
	element[4] = cos((float)radians);
}

void Matrix3f::InitRotationMatrix(float angle, float x, float y, float z){
	float c = cos((float)angle);
	float s = sin((float)angle);
	float c1 = 1 - c;
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[3] = x * y * c1 - z * s;
	element[4] = y * y * c1 + c;
	element[5] = y * z * c1 + c * s;
	element[6] = x * z * c1 + y * s;
	element[7] = y * z * c1 - x * s;
	element[8] = z * z * c1 + c;
}

void Matrix3f::InitRotationMatrix(float angle, const Vector3f & vector)
{
	float c = cos((float)angle);
	float s = sin((float)angle);
	float c1 = 1 - c;
	float x = vector[0];
	float y = vector[1];
	float z = vector[2];
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[3] = x * y * c1 - z * s;
	element[4] = y * y * c1 + c;
	element[5] = y * z * c1 + c * s;
	element[6] = x * z * c1 + y * s;
	element[7] = y * z * c1 - x * s;
	element[8] = z * z * c1 + c;
}


// ************************************************************************//
// Transposition and invertion
// ************************************************************************//
void Matrix3f::Transpose(){
	float arr[9];
	for (int i = 0; i < 9; ++i){
		arr[i] = element[i];
	}
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			element[x*3 + y] = arr[y*3 + x];
		}
	}
}

Matrix3f Matrix3f::TransposedCopy() const {
	Matrix3f newMatrix;
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			newMatrix.element[x*3 + y] = element[y*3 + x];
		}
	}
	return newMatrix;
}

// Utility function to find the determinant of the matrix
float Matrix3f::getDeterminant() const {
	throw 3;
	return 1.0f;
}

/*************************************************************/
/* Matrix inversion: http://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3.C3.973_matrices
/*************************************************************/
void InvertMatrix3f(float *src, float *dst){
	float det; /* determinant */

	// Get determinant
	det = src[0] * (src[4] * src[8] - src[7] * src[5]) - src[3] * (src[8] * src[1] - src[7] * src[2]) + src[6] * (src[1] * src[5] - src[4] * src[2]);
	if (det == 0){
		std::cout<<"\nUnable to invert matrix!";
		return;
	}

	dst[0] = src[4] * src[8] - src[7] * src[5];
	dst[1] = src[7] * src[2] - src[1] * src[8];
	dst[2] = src[1] * src[5] - src[4] * src[2];
	dst[3] = src[6] * src[5] - src[3] * src[8];
	dst[4] = src[0] * src[8] - src[6] * src[2];
	dst[5] = src[2] * src[3] - src[0] * src[5];
	dst[6] = src[3] * src[7] - src[6] * src[4];
	dst[7] = src[6] * src[1] - src[0] * src[7];
	dst[8] = src[0] * src[4] - src[3] * src[1];

	det = 1.0f / det;
	// Divide everything by the determinant
	for (int j = 0; j < 9; j++)
		dst[j] *= det;
}

void Matrix3f::Invert(){
	float newArr[9];
	InvertMatrix3f(element, newArr);
	for (int i = 0; i < 9 ; ++i)
		element[i] = newArr[i];
}

Matrix3f Matrix3f::InvertedCopy() const {
	float newArr[9];
	float src[9];
	memcpy(src, element, sizeof(float) * 9);
	InvertMatrix3f(src, newArr);
	return Matrix3f(newArr);
}

// ************************************************************************//
// Content access
// ************************************************************************//
/** Fills an array to the pointed location with a copy of the matrix's current contents. */
void Matrix3f::getContents(float * arr){
	for (int i = 0; i < 9; ++i){
		arr[i] = element[i];
	}
}

// ************************************************************************//
// Product functions
// ************************************************************************//

/** Product with Matrix */
Matrix3f Matrix3f::product(const Matrix3f factor) const {
	float newArray[9];
	float tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	return Matrix3f(newArray);
}

/** Product with Vector */
Vector3f Matrix3f::product(const Vector3f & vector) const 
{
	float newArray[3];
	float tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 3; i++){
			tempResult += element[y + i * 3] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector3f(newArray);
}

Matrix3f Matrix3f::operator * (const Matrix3f factor) const {
	float newArray[9];
	float tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	return Matrix3f(newArray);
}

Vector3f Matrix3f::operator * (const Vector3f & vector) const {
	float newArray[3];
	float tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 3; i++){
			tempResult += element[y + i * 3] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector3f(newArray);
}

void Matrix3f::operator *= (Matrix3f factor){
	float newArray[9];
	float tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	memcpy(element, newArray, sizeof(float) * 9);
}

Vector3f Matrix3f::operator [](const unsigned int index){
	return Vector3f(&(element[index*3]));
}
