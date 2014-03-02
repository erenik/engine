#include "Matrix4d.h"
#include <cstring>

// ************************************************************************//
// Constructors
// ************************************************************************//
Matrix4d::Matrix4d(){
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[5] = 1;
	element[10] = 1;
	element[15] = 1;
}

Matrix4d::Matrix4d( double elements[4][4]){
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			element[x * 4 + y] = elements[x][y];
		}
	}
}

Matrix4d::Matrix4d( double elements[16]){
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			const int index = x * 4 + y;
			element[index] = elements[index];
		}
	}
}

Matrix4d::Matrix4d(Matrix3d &base){
	LoadIdentity();
	double data[9];
	base.getContents(data);
	element[0] = data[0];
	element[1] = data[1];
	element[2] = data[2];
	element[4] = data[3];
	element[5] = data[4];
	element[6] = data[5];
	element[8] = data[6];
	element[9] = data[7];
	element[10] = data[8];
}

Matrix4d::Matrix4d(const Matrix4d& base){
	for (int i = 0; i < 16; ++i){
		element[i] = base.element[i];
	}
}

/** Copy constructor */
Matrix4d::Matrix4d(const Matrix4f& base){
	for (int i = 0; i < 16; ++i){
		element[i] = (double) base.element[i];
	}
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Matrix4d& mat){
	for (int i = 0; i < 16; ++i){
		os << mat.element[i];
		if (i < 15)
			os << " ";
	}
	return os;
}


// ************************************************************************//
// Initialization functions.
// ************************************************************************//
Matrix4d Matrix4d::LoadIdentity(){
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[5] = 1;
	element[10] = 1;
	element[15] = 1;
	return Matrix4d(element);
}

/// Conversion
Matrix3f Matrix4d::GetMatrix3f(){
    Matrix3f matrix;
#define mat matrix.element
    mat[0] = element[0];
    mat[1] = element[1];
    mat[2] = element[2];

    mat[3] = element[4];
    mat[4] = element[5];
    mat[5] = element[6];

    mat[6] = element[8];
    mat[7] = element[9];
    mat[8] = element[10];
#undef mat
    return matrix;
}

/// Returns target column of the matrix.
Vector4d Matrix4d::GetColumn(int columnIndex)
{
	int s = columnIndex * 4;
	return Vector4d(s, s+1, s+2, s+3);	
}

Matrix4d Matrix4d::InitRotationMatrixX(double radians){
	LoadIdentity();
	element[5] = cos((double)radians);
	element[6] = sin((double)radians);
	element[9] = -sin((double)radians);
	element[10] = cos((double)radians);
	return Matrix4d(element);
}

Matrix4d Matrix4d::GetRotationMatrixX(double radians){
	double element[16];
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[15] = 1;
	element[5] = cos((double)radians);
	element[6] = sin((double)radians);
	element[9] = -sin((double)radians);
	element[10] = cos((double)radians);
	return Matrix4d(element);
}

Matrix4d Matrix4d::InitRotationMatrixY(double radians){
	LoadIdentity();
	element[0] = cos((double)radians);
	element[8] = sin((double)radians);
	element[2] = -sin((double)radians);
	element[10] = cos((double)radians);
	return Matrix4d(element);
}

Matrix4d Matrix4d::GetRotationMatrixY(double radians){
	double element[16];
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[5] = 1;
	element[15] = 1;
	element[0] = cos((double)radians);
	element[8] = sin((double)radians);
	element[2] = -sin((double)radians);
	element[10] = cos((double)radians);
	return Matrix4d(element);
}

Matrix4d Matrix4d::InitRotationMatrixZ(double radians){
	LoadIdentity();
	element[0] = cos((double)radians);
	element[1] = sin((double)radians);
	element[4] = -sin((double)radians);
	element[5] = cos((double)radians);
	return Matrix4d(element);
}


/** Initializes and returns a rotation matrix around the Z-axis. */
Matrix4d Matrix4d::GetRotationMatrixZ(double radians){
	double element[16];
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[10] = 1;
	element[15] = 1;
	element[0] = cos((double)radians);
	element[1] = sin((double)radians);
	element[4] = -sin((double)radians);
	element[5] = cos((double)radians);
	return Matrix4d(element);
}

Matrix4d Matrix4d::InitRotationMatrix(double angle, double x, double y, double z){
	LoadIdentity();
	double c = cos((double)angle);
	double s = sin((double)angle);
	double c1 = 1 - c;
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[4] = x * y * c1 - z * s;
	element[5] = y * y * c1 + c;
	element[6] = y * z * c1 + x * s;		// x -> c
	element[8] = x * z * c1 + y * s;
	element[9] = y * z * c1 - x * s;
	element[10] = z * z * c1 + c;
	return Matrix4d(element);
}

Matrix4d Matrix4d::InitRotationMatrix(double angle, Vector3d vector){
	LoadIdentity();
	double c = cos((double)angle);
	double s = sin((double)angle);
	double c1 = 1 - c;
	double x = vector.GetX();
	double y = vector.GetY();
	double z = vector.GetZ();
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[4] = x * y * c1 - z * s;
	element[5] = y * y * c1 + c;
	element[6] = y * z * c1 + c * s;
	element[8] = x * z * c1 + y * s;
	element[9] = y * z * c1 - x * s;
	element[10] = z * z * c1 + c;
	return Matrix4d(element);
}

/** Initializes a translation matrix using provided vector. */
Matrix4d Matrix4d::InitTranslationMatrix(Vector3f translation){
	Matrix4d mat;
	mat.translate(translation);
	return mat;
}

void Matrix4d::InitProjectionMatrix(double left, double right, double bottom, double top, double near, double far){
	LoadIdentity();
	element[0] = - 2.0f * near / (right - left);
	element[5] = - 2.0f * near / (top - bottom);
	element[8] = (right + left) / (right - left);
	element[9] = (top + bottom) / (top - bottom);
	element[10] = - ((-far) + (-near)) / ((-far) - (-near));
	element[11] = -1;
	element[14] = -2 * -far * -near / ((-far) - (-near));
	element[15] = 0;
}

void Matrix4d::InitOrthoProjectionMatrix(double left, double right, double bottom, double top, double near, double far){
	LoadIdentity();
	element[0] = 2.0f / (right - left);
	element[5] = 2.0f / (top - bottom);
	element[10] = 2.0f  / (far - near);
	element[12] = -(right + left) / (right - left);
	element[13] = -(top + bottom) / (top - bottom);
	element[14] = -(far + near) / (far - near);
}


// ************************************************************************//
// Transposition and invertion
// ************************************************************************//
void Matrix4d::Transpose(){
	double arr[16];
	for (int i = 0; i < 16; ++i){
		arr[i] = element[i];
	}
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			element[x*4 + y] = arr[y*4 + x];
		}
	}
}

Matrix4d Matrix4d::TransposedCopy() const {
	Matrix4d newMatrix;
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			newMatrix.element[x*4 + y] = element[y*4 + x];
		}
	}
	return newMatrix;
}

// Utility function to find the determinant of the matrix
double Matrix4d::getDeterminant() const {
	throw 3;
	return 1.0f;
}

/************************************************************
// Cramer's rule from Intel's source: ftp://download.intel.com/design/PentiumIII/sml/24504301.pdf
// Cramer's rule brief description:
// 1. Transpose the given matrix
// 2. Calculate cofactors of all matrix elements
//	  A new matrix is formed from all cofactors of the given matrix elements
// 3. Calculate the determinant of the given matrix
// 4. Multiply the matrix obtained in step 3 by the reciprocal of the determinant
/************************************************************
* input:
* mat - pointer to array of 16 doubles (source matrix)
* output:
* dst - pointer to array of 16 doubles (invert matrix)
*************************************************************/
bool InvertMatrix4d(double *mat, double *dst){
	double tmp[12]; /* temp array for pairs */
	double src[16]; /* array of transpose source matrix */
	double det; /* determinant */
	/* transpose matrix */
	for (int i = 0; i < 4; i++) {
		src[i] = mat[i*4];
		src[i + 4] = mat[i*4 + 1];
		src[i + 8] = mat[i*4 + 2];
		src[i + 12] = mat[i*4 + 3];
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	/* calculate determinant */
	det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
	if (det == 0)
		return false;
	/* calculate matrix inverse */
	det = 1/det;
	for (int j = 0; j < 16; j++)
		dst[j] *= det;
	return true;
}

void Matrix4d::Invert(){
	double newArr[16];
	InvertMatrix4d(element, newArr);
	for (int i = 0; i < 16; ++i)
		element[i] = newArr[i];
}

Matrix4d Matrix4d::InvertedCopy() const {
	double newArr[16];
	double src[16];
	memcpy(src, element, sizeof(double) * 16);
	InvertMatrix4d(src, newArr);
	return Matrix4d(newArr);
}

// ************************************************************************//
// 3D-operations
// ************************************************************************//
Matrix4d Matrix4d::translate(double x, double y, double z){
	element[12] += x;
	element[13] += y;
	element[14] += z;
	Matrix4d mx = Matrix4d(element);
	return mx;
}

Matrix4d Matrix4d::translate(Vector3d vec){
	element[12] += vec.GetX();
	element[13] += vec.GetY();
	element[14] += vec.GetZ();
	return Matrix4d(element);
}
/// Builds a translation matrix.
Matrix4d Matrix4d::Translation(double x, double y, double z){
	Matrix4d mat;
	mat.element[12] += x;
	mat.element[13] += y;
	mat.element[14] += z;
	return mat;
}

Matrix4d Matrix4d::Translation(Vector3d vec){
	Matrix4d mat;
	mat.element[12] += vec.x;
	mat.element[13] += vec.y;
	mat.element[14] += vec.z;
	return mat;
}


Matrix4d Matrix4d::scale(double ratio){
	element[0] *= ratio;
	element[5] *= ratio;
	element[10] *= ratio;
	return Matrix4d(element);
}

Matrix4d Matrix4d::scale(double x, double y, double z){
	element[0] *= x;
	element[5] *= y;
	element[10] *= z;
	return Matrix4d(element);
}

Matrix4d Matrix4d::scale(Vector3d vec){
	element[0] *= vec.x;
	element[5] *= vec.y;
	element[10] *= vec.z;
	return Matrix4d(element);
}

// ************************************************************************//
// Content access
// ************************************************************************//
/** Fills an array to the pointed location with a copy of the matrix's current contents. */
void Matrix4d::getContents(double * arr){
	for (int i = 0; i < 16; ++i){
		arr[i] = element[i];
	}
}

// ************************************************************************//
// Product functions
// ************************************************************************//

/** Direct multiplication
	Postcondition: Multiplies the provided matrix into the left one and returns a copy of the current one.
*/
Matrix4d Matrix4d::multiply(const Matrix4d matrix){
	double newArray[16];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * matrix.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	for (int i = 0; i < 16; ++i)
		element[i] = newArray[i];
	return Matrix4d(element);
}

/** Product with Matrix */
Matrix4d Matrix4d::product(const Matrix4d factor) const {
	double newArray[16];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * factor.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	return Matrix4d(newArray);
}

/** Product with Vector */
Vector4d Matrix4d::product(Vector4d vector) const {
	double newArray[4];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 4; i++){
			tempResult += element[y + i * 4] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector4d(newArray);
}

Vector4f Matrix4d::product(Vector4f vector) const {
	float newArray[4];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 4; i++){
			tempResult += (float)(element[y + i * 4] * vector[i]);
		}
		newArray[y] = tempResult;
	}
	return Vector4f(newArray);
}


Matrix4d Matrix4d::operator * (const Matrix4d factor) const {
	double newArray[16];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * factor.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	return Matrix4d(newArray);
}

Vector4d Matrix4d::operator * (Vector4d vector) const {
	double newArray[4];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 4; i++){
			tempResult += element[y + i * 4] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector4d(newArray);
}

void Matrix4d::operator *= (Matrix4d factor){
	double newArray[16];
	double tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * factor.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	memcpy(element, newArray, sizeof(double) * 16);
}

Vector4d Matrix4d::operator [](const unsigned int index){
	return Vector4d(&(element[index*4]));
}
