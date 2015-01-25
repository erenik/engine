/// Emil Hedemalm
/// 2013-07-14

#include "AEMath.h"
#include "Vector2i.h"
#include "Vector2f.h"
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
Vector2i::Vector2i(){
	x = y = 0;
}


Vector2i::Vector2i( int ix,  int iy)
{
	x = ix;
	y = iy;
}

Vector2i::Vector2i(int arr[])
{
	x = arr[0];
	y = arr[1];
}

Vector2i::Vector2i(const Vector2i & base)
{
	x = base[0];
	y = base[1];
}

/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector2i::Vector2i(const Vector2f& base)
{
	x = (int) RoundFloat(base[0]);
	y = (int) RoundFloat(base[1]);
}

/** Constructor, based on Vector3i equivalent
*/
Vector2i::Vector2i(const Vector3i& base){
	x = base[0];
	y = base[1];
}

/** Constructor, based on Vector3f equivalent
*/
Vector2i::Vector2i(const Vector3f & base){
	x = (int) RoundFloat((base[0]));
	y = (int) RoundFloat((base[1]));
}

/// Similar to clamp, ensures that this vector's values are within the given range (including the limits)
void Vector2i::Limit(Vector2i min, Vector2i max)
{
	if (x < min[0])
		x = min[0];
	else if (x > max[0])
		x = max[0];
	if (y < min[1])
		y = min[1];
	else if (y > max[1])
		y = max[1];
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector2i& vec){
	os << vec[0] << " " << vec[1];
	return os;
}

/// Writes to file stream.
void Vector2i::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(int));
	file.write((char*)&y, sizeof(int));
}
/// Reads from file stream.
void Vector2i::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(int));
	file.read((char*)&y, sizeof(int));
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector2i::add(Vector2i addend){
	x += addend[0];
	y += addend[1];
}


void Vector2i::subtract(Vector2i subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
}

void Vector2i::scale(int ratio){
	x *= ratio;
	y *= ratio;
}

void Vector2i::scale(int ix, int iy){
	x *= ix;
	y *= iy;
}


// ************************************************************************//
// Operator overloading
// ************************************************************************//

// Unary - operator (switch signs of all sub-elements)
Vector2i Vector2i::operator - () const {
	return Vector2i(-x, -y);
}


/// Binary operator.
bool Vector2i::operator == (const Vector2i other) const
{
	if (x == other[0] && y == other[1])
		return true;
	return false;
}

/// Binary operator.
bool Vector2i::operator != (const Vector2i other) const 
{
	if (x != other[0] || y != other[1])
		return true;
	return false;
}


Vector2i  Vector2i::operator + (Vector2i addend) const {
	Vector2i  newVec;
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	return newVec;
}


Vector2i  Vector2i::operator - (Vector2i subtractor) const {
	Vector2i  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	return newVec;
}

/// Returns a subtracted vector based on this vector and the subtractor.
Vector2i  Vector2i::operator * (const Vector2i elementMultiplier) const 
{
	Vector2i newVec;
	newVec[0] = x * elementMultiplier[0];
	newVec[1] = y * elementMultiplier[1];
	return newVec;
};


/// Multiplication with int
Vector2i operator * (int multiplier, Vector2i& vector){
	Vector2i  newVec;
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	return newVec;
}


void Vector2i::operator += (Vector2i addend){
	x += addend[0];
	y += addend[1];
}


void Vector2i::operator -= (const Vector2i  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
}
/// Internal element division
void Vector2i::operator /= (const int &f){
	x /= f;
	y /= f;
}
/// Internal element multiplication
void Vector2i::operator *= (const int &f){
	x *= f;
	y *= f;
}

/// Internal element multiplication
Vector2i Vector2i::operator * (const float &f) const {
	return Vector2i((int) (x * f), (int) (y * f));
}
/// Internal element multiplication
Vector2i Vector2i::operator * (const int &f) const {
	return Vector2i(x * f, y * f);
}
/// Internal element division.
Vector2i Vector2i::operator / (const int &f) const {
	return Vector2i(x / f, y / f);
}

int & Vector2i::operator [](int index)
{
	switch(index){
		case 0:
			return x;
		case 1:
			return y;
		default:
			throw 1003;
	}
}
/// Operator overloading for the array-access operator []
const int & Vector2i::operator [](int index) const
{
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
Vector2i Vector2i::ElementMultiplication(const Vector2i otherVector) const {
	return Vector2i(x * otherVector[0], y * otherVector[1]);
}

/// Calculates the length of the vector.
float Vector2i::Length() const {
	int sum = x * x + y * y;
	if (sum == 0)
		return 0;
	assert(abs(sum) != 0 && "Vector2i::Length");
	return sqrt((float)sum);
}
/// Calculates the squared length of the vector.
int Vector2i::LengthSquared() const {
	return x * x + y * y;
}

/// Corresponds to the area.
int Vector2i::GeometricSum()
{
	return x * y;
}

Vector2i Vector2i::Normalize(){
	int vecLength = (int) Length();
	if (vecLength < ZERO){
		// assert(vecLength != 0 && "Vector2i::Normalize");
		vecLength = 1;
	}

	x = x / vecLength;
	y = y / vecLength;
	return Vector2i(x, y);
}

/** Returns a normalized copy of this vector. */
Vector2i Vector2i::NormalizedCopy() const 
{
	int vecLength = (int) Length();
	if (vecLength < ZERO){
		return Vector2i();
	}
	return Vector2i(x / vecLength, y / vecLength);
};

/// Absolute values version.
Vector2i Vector2i::AbsoluteValues()
{
	return Vector2i(x < 0? -x : x, y < 0? -y : y);
}

/// Utility functions
Vector2i Vector2i::Minimum(const Vector2i & vec1, const Vector2i & vec2){
	return Vector2i(
		vec1[0] < vec2[0] ? vec1[0] : vec2[0],
		vec1[1] < vec2[1] ? vec1[1] : vec2[1]
	);
}
Vector2i Vector2i::Maximum(const Vector2i & vec1, const Vector2i & vec2){
	return Vector2i(
		vec1[0] > vec2[0] ? vec1[0] : vec2[0],
		vec1[1] > vec2[1] ? vec1[1] : vec2[1]
	);
}