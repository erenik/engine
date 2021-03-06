/// Emil Hedemalm
/// 2015-02-17
/// SSEing

#include "Matrix4f.h"
#include <cassert>
#include <cstring>
#include <iomanip>

// ************************************************************************//
// Constructors
// ************************************************************************//
Matrix4f::Matrix4f(){
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[5] = 1;
	element[10] = 1;
	element[15] = 1;
}

Matrix4f::Matrix4f( float elements[4][4]){
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			element[x * 4 + y] = elements[x][y];
		}
	}
}

Matrix4f::Matrix4f( float elements[16]){
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			const int index = x * 4 + y;
			element[index] = elements[index];
		}
	}
}

Matrix4f::Matrix4f(Matrix3f &base){
	LoadIdentity();
	float data[9];
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

Matrix4f::Matrix4f(const Matrix4d& base){
	for (int i = 0; i < 16; ++i){
		element[i] = (float) base.element[i];
	}
}

/// For creating e.g. a rotation matrix based on given vectors.
void Matrix4f::SetVectors(ConstVec3fr xVec, ConstVec3fr yVec, ConstVec3fr zVec)
{
	element[0] = xVec.x;
	element[1] = xVec.y;
	element[2] = xVec.z;
	element[4] = yVec.x;
	element[5] = yVec.y;
	element[6] = yVec.z;
	element[8] = zVec.x;
	element[9] = zVec.y;
	element[10] = zVec.z;
}


/// o.o Create matrices!
List<Matrix4f> Matrix4f::FromFloatList(List<float> floatList, int numMatricesToExtract, bool transpose)
{
	List<Matrix4f> matrices;
	for (int i = 0; i < numMatricesToExtract; ++i)
	{
		Matrix4f matrix = &floatList[i * 16];
		if (transpose)
			matrix.Transpose();
		matrices.Add(matrix);
	}
	return matrices;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Matrix4f& mat){
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
void Matrix4f::LoadIdentity(){
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[5] = 1;
	element[10] = 1;
	element[15] = 1;
}

/// Mainly for comparison.
Matrix4f Matrix4f::Identity()
{
	Matrix4f mat;
	return mat;
}

bool Matrix4f::IsIdentity()
{
	Matrix4f mat;
	for (int i = 0; i < 16; ++i)
	{
		if (mat[i] != element[i])
			return false;
	}
	return true;
}

/// Conversion
Matrix3f Matrix4f::GetMatrix3f(){
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
Vector4f Matrix4f::GetColumn(int columnIndex)
{
	int s = columnIndex * 4;
	return Vector4f(element[s], element[s+1], element[s+2], element[s+3]);	
}


/*
Matrix4f Matrix4f::InitRotationMatrixX(float radians){
	LoadIdentity();
	element[5] = cos((float)radians);
	element[6] = sin((float)radians);
	element[9] = -sin((float)radians);
	element[10] = cos((float)radians);
}
*/

/// Initializes and returns a rotation matrix around the X-axis. 
Matrix4f Matrix4f::InitRotationMatrixX(float radians){
	float element[16];
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[0] = 1;
	element[15] = 1;
	element[5] = cos((float)radians);
	element[6] = sin((float)radians);
	element[9] = -sin((float)radians);
	element[10] = cos((float)radians);
	return Matrix4f(element);
}

/** Initializes and returns a rotation matrix around the X-axis. */
Matrix4f Matrix4f::InitRotationMatrixY(float radians){
	float element[16];
	for (int i = 0; i < 16; ++i){
		element[i] = 0;
	}
	element[5] = 1;
	element[15] = 1;
	element[0] = cos((float)radians);
	element[8] = sin((float)radians);
	element[2] = -sin((float)radians);
	element[10] = cos((float)radians);
	return Matrix4f(element);
}

/// Initializes a rotation matrix around the Z-axis.
Matrix4f Matrix4f::InitRotationMatrixZ(float radians)
{
	Matrix4f mat;
	mat.element[0] = cos((float)radians);
	mat.element[1] = sin((float)radians);
	mat.element[4] = -sin((float)radians);
	mat.element[5] = cos((float)radians);
	return mat;
}

void Matrix4f::InitRotationMatrix(float angle, float x, float y, float z){
	LoadIdentity();
	float c = cos((float)angle);
	float s = sin((float)angle);
	float c1 = 1 - c;
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[4] = x * y * c1 - z * s;
	element[5] = y * y * c1 + c;
	element[6] = y * z * c1 + c * s;
	element[8] = x * z * c1 + y * s;
	element[9] = y * z * c1 - x * s;
	element[10] = z * z * c1 + c;
}

void Matrix4f::InitRotationMatrix(float angle, const Vector3f & vector){
	LoadIdentity();
	float c = cos((float)angle);
	float s = sin((float)angle);
	float c1 = 1 - c;
	float x = vector[0];
	float y = vector[1];
	float z = vector[2];
	element[0] = x * x * c1 + c;
	element[1] = y * x * c1 + z * s;
	element[2] = x * z * c1 - y * s;
	element[4] = x * y * c1 - z * s;
	element[5] = y * y * c1 + c;
	element[6] = y * z * c1 + c * s;
	element[8] = x * z * c1 + y * s;
	element[9] = y * z * c1 - x * s;
	element[10] = z * z * c1 + c;
}

/** Initializes a translation matrix using provided vector. */
Matrix4f Matrix4f::InitTranslationMatrix(const Vector3f & translation)
{
	Matrix4f mat;
	mat.element[12] += translation[0];
	mat.element[13] += translation[1];
	mat.element[14] += translation[2];
	return mat;
}

/// Creates a scaling matrix (XYZ)
Matrix4f Matrix4f::InitScalingMatrix(const Vector3f & scale)
{
	Matrix4f mat;
	mat.element[0] = scale[0];
	mat.element[5] = scale[0];
	mat.element[10] = scale[0];
	return mat;
}

void Matrix4f::InitProjectionMatrix(float left, float right, float bottom, float top, float near, float far)
{
	Matrix4d proj;
	proj.InitProjectionMatrix(left,right, bottom, top, near,far);
	*this = proj;
	/*
	LoadIdentity();
	element[0] = 2.0f * near / (right - left);
	element[5] = 2.0f * near / (top - bottom);
	element[8] = (right + left) / (right - left);
	element[9] = (top + bottom) / (top - bottom);
	element[10] = - (far + near) / (far - near);
	element[11] = -1;
	element[14] = -2 * far * near / (far - near);
	element[15] = 0;
	*/
}

void Matrix4f::InitOrthoProjectionMatrix(float left, float right, float bottom, float top, float near, float far)
{
	Matrix4d ortho;
	ortho.InitOrthoProjectionMatrix(left,right,bottom,top,near,far);
	*this = ortho;
	/*
	LoadIdentity();
	element[0] = 2.0f / (right - left);
	element[5] = 2.0f / (top - bottom);
	element[10] = - 2.0f  / (far - near);
	element[12] = -(right + left) / (right - left);
	element[13] = -(top + bottom) / (top - bottom);
	element[14] = (far + near) / (far - near);*/
}


// ************************************************************************//
// Transposition and invertion
// ************************************************************************//
void Matrix4f::Transpose(){
	float arr[16];
	for (int i = 0; i < 16; ++i){
		arr[i] = element[i];
	}
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			element[x*4 + y] = arr[y*4 + x];
		}
	}
}

Matrix4f Matrix4f::TransposedCopy() const {
	Matrix4f newMatrix;
	for (int x = 0; x < 4; ++x){
		for (int y = 0; y < 4; ++y){
			newMatrix.element[x*4 + y] = element[y*4 + x];
		}
	}
	return newMatrix;
}

// Utility function to find the determinant of the matrix
float Matrix4f::getDeterminant() const {
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
* mat - pointer to array of 16 floats (source matrix)
* output:
* dst - pointer to array of 16 floats (invert matrix)
*************************************************************/
/// Returs false upon failure (bad determinant, and thus bad matrix).
bool InvertMatrix4f(float *mat, float *dst){
	float tmp[12]; /* temp array for pairs */
	float src[16]; /* array of transpose source matrix */
	float det; /* determinant */
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

void Matrix4f::Invert(){
	float newArr[16];
	InvertMatrix4f(element, newArr);
	for (int i = 0; i < 16; ++i)
		element[i] = newArr[i];
}

Matrix4f Matrix4f::InvertedCopy() const {
	float newArr[16];
	float src[16];
	memcpy(src, element, sizeof(float) * 16);
	InvertMatrix4f(src, newArr);
	return Matrix4f(newArr);
}

// ************************************************************************//
// 3D-operations
// ************************************************************************//
void Matrix4f::Translate(float x, float y, float z){
	element[12] += x;
	element[13] += y;
	element[14] += z;
}

/// Builds a 3D translation matrix using given vector.
const Matrix4f Matrix4f::Translation(const Vector3f & vec)
{
	Matrix4f mat;
	mat.element[12] += vec[0];
	mat.element[13] += vec[1];
	mat.element[14] += vec[2];
	return mat;
}

/// Array must be at least 4 long.
void Matrix4f::GetRow(int row, float * intoArray) const
{
	intoArray[0] = element[0+row];
	intoArray[1] = element[4+row];
	intoArray[2] = element[8+row];
	intoArray[3] = element[12+row];
}


/*
void Matrix4f::Translate(Vector3f vec){
	element[12] += vec[0];
	element[13] += vec[1];
	element[14] += vec[2];
}
*/
/*
/// Returns an initialized translation-matrix using given vector. 
Matrix4f Matrix4f::Translation(Vector3f trans){
	Matrix4f mat;
	mat.Translate(trans);
	return mat;
}
*/

void Matrix4f::Scale(float ratio)
{
	element[0] *= ratio;
	element[5] *= ratio;
	element[10] *= ratio;
}

void Matrix4f::Scale(float x, float y, float z){
	element[0] *= x;
	element[5] *= y;
	element[10] *= z;
}

/// Scale using a given vector
void Matrix4f::Scale(const Vector3f & scalingVector){
    /// TODO: Re-examine. Seems faulty.
    element[0] *= scalingVector[0];
	element[5] *= scalingVector[1];
	element[10] *= scalingVector[2];
}

/// Builds a 3D scaling matrix using given vector.
const Matrix4f Matrix4f::Scaling(const Vector3f & scalingVector)
{
	Matrix4f scaling;
    scaling.element[0] *= scalingVector[0];
	scaling.element[5] *= scalingVector[1];
	scaling.element[10] *= scalingVector[2];
	return scaling;
}

/// Returns true if it has non-0 values in the scale diagonal (elements 0, 5 and 10)
bool Matrix4f::HasValidScale()
{
	if (element[0] && element[5] && element[10])
		return true;
	return false;
}


// ************************************************************************//
// Content access
// ************************************************************************//

/** Returns a pointer to the start of the matrix element array. */
float * Matrix4f::getPointer()
{
	return element; 
}

const float * Matrix4f::getConstPointer() const 
{
	return element;
}
	

/** Fills an array to the pointed location with a copy of the matrix's current contents. */
void Matrix4f::getContents(float * arr){
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
void Matrix4f::Multiply(ConstMat4r matrix)
{
#ifdef USE_SSE
	Matrix4f newMat;
	// Gather rows from left matrix (this one).
	SSEVec row[4];
	GetRow(0, row[0].v);
	GetRow(1, row[1].v);
	GetRow(2, row[2].v);
	GetRow(3, row[3].v);

	// Gather columns from the right matrix (argument).
	// Already available. .. matrix.col
	matrix.col0;
	SSEVec result[4];
//	SSEVec result, result2, result3, result4;

	const SSEVec * col = matrix.cols;

#define MOAR
#ifdef MOAR

	// Multiply first 4 (first column)
#define MUL(i,j) result[i].data = _mm_mul_ps(row[i].data, col[j].data);
#define MULTIPLY_ROWS_WITH_COLUMN(c) \
	MUL(0,c); \
	MUL(1,c); \
	MUL(2,c); \
	MUL(3,c); 
	// First 4.
	MULTIPLY_ROWS_WITH_COLUMN(0);

	// Sum the products up
	SSEVec adder1, adder2, adder3, adder4;
#define LOAD_ADDERS(i) \
	adder1.v[i] = result[i].x;\
	adder2.v[i] = result[i].y;\
	adder3.v[i] = result[i].z;\
	adder4.v[i] = result[i].w;
	// Load adders 
	// Do first 2 additions.
	// Final addition into column.
#define SUM_INTO_COLUMN(c) \
	LOAD_ADDERS(0); \
	LOAD_ADDERS(1); \
	LOAD_ADDERS(2); \
	LOAD_ADDERS(3); \
	result[0].data = _mm_add_ps(adder1.data, adder2.data); \
	result[1].data = _mm_add_ps(adder3.data, adder4.data); \
	newMat.cols[c].data = _mm_add_ps(result[0].data, result[1].data); 

	// First column done.
	SUM_INTO_COLUMN(0);

	// Second
	MULTIPLY_ROWS_WITH_COLUMN(1);
	SUM_INTO_COLUMN(1);
	MULTIPLY_ROWS_WITH_COLUMN(2);
	SUM_INTO_COLUMN(2);
	MULTIPLY_ROWS_WITH_COLUMN(3);
	SUM_INTO_COLUMN(3);
	// Hopefully done now.

#else
	/// Do multiplications? One for each element in the new matrix.
	for (int i = 0; i < 16; ++i)
	{
		result.data = _mm_mul_ps(row[i % 4].data, col[i / 4].data);
		// Sum it up?
		newMat.element[i] = result.x + result.y + result.z + result.w;
	}
#endif

//	newMat.Print();
	// Paste over new data to us.
	col0 = newMat.col0;
	col1 = newMat.col1;
	col2 = newMat.col2;
	col3 = newMat.col3;
#else
	float newArray[16];
	float tempResult;
	for (int y = 0; y < 4; y++){	
		for(int x = 0; x < 4; x++){	
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * matrix.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	for (int i = 0; i < 16; ++i)
		element[i] = newArray[i];
//	*this = Matrix4f(element);
#endif
}

/** Product with Matrix */
Matrix4f Matrix4f::Product(ConstMat4r factor) const 
{
	float newArray[16];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * factor.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	return Matrix4f(newArray);
}

/** Product with Vector */
Vector4f Matrix4f::Product(const Vector4f & vector) const 
{
#ifdef USE_SSE
	/// Matrix product using SSE instructions...!
	Vector4f result;
	// Col*Row result
	SSEVec result1, result2, result3, result4;
	SSEVec matrixCol;
	

#define TEST_SSE_VEC
#ifdef TEST_SSE_VEC
	// Load data.
	float row[4];
	GetRow(0, row);
	matrixCol.data = _mm_loadu_ps(row);
	// Do multiplication.
	result1.data = _mm_mul_ps(matrixCol.data, vector.data);
	// Repeat for other 3 columns
	GetRow(1, row);
	matrixCol.data = _mm_loadu_ps(row);
	result2.data = _mm_mul_ps(matrixCol.data, vector.data);
	GetRow(2, row);
	matrixCol.data = _mm_loadu_ps(row);
	result3.data = _mm_mul_ps(matrixCol.data, vector.data);
	GetRow(3, row);
	matrixCol.data = _mm_loadu_ps(row);
	result4.data = _mm_mul_ps(matrixCol.data, vector.data);
#else
	// Load data.
	float row[4];
	GetRow(0, row);
	matrixCol = _mm_loadu_ps(row);
	// Do multiplication.
	result1 = _mm_mul_ps(matrixCol, vector.data);
	// Repeat for other 3 columns
	GetRow(1, row);
	matrixCol = _mm_loadu_ps(row);
	result2 = _mm_mul_ps(matrixCol, vector.data);
	GetRow(2, row);
	matrixCol = _mm_loadu_ps(row);
	result3 = _mm_mul_ps(matrixCol, vector.data);
	GetRow(3, row);
	matrixCol = _mm_loadu_ps(row);
	result4 = _mm_mul_ps(matrixCol, vector.data);
#endif
	// Okay, got the 16 first multiplication results in, now we want to add together each of them individually, if possible...
	SSEVec adder1, adder2, adder3, adder4;

#ifdef TEST_SSE_VEC
#define LOAD_ADDERS(index, withData) \
	adder1.v[index] = withData.x;\
	adder2.v[index] = withData.y;\
	adder3.v[index] = withData.z;\
	adder4.v[index] = withData.w;

	LOAD_ADDERS(0, result1);
	LOAD_ADDERS(1, result2);
	LOAD_ADDERS(2, result3);
	LOAD_ADDERS(3, result4);
	
#else
	adder1.v[0] = result1.v[0];
	adder2.v[0] = result1.v[1];
	adder3.v[0] = result1.v[2];
	adder4.v[0] = result1.v[3];

	adder1.v[1] = result2.v[0];
	adder2.v[1] = result2.v[1];
	adder3.v[1] = result2.v[2];
	adder4.v[1] = result2.v[3];

	adder1.v[2] = result3.v[0];
	adder2.v[2] = result3.v[1];
	adder3.v[2] = result3.v[2];
	adder4.v[2] = result3.v[3];

	adder1.v[3] = result4.v[0];
	adder2.v[3] = result4.v[1];
	adder3.v[3] = result4.v[2];
	adder4.v[3] = result4.v[3];
#endif

	// Do addition.
#ifdef TEST_SSE_VEC
	result1.data = _mm_add_ps(adder1.data, adder2.data);
	result2.data = _mm_add_ps(adder3.data, adder4.data);
	/// Final addition.
	result.data = _mm_add_ps(result1.data, result2.data); 
#else
	result1 = _mm_add_ps(adder1, adder2);
	result2 = _mm_add_ps(adder3, adder4);
	/// Final addition.
	result.data = _mm_add_ps(result1, result2); 
#endif

	return result;
#else
	float newArray[4];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 4; i++){
			tempResult += element[y + i * 4] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector4f(newArray);
#endif
}

Matrix4f Matrix4f::operator * (ConstMat4r matrix) const 
{
#ifdef USE_SSE
	Matrix4f newMat;
	// Gather rows from left matrix (this one).
	SSEVec row[4];
	GetRow(0, row[0].v);
	GetRow(1, row[1].v);
	GetRow(2, row[2].v);
	GetRow(3, row[3].v);

	// Gather columns from the right matrix (argument).
	// Already available. .. matrix.col
	matrix.col0;
	SSEVec result[4];
	const SSEVec * col = matrix.cols;

	// Multiply first 4 (first column)
#define MUL(i,j) result[i].data = _mm_mul_ps(row[i].data, col[j].data);
#define MULTIPLY_ROWS_WITH_COLUMN(c) \
	MUL(0,c); \
	MUL(1,c); \
	MUL(2,c); \
	MUL(3,c); 
	// First 4.
	MULTIPLY_ROWS_WITH_COLUMN(0);

	// Sum the products up
	SSEVec adder1, adder2, adder3, adder4;
#define LOAD_ADDERS(i) \
	adder1.v[i] = result[i].x;\
	adder2.v[i] = result[i].y;\
	adder3.v[i] = result[i].z;\
	adder4.v[i] = result[i].w;
	// Load adders 
	// Do first 2 additions.
	// Final addition into column.
#define SUM_INTO_COLUMN(c) \
	LOAD_ADDERS(0); \
	LOAD_ADDERS(1); \
	LOAD_ADDERS(2); \
	LOAD_ADDERS(3); \
	result[0].data = _mm_add_ps(adder1.data, adder2.data); \
	result[1].data = _mm_add_ps(adder3.data, adder4.data); \
	newMat.cols[c].data = _mm_add_ps(result[0].data, result[1].data); 

	// First column done.
	SUM_INTO_COLUMN(0);

	// Second
	MULTIPLY_ROWS_WITH_COLUMN(1);
	SUM_INTO_COLUMN(1);
	MULTIPLY_ROWS_WITH_COLUMN(2);
	SUM_INTO_COLUMN(2);
	MULTIPLY_ROWS_WITH_COLUMN(3);
	SUM_INTO_COLUMN(3);
	// Hopefully done now.
	return newMat;
#else
	float newArray[16];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * matrix.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	return Matrix4f(newArray);
#endif
}

Vector4f Matrix4f::operator * (const Vector4f & vector) const 
{
#ifdef USE_SSE
		/// Matrix product using SSE instructions...!
	Vector4f result;
	// Col*Row result
	SSEVec result1, result2, result3, result4;
	SSEVec matrixCol;
	
	// Load data.
	float row[4];
	GetRow(0, row);
	matrixCol.data = _mm_loadu_ps(row);
	// Do multiplication.
	result1.data = _mm_mul_ps(matrixCol.data, vector.data);
	// Repeat for other 3 columns
	GetRow(1, row);
	matrixCol.data = _mm_loadu_ps(row);
	result2.data = _mm_mul_ps(matrixCol.data, vector.data);
	GetRow(2, row);
	matrixCol.data = _mm_loadu_ps(row);
	result3.data = _mm_mul_ps(matrixCol.data, vector.data);
	GetRow(3, row);
	matrixCol.data = _mm_loadu_ps(row);
	result4.data = _mm_mul_ps(matrixCol.data, vector.data);

	// Okay, got the 16 first multiplication results in, now we want to add together each of them individually, if possible...
	SSEVec adder1, adder2, adder3, adder4;
	adder1.v[0] = result1.v[0];
	adder2.v[0] = result1.v[1];
	adder3.v[0] = result1.v[2];
	adder4.v[0] = result1.v[3];

	adder1.v[1] = result2.v[0];
	adder2.v[1] = result2.v[1];
	adder3.v[1] = result2.v[2];
	adder4.v[1] = result2.v[3];

	adder1.v[2] = result3.v[0];
	adder2.v[2] = result3.v[1];
	adder3.v[2] = result3.v[2];
	adder4.v[2] = result3.v[3];

	adder1.v[3] = result4.v[0];
	adder2.v[3] = result4.v[1];
	adder3.v[3] = result4.v[2];
	adder4.v[3] = result4.v[3];

	// Do addition.
	result1.data = _mm_add_ps(adder1.data, adder2.data);
	result2.data = _mm_add_ps(adder3.data, adder4.data);
	
//	result[x] = tempResult.v[0] + tempResult.v[1] + tempResult.v[2] + tempResult.v[3];
	/// Final addition.
	result.data = _mm_add_ps(result1.data, result2.data); 

	return result;
#else
	float newArray[4];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		tempResult = 0;
		for(int i = 0; i < 4; i++){
			tempResult += element[y + i * 4] * vector[i];
		}
		newArray[y] = tempResult;
	}
	return Vector4f(newArray);
#endif
}

void Matrix4f::operator *= (ConstMat4r factor){
	float newArray[16];
	float tempResult;
	for (int y = 0; y < 4; y++){	// Rows
		for(int x = 0; x < 4; x++){	// Columns
			tempResult = 0;

			for(int i = 0; i < 4; i++){
				tempResult += element[y + i * 4] * factor.element[i + x * 4];
			}
			newArray[y + x * 4] = tempResult;
		}
	}
	memcpy(element, newArray, sizeof(float) * 16);
}

/*
Vector4f Matrix4f::operator [](const unsigned int index){
	return Vector4f(&(element[index*4]));
}
*/

float & Matrix4f::operator[](const unsigned int index)
{
	return element[index];
}


/** Operator overloading for the array-access. Returns specified column as as shown below:
Col	  0    1    2    3
	[ 0] [ 4] [ 8] [12]
	[ 1] [ 5] [ 9] [13]
	[ 2] [ 6] [10] [14]
	[ 3] [ 7] [11] [15]*/
void Matrix4f::Print()
{
	std::cout<<"\nMatrix4f::Print:";
	int row = 0, column = 0;
	for (int i = 0; i < 16; ++i)
	{
		if (i % 4 == 0) 
			std::cout<<"\n";
		column = i % 4;
		row = i / 4;
		int index = row + column * 4;
		std::cout<<" "<<std::fixed<<std::setw(10)<<std::setprecision(3)<<element[index];
	}
}


/// Tests all functions. Makes sure they work fine.
void Matrix4f::UnitTest()
{
	List<Matrix4f> matricesToTest;
	matricesToTest.Add(Matrix4f());

	/// Multiply 3 regular vectors.
	List<Vector3f> vectorsToTest;
	vectorsToTest.Add(Vector3f(1,0,0), Vector3f(0,1,0), Vector3f(0,0,1));

	for (int i = 0; i < matricesToTest.Size(); ++i)
	{
		Matrix4f & mat = matricesToTest[i];

		for (int j = 0; j < vectorsToTest.Size(); ++j)
		{
			Vector3f & vector = vectorsToTest[j];
			Vector4f product = mat * vector;
			assert(product.Length3() == 1);
		}		
	}

	Matrix4f scalingMatrix = Matrix4f::Scaling(Vector3f(2,2,2));
	Matrix4f scalingMatrix2 = Matrix4f::Scaling(Vector3f(1.5,3,4.5));

	Matrix4f rot = Matrix4f::InitRotationMatrixX(0.4f);
	Matrix4f rot2 = Matrix4f::InitRotationMatrixY(0.2f);
	Matrix4f trans = Matrix4f::Translation(Vector3f(10,2,7));
	Matrix4f rot3 = Matrix4f::InitRotationMatrixZ(0.1f);

	Matrix4f product = rot.Product(rot2).Product(trans).Product(rot3);
//	Matrix4f mult = rot.Multiply(rot2).Multiply(trans).Multiply(rot3);

//	product.Print();
//	mult.Print();

	Vector3f a(1,0,0), b(0,1,0), c(0,0,1);
	Vector3f cross = b.CrossProduct(c);
	assert(AbsoluteValue(cross.x) > 0.95f);
	Vector3f cross2 = a.CrossProduct(b);
	Vector3f cross3 = c.CrossProduct(a);
	
	Vector3f xz(1,0,1), y(0,1,0);
	Vector3f cross4 = xz.CrossProduct(y);
	assert(cross4.x < 0 && cross4.z > 0);

}

