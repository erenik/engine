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
#else
	x = y = z = 0;
	w = 1;
#endif
}

Vector4f::Vector4f( float ix,  float iy,  float iz,  float iw)
{
#ifdef USE_SSE
	x = ix;
	y = iy;
	z = iz;
	w = iw;
	/*
	float arr[4];
	arr[0] = ix;
	arr[1] = iy;
	arr[2] = iz;
	arr[3] = iw;
	data = _mm_loadu_ps(arr);
	*/
#else
	x = ix;
	y = iy;
	z = iz;
	w = iw;
#endif
}

Vector4f::Vector4f(float arr[])
{
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
	arr[0] = (float)base[0];
	arr[1] = (float)base[1];
	arr[2] = (float)base[2];
	arr[3] = (float)base[3];
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
	w = iw;
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
void Vector4f::ReadFrom(const String & string, const String & tokenizers /* = " ,"*/)
{
	List<String> tokens = string.Tokenize(tokenizers);
	x = tokens[0].ParseFloat();
	y = tokens[1].ParseFloat();
	z = tokens[2].ParseFloat();
	w = tokens[3].ParseFloat();
}

/// Parses from string. Expects in the form of first declaring order "XY", "X Y" or "YX", then parses the space-separated values.
void Vector4f::ParseFrom(const String & str)
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
		else if (c == 'z' || c == 'Z')
			order.Add(&z);
		else if (c == 'w' || c == 'W')
			order.Add(&w);
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
#else
	newVec[0] = x - subtractor[0];
	newVec[1] = y - subtractor[1];
	newVec[2] = z - subtractor[2];
	newVec[3] = w - subtractor[3];
#endif
	return newVec;
}

/// Multiplication with float
Vector4f operator * (float multiplier, const Vector4f& vector)
{
	Vector4f  newVec;
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&multiplier);
	newVec.data = _mm_mul_ps(vector.data, mul);
#else
	newVec[0] = vector[0] * multiplier;
	newVec[1] = vector[1] * multiplier;
	newVec[2] = vector[2] * multiplier;
#endif
	return newVec;
}


void Vector4f::operator += (const Vector4f & addend)
{
#ifdef USE_SSE
	data = _mm_add_ps(data, addend.data);
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
#else
	x *= floatur;
	y *= floatur;
	z *= floatur;
	w *= floatur;
#endif
}


/// Internal element multiplication
Vector3f Vector4f::operator * (const float &f) const 
{
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&f);
	Vector3f vec;
	vec.data = _mm_mul_ps(data, mul);
	return vec;
#else
	return Vector3f(x * f, y * f, z * f);
#endif
}
/// Internal element division.
Vector3f Vector4f::operator / (const float &f) const 
{
#ifdef USE_SSE
	__m128 mul = _mm_load1_ps(&f);
	Vector3f vec;
	vec.data = _mm_div_ps(data, mul);
	return vec;
#else
	return Vector3f(x / f, y / f, z / f);
#endif
}

/// Conversion equal-conversion operator
Vector4f& Vector4f::operator = (const Vector4d &other)
{
#ifdef USE_SSE
	float arr[4];
	arr[0] = (float)other[0];
	arr[1] = (float)other[1];
	arr[2] = (float)other[2];
	arr[3] = (float)other[3];
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
	return v[index];
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
	return v[index];
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

float Vector4f::Length3() const
{
#ifdef USE_SSE
	float sum;
	SSEVec sse;
	sse.data = _mm_mul_ps(data, data);
	sum = sqrt(sse.v[0] + sse.v[1] + sse.v[2]);
//	sum = sqrt(sse.m128_f32[0] + sse.m128_f32[1] + sse.m128_f32[2]); // Old
	return sum;
#else
	return sqrt(pow((float)x, 2) + pow((float)y, 2)+ pow((float)z, 2));
#endif
}

/** Calculates the length of the vector, considering only {x y z}. */
 float Vector4f::Length3Squared() const 
 {
	return x * x + y * y + z * z;
 }


void Vector4f::Normalize3()
{
	float vecLength = Length3();
#ifdef USE_SSE
	__m128 sse = _mm_load1_ps(&vecLength);
	data = _mm_div_ps(data, sse);
#else
	x = x / vecLength;
	y = y / vecLength;
	z = z / vecLength;
#endif
}

/// Returns a normalized (x,y,z) copy of the given vector.
Vector4f Vector4f::NormalizedCopy() const
{
    float vecLength = Length3();
#ifdef USE_SSE
	Vector4f vec;
	__m128 sse = _mm_load1_ps(&vecLength);
	vec.data = _mm_div_ps(data, sse);
	return vec;
#else
    return (*this / vecLength);
#endif
}


#ifdef USE_SSE
// Loads data into __m128 structure.
void Vector4f::PrepareForSIMD()
{
	data = _mm_setr_ps(x,y,z,w);
}
#endif



/*
Should be able to place in class!
union
{
    __m128 simd;
    float v[4];
    struct { float x, y, z, w; }
}
*/
