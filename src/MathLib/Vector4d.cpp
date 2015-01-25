#include "Vector4d.h"

Vector4d::Vector4d(){
	x = y = z = w = 0;
}

Vector4d::Vector4d( double ix,  double iy,  double iz,  double iw){
	x = ix;
	y = iy;
	z = iz;
	w = iw;
}

Vector4d::Vector4d(double arr[]){
	x = arr[0];
	y = arr[1];
	z = arr[2];
	w = arr[3];
}

Vector4d::Vector4d(const Vector4d & base){
	x = base[0];
	y = base[1];
	z = base[2];
	w = base[3];
}

// Constructors from other Vector classes
Vector4d::Vector4d(const Vector3d  & base, double iw){
	x = base[0];
	y = base[1];
	z = base[2];
	w = iw;
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector4d::add(Vector4d addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
	w += addend[3];
}


void Vector4d::subtract(Vector4d subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
	w -= subtractor[3];
}

void Vector4d::scale(double ratio){
	x *= ratio;
	y *= ratio;
	z *= ratio;
	w *= ratio;
}

void Vector4d::scale(double ix, double iy, double iz, double iw){
	x *= ix;
	y *= iy;
	z *= iz;
	w *= iw;
}

// Unary operators

// Unary - operator (switch signs of all sub-elements)
Vector4d Vector4d::operator - () const
{
	return Vector4d(-x, -y, -z, -w);
}


// ************************************************************************//
// Operator overloading 
// ************************************************************************//

Vector4d  Vector4d::operator + (Vector4d addend){
	Vector4d  newVec;
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
	newVec[3] = w + addend[3];
	return newVec;
}


Vector4d  Vector4d::operator - (Vector4d subtractor){
	Vector4d  newVec;
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	newVec[3] = w - subtractor[3];
	return newVec;
}


void Vector4d::operator += (Vector4d addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
	w += addend[3];
}


void Vector4d::operator -= (const Vector4d  subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
	w -= subtractor[3];
}

/// Internal element multiplication
Vector4d Vector4d::operator * (const float &f) const {
	return Vector4d(x * f, y * f, z * f, w * f);
}

double& Vector4d::operator[](const unsigned int index){
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
/// Operator overloading for the array-access operator []
const double& Vector4d::operator[] (const unsigned int index) const
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

double Vector4d::ScalarProduct(Vector4d otherVector){
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2] + w * otherVector[3];
}
// Same thing as scalar product, silly!
double Vector4d::DotProduct(Vector4d otherVector){
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2] + w * otherVector[3];
}

Vector3d Vector4d::CrossProduct(Vector3d otherVector){
	return Vector3d (y * otherVector[0] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

Vector3d Vector4d::CrossProduct(Vector4d otherVector){
	return Vector3d (y * otherVector[2] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

double Vector4d::Length3() const{
	return sqrt(pow((double)x, 2) + pow((double)y, 2)+ pow((double)z, 2));
}

void Vector4d::Normalize3(){
	double vecLength = Length3();
	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
}

/// Returns a normalized (xyz) copy of the current vector.
Vector4d Vector4d::NormalizedCopy() const{
	Vector4d newVec;
	double vecLength = Length3();
	newVec[0] = x / vecLength;
	newVec[1] = y / vecLength;
	newVec[2] = z / vecLength;
	return newVec;
}