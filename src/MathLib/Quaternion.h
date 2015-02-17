// Emil Hedemalm
// 2013-09-04

#ifndef QUATERNION_H
#define QUATERNION_H

#include "Matrix4f.h"
class Vector3f;

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
    /// Normalize to length/magnitude 1.
    void Normalize();

	// Unary operator overloading
	Quaternion operator - () const;

    /// Quaterntion-Quaterntion multiplication
//    void operator +=(const Quaternion & addend);
    void operator *=(const Quaternion & multiplier);
    Quaternion Multiply(const Quaternion & quaternion);
    Quaternion operator * (const Quaternion &f) const;

    /// Multiplication with floats
	friend Quaternion operator * (const float multiplier, const Quaternion q);

    /// Rotate by vector and scale (distance?)
    void Rotate(const Vector3f & byVector, float andScale);

    /// Wosh. Similar to rotate? or no?
    void ApplyAngularVelocity(const Vector3f & velocity, float time);

    /// Coordinates, expressed in the xi + yj + zk + w format.
#ifdef USE_SSE
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
