// Emil Hedemalm
// 2013-09-04

#include "Quaternion.h"

/// References: Morgan Kaufmann, Game Physics Engine Development page 188~ish

Quaternion::Quaternion(){
    x = y = z = 0;
    w = 1;
}

Quaternion::Quaternion(Vector3f vec, float w)
: x(vec.x), y(vec.y), z(vec.z), w(w)
{
}

Quaternion::Quaternion(const Quaternion& other)
: x(other.x), y(other.y), z(other.z), w(other.w)
{
}

Quaternion::Quaternion(float x, float y, float z, float w /*= 0*/)
: x(x), y(y), z(z), w(w)
{
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
void Quaternion::Normalize(){
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
}

/// Addition!
void Quaternion::operator +=(const Quaternion & addend){
    x += addend.x;
    y += addend.y;
    z += addend.z;
    w += addend.w;
}

/// Quaternion-Quaternion multiplication
void Quaternion::operator *=(const Quaternion & multiplier){
    Multiply(multiplier);
}

/// Rotate moar!
Quaternion Quaternion::Multiply(const Quaternion & q2){

    Vector3f v(x,y,z), v2(q2.x, q2.y, q2.z);

    float w2 = w * q2.w - v.DotProduct(v2);
    Vector3f result = w * v2 + q2.w * v + v.CrossProduct(v2);

    w = w2;
    x = result.x;
    y = result.y;
    z = result.z;

    return Quaternion(*this);

/*
    /// First copy ourselves.
    Quaternion q = *this;
    w = q.w * q2.w - q.x * q2.x - q.y * q2.y - q.z * q2.z;
    x = q.w * q2.x + q.x * q2.w - q.y * q2.z - q.z * q2.y;
    y = q.w * q2.y - q.x * q2.z + q.y * q2.w - q.z * q2.x;
    z = q.w * q2.z + q.x * q2.y - q.y * q2.x + q.z * q2.w;
    */
}

Quaternion Quaternion::operator * (const Quaternion &multiplier) const{
    Quaternion temp = *this;
    Quaternion q = temp.Multiply(multiplier);
    return q;
}

/// Multiplication with floats
Quaternion operator * (const float mult, const Quaternion q){
    return Quaternion(q.x * mult, q.y * mult, q.z * mult, q.w * mult);
}

/// Rotate by vector
void Quaternion::Rotate(const Vector3f & vector, float scale){
    Quaternion q(vector.x * scale, vector.y * scale, vector.z * scale, 0);
    (*this) *= q;
}

/// Wosh. Similar to rotate? or no?
void Quaternion::ApplyAngularVelocity(const Vector3f & velocity, float time){
    Quaternion q(velocity.x * time, velocity.y * time, velocity.z * time, 0);
    q *= *this;

    x += q.x * 0.5f;
    y += q.y * 0.5f;
    z += q.z * 0.5f;
    w += q.w * 0.5f;
}





