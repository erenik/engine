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
	x = base.x;
	y = base.y;
	this->z = z;
	this->w = w;
}

Vector4i::Vector4i(const Vector3f & base, int w)
{
	x = (int)RoundFloat(base.x);
	y = (int)RoundFloat(base.y);
	z = (int)RoundFloat(base.z);
	this->w = w;
}

Vector4i::Vector4i(const Vector4i & base)
{
	x = base.x;
	y = base.y;
	z = base.z;
	w = base.w;
}

// Constructors from other Vector classes
Vector4i::Vector4i(const Vector4f& base)
{
	x = (int)RoundFloat(base.x);
	y = (int)RoundFloat(base.y);
	z = (int)RoundFloat(base.z);
	w = (int)RoundFloat(base.w);
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
	os << vec.x << " " << vec.y << " " << vec.z;
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
	ClampFloat(x, min.x, max.x);
	ClampFloat(y, min.y, max.y);
	ClampFloat(z, min.z, max.z);
}*/

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector4i::Add(Vector4i addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector4i::Subtract(Vector4i subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
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
	if (comparand.x == x &&
		comparand.y == y &&
		comparand.z == z &&
		comparand.w == w)
		return true;
	return false;
}

bool Vector4i::operator != (const Vector4i comparand) const 
{
	if (comparand.x != x ||
		comparand.y != y ||
		comparand.z != z ||
		comparand.w != w)
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
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	newVec.w = w + addend.w;
	return newVec;
}


Vector4i  Vector4i::operator - (Vector4i subtractor) const 
{
	Vector4i  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	newVec.w = w - subtractor.w;
	return newVec;
}

/// Multiplication with float
Vector4i operator * (float multiplier, const Vector4i vector)
{
	Vector4i  newVec;
	newVec.x = vector.x * multiplier;
	newVec.y = vector.y * multiplier;
	newVec.z = vector.z * multiplier;
	newVec.w = vector.w * multiplier;
	return newVec;
}

/// Adds addend to this vector.
void Vector4i::operator += (Vector4i addend)
{
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector4i::operator -= (const Vector4i  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
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
	x *= f.x;
	y *= f.y;
	z *= f.z;
}
	
/// Internal element multiplication
void Vector4i::operator *= (const Matrix4f &mat)
{
	Vector4f newVec = mat.Product(Vector4f(*this));
	x = newVec.x;
	y = newVec.y;
	z = newVec.z;
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

int Vector4i::operator [](int index)
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
	return x * otherVector.x + y * otherVector.y + z * otherVector.z;
}
// Same thing as scalar product, silly!
float Vector4i::DotProduct(Vector4i otherVector) const {
	return x * otherVector.x + y * otherVector.y + z * otherVector.z;
}

Vector4i Vector4i::CrossProduct(Vector4i otherVector) const {
	return Vector4i (y * otherVector.z - z * otherVector.y, z * otherVector.x - x * otherVector.z, x * otherVector.y - y * otherVector.x);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector4i Vector4i::ElementMultiplication(const Vector4i otherVector) const {
	return Vector4i(x * otherVector.x, y * otherVector.y, z * otherVector.z);
}
Vector4i Vector4i::ElementDivision(const Vector4i otherVector) const 
{
	return Vector4i(x / otherVector.x, y / otherVector.y, z / otherVector.z);
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
		vec1.x < vec2.x ? vec1.x : vec2.x,
		vec1.y < vec2.y ? vec1.y : vec2.y,
		vec1.z < vec2.z ? vec1.z : vec2.z,
		1
	);
}
Vector4i Vector4i::Maximum(const Vector4i & vec1, const Vector4i & vec2){
	return Vector4i(
		vec1.x > vec2.x ? vec1.x : vec2.x,
		vec1.y > vec2.y ? vec1.y : vec2.y,
		vec1.z > vec2.z ? vec1.z : vec2.z,
		1
	);
}
