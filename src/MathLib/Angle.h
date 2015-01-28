/// Emil Hedemalm
/// 2015-01-27
/// An angle, circular is how it is.

#ifndef ANGLE_H
#define ANGLE_H

#include "Vector2f.h"

struct Angle 
{
public:
	Angle();
	Angle(float radians);
	Angle(const Vector2f & fromNormalizedVector);
	/// Assumes normalized vector.
	static Angle FromVector(const Vector2f & normalizedVector);
	static Angle FromDegrees(float degrees);
	static Angle FromRadians(float radians);
	float Degrees() const;
	float Radians() const;
	Angle operator + (const Angle & angle) const;
	Angle operator - (const Angle & angle) const;
	const Angle & operator += (const Angle & angle);
	const Angle & operator -= (const Angle & angle);

	/// Truncates so that the angle does not exceed this value in either positive or negative direction.
	void Truncate(float minMaxValueInRadians);

	static void UnitTest();
private:
	/// Internal representation?
	float radians;
	
	/// Normalizes contents between the intervals of -PI to +PI
	void Normalize();

};

#endif



