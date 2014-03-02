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
	x = base.x;
	y = base.y;
	z = base.z;
	w = base.w;
}

// Constructors from other Vector classes
Vector4d::Vector4d(const Vector3d  & base, double iw){
	x = base.GetX();
	y = base.GetY();
	z = base.GetZ();
	w = iw;
}

// ************************************************************************//
// Arithmetics
// ************************************************************************//

void Vector4d::add(Vector4d addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
	w += addend.w;
}


void Vector4d::subtract(Vector4d subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
	w -= subtractor.w;
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

// ************************************************************************//
// Operator overloading 
// ************************************************************************//

Vector4d  Vector4d::operator + (Vector4d addend){
	Vector4d  newVec;
	newVec.x = x + addend.x;
	newVec.y = y + addend.y;
	newVec.z = z + addend.z;
	newVec.w = w + addend.w;
	return newVec;
}


Vector4d  Vector4d::operator - (Vector4d subtractor){
	Vector4d  newVec;
	newVec.x = x - subtractor.x;
	newVec.y = y - subtractor.y;
	newVec.z = z - subtractor.z;
	newVec.w = w - subtractor.w;
	return newVec;
}


void Vector4d::operator += (Vector4d addend){
	x += addend.x;
	y += addend.y;
	z += addend.z;
	w += addend.w;
}


void Vector4d::operator -= (const Vector4d  subtractor){
	x -= subtractor.x;
	y -= subtractor.y;
	z -= subtractor.z;
	w -= subtractor.w;
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

// ************************************************************************//
// Vector operations
// ************************************************************************//

double Vector4d::ScalarProduct(Vector4d otherVector){
	return x * otherVector.x + y * otherVector.y + z * otherVector.z + w * otherVector.w;
}
// Same thing as scalar product, silly!
double Vector4d::DotProduct(Vector4d otherVector){
	return x * otherVector.x + y * otherVector.y + z * otherVector.z + w * otherVector.w;
}

Vector3d Vector4d::CrossProduct(Vector3d otherVector){
	return Vector3d (y * otherVector.GetX() - z * otherVector.GetY(), z * otherVector.GetX() - x * otherVector.GetZ(), x * otherVector.GetY() - y * otherVector.GetX());
}

Vector3d Vector4d::CrossProduct(Vector4d otherVector){
	return Vector3d (y * otherVector.z - z * otherVector.y, z * otherVector.x - x * otherVector.z, x * otherVector.y - y * otherVector.x);
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
	newVec.x = x / vecLength;
	newVec.y = y / vecLength;
	newVec.z = z / vecLength;
	return newVec;
}