/// Emil Hedemalm
/// 2013-07-14

#include "AEMath.h"
#include "Vector2f.h"
#include "Vector2i.h"
#include "Vector3i.h"
#include "Vector3f.h"
#include <cassert>
#include <fstream>

#ifndef abs
#define abs(b) ((b < 0)? (-b) : (b))
#endif

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector2f::Vector2f(){
	x = y = 0;
}


Vector2f::Vector2f( float ix,  float iy){
	x = ix;
	y = iy;
}

Vector2f::Vector2f(float arr[]){
	x = arr[0];
	y = arr[1];
}


Vector2f::Vector2f(const Vector2f & base){
	x = base.x;
	y = base.y;
}


/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector2f::Vector2f(const Vector2i& base){
	x = base.x;
	y = base.y;
}
/** Constructor, based on Vector3i equivalent
*/
Vector2f::Vector2f(const Vector3i& base){
	x = base.x;
	y = base.y;
}

/** Constructor, based on Vector3f equivalent
*/
Vector2f::Vector2f(const Vector3f & base){
	x = base.x;
	y = base.y;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector2f& vec){
	os << vec.x << " " << vec.y;
	return os;
}

/// Writes to file stream.
void Vector2f::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(float));
	file.write((char*)&y, sizeof(float));
}
/// Reads from file stream.
void Vector2f::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(float));
	file.read((char*)&y, sizeof(float));
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector2f::add(Vector2f addend){
	x += addend.x;
	y += addend.y;
}


void Vector2f::subtract(Vector2f subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
}

void Vector2f::scale(float ratio){
	x *= ratio;
	y *= ratio;
}

void Vector2f::scale(float ix, float iy){
	x *= ix;
	y *= iy;
}


// ************************************************************************//
// Operator overloading
// ************************************************************************//

// Unary - operator (switch signs of all sub-elements)
Vector2f Vector2f::operator - () const {
	return Vector2f(-x, -y);
}


/// Binary operator.
bool Vector2f::operator == (const Vector2f other) const
{
	if (x == other.x && y == other.y)
		return true;
	return false;
}


Vector2f  Vector2f::operator + (Vector2f addend) const {
	Vector2f  newVec;
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	return newVec;
}


Vector2f  Vector2f::operator - (Vector2f subtractor) const {
	Vector2f  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	return newVec;
}

/// Multiplication with float
Vector2f operator * (float multiplier, Vector2f& vector){
	Vector2f  newVec;
	newVec.x = vector.x * multiplier;
	newVec.y = vector.y * multiplier;
	return newVec;
}


void Vector2f::operator += (Vector2f addend){
	x += addend.x;
	y += addend.y;
}


void Vector2f::operator -= (const Vector2f  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
}
/// Internal element division
void Vector2f::operator /= (const float &f){
	x /= f;
	y /= f;
}
/// Internal element multiplication
void Vector2f::operator *= (const float &f){
	x *= f;
	y *= f;
}
/// Internal element multiplication
Vector2f Vector2f::operator * (const float &f) const {
	return Vector2f(x * f, y * f);
}
/// Internal element division.
Vector2f Vector2f::operator / (const float &f) const {
	return Vector2f(x / f, y / f);
}

float Vector2f::operator [](int index){
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		default:
			throw 1003;
	}
}


// ************************************************************************//
// Vector operations
// ************************************************************************//
/// Multiplies the elements in the two vectors internally, returning the product.
Vector2f Vector2f::ElementMultiplication(const Vector2f otherVector) const {
	return Vector2f(x * otherVector.x, y * otherVector.y);
}

/// Calculates the length of the vector.
float Vector2f::Length() const {
	float sum = x * x + y * y;
	if (sum == 0)
		return 0;
	assert(abs(sum) != 0 && "Vector2f::Length");
	return sqrt((float)sum);
}
/// Calculates the squared length of the vector.
float Vector2f::LengthSquared() const {
	return x * x + y * y;
}

Vector2f Vector2f::Normalize(){
	float vecLength = Length();
	if (vecLength < ZERO){
		// assert(vecLength != 0 && "Vector2f::Normalize");
		vecLength = 1.0f;
	}

	x = x / vecLength;
	y = y / vecLength;
	return Vector2f(x, y);
}

/** Returns a normalized copy of this vector. */
Vector2f Vector2f::NormalizedCopy() const {
	float vecLength = Length();
	if (vecLength < ZERO){
		return Vector2f();
	}
	return Vector2f(x / vecLength, y / vecLength);
};



/// Utility functions
Vector2f Vector2f::Minimum(const Vector2f & vec1, const Vector2f & vec2){
	return Vector2f(
		vec1.x < vec2.x ? vec1.x : vec2.x,
		vec1.y < vec2.y ? vec1.y : vec2.y
	);
}
Vector2f Vector2f::Maximum(const Vector2f & vec1, const Vector2f & vec2){
	return Vector2f(
		vec1.x > vec2.x ? vec1.x : vec2.x,
		vec1.y > vec2.y ? vec1.y : vec2.y
	);
}