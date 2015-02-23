/// Emil Hedemalm
/// 2015-02-19
/// Class for calculating sets of angles quickly and easily.

#include "Angle3.h"

Angle3::Angle3()
{}

Angle3::Angle3(float angle1InRadians, float angle2InRadians, float angle3InRadians)
{	
	x = angle1InRadians;
	y = angle2InRadians;
	z = angle3InRadians;
}

Angle3::Angle3(Angle angle1, Angle angle2, Angle angle3)
{
	x = angle1;
	y = angle2;
	z = angle3;
}


Angle3::Angle3(const Vector3f & vectorOfAngles)
{
	x = vectorOfAngles.x;
	y = vectorOfAngles.y;
	z = vectorOfAngles.z;
}

Angle3 Angle3::operator - (const Angle3 & other) const 
{
	return Angle3(x - other.x, y - other.y, z - other.z);
}
Angle3 Angle3::operator + (const Angle3 & other) const
{
	return Angle3(x + other.x, y + other.y, z + other.z);
}

Angle3 Angle3::operator * (const float & ratio) const
{
	return Angle3(x * ratio, y * ratio, z * ratio);
}	

const Angle3 & Angle3::operator += (const Angle3 & other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

/// Transitions from this angle to the other by the given ratio. 0 would return this, and 1 would return the other.
void Angle3::SmoothTo(const Angle3 & other, float ratio)
{
	Angle3 toOther = other - *this;
	*this += toOther * ratio;
}

/// Fetches a set of angles based on a required rotation between two vectors.
Angle3 Angle3::GetRequiredRotation(ConstVec3fr fromVector, ConstVec3fr toVector)
{
	Angle3 required;
	// Extract first vector's yaw.
	Vector2f xz1(fromVector.x, fromVector.z);
	Vector2f xz1Normalized = xz1.NormalizedCopy();
	Angle yaw1(xz1Normalized);
	// Second yaw.
	Vector2f xz2(toVector.x, toVector.z);
	Vector2f xz2Normalized = xz2.NormalizedCopy();
	Angle yaw2(xz2Normalized);
	/// Yaw diff
	Angle yawDiff = yaw2 - yaw1;
	required.y = yawDiff.radians;

	// Pitch 1
	Vector2f xzY1(xz1.Length(), fromVector.y);
	Vector2f xzY1Normalized = xzY1.NormalizedCopy();
	Angle pitch1(xzY1Normalized);
	// Pitch 2
	Vector2f xzY2(xz2.Length(), toVector.y);
	Vector2f xzY2Normalized = xzY2.NormalizedCopy();
	Angle pitch2(xzY2Normalized);
	// Diff
	Angle pitchDiff = pitch2 - pitch1;
	required.x = pitchDiff.radians;
	return required;
}

Vector3f Angle3::VectorFromPitchYawForwardZMinus(float pitch, float yaw)
{
	Vector3f vec;
	float y = sin(pitch);
	vec.y = y;
	float xzLen = cos(pitch);
	float modulatedYaw = PI - yaw;
	float x = sin(modulatedYaw) * xzLen;
	float z = cos(modulatedYaw) * xzLen;
	vec.x = x;
	vec.z = z;
	return vec;
}


/// Printing out data
std::ostream& operator <<(std::ostream& os, const Angle3& ang3)
{
	os << ang3.x.Radians() << " " << ang3.y.Radians() << " " << ang3.z.Radians();
	return os;
}
