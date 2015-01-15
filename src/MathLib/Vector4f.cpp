// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector4f.h
// CLASS PROVIDED: Vector4f  (a four-dimensional vector class using floats)

#include "Vector4f.h"
#include <fstream>

#include "String/AEString.h"

Vector4f::Vector4f(){
	x = y = z = 0;
	w = 1;
}

Vector4f::Vector4f( float ix,  float iy,  float iz,  float iw){
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}

Vector4f::Vector4f(float arr[]){
	x = arr[0];
	y = arr[1];
	z = arr[2];
	w = arr[3];
}

Vector4f::Vector4f(const Vector4f & base){
	x = base.x;
	y = base.y;
	z = base.z;
	w = base.w;
}

/**	Copy Conversion Constructor
	Postcondition: Initializes a 4D vector to have same values as the referenced vector.
*/
Vector4f::Vector4f(const Vector4d & base){
	x = (float)base.x;
	y = (float)base.y;
	z = (float)base.z;
	w = (float)base.w;
}

// Constructors from other Vector classes
Vector4f::Vector4f(const Vector3f  & base, float iw){
	x = base.GetX();
	y = base.GetY();
	z = base.GetZ();
	w = iw;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector4f& vec){
	os << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
	return os;
}


/// Writes to file stream.
void Vector4f::WriteTo(std::fstream & file){
	file.write((char*)&x, sizeof(float));
	file.write((char*)&y, sizeof(float));
	file.write((char*)&z, sizeof(float));
	file.write((char*)&w, sizeof(float));
}
/// Reads from file stream.
void Vector4f::ReadFrom(std::fstream & file){
	file.read((char*)&x, sizeof(float));
	file.read((char*)&y, sizeof(float));
	file.read((char*)&z, sizeof(float));
	file.read((char*)&w, sizeof(float));
}

/// Reads from String. Expects space-separated values. E.g. 3 8.14 -15 0.0
void Vector4f::ReadFrom(const String & string)
{
	List<String> tokens = string.Tokenize(" ");
	x = tokens[0].ParseFloat();
	y = tokens[1].ParseFloat();
	z = tokens[2].ParseFloat();
	w = tokens[3].ParseFloat();
}


/// Clamp to an interval.
void Vector4f::Clamp(float min, float max)
{
	ClampFloat(x, min, max);
	ClampFloat(y, min, max);
	ClampFloat(z, min, max);
	ClampFloat(w, min, max);
}


// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector4f::add(Vector4f addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
	w += addend.w;
}


void Vector4f::subtract(Vector4f subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
	w -= subtractor.w;
}

void Vector4f::scale(float ratio){
	x *= ratio;
	y *= ratio;
	z *= ratio;
	w *= ratio;
}

void Vector4f::scale(float ix, float iy, float iz, float iw){
	x *= ix;
	y *= iy;
	z *= iz;
	w *= iw;
}

/** Scales the XYZ parts. */
void Vector4f::MultiplyXYZ(float multiplier){
	x *= multiplier;
	y *= multiplier;
	z *= multiplier;
}

// ************************************************************************//
// Operator overloading
// ************************************************************************//

// Unary - operator (switch signs of all sub-elements)
Vector4f Vector4f::operator - () const {
	return Vector4f(-x, -y, -z, w);
}



Vector4f  Vector4f::operator + (Vector4f addend) const {
	Vector4f  newVec;
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	newVec.w = w + addend.w;
	return newVec;
}


Vector4f  Vector4f::operator - (Vector4f subtractor) const {
	Vector4f  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	newVec.w = w - subtractor.w;
	return newVec;
}

/// Multiplication with float
Vector4f operator * (float multiplier, const Vector4f& vector){
	Vector4f  newVec;
	newVec.x = vector.x * multiplier;
	newVec.y = vector.y * multiplier;
	newVec.z = vector.z * multiplier;
	return newVec;
}


void Vector4f::operator += (Vector4f addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
	w += addend.w;
}


void Vector4f::operator -= (const Vector4f  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
	w -= subtractor.w;
}

/// Multiplicator o-o
void Vector4f::operator *= (const float floatur){
	x *= floatur;
	y *= floatur;
	z *= floatur;
	w *= floatur;
}

/// Internal element multiplication
Vector3f Vector4f::operator * (const float &f) const {
	return Vector3f(x * f, y * f, z * f);
}
/// Internal element division.
Vector3f Vector4f::operator / (const float &f) const {
	return Vector3f(x / f, y / f, z / f);
}

/// Conversion equal-conversion operator
Vector4f& Vector4f::operator = (const Vector4d &other){
	this->x = (float)other.x;
	this->y = (float)other.y;
	this->z = (float)other.z;
	this->w = (float)other.w;
	return *this;
}

/// Conversion equal-conversion operator
Vector4f& Vector4f::operator = (const Vector4f &other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}



float& Vector4f::operator[](const unsigned int index){
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

 float Vector4f::ScalarProduct(Vector4f otherVector){
	return x * otherVector.x + y * otherVector.y + z * otherVector.z + w * otherVector.w;
}
// Same thing as scalar product, silly!
 float Vector4f::DotProduct(Vector4f otherVector){
	return x * otherVector.x + y * otherVector.y + z * otherVector.z + w * otherVector.w;
}

 Vector3f Vector4f::CrossProduct(Vector3f otherVector){
	 return Vector3f (y * otherVector.GetX() - z * otherVector.GetY(), z * otherVector.GetX() - x * otherVector.GetZ(), x * otherVector.GetY() - y * otherVector.GetX());
 }

Vector3f Vector4f::CrossProduct(Vector4f otherVector){
	 return Vector3f (y * otherVector.z - z * otherVector.y, z * otherVector.x - x * otherVector.z, x * otherVector.y - y * otherVector.x);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector4f Vector4f::ElementMultiplication(const Vector4f otherVector) const {
	return Vector4f(x * otherVector.x, y * otherVector.y, z * otherVector.z, w * otherVector.w);
}

 float Vector4f::Length3() const{
	return sqrt(pow((float)x, 2) + pow((float)y, 2)+ pow((float)z, 2));
}

/** Calculates the length of the vector, considering only {x y z}. */
 float Vector4f::Length3Squared() const 
 {
	return x * x + y * y + z * z;
 }


void Vector4f::Normalize3(){
	float vecLength = Length3();
	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
}

/// Returns a normalized (x,y,z) copy of the given vector.
Vector4f Vector4f::NormalizedCopy() const{
    float invVecLength = 1.0f / Length3();
    Vector4f newVec(x * invVecLength, y * invVecLength, z * invVecLength, 1);
    return newVec;
}
