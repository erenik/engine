/// Emil Hedemalm
/// 2013-07-14

#include "AEMath.h"
#include "Vector2f.h"
#include "Vector2i.h"
#include "Vector3i.h"
#include "Vector3f.h"
#include <cassert>
#include <fstream>

#include "String/AEString.h"

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
	x = base[0];
	y = base[1];
}


/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector2f::Vector2f(const Vector2i& base){
	x = base[0];
	y = base[1];
}
/** Constructor, based on Vector3i equivalent
*/
Vector2f::Vector2f(const Vector3i& base){
	x = base[0];
	y = base[1];
}

/** Constructor, based on Vector3f equivalent
*/
Vector2f::Vector2f(const Vector3f & base){
	x = base[0];
	y = base[1];
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector2f& vec){
	os << vec[0] << " " << vec[1];
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

/// Parses from string. Expects in the form of first declaring order "XY", "X Y" or "YX", then parses the space-separated values.
void Vector2f::ParseFrom(const String & str)
{
	String string = str;
	List<float*> order;
	for (int i = 0; i < string.Length(); ++i)
	{
		char c = string.c_str()[i];
		if (c == 'x' || c == 'X')
			order.Add(&x);
		else if (c == 'y' || c == 'Y')
			order.Add(&y);
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


/// Clamp to an interval.
void Vector2f::Clamp(float min, float max)
{
	ClampFloat(x, min, max);
	ClampFloat(y, min, max);
}

void Vector2f::Clamp(Vector2f min, Vector2f max)
{
	ClampFloat(x, min.x, max.x);
	ClampFloat(y, min.y, max.y);
}


// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector2f::add(Vector2f addend){
	x += addend[0];
	y += addend[1];
}


void Vector2f::subtract(Vector2f subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
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
/// Binary operator.
bool Vector2f::operator == (const Vector2f other) const
{
	if (x == other[0] && y == other[1])
		return true;
	return false;
}

/// This will return true if and only if all three components (x,y,z) are smaller than their corresponding comparands in the vector comparand.
bool Vector2f::operator < (const Vector2f & comparand) const
{
	if (x < comparand.x &&
		y < comparand.y)
		return true;
	return false;
}
/// This will return true if and only if all three components (x,y,z) are larger than their corresponding comparands in the vector comparand.
bool Vector2f::operator > (const Vector2f & comparand) const
{
	if (x > comparand.x &&
		y > comparand.y)
		return true;
	return false;
}


// Unary - operator (switch signs of all sub-elements)
Vector2f Vector2f::operator - () const {
	return Vector2f(-x, -y);
}

Vector2f  Vector2f::operator + (Vector2f addend) const {
	Vector2f  newVec;
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	return newVec;
}


Vector2f  Vector2f::operator - (Vector2f subtractor) const {
	Vector2f  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	return newVec;
}

/// Returns a subtracted vector based on this vector and the subtractor.
Vector2f Vector2f::operator * (const Vector2f elementMultiplier) const 
{
	Vector2f newVec;
	newVec[0] = x * elementMultiplier[0];
	newVec[1] = y * elementMultiplier[1];
	return newVec;
}

/// Multiplication with float
Vector2f operator * (float multiplier, Vector2f& vector){
	Vector2f  newVec;
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	return newVec;
}

/// Multiplication with floats
Vector2f operator * (float multiplier, const Vector2f & vec)
{
	Vector2f newVec;
	newVec.x = vec.x * multiplier;
	newVec.y = vec.y * multiplier;
	return newVec;
}



void Vector2f::operator += (Vector2f addend){
	x += addend[0];
	y += addend[1];
}


void Vector2f::operator -= (const Vector2f  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
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
/// Per-element multiplication
void Vector2f::operator *= (const Vector2f &vec)
{
	x *= vec[0];
	y *= vec[1];
}

/// Internal element multiplication
Vector2f Vector2f::operator * (const float &f) const {
	return Vector2f(x * f, y * f);
}
/// Internal element division.
Vector2f Vector2f::operator / (const float &f) const {
	return Vector2f(x / f, y / f);
}

/// Per element division.
Vector2f Vector2f::operator / (const Vector2f &v) const 
{
	return Vector2f(x / v[0], y / v[1]); 
}


float & Vector2f::operator [](int index){
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
const float & Vector2f::operator [](int index) const
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
Vector2f Vector2f::ElementMultiplication(const Vector2f otherVector) const {
	return Vector2f(x * otherVector[0], y * otherVector[1]);
}
/// Make sure all elements are non-0 before calling this...
Vector2f Vector2f::ElementDivision(const Vector2f dividend) const 
{
	return Vector2f(x / dividend[0], y / dividend[1]);
}

// Dot product.
float Vector2f::DotProduct(const Vector2f otherVector) const 
{
	return x * otherVector[0] + y * otherVector[1];
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
		vec1[0] < vec2[0] ? vec1[0] : vec2[0],
		vec1[1] < vec2[1] ? vec1[1] : vec2[1]
	);
}
Vector2f Vector2f::Maximum(const Vector2f & vec1, const Vector2f & vec2){
	return Vector2f(
		vec1[0] > vec2[0] ? vec1[0] : vec2[0],
		vec1[1] > vec2[1] ? vec1[1] : vec2[1]
	);
}

/// Comparison.
bool Vector2f::IsWithinMinMax(Vector2f min, Vector2f max)
{
	if (x < max[0] &&
		x > min[0] &&
		y < max[1] &&
		y > min[1])
		return true;
	return false;
}
