#include "Vector3d.h"

// ************************************************************************//
// Constructors
// ************************************************************************//
Vector3d::Vector3d(){
	x = y = z = 0;
}


Vector3d::Vector3d( double ix,  double iy,  double iz){
	x = ix;
	y = iy;
	z = iz;
}

/*
Vector3d::Vector3d(Vertex3d v1, Vertex3d v2)
{
	x = v2[0] - v1[0];
	y = v2[1] - v1[1];
	z = v2[2] - v1[2];
}
*/
Vector3d::Vector3d(double arr[]){
	x = arr[0];
	y = arr[1];
	z = arr[2];
}


Vector3d::Vector3d(const Vector3d & base){
	x = base[0];
	y = base[1];
	z = base[2];
}

/**	Copy Conversion Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3d::Vector3d(const Vector3f& base){
	x = base[0];
	y = base[1];
	z = base[2];
}

// Constructors from other Vector classes
Vector3d::Vector3d(const Vector4d& base){
	x = base[0];
	y = base[1];
	z = base[2];
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector3d::add(Vector3d addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}


void Vector3d::subtract(Vector3d subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
}

void Vector3d::scale(double ratio){
	x *= ratio;
	y *= ratio;
	z *= ratio;
}

void Vector3d::scale(double ix, double iy, double iz){
	x *= ix;
	y *= iy;
	z *= iz;
}


// ************************************************************************//
// Operator overloading 
// ************************************************************************//

Vector3d  Vector3d::operator + (Vector3d addend) const {
	Vector3d  newVec;
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
	return newVec;
}


Vector3d  Vector3d::operator - (Vector3d subtractor) const {
	Vector3d  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	return newVec;
}


void Vector3d::operator += (Vector3d addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
}


void Vector3d::operator -= (const Vector3d  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
}

/// Internal element multiplication
Vector3d Vector3d::operator * (const double &d) const {
	return Vector3d(x * d, y * d, z * d);
}

double & Vector3d::operator [](int index){
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
const double Vector3d::operator [](int index) const
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


// ************************************************************************//
// Vector operations
// ************************************************************************//
// Same thing as scalar product, silly!
double Vector3d::DotProduct(Vector3d otherVector) const {
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2];
}

Vector3d Vector3d::CrossProduct(Vector3d otherVector) const {
	return Vector3d (y * otherVector[2] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

double Vector3d::Length(){
	return sqrt(pow((double)x, 2) + pow((double)y, 2)+ pow((double)z, 2));
}


void Vector3d::Normalize(){
	double vecLength = Length();
	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
}

