/// Emil Hedemalm
/// 2013-03-01

#include "AEMath.h"
#include "Vector2i.h"
#include "Vector2f.h"
#include "Vector3i.h"
#include "Vector3f.h"
#include "Vector4d.h"
#include <cassert>
#include <fstream>
#include "Matrix4f.h"

#include "String/AEString.h"

#ifndef abs
#define abs(b) ((b < 0)? (-b) : (b))
#endif

#ifdef USE_SSE
/*
#define x (*this)[0]
#define y (*this)[1]
#define z (*this)[2]
*/
#endif

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector3f::Vector3f()
{
#ifdef USE_SSE
	static float arr[4] = {0,0,0,1};
	data = _mm_loadu_ps(arr);
#else
	x = y = z = 0;
#endif
}


Vector3f::Vector3f( float ix,  float iy,  float iz)
{
#ifdef USE_SSE
	float arr[4] = {ix,iy,iz,1};
	data = _mm_loadu_ps(arr);
#else
	x = ix;
	y = iy;
	z = iz;
#endif
}

Vector3f::Vector3f(float arr[])
{
#ifdef USE_SSE
	float arr2[4] = {arr[0],arr[1],arr[2],1};
	data = _mm_loadu_ps(arr2);
#else
    assert(arr);
	x = arr[0];
	y = arr[1];
	z = arr[2];
#endif
}



/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector2i& base, float iz /* = 0*/)
{
#ifdef USE_SSE
	float arr[4] = {(float)base[0],(float)base[1],iz,1.f};
	data = _mm_loadu_ps(arr);
#else
	x = base[0];
	y = base[1];
	this->z = z;
#endif
}

Vector3f::Vector3f(const Vector2f & base, float iz /* = 0*/)
{
#ifdef USE_SSE
	float arr[4] = {base[0], base[1], iz, 1};
	data = _mm_loadu_ps(arr);
#else
	x = base[0];
	y = base[1];
	this->z = z;
#endif
}

Vector3f::Vector3f(const Vector3f & base)
{
#ifdef USE_SSE
	data = base.data;
#else
	x = base[0];
	y = base[1];
	z = base[2];
#endif
}
/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector3d& base)
{
#ifdef USE_SSE
	float arr[4] = {(float)base[0], (float)base[1], (float)base[2], 1};
	data = _mm_loadu_ps(arr);
#else
	x = (float)base[0];
	y = (float)base[1];
	z = (float)base[2];
#endif
}
/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector3i& base)
{
#ifdef USE_SSE
	float arr[4] = {(float)base[0], (float)base[1], (float)base[2], 1};
	data = _mm_loadu_ps(arr);
#else
	x = (float)base[0];
	y = (float)base[1];
	z = (float)base[2];	
#endif
}

// Constructors from other Vector classes
Vector3f::Vector3f(const Vector4f& base)
{
#ifdef USE_SSE
	data = base.data;
#else
	x = base[0];
	y = base[1];
	z = base[2];
#endif
}
/**	Conversion Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector. The w-value is discarded.
*/
Vector3f::Vector3f(const Vector4d& base)
{
#ifdef USE_SSE
	float arr[4] = {(float)base[0], (float)base[1], (float)base[2], 1};
	data = _mm_loadu_ps(arr);
#else
	x = (float)base[0];
	y = (float)base[1];
	z = (float)base[2];
#endif
}

/// Virtual destructor so sub-classes get de-allocated appropriately.
Vector3f::~Vector3f()
{
}

/// o.o Create Vectors!
List<Vector3f> Vector3f::FromFloatList(List<float> floatList, int numVectorsToExtract)
{
	List<Vector3f> vectors;
	for (int i = 0; i < numVectorsToExtract; ++i)
	{
		Vector3f vector = &floatList[i * 3];
		vectors.Add(vector);
	}
	return vectors;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector3f& vec){
	os << vec[0] << " " << vec[1] << " " << vec[2];
	return os;
}

/// Writes to file stream.
void Vector3f::WriteTo(std::fstream & file)
{
	file.write((char*)&(*this)[0], sizeof(float));
	file.write((char*)&(*this)[1], sizeof(float));
	file.write((char*)&(*this)[2], sizeof(float));
}
/// Reads from file stream.
void Vector3f::ReadFrom(std::fstream & file){
	file.read((char*)&(*this)[0], sizeof(float));
	file.read((char*)&(*this)[1], sizeof(float));
	file.read((char*)&(*this)[2], sizeof(float));
}

/// Reads from String. Expects space-separated values. E.g. 3 8.14 -15
void Vector3f::ReadFrom(const String & string)
{
	List<String> tokens = string.Tokenize(" ");
	(*this)[0] = tokens[0].ParseFloat();
	(*this)[1] = tokens[1].ParseFloat();
	(*this)[2] = tokens[2].ParseFloat();
}

/// Parses from string. Expects in the form of first declaring order "XY", "X Y" or "YX", then parses the space-separated values.
void Vector3f::ParseFrom(const String & str)
{
	String string = str;
	List<float*> order;
	for (int i = 0; i < string.Length(); ++i)
	{
		char c = string.c_str()[i];
		if (c == 'x' || c == 'X')
			order.Add(&(*this)[0]);
		else if (c == 'y' || c == 'Y')
			order.Add(&(*this)[1]);
		else if (c == 'z' || c == 'Z')
			order.Add(&(*this)[2]);
	}
	List<String> parts = string.Tokenize(" ");
	int numbersParsed = 0;
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		if (!part.IsNumber())
			continue;
		float number = part.ParseFloat();
		float * ptr = order[numbersParsed];
		*ptr = number;
		++numbersParsed;
		if (numbersParsed >= order.Size())
			break;
	}
}

/// Returns abs-version.
Vector3f Vector3f::Abs() const
{
#ifdef USE_SSE
	Vector3f newVec;
	newVec.data = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), data), data);
	return newVec;
#else
	return Vector3f(x > 0? x : -x, y > 0? y : -y, z > 0? z : -z);
#endif
}


/// Clamp to an interval.
void Vector3f::Clamp(float min, float max)
{
	ClampFloat(x, min, max);
	ClampFloat(y, min, max);
	ClampFloat(z, min, max);
}

void Vector3f::Clamp(const Vector3f & min, const Vector3f & max)
{
	ClampFloat(x, min[0], max[0]);
	ClampFloat(y, min[1], max[1]);
	ClampFloat(z, min[2], max[2]);
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector3f::add(const Vector3f & addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}


void Vector3f::subtract(const Vector3f & subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
}

void Vector3f::scale(float ratio){
	x *= ratio;
	y *= ratio;
	z *= ratio;
}

void Vector3f::scale(float ix, float iy, float iz){
	x *= ix;
	y *= iy;
	z *= iz;
}


// ************************************************************************//
// Operator overloading
// ************************************************************************//
/// o.o
bool Vector3f::operator == (ConstVec3fr comparand) const
{
	if (x == comparand.x &&
		y == comparand.y &&
		z == comparand.z)
		return true;
	return false;
}

/// Comparison operators
bool Vector3f::operator != (const Vector3f & comparand) const 
{
	if (comparand[0] != x ||
		comparand[1] != y ||
		comparand[2] != z)
		return true;
	return false;
}

/// This will return true if and only if all three components (x,y,z) are smaller than their corresponding comparands in the vector comparand.
bool Vector3f::operator < (const Vector3f & comparand) const
{
	if (x < comparand.x &&
		y < comparand.y &&
		z < comparand.z)
		return true;
	return false;
}
/// This will return true if and only if all three components (x,y,z) are larger than their corresponding comparands in the vector comparand.
bool Vector3f::operator > (const Vector3f & comparand) const
{
	if (x > comparand.x &&
		y > comparand.y &&
		z > comparand.z)
		return true;
	return false;
}


// Unary - operator (switch signs of all sub-elements)
Vector3f Vector3f::operator - () const 
{
	return Vector3f(-x, -y, -z);
}


Vector3f  Vector3f::operator + (const Vector3f & addend) const 
{
	Vector3f  newVec;
#ifdef USE_SSE
	// Load data as needed.
	newVec.data = _mm_add_ps(data, addend.data);
#else
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
#endif
	return newVec;
}


Vector3f  Vector3f::operator - (const Vector3f & subtractor) const 
{
	Vector3f  newVec;
#ifdef USE_SSE
	// Load data as needed.
	newVec.data = _mm_sub_ps(data, subtractor.data);
#else
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
#endif
	return newVec;
}

/// Multiplication with float
Vector3f operator * (float multiplier, const Vector3f & vector)
{
	Vector3f  newVec;
#ifdef USE_SSE
	// Load data as needed.
	__m128 mul = _mm_load1_ps(&multiplier);
	newVec.data = _mm_mul_ps(vector.data, mul);
#else
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	newVec[2] = vector[2] * multiplier;
#endif
	return newVec;
}

/// Adds addend to this vector.
void Vector3f::operator += (const Vector3f & addend)
{
#ifdef USE_SSE
	// Load data as needed.
	data = _mm_add_ps(data, addend.data);
#else
	x += addend[0];
	y += addend[1];
	z += addend[2];
#endif
}


void Vector3f::operator -= (const Vector3f & subtractor)
{
#ifdef USE_SSE
	// Load data as needed.
	data = _mm_sub_ps(data, subtractor.data);
#else
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
#endif
}
/// Internal element division
void Vector3f::operator /= (const float & f)
{
#ifdef USE_SSE
	__m128 dividend = _mm_load1_ps(&f);
	data = _mm_div_ps(data, dividend);
#else
	x /= f;
	y /= f;
	z /= f;
#endif
}
/// Internal element multiplication
void Vector3f::operator *= (const float &f)
{
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&f);
	data = _mm_mul_ps(data, mul);
#else
	x *= f;
	y *= f;
	z *= f;
#endif
}

/// Internal element multiplication
void Vector3f::operator *= (const Vector3f & vec)
{
#ifdef USE_SSE
	data = _mm_mul_ps(data, vec.data);
#else
	x *= f[0];
	y *= f[1];
	z *= f[2];
#endif
}
	
/// Internal element multiplication
void Vector3f::operator *= (const Matrix4f &mat)
{
	Vector4f newVec = mat.Product(Vector4f(*this));
	x = newVec[0];
	y = newVec[1];
	z = newVec[2];
}

/// Internal element multiplication
Vector3f Vector3f::operator * (const float &f) const 
{
	Vector3f newVec;
#ifdef USE_SSE
	__m128 sse = _mm_load1_ps(&f);
	newVec.data = _mm_mul_ps(data, sse);
#else
	vec = Vector3f(x * f, y * f, z * f);
#endif
	return newVec;
}
/// Internal element division.
Vector3f Vector3f::operator / (const float &f) const 
{	
	Vector3f newVec;
#ifdef USE_SSE
	__m128 sse = _mm_load1_ps(&f);
	newVec.data = _mm_div_ps(data, sse);
#else
	newVec = Vector3f(x / f, y / f, z / f);
#endif
	return newVec;
}

float & Vector3f::operator [](int index)
{
#ifdef USE_SSE
	return data.m128_f32[index];
#else
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw 1003;
	}
#endif
}

/// Operator overloading for the array-access operator []
const float & Vector3f::operator [](int index) const
{
#ifdef USE_SSE
	return data.m128_f32[index];
#else
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw 1003;
	}	
#endif
}


// ************************************************************************//
// Vector operations
// ************************************************************************//
float Vector3f::ScalarProduct(const Vector3f & otherVector) const {
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2];
}
// Same thing as scalar product, silly!
float Vector3f::DotProduct(const Vector3f & otherVector) const {
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2];
}

Vector3f Vector3f::CrossProduct(const Vector3f & otherVector) const {
	return Vector3f (y * otherVector[2] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector3f Vector3f::ElementMultiplication(const Vector3f & otherVector) const {
	return Vector3f(x * otherVector[0], y * otherVector[1], z * otherVector[2]);
}
Vector3f Vector3f::ElementDivision(const Vector3f & otherVector) const 
{
	return Vector3f(x / otherVector[0], y / otherVector[1], z / otherVector[2]);
}

/// Calculates the length of the vector.
float Vector3f::Length() const 
{
	float sum = x * x + y * y + z * z;
	if (sum == 0)
		return 0;
	assert(abs(sum) != 0 && "VEctor3f::Length");
	return sqrt(sum);
}
/// Calculates the squared length of the vector.
float Vector3f::LengthSquared() const {
	return x * x + y * y + z * z;
}

Vector3f Vector3f::Normalize(){
	float vecLength = Length();
	if (vecLength < ZERO){
		// assert(vecLength != 0 && "VEctor3f::Normalize");
		vecLength = 1.0f;
	}

	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
	return Vector3f(x, y, z);
}

/** Returns a normalized copy of this vector. */
Vector3f Vector3f::NormalizedCopy() const {
	float vecLength = Length();
	if (vecLength < ZERO){
		return Vector3f();
	}
	return Vector3f(x / vecLength, y / vecLength, z / vecLength);
};

/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
const float Vector3f::MaxPart() const 
{
#ifdef USE_SSE
	Vector3f av(abs(x), abs(y), abs(z));
	if (av[0] > av[1])
	{
		if (av[0] > av[2])
			return av[0];
		return av[2];
	} 
	if (av[1] > av[2])
		return av[1];
	return av[2];
#else
	if (AbsoluteValue(x) > AbsoluteValue(y)){
		if (AbsoluteValue(x) > AbsoluteValue(z))
			return AbsoluteValue(x);
		return AbsoluteValue(z);
	}
	else if (AbsoluteValue(y) > AbsoluteValue(z))
		return AbsoluteValue(y);
	return AbsoluteValue(z);
#endif
};

/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
const float Vector3f::MinPart() const 
{
#ifdef USE_SSE
	assert(false);
	return 0;
#else
	if ((x) < (y)){
		if ((x) < (z))
			return (x);
		return (z);
	}
	else if ((y) < (z))
		return (y);
	return (z);
#endif
};

/// Returns the absolute value of the sub-component (x,y,z) of lowest absolute value.
const float Vector3f::MinPartAbs() const
{
#ifdef USE_SSE
	if (x < y)
	{
		if (x < z)
			return abs(x);
		return abs(y);
	} 
	if (y < z)
		return abs(y);
	return abs(z);
#else
	if (AbsoluteValue(x) < AbsoluteValue(y)){
		if (AbsoluteValue(x) < AbsoluteValue(z))
			return AbsoluteValue(x);
		return AbsoluteValue(z);
	}
	else if (AbsoluteValue(y) < AbsoluteValue(z))
		return AbsoluteValue(y);
	return AbsoluteValue(z);
#endif
};


/// Utility functions
Vector3f Vector3f::Minimum(const Vector3f & vec1, const Vector3f & vec2){
	return Vector3f(
		vec1[0] < vec2[0] ? vec1[0] : vec2[0],
		vec1[1] < vec2[1] ? vec1[1] : vec2[1],
		vec1[2] < vec2[2] ? vec1[2] : vec2[2]
	);
}
Vector3f Vector3f::Maximum(const Vector3f & vec1, const Vector3f & vec2){
	return Vector3f(
		vec1[0] > vec2[0] ? vec1[0] : vec2[0],
		vec1[1] > vec2[1] ? vec1[1] : vec2[1],
		vec1[2] > vec2[2] ? vec1[2] : vec2[2]
	);
}
// Rounds to nearest digit!
void Vector3f::Round(){
	x = floor(x+0.5f);
	y = floor(y+0.5f);
	z = floor(z+0.5f);
}


Vector3i Vector3f::Rounded()
{
	return Vector3i(floor(x+0.5f), floor(y+0.5f), floor(z+0.5f));
}

#ifdef USE_SSE
// Loads data into __m128 structure.
void Vector3f::PrepareForSIMD()
{
	data = _mm_setr_ps(x,y,z,1);
}
#endif
