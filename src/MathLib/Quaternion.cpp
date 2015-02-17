// Emil Hedemalm
// 2013-09-04

#include "Quaternion.h"
#include <cassert>
#include "Trigonometry.h"

/// References: Morgan Kaufmann, Game Physics Engine Development page 188~ish

Quaternion::Quaternion(){
    x = y = z = 0;
    w = 1;
}

/// Generates a rotation quaternion based on given axis and rotation around it
Quaternion::Quaternion(const Vector3f & axis, float angle)
: axis(axis), angle(angle)
{
	/*
	x = axis[0];
	y = axis[1];
	z = axis[2];
	w = angle;
	*/
	RecalculateXYZW();
}

Quaternion::Quaternion(const Quaternion& other)
: x(other.x), y(other.y), z(other.z), w(other.w), axis(other.axis), angle(other.angle)
{
}

Quaternion::Quaternion(float x, float y, float z, float w /*= 0*/)
: x(x), y(y), z(z), w(w)
{
}

/// Returns the absolute value of the highest value of x, y, and z-components.
float Quaternion::MaxPart()
{
	if (AbsoluteValue(x) > AbsoluteValue(y)){
		if (AbsoluteValue(x) > AbsoluteValue(z))
			return AbsoluteValue(x);
		return AbsoluteValue(z);
	}
	else if (AbsoluteValue(y) > AbsoluteValue(z))
		return AbsoluteValue(y);
	return AbsoluteValue(z);
}


/// Printing out data
std::ostream& operator <<(std::ostream& os, const Quaternion& q){
    os << q.x << " " << q.y << " " << q.z << " " << q.w;
    return os;
}

/// Extract a matrix from this quaternion.
Matrix4f Quaternion::Matrix() const {

    Matrix4f matrix;
    float * arr = matrix.getPointer();
    // First column
    arr[0] = 1 - (2 * y * y + 2 * z * z);
    arr[1] = 2 * x * y - 2 * z * w;
    arr[2] = 2 * x * z + 2 * y * w;
    arr[3] = 0;

    arr[4] = 2 * x * y + 2 * z * w;
    arr[5] = 1 - (2 * x * x + 2 * z * z);
    arr[6] = 2 * y * z - 2 * x * w;
    arr[7] = 0;

    arr[8] = 2 * x * z - 2 * y * w;
    arr[9] = 2 * y * z + 2 * x * w;
    arr[10] = 1 - (2 * x * x + 2 * y * y);
    arr[11] = 0;

    arr[12] = arr[13] = arr[14] = 0;
    arr[15] = 1;

    return matrix;
}

/// Normalize to length/magnitude 1.
void Quaternion::Normalize()
{
#ifdef USE_SSE
	SSEVec sse;
	sse.data = _mm_mul_ps(data, data);
	float size = sse.x + sse.y + sse.z + sse.w;
	if (size == 0)
	{
		w = 1;
		return;
	}
	float invSize = 1 / sqrt(size);
	sse.data = _mm_load1_ps(&invSize);
	data = _mm_mul_ps(data, sse.data);
#else
    float size = x*x + y*y + z*z + w*w;
    if (size == 0){
        w = 1;
        return;
    }
    float invSize = 1.0f / sqrt(size);
    x *= invSize;
    y *= invSize;
    z *= invSize;
    w *= invSize;
#endif
}

// Unary operator overloading
Quaternion Quaternion::operator - () const
{
	Quaternion quat(x,y,z,-w);
	return quat;
}


/*
/// Addition!
void Quaternion::operator +=(const Quaternion & addend)
{
	// There is no addition in quaternions.
	assert(false);
    x += addend[0];
    y += addend[1];
    z += addend[2];
    w += addend[3];
}*/

/// Quaternion-Quaternion multiplication
void Quaternion::operator *=(const Quaternion & multiplier){
    Multiply(multiplier);
}

/// Rotate moar!
Quaternion Quaternion::Multiply(const Quaternion & q2)
{
#ifdef USE_SSE
	// Some few optimizations, not too much.
	Vector3f v, v2;
	v.data = data;
	v2.data = q2.data;
	/// 3 products are being used below, do them here then.
	__m128 data1, data2;

    float w2 = w * q2.w - v.DotProduct(v2);
    Vector3f result;
	data1 = _mm_load1_ps(&w);
	data2 = _mm_load1_ps(&q2.w);
	data = result.data = _mm_add_ps(_mm_add_ps(_mm_mul_ps(data1, v2.data), _mm_mul_ps(data2, v.data)), v.CrossProduct(v2).data);
	w = w2;
#else
    Vector3f v(x,y,z), v2(q2.x, q2.y, q2.z);

    float w2 = w * q2.w - v.DotProduct(v2);
    Vector3f result = w * v2 + q2.w * v + v.CrossProduct(v2);

    w = w2;
    x = result[0];
    y = result[1];
    z = result[2];

#endif
    return Quaternion(*this);
}

Quaternion Quaternion::operator * (const Quaternion &q2) const
{
#ifdef USE_SSE
	Quaternion quat;
	// Some few optimizations, not too much.
	Vector3f v, v2;
	v.data = data;
	v2.data = q2.data;
	/// 3 products are being used below, do them here then.
	__m128 data1, data2;
    float w2 = w * q2.w - v.DotProduct(v2);
    Vector3f result;
	data1 = _mm_load1_ps(&w);
	data2 = _mm_load1_ps(&q2.w);
	quat.data = result.data = _mm_add_ps(_mm_add_ps(_mm_mul_ps(data1, v2.data), _mm_mul_ps(data2, v.data)), v.CrossProduct(v2).data);
	quat.w = w2;
	// Copy remaining stats.
	quat.axis.data = axis.data;
	quat.angle = angle;
	return quat;
#else
    Quaternion temp = *this;
    Quaternion q = temp.Multiply(q2);
    return q;
#endif
}

/// Multiplication with floats
Quaternion operator * (const float mult, const Quaternion & q){
    return Quaternion(q.x * mult, q.y * mult, q.z * mult, q.w * mult);
}

/// Rotate by vector
void Quaternion::Rotate(const Vector3f & vector, float scale){
    Quaternion q(vector[0] * scale, vector[1] * scale, vector[2] * scale, 0);
    (*this) *= q;
}

/// Wosh. Similar to rotate? or no?
void Quaternion::ApplyAngularVelocity(const Vector3f & velocity, float time)
{
    Quaternion q(velocity[0] * time, velocity[1] * time, velocity[2] * time, 0);
    q *= *this;

    x += q.x * 0.5f;
    y += q.y * 0.5f;
    z += q.z * 0.5f;
    w += q.w * 0.5f;
}

// Recalculates them based on axis and angle.
void Quaternion::RecalculateXYZW()
{
	float halfAngle = angle * 0.5f;
	float sinHalfAngle = FastSin(halfAngle);
	float cosHalfAngle = FastCos(halfAngle);

#ifdef USE_SSE
	__m128 sse = _mm_load1_ps(&sinHalfAngle);
	data = _mm_mul_ps(axis.data, sse);
	w = cosHalfAngle;
#else
	x = axis.x * sinHalfAngle;
	y = axis.y * sinHalfAngle;
	z = axis.z * sinHalfAngle;
	w = cosHalfAngle;
#endif
}




