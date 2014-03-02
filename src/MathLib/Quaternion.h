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
    Quaternion(Vector3f vec, float w);
    Quaternion(const Quaternion& other);
    Quaternion(float x, float y, float z, float w = 1);

    /// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Quaternion& q);

    /// Extract a matrix from this quaternion.
    Matrix4f Matrix() const;
    /// Normalize to length/magnitude 1.
    void Normalize();

    /// Quaterntion-Quaterntion multiplication
    void operator +=(const Quaternion & addend);
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
    float x,y,z,w;
private:
};


#endif
