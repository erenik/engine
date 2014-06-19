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

#ifndef abs
#define abs(b) ((b < 0)? (-b) : (b))
#endif

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector3f::Vector3f(){
	x = y = z = 0;
}


Vector3f::Vector3f( float ix,  float iy,  float iz){
	x = ix;
	y = iy;
	z = iz;
}

Vector3f::Vector3f(Vertex3f v1, Vertex3f v2){
	x = v2.x - v1.x;
	y = v2.y - v1.y;
	z = v2.z - v1.z;
}

Vector3f::Vector3f(float arr[]){
    assert(arr);
	x = arr[0];
	y = arr[1];
	z = arr[2];
}



/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector2i& base, float z /* = 0*/)
{
	x = base.x;
	y = base.y;
	this->z = z;
}

Vector3f::Vector3f(const Vector2f & base, float z /* = 0*/)
{
	x = base.x;
	y = base.y;
	this->z = z;
}

Vector3f::Vector3f(const Vector3f & base){
	x = base.x;
	y = base.y;
	z = base.z;
}
/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector3d& base){
	x = (float)base.x;
	y = (float)base.y;
	z = (float)base.z;
}
/**	Copy Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3f::Vector3f(const Vector3i& base){
	x = (float)base.x;
	y = (float)base.y;
	z = (float)base.z;	
}

// Constructors from other Vector classes
Vector3f::Vector3f(const Vector4f& base){
	x = base.GetX();
	y = base.GetY();
	z = base.GetZ();
}
/**	Conversion Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector. The w-value is discarded.
*/
Vector3f::Vector3f(const Vector4d& base){
	x = (float)base.x;
	y = (float)base.y;
	z = (float)base.z;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector3f& vec){
	os << vec.x << " " << vec.y << " " << vec.z;
	return os;
}

/// Writes to file stream.
void Vector3f::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(float));
	file.write((char*)&y, sizeof(float));
	file.write((char*)&z, sizeof(float));
}
/// Reads from file stream.
void Vector3f::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(float));
	file.read((char*)&y, sizeof(float));
	file.read((char*)&z, sizeof(float));
}


// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector3f::add(Vector3f addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector3f::subtract(Vector3f subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
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

/// Comparison operators
bool Vector3f::operator != (const Vector3f comparand) const {
	if (comparand.x != x ||
		comparand.y != y ||
		comparand.z != z)
		return true;
	return false;
}


// Unary - operator (switch signs of all sub-elements)
Vector3f Vector3f::operator - () const {
	return Vector3f(-x, -y, -z);
}


Vector3f  Vector3f::operator + (Vector3f addend) const {
	Vector3f  newVec;
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	return newVec;
}


Vector3f  Vector3f::operator - (Vector3f subtractor) const {
	Vector3f  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	return newVec;
}

/// Multiplication with float
Vector3f operator * (float multiplier, const Vector3f vector){
	Vector3f  newVec;
	newVec.x = vector.x * multiplier;
	newVec.y = vector.y * multiplier;
	newVec.z = vector.z * multiplier;
	return newVec;
}

/// Adds addend to this vector.
void Vector3f::operator += (Vector3f addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector3f::operator -= (const Vector3f  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
}
/// Internal element division
void Vector3f::operator /= (const float &f){
	x /= f;
	y /= f;
	z /= f;
}
/// Internal element multiplication
void Vector3f::operator *= (const float &f){
	x *= f;
	y *= f;
	z *= f;
}
/// Internal element multiplication
void Vector3f::operator *= (const Matrix4f &mat){
	Vector4f newVec = mat.product(Vector4f(*this));
	x = newVec.x;
	y = newVec.y;
	z = newVec.z;
}

/// Internal element multiplication
Vector3f Vector3f::operator * (const float &f) const {
	return Vector3f(x * f, y * f, z * f);
}
/// Internal element division.
Vector3f Vector3f::operator / (const float &f) const {
	return Vector3f(x / f, y / f, z / f);
}

float Vector3f::operator [](int index){
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


// ************************************************************************//
// Vector operations
// ************************************************************************//
float Vector3f::ScalarProduct(Vector3f otherVector) const {
	return x * otherVector.x + y * otherVector.y + z * otherVector.z;
}
// Same thing as scalar product, silly!
float Vector3f::DotProduct(Vector3f otherVector) const {
	return x * otherVector.x + y * otherVector.y + z * otherVector.z;
}

Vector3f Vector3f::CrossProduct(Vector3f otherVector) const {
	return Vector3f (y * otherVector.z - z * otherVector.y, z * otherVector.x - x * otherVector.z, x * otherVector.y - y * otherVector.x);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector3f Vector3f::ElementMultiplication(const Vector3f otherVector) const {
	return Vector3f(x * otherVector.x, y * otherVector.y, z * otherVector.z);
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



/// Utility functions
Vector3f Vector3f::Minimum(const Vector3f & vec1, const Vector3f & vec2){
	return Vector3f(
		vec1.x < vec2.x ? vec1.x : vec2.x,
		vec1.y < vec2.y ? vec1.y : vec2.y,
		vec1.z < vec2.z ? vec1.z : vec2.z
	);
}
Vector3f Vector3f::Maximum(const Vector3f & vec1, const Vector3f & vec2){
	return Vector3f(
		vec1.x > vec2.x ? vec1.x : vec2.x,
		vec1.y > vec2.y ? vec1.y : vec2.y,
		vec1.z > vec2.z ? vec1.z : vec2.z
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