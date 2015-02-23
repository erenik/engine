/// Emil Hedemalm
/// 2015-02-19
/// Class for calculating sets of angles quickly and easily.

#ifndef ANGLE3_H
#define ANGLE3_H

#include <iostream>
#include "Angle.h"
#include "Vector3f.h"

class Angle3 
{
public:
	Angle3();
	Angle3(float angle1InRadians, float angle2InRadians, float angle3InRadians);
	Angle3(Angle angle1, Angle angle2, Angle angle3);
	Angle3(const Vector3f & vectorOfAngles);
	Angle3 operator - (const Angle3 & other) const;
	Angle3 operator + (const Angle3 & other) const;
	Angle3 operator * (const float & ratio) const;
	
	const Angle3 & operator += (const Angle3 & other);
		
	/// Transitions from this angle to the other by the given ratio. 0 would return this, and 1 would return the other.
	void SmoothTo(const Angle3 & other, float ratio);
	
	/// Fetches a set of angles based on a required rotation between two vectors.
	static Angle3 GetRequiredRotation(ConstVec3fr fromVector, ConstVec3fr toVector);
	static Vector3f VectorFromPitchYawForwardZMinus(float pitch, float yaw);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Angle3 & vec);

	Angle x,y,z;
};

#endif
