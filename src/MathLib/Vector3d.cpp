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

Vector3d::Vector3d(Vertex3d v1, Vertex3d v2){
	x = v2.x - v1.x;
	y = v2.y - v1.y;
	z = v2.z - v1.z;
}

Vector3d::Vector3d(double arr[]){
	x = arr[0];
	y = arr[1];
	z = arr[2];
}


Vector3d::Vector3d(const Vector3d & base){
	x = base.x;
	y = base.y;
	z = base.z;
}

/**	Copy Conversion Constructor
	Postcondition: Initializes a 3D vector to have same values as the referenced vector.
*/
Vector3d::Vector3d(const Vector3f& base){
	x = base.x;
	y = base.y;
	z = base.z;
}

// Constructors from other Vector classes
Vector3d::Vector3d(const Vector4d& base){
	x = base.GetX();
	y = base.GetY();
	z = base.GetZ();
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector3d::add(Vector3d addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector3d::subtract(Vector3d subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
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
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	return newVec;
}


Vector3d  Vector3d::operator - (Vector3d subtractor) const {
	Vector3d  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	return newVec;
}


void Vector3d::operator += (Vector3d addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
}


void Vector3d::operator -= (const Vector3d  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
}

/// Internal element multiplication
Vector3d Vector3d::operator * (const double &d) const {
	return Vector3d(x * d, y * d, z * d);
}

double Vector3d::operator [](int index){
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
	return x * otherVector.x + y * otherVector.y + z * otherVector.z;
}

Vector3d Vector3d::CrossProduct(Vector3d otherVector) const {
	return Vector3d (y * otherVector.z - z * otherVector.y, z * otherVector.x - x * otherVector.z, x * otherVector.y - y * otherVector.x);
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

