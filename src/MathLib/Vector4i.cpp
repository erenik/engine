/// Emil Hedemalm
/// 2014-09-29
/// Integer-based 4-element vector.

#include "Vector4i.h"

#include "AEMath.h"
#include "Vector2i.h"
#include "Vector2f.h"
#include "Vector4d.h"
#include <cassert>
#include <fstream>
#include "Matrix4f.h"

#include "String/AEString.h"

#ifndef abs
#define abs(b) ((b < 0)? (-b) : (b))
#endif

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector4i::Vector4i()
{
	x = y = z = 0;
}


Vector4i::Vector4i(int ix,  int iy,  int iz, int iw)
{
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}

Vector4i::Vector4i(int arr[]){
    assert(arr);
	x = arr[0];
	y = arr[1];
	z = arr[2];
	w = arr[3];
}



/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector4i::Vector4i(const Vector2i& base, int z, int w)
{
	x = base[0];
	y = base[1];
	this->z = z;
	this->w = w;
}

Vector4i::Vector4i(const Vector3f & base, int w)
{
	x = (int)RoundFloat(base[0]);
	y = (int)RoundFloat(base[1]);
	z = (int)RoundFloat(base[2]);
	this->w = w;
}

Vector4i::Vector4i(const Vector4i & base)
{
	x = base[0];
	y = base[1];
	z = base[2];
	w = base[3];
}

// Constructors from other Vector classes
Vector4i::Vector4i(const Vector4f& base)
{
	x = (int)RoundFloat(base[0]);
	y = (int)RoundFloat(base[1]);
	z = (int)RoundFloat(base[2]);
	w = (int)RoundFloat(base[3]);
}

/*
/// Virtual destructor so sub-classes get de-allocated appropriately.
Vector4i::~Vector4i()
{
}

/// o.o Create Vectors!
List<Vector4i> Vector4i::FromFloatList(List<float> floatList, int numVectorsToExtract)
{
	List<Vector4i> vectors;
	for (int i = 0; i < numVectorsToExtract; ++i)
	{
		Vector4i vector = &floatList[i * 3];
		vectors.Add(vector);
	}
	return vectors;
}
*/

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector4i& vec){
	os << vec[0] << " " << vec[1] << " " << vec[2];
	return os;
}

/// Writes to file stream.
void Vector4i::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(float));
	file.write((char*)&y, sizeof(float));
	file.write((char*)&z, sizeof(float));
}
/// Reads from file stream.
void Vector4i::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(float));
	file.read((char*)&y, sizeof(float));
	file.read((char*)&z, sizeof(float));
}

/*
/// Reads from String. Expects space-separated values. E.g. 3 8.14 -15
void Vector4i::ReadFrom(const String & string)
{
	List<String> tokens = string.Tokenize(" ");
	x = tokens[0].ParseFloat();
	y = tokens[1].ParseFloat();
	z = tokens[2].ParseFloat();
}


/// Clamp to an interval.
void Vector4i::Clamp(float min, float max)
{
	ClampFloat(x, min, max);
	ClampFloat(y, min, max);
	ClampFloat(z, min, max);
}

void Vector4i::Clamp(Vector4i min, Vector4i max)
{
	ClampFloat(x, min[0], max[0]);
	ClampFloat(y, min[1], max[1]);
	ClampFloat(z, min[2], max[2]);
}*/

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector4i::Add(Vector4i addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}


void Vector4i::Subtract(Vector4i subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
}

void Vector4i::Scale(float ratio)
{
	x *= ratio;
	y *= ratio;
	z *= ratio;
}

void Vector4i::Scale(float ix, float iy, float iz)
{
	x *= ix;
	y *= iy;
	z *= iz;
}


// ************************************************************************//
// Operator overloading
// ************************************************************************//

/// Comparison operators
bool Vector4i::operator == (const Vector4i comparand) const 
{
	if (comparand[0] == x &&
		comparand[1] == y &&
		comparand[2] == z &&
		comparand[3] == w)
		return true;
	return false;
}

bool Vector4i::operator != (const Vector4i comparand) const 
{
	if (comparand[0] != x ||
		comparand[1] != y ||
		comparand[2] != z ||
		comparand[3] != w)
		return true;
	return false;
}


// Unary - operator (switch signs of all sub-elements)
Vector4i Vector4i::operator - () const 
{
	return Vector4i(-x, -y, -z, -w);
}


Vector4i  Vector4i::operator + (Vector4i addend) const {
	Vector4i  newVec;
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
	newVec[3] = w + addend[3];
	return newVec;
}


Vector4i  Vector4i::operator - (Vector4i subtractor) const 
{
	Vector4i  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	newVec[3] = w - subtractor[3];
	return newVec;
}

/// Multiplication with float
Vector4i operator * (float multiplier, const Vector4i vector)
{
	Vector4i  newVec;
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	newVec[2] = vector[2] * multiplier;
	newVec[3] = vector[3] * multiplier;
	return newVec;
}

/// Adds addend to this vector.
void Vector4i::operator += (Vector4i addend)
{
	x += addend[0];
	y += addend[1];
	z += addend[2];
}


void Vector4i::operator -= (const Vector4i  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
}

/// Internal element division
void Vector4i::operator /= (const float &f){
	x /= f;
	y /= f;
	z /= f;
}
/// Internal element multiplication
void Vector4i::operator *= (const float &f){
	x *= f;
	y *= f;
	z *= f;
}

/*
/// Internal element multiplication
void Vector4i::operator *= (const Vector4i &f)
{
	x *= f[0];
	y *= f[1];
	z *= f[2];
}
	
/// Internal element multiplication
void Vector4i::operator *= (const Matrix4f &mat)
{
	Vector4f newVec = mat.Product(Vector4f(*this));
	x = newVec[0];
	y = newVec[1];
	z = newVec[2];
}

/// Internal element multiplication
Vector4i Vector4i::operator * (const float &f) const {
	return Vector4i(x * f, y * f, z * f);
}
/// Internal element division.
Vector4i Vector4i::operator / (const float &f) const {
	return Vector4i(x / f, y / f, z / f);
}
*/

int & Vector4i::operator [](int index)
{
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		default:
			throw 1003;
	}
}

/// Operator overloading for the array-access operator []
const int & Vector4i::operator [](int index) const
{
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		default:
			throw 1003;
	}
}

// ************************************************************************//
// Vector operations
// ************************************************************************//
/*
float Vector4i::ScalarProduct(Vector4i otherVector) const 
{
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2];
}
// Same thing as scalar product, silly!
float Vector4i::DotProduct(Vector4i otherVector) const {
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2];
}

Vector4i Vector4i::CrossProduct(Vector4i otherVector) const {
	return Vector4i (y * otherVector[2] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector4i Vector4i::ElementMultiplication(const Vector4i otherVector) const {
	return Vector4i(x * otherVector[0], y * otherVector[1], z * otherVector[2]);
}
Vector4i Vector4i::ElementDivision(const Vector4i otherVector) const 
{
	return Vector4i(x / otherVector[0], y / otherVector[1], z / otherVector[2]);
}
*/
/// Calculates the length of the vector.
float Vector4i::Length() const 
{
	float sum = x * x + y * y + z * z;
	if (sum == 0)
		return 0;
	assert(abs(sum) != 0 && "VEctor3f::Length");
	return sqrt(sum);
}
/*
/// Calculates the squared length of the vector.
float Vector4i::LengthSquared() const {
	return x * x + y * y + z * z;
}

Vector4i Vector4i::Normalize(){
	float vecLength = Length();
	if (vecLength < ZERO){
		// assert(vecLength != 0 && "VEctor3f::Normalize");
		vecLength = 1.0f;
	}

	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
	return Vector4i(x, y, z);
}
*/

/** Returns a normalized copy of this vector. */
Vector4i Vector4i::NormalizedCopy() const {
	float vecLength = Length();
	if (vecLength < ZERO){
		return Vector4i();
	}
	return Vector4i(x / vecLength, y / vecLength, z / vecLength, 1);
};



/// Utility functions
Vector4i Vector4i::Minimum(const Vector4i & vec1, const Vector4i & vec2){
	return Vector4i(
		vec1[0] < vec2[0] ? vec1[0] : vec2[0],
		vec1[1] < vec2[1] ? vec1[1] : vec2[1],
		vec1[2] < vec2[2] ? vec1[2] : vec2[2],
		1
	);
}
Vector4i Vector4i::Maximum(const Vector4i & vec1, const Vector4i & vec2){
	return Vector4i(
		vec1[0] > vec2[0] ? vec1[0] : vec2[0],
		vec1[1] > vec2[1] ? vec1[1] : vec2[1],
		vec1[2] > vec2[2] ? vec1[2] : vec2[2],
		1
	);
}
