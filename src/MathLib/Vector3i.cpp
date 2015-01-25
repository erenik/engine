/// Emil Hedemalm
/// 2013-07-14

#include "AEMath.h"
#include "Vector2i.h"
#include "Vector3i.h"
#include "Vector4i.h"
#include "Vector3f.h"
#include <cassert>
#include <fstream>

#ifndef abs
#define abs(b) ((b < 0)? (-b) : (b))
#endif

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector3i::Vector3i(){
	x = y = z = 0;
}


Vector3i::Vector3i( int ix,  int iy, int iz){
	x = ix;
	y = iy;
	z = iz;
}

Vector3i::Vector3i(int arr[]){
	x = arr[0];
	y = arr[1];
	z = arr[2];
}

Vector3i::Vector3i(const Vector3i & base){
	x = base.x;
	y = base.y;
	z = base.z;
}

/** Constructor, based on Vector2i equivalent
*/
Vector3i::Vector3i(const Vector2i & base){
	x = base.x;
	y = base.y;
	z = 0;
}
Vector3i::Vector3i(const Vector4i& base)
{
	x = base.x;
	y = base.y;
	z = base.z;
}

/** Constructor, based on Vector3f equivalent
*/
Vector3i::Vector3i(const Vector3f & base){
	x = RoundFloat((base.x));
	y = RoundFloat((base.y));
	z = RoundFloat(base.z);
}

Vector3i::Vector3i(const Vector4f& base)
{
	x = RoundFloat(base.x);
	y = RoundFloat(base.y);
	z = RoundFloat(base.z);
}


/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector3i& vec){
	os << vec.x << " " << vec.y << " " << vec.z;
	return os;
}

/// Writes to file stream.
void Vector3i::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(int));
	file.write((char*)&y, sizeof(int));
	file.write((char*)&z, sizeof(int));
}
/// Reads from file stream.
void Vector3i::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(int));
	file.read((char*)&y, sizeof(int));
	file.read((char*)&z, sizeof(int));
}

///
int Vector3i::DotProduct(Vector3i otherVec)
{
	return x * otherVec.x + y * otherVec.y + z * otherVec.z;
}


// ************************************************************************//
// Arithmetics
// ************************************************************************//
void Vector3i::add(Vector3i addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}
void Vector3i::subtract(Vector3i subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
}
void Vector3i::scale(int ratio){
	x *= ratio;
	y *= ratio;
	z *= ratio;
}
void Vector3i::scale(int ix, int iy, int iz){
	x *= ix;
	y *= iy;
	z *= iy;
}
// ************************************************************************//
// Operator overloading
// ************************************************************************//
// Unary - operator (switch signs of all sub-elements)
Vector3i Vector3i::operator - () const {
	return Vector3i(-x, -y, -z);
}
Vector3i  Vector3i::operator + (Vector3i addend) const {
	Vector3i  newVec;
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	return newVec;
}


Vector3i  Vector3i::operator - (Vector3i subtractor) const {
	Vector3i  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	return newVec;
}

/// Multiplication with int
Vector3i operator * (int multiplier, Vector3i& vector){
	Vector3i  newVec;
	newVec.x = vector.x * multiplier;
	newVec.y = vector.y * multiplier;
	newVec.z = vector.z * multiplier;
	return newVec;
}
void Vector3i::operator += (Vector3i addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}
void Vector3i::operator -= (const Vector3i  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
}
/// Internal element division
void Vector3i::operator /= (const float &f){
	x /= f;
	y /= f;
	z /= f;
}
/// Internal element multiplication
void Vector3i::operator *= (const float &f){
	x *= f;
	y *= f;
	z *= f;
}
/// Internal element multiplication
Vector3f Vector3i::operator * (const float &f) const {
	return Vector3f(x * f, y * f, z * f);
}
/// Internal element division.
Vector3f Vector3i::operator / (const float &f) const {
	return Vector3f(x / f, y / f, z / f);
}

int Vector3i::operator [](int index){
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		default:
			throw 1003;
	}
}


/// Comparison operators
bool Vector3i::operator == (const Vector3i other){
	if (x != other.x ||
		y != other.y ||
		z != other.z)
		return false;
	return true;
}

/// Comparison operators
bool Vector3i::operator != (const Vector3i other)
{
	if (x != other.x ||
		y != other.y ||
		z != other.z)
		return true;
	return false;
}


// ************************************************************************//
// Vector operations
// ************************************************************************//
/// Multiplies the elements in the two vectors internally, returning the product.
Vector3i Vector3i::ElementMultiplication(const Vector3i otherVector) const {
	return Vector3i(x * otherVector.x, y * otherVector.y, z * otherVector.z);
}

/// Calculates the length of the vector.
float Vector3i::Length() const {
	int sum = x * x + y * y + z * z;
	if (sum == 0)
		return 0;
	assert(abs(sum) != 0 && "Vector3i::Length");
	return sqrt((float)sum);
}
/// Calculates the squared length of the vector.
int Vector3i::LengthSquared() const {
	return x * x + y * y + z * z;
}

Vector3i Vector3i::Normalize(){
	int vecLength = Length();
	if (vecLength < ZERO){
		// assert(vecLength != 0 && "Vector3i::Normalize");
		vecLength = 1.0f;
	}
	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
	return Vector3i(x, y, z);
}

/** Returns a normalized copy of this vector. */
Vector3i Vector3i::NormalizedCopy() const {
	int vecLength = Length();
	if (vecLength < ZERO){
		return Vector3i();
	}
	return Vector3i(x / vecLength, y / vecLength, z / vecLength);
};



/// Utility functions
Vector3i Vector3i::Minimum(const Vector3i & vec1, const Vector3i & vec2){
	return Vector3i(
		vec1.x < vec2.x ? vec1.x : vec2.x,
		vec1.y < vec2.y ? vec1.y : vec2.y,
		vec1.z < vec2.z ? vec1.z : vec2.z
	);
}
Vector3i Vector3i::Maximum(const Vector3i & vec1, const Vector3i & vec2){
	return Vector3i(
		vec1.x > vec2.x ? vec1.x : vec2.x,
		vec1.y > vec2.y ? vec1.y : vec2.y,
		vec1.z > vec2.z ? vec1.z : vec2.z
	);
}