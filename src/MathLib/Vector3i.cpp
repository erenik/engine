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

Vector3i::Vector3i(const Vector3i & base)
{
	x = base[0];
	y = base[1];
	z = base[2];
}

/** Constructor, based on Vector2i equivalent
*/
Vector3i::Vector3i(const Vector2i & base){
	x = base[0];
	y = base[1];
	z = 0;
}
Vector3i::Vector3i(const Vector4i& base)
{
	x = base[0];
	y = base[1];
	z = base[2];
}

/** Constructor, based on Vector3f equivalent
*/
Vector3i::Vector3i(const Vector3f & base){
	x = RoundFloat((base[0]));
	y = RoundFloat((base[1]));
	z = RoundFloat(base[2]);
}

Vector3i::Vector3i(const Vector4f& base)
{
	x = RoundFloat(base[0]);
	y = RoundFloat(base[1]);
	z = RoundFloat(base[2]);
}


/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector3i& vec){
	os << vec[0] << " " << vec[1] << " " << vec[2];
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
	return x * otherVec[0] + y * otherVec[1] + z * otherVec[2];
}


// ************************************************************************//
// Arithmetics
// ************************************************************************//
void Vector3i::add(Vector3i addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}
void Vector3i::subtract(Vector3i subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
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
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
	return newVec;
}


Vector3i  Vector3i::operator - (Vector3i subtractor) const {
	Vector3i  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	return newVec;
}

/// Multiplication with int
Vector3i operator * (int multiplier, Vector3i& vector){
	Vector3i  newVec;
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	newVec[2] = vector[2] * multiplier;
	return newVec;
}
void Vector3i::operator += (Vector3i addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}
void Vector3i::operator -= (const Vector3i  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
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

int & Vector3i::operator [](int index){
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
}

/// Operator overloading for the array-access operator []
const int Vector3i::operator [](int index) const
{
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
}



/// Comparison operators
bool Vector3i::operator == (const Vector3i other){
	if (x != other[0] ||
		y != other[1] ||
		z != other[2])
		return false;
	return true;
}

/// Comparison operators
bool Vector3i::operator != (const Vector3i other)
{
	if (x != other[0] ||
		y != other[1] ||
		z != other[2])
		return true;
	return false;
}


// ************************************************************************//
// Vector operations
// ************************************************************************//
/// Multiplies the elements in the two vectors internally, returning the product.
Vector3i Vector3i::ElementMultiplication(const Vector3i otherVector) const {
	return Vector3i(x * otherVector[0], y * otherVector[1], z * otherVector[2]);
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
		vec1[0] < vec2[0] ? vec1[0] : vec2[0],
		vec1[1] < vec2[1] ? vec1[1] : vec2[1],
		vec1[2] < vec2[2] ? vec1[2] : vec2[2]
	);
}
Vector3i Vector3i::Maximum(const Vector3i & vec1, const Vector3i & vec2){
	return Vector3i(
		vec1[0] > vec2[0] ? vec1[0] : vec2[0],
		vec1[1] > vec2[1] ? vec1[1] : vec2[1],
		vec1[2] > vec2[2] ? vec1[2] : vec2[2]
	);
}