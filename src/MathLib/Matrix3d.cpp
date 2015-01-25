#include "Matrix3d.h"
#include <cstring>

// ************************************************************************//
// Constructors
// ************************************************************************//
Matrix3d::Matrix3d(){
	for (int i = 0; i < 9; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[4] = 1;
	element[8] = 1;
}

Matrix3d::Matrix3d( double elements[3][3]){
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			element[x * 3 + y] = elements[x][y];
		}
	}
}

Matrix3d::Matrix3d(double elements[9]){
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			const int index = x * 3 + y;
			element[index] = elements[index];
		}
	}
}

Matrix3d::Matrix3d(Matrix4d &base){
	double data[16];
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

Matrix3d::Matrix3d(const Matrix3d& base){
	for (int i = 0; i < 9; ++i){
		element[i] = base.element[i];
	}
}

// ************************************************************************//
// Initialization functions.
// ************************************************************************//
void Matrix3d::LoadIdentity(){
	for (int i = 0; i < 9; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[4] = 1;
	element[8] = 1;
}

void Matrix3d::InitRotationMatrixX(double radians){
	element[4] = cos((double)radians);
	element[5] = sin((double)radians);
	element[7] = -sin((double)radians);
	element[8] = cos((double)radians);
}

void Matrix3d::InitRotationMatrixY(double radians){
	element[0] = cos((double)radians);
	element[6] = sin((double)radians);
	element[2] = -sin((double)radians);
	element[8] = cos((double)radians);
}

void Matrix3d::InitRotationMatrixZ(double radians){
	element[0] = cos((double)radians);
	element[1] = sin((double)radians);
	element[3] = -sin((double)radians);
	element[4] = cos((double)radians);
}

void Matrix3d::InitRotationMatrix(double angle, double x, double y, double z){
	double c = cos((double)angle);
	double s = sin((double)angle);
	double c1 = 1 - c;
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

void Matrix3d::InitRotationMatrix(double angle, Vector3d vector){
	double c = cos((double)angle);
	double s = sin((double)angle);
	double c1 = 1 - c;
	double x = vector[0];
	double y = vector[1];
	double z = vector[2];
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
void Matrix3d::Transpose(){
	double arr[9];
	for (int i = 0; i < 9; ++i){
		arr[i] = element[i];
	}
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			element[x*3 + y] = arr[y*3 + x];
		}
	}
}

Matrix3d Matrix3d::TransposedCopy() const {
	Matrix3d newMatrix;
	for (int x = 0; x < 3; ++x){
		for (int y = 0; y < 3; ++y){
			newMatrix.element[x*3 + y] = element[y*3 + x];
		}
	}
	return newMatrix;
}

// Utility function to find the determinant of the matrix
double Matrix3d::getDeterminant() const {
	throw 3;
	return 1.0f;
}

/*************************************************************/
/* Matrix inversion: http://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3.C3.973_matrices
/*************************************************************/
void InvertMatrix3d(double *src, double *dst){
	double det; /* determinant */

	// Get determinant
	det = src[0] * (src[4] * src[8] - src[7] * src[5]) - src[3] * (src[8] * src[1] - src[7] * src[2]) + src[6] * (src[1] * src[5] - src[4] * src[2]);
	if (det == 0)
		throw 999;

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

void Matrix3d::Invert(){
	double newArr[9];
	InvertMatrix3d(element, newArr);
	for (int i = 0; i < 9 ; ++i)
		element[i] = newArr[i];
}

Matrix3d Matrix3d::InvertedCopy() const {
	double newArr[9];
	double src[9];
	memcpy(src, element, sizeof(double) * 9);
	InvertMatrix3d(src, newArr);
	return Matrix3d(newArr);
}

// ************************************************************************//
// Content access
// ************************************************************************//
/** Fills an array to the pointed location with a copy of the matrix's current contents. */
void Matrix3d::getContents(double * arr){
	for (int i = 0; i < 9; ++i){
		arr[i] = element[i];
	}
}

// ************************************************************************//
// Product functions
// ************************************************************************//

/** Product with Matrix */
Matrix3d Matrix3d::product(const Matrix3d factor) const {
	double newArray[9];
	double tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	return Matrix3d(newArray);
}

/** Product with Vector */
Vector3d Matrix3d::product(Vector3d vector) const {
	double newArray[3];
	double tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 3; i++){
			tempResult += element[y + i * 3] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector3d(newArray);
}

Matrix3d Matrix3d::operator * (const Matrix3d factor) const {
	double newArray[9];
	double tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	return Matrix3d(newArray);
}

Vector3d Matrix3d::operator * (Vector3d vector) const {
	double newArray[3];
	double tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 3; i++){
			tempResult += element[y + i * 3] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector3d(newArray);
}

void Matrix3d::operator *= (Matrix3d factor){
	double newArray[9];
	double tempResult;
	for (int y = 0; y < 3; y++){	// Rows
		for(int x = 0; x < 3; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 3; i++){
				tempResult += element[y + i * 3] * factor.element[i + x * 3];
			}
			newArray[y + x * 3] = tempResult;
		}
	}
	memcpy(element, newArray, sizeof(double) * 9);
}

Vector3d Matrix3d::operator [](const unsigned int index){
	return Vector3d(&(element[index*3]));
}
