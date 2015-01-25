// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector4f.h
// CLASS PROVIDED: Vector4f  (a four-dimensional vector class using floats)

#include "Vector4f.h"
#include <fstream>

#include "String/AEString.h"

// bool Vector4f::useSSE = false;

#ifdef USE_SSE
#define x (*this)[0]
#define y (*this)[1]
#define z (*this)[2]
#define w (*this)[3]
#endif


void* Vector4f::operator new(size_t size)
{
	std::cout<<"\nlall";
	void *storage = malloc(size * sizeof(Vector4f));
    if(NULL == storage) {
            throw "allocation fail : no free memory";
    }	
}
void Vector4f::operator delete(void* mem)
{
	mem;
}


Vector4f::Vector4f()
{
#ifdef USE_SSE
	float arr[4] = {0,0,0,1};
	data = _mm_loadu_ps(arr);
#endif
	x = y = z = 0;
	w = 1;
}

Vector4f::Vector4f( float ix,  float iy,  float iz,  float iw)
{
#ifdef USE_SSE
	float arr[4];
	arr[0] = ix;
	arr[1] = iy;
	arr[2] = iz;
	arr[3] = iw;
	data = _mm_loadu_ps(arr);
#endif
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

Vector4f::Vector4f(const Vector4f & base)
{
#ifdef USE_SSE
	data = base.data;
	x = base[0];
	y = base[1];
	z = base[2];
	w = base[3];
#else
	x = base[0];
	y = base[1];
	z = base[2];
	w = base[3];
#endif
}

/**	Copy Conversion Constructor
	Postcondition: Initializes a 4D vector to have same values as the referenced vector.
*/
Vector4f::Vector4f(const Vector4d & base)
{
#ifdef USE_SSE
	float arr[4];
	arr[0] = base[0];
	arr[1] = base[1];
	arr[2] = base[2];
	arr[3] = base[3];
	data = _mm_loadu_ps(arr);
#else
	x = (float)base[0];
	y = (float)base[1];
	z = (float)base[2];
	w = (float)base[3];
#endif
}

// Constructors from other Vector classes
Vector4f::Vector4f(const Vector3f  & base, float iw)
{
#ifdef USE_SSE
	data = base.data;
	data.m128_f32[3] = iw;
#else
	x = base[0];
	y = base[1];
	z = base[2];
	w = iw;
#endif
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const Vector4f& vec){
	os << vec[0] << " " << vec[1] << " " << vec[2] << " " << vec[3];
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

void Vector4f::add(const Vector4f & addend){
	x += addend[0];
	y += addend[1];
	z += addend[2];
	w += addend[3];
}


void Vector4f::subtract(const Vector4f & subtractor){
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
	w -= subtractor[3];
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

// For getting parts straight away o.o;
Vector4f  Vector4f::operator + (const Vector4f & addend) const {
	Vector4f  newVec;
#ifdef USE_SSE
	newVec.data = _mm_add_ps(data, addend.data);
#else
	newVec[0] = x + addend[0];
	newVec[1] = y + addend[1];
	newVec[2] = z + addend[2];
	newVec[3] = w + addend[3];	
#endif
	/*
	if (useSSE)
	{
		u4f u;
		__m128 left = _mm_setr_ps(x,y,z,w);
		__m128 right = _mm_setr_ps(addend[0], addend[1], addend[2], addend[3]);
		u.f = _mm_add_ps(left, right);
		newVec[0] = u.v[0];
		newVec[1] = u.v[1];
		newVec[2] = u.v[2];
		newVec[3] = u.v[3];
	}
	else 
	{
		newVec[0] = x + addend[0];
		newVec[1] = y + addend[1];
		newVec[2] = z + addend[2];
		newVec[3] = w + addend[3];	
	}*/
	return newVec;
}


Vector4f  Vector4f::operator - (const Vector4f & subtractor) const {
	Vector4f  newVec;
#ifdef USE_SSE
	newVec.data = _mm_sub_ps(data, subtractor.data);
	newVec[0] = data.m128_f32[0];
	newVec[1] = data.m128_f32[1];
	newVec[2] = data.m128_f32[2];
	newVec[3] = data.m128_f32[3];
#else
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	newVec[3] = w - subtractor[3];
#endif
	return newVec;
}

/// Multiplication with float
Vector4f operator * (float multiplier, const Vector4f& vector){
	Vector4f  newVec;
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	newVec[2] = vector[2] * multiplier;
	return newVec;
}


void Vector4f::operator += (const Vector4f & addend)
{
#ifdef USE_SSE
	data = _mm_add_ps(data, addend.data);
	x = data.m128_f32[0];
	y = data.m128_f32[1];
	z = data.m128_f32[2];
	w = data.m128_f32[3];
#else
	x += addend[0];
	y += addend[1];
	z += addend[2];
	w += addend[3];
#endif
}


void Vector4f::operator -= (const Vector4f & subtractor){
#ifdef USE_SSE
	data = _mm_sub_ps(data, subtractor.data);
	x = data.m128_f32[0];
	y = data.m128_f32[1];
	z = data.m128_f32[2];
	w = data.m128_f32[3];
#else
	x -= subtractor[0];
	y -= subtractor[1];
	z -= subtractor[2];
	w -= subtractor[3];
#endif
}

/// Multiplicator o-o
void Vector4f::operator *= (const float floatur){
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&floatur);
	data = _mm_mul_ps(data, mul);
	x = data.m128_f32[0];
	y = data.m128_f32[1];
	z = data.m128_f32[2];
	w = data.m128_f32[3];
#else
	x *= floatur;
	y *= floatur;
	z *= floatur;
	w *= floatur;
#endif
}

/// Dividor o-o
void Vector4f::operator /= (const float floatur){
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&floatur);
	data = _mm_div_ps(data, mul);
	x = data.m128_f32[0];
	y = data.m128_f32[1];
	z = data.m128_f32[2];
	w = data.m128_f32[3];
#else
	x *= floatur;
	y *= floatur;
	z *= floatur;
	w *= floatur;
#endif
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
Vector4f& Vector4f::operator = (const Vector4d &other)
{
#ifdef USE_SSE
	float arr[4];
	arr[0] = other[0];
	arr[1] = other[1];
	arr[2] = other[2];
	arr[3] = other[3];
	data = _mm_loadu_ps(arr);
#else
	this->x = (float)other[0];
	this->y = (float)other[1];
	this->z = (float)other[2];
	this->w = (float)other[3];
#endif
	return *this;
}

/// Conversion equal-conversion operator
Vector4f& Vector4f::operator = (const Vector4f &other)
{
#ifdef USE_SSE
	data = other.data;
	x = other[0];
	y = other[1];
	z = other[2];
	w = other[3];
#else
	x = other[0];
	y = other[1];
	z = other[2];
	w = other[3];
#endif
	return *this;
}



float& Vector4f::operator[](const unsigned int index)
{
#ifdef USE_SSE
	return data.m128_f32[index];
#else
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
#endif
}

/// Operator overloading for the array-access operator []
const float& Vector4f::operator[] (const unsigned int index) const
{
#ifdef USE_SSE
	return data.m128_f32[index];
#else
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
#endif
}


// ************************************************************************//
// Vector operations
// ************************************************************************//

float Vector4f::ScalarProduct(const Vector4f & otherVector)
{
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2] + w * otherVector[3];
}
// Same thing as scalar product, silly!
 float Vector4f::DotProduct(const Vector4f & otherVector){
	return x * otherVector[0] + y * otherVector[1] + z * otherVector[2] + w * otherVector[3];
}

Vector3f Vector4f::CrossProduct(const Vector3f & otherVector)
{
	return Vector3f (y * otherVector[0] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

Vector3f Vector4f::CrossProduct(const Vector4f & otherVector){
	 return Vector3f (y * otherVector[2] - z * otherVector[1], z * otherVector[0] - x * otherVector[2], x * otherVector[1] - y * otherVector[0]);
}

/// Multiplies the elements in the two vectors internally, returning the product.
Vector4f Vector4f::ElementMultiplication(const Vector4f & otherVector) const {
	return Vector4f(x * otherVector[0], y * otherVector[1], z * otherVector[2], w * otherVector[3]);
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


#ifdef USE_SSE
// Loads data into __m128 structure.
void Vector4f::PrepareForSIMD()
{
	data = _mm_setr_ps(x,y,z,w);
}
#endif
