// Emil Hedemalm
// 2013-09-04

#ifndef QUATERNION_H
#define QUATERNION_H

#include "Matrix4f.h"
class Vector3f;

#define ConstQuatRef const Quaternion &

/** Could have been based off of the Vector4f class, but to avoid confusion we will have a separate class entirely for the quaternions!
    Require a length of 1 to represent an orientation.
*/
class Quaternion {
public:
    Quaternion();
	/// Generates a rotation quaternion based on given axis and rotation around it
    Quaternion(const Vector3f & axis, float angle);
    Quaternion(const Quaternion& other);
    Quaternion(float x, float y, float z, float w = 1);

	/// Returns the absolute value of the highest value of x, y, and z-components.
	float MaxPart();

    /// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Quaternion& q);

    /// Extract a matrix from this quaternion.
    Matrix4f Matrix() const;
    /// Normalize to length/magnitude 1. <- useful how?
    void Normalize();

	/// Returns quaternion rotation between this and q2 according to ratio. 0.0 is at this, 1.0 is at q2, 0.5 in the middle.
	Quaternion SlerpTo(ConstQuatRef q2, float ratio);

	/// Based on Qxyzw, calculates axis around which this quaternion is turning.
	Vector3f GetAxis();
	/// Based on Qzyzw, calculates angle around the axis which this quaternion is turning.
	float GetAngle();

	// Unary operator overloading
	Quaternion operator - () const;

    /// Quaterntion-Quaterntion multiplication
//    void operator +=(const Quaternion & addend);
    void operator *=(const Quaternion & multiplier);
    Quaternion Multiply(const Quaternion & quaternion);
    Quaternion operator * (const Quaternion &f) const;

    /// Multiplication with floats
	friend Quaternion operator * (const float multiplier, const Quaternion & q);
//    Quaternion operator *= (const float multiplier);

    /// Rotate by vector and scale (distance?)
    void Rotate(const Vector3f & byVector, float andScale);

    /// Wosh. Similar to rotate? or no?
    void ApplyAngularVelocity(const Vector3f & velocity, float time);

    /// Coordinates, expressed in the xi + yj + zk + w format.
#ifdef USE_SSE
#define SSE_QUAT
	union 
	{
		struct {
			float x,y,z,w;
		};
		__m128 data;
	};
#else
    float x,y,z,w;
#endif
	// Angle in radians
	float angle;
	// Axis by which we rotate.
	Vector3f axis;
	// Recalculates them based on axis and angle.
	void RecalculateXYZW();
private:
};


#endif
