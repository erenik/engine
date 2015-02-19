/// Emil Hedemalm
/// 2015-01-27
/// An angle, circular is how it is.

#ifndef ANGLE_H
#define ANGLE_H

#include "Vector2f.h"

class Angle 
{
	friend class Angle3;
public:
	Angle();
	Angle(float normalizedX, float normalizedY);
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
	Angle operator * (const float & ratio) const;
	const Angle & operator += (const Angle & angle);
	const Angle & operator -= (const Angle & angle);

	const Angle & operator *= (const float & f);
	Vector2f ToVector2f();

	float Cosine();
	float Sine();
	/// Truncates so that the angle does not exceed this value in either positive or negative direction.
	void Truncate(float minMaxValueInRadians);

	/// Asserts angle being a decent number (not undefined).
	bool IsGood();

	static void UnitTest();
private:
	/// Internal representation?
	float radians;
	
	/// Normalizes contents between the intervals of -PI to +PI
	void Normalize();

};


#endif



