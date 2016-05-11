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
#ifdef SSE_QUAT
	SSEVec sse;
	__m128 sse2 = data, sse3 = data;
	sse.data = _mm_mul_ps(sse2, sse3);
	float size = sse.x + sse.y + sse.z + sse.w;
	if (size == 0)
	{
		w = 1;
		return;
	}
	float invSize = 1 / sqrt(size);
	sse.data = _mm_load1_ps(&invSize);
	sse3 = _mm_mul_ps(data, sse.data);
	data = sse3;
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

/// Returns quaternion rotation between this and q2 according to ratio. 0.0 is at this, 1.0 is at q2, 0.5 in the middle.
Quaternion Quaternion::SlerpTo(ConstQuatRef q2, float ratio)
{
	// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
	// qm = (qa * sin((1-t)theta) + qb * sin (t * theta)) / sin(theta)
	// where qm = interpolated, qa = Q1, qb = q2, t = scalar between 0 and 1, theta is half the angle between qa and qb.
	Quaternion interpolated;
	assert(false && "bad");
	return interpolated;
	/*
	// cos(theta/2) = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y+ qa.z*qb.z
	float cosThetaDiv2 = w * q2.w + x * q2.x + y * q2.y + z * q2.z;
	if (abs(cosThetaDiv2) >= 1.0) // Equal or theta is 0
	{
		return 
	}

	quat slerp(quat qa, quat qb, double t) {
	// quaternion to return
	quat qm = new quat();
	// Calculate angle between them.
	double cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (abs(cosHalfTheta) >= 1.0){
		qm.w = qa.w;qm.x = qa.x;qm.y = qa.y;qm.z = qa.z;
		return qm;
	}
	// Calculate temporary values.
	double halfTheta = acos(cosHalfTheta);
	double sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001){ // fabs is floating point absolute
		qm.w = (qa.w * 0.5 + qb.w * 0.5);
		qm.x = (qa.x * 0.5 + qb.x * 0.5);
		qm.y = (qa.y * 0.5 + qb.y * 0.5);
		qm.z = (qa.z * 0.5 + qb.z * 0.5);
		return qm;
	}
	double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
	double ratioB = sin(t * halfTheta) / sinHalfTheta; 
	//calculate Quaternion.
	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);
	return qm;

	float sinTheta = sin(acos(cos(theta)));
	*/
}

/// Based on Qxyzw, calculates axis around which this quaternion is turning.
Vector3f Quaternion::GetAxis()
{
	Vector3f axis;
	float w2 = w * w;
	float root = sqrt(1 - w2);
	if (root == 0)
		return axis;
	axis.x = x / root;
	axis.y = y / root;
	axis.z = z / root;
	return axis;
}
/// Based on Qzyzw, calculates angle around the axis which this quaternion is turning.
float Quaternion::GetAngle()
{
	return 2 * acos(w);
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
#ifdef SSE_QUAT
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
#ifdef SSE_QUAT
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

//Quaternion operator *= (const float multiplier)
//{
	
//}


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

#ifdef SSE_QUAT
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




