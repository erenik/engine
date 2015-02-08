/// Emil Hedemalm
/// 2015-01-27
/// An angle, circular is how it is.

#include "Angle.h"

#include "Trigonometry.h"

Angle::Angle()
{
	radians = 0;
}

Angle::Angle(float radians)
: radians(radians)
{
	Normalize();
}

Angle::Angle(float x, float y)
{
	radians = GetAngler(x,y);
}

Angle::Angle(const Vector2f & normalizedVector)
{
	radians = GetAngler(normalizedVector.x, normalizedVector.y);
}

/// Assumes normalized vector.
Angle Angle::FromVector(const Vector2f & normalizedVector)
{
	return Angle(normalizedVector);
}

Angle Angle::FromDegrees(float degrees)
{
	return Angle(DEGREES_TO_RADIANS(degrees));
}

Angle Angle::FromRadians(float radians)
{
	return Angle(radians);
}

float Angle::Degrees() const
{
	return RADIANS_TO_DEGREES(radians);
}

float Angle::Radians() const
{
	return radians;
}

Angle Angle::operator + (const Angle & angle) const
{
	return Angle(radians + angle.radians);
}

Angle Angle::operator - (const Angle & angle) const
{
	return Angle(radians - angle.radians);
}

const Angle & Angle::operator += (const Angle & angle)
{
	radians += angle.radians;
	Normalize();
	return *this;
}

const Angle & Angle::operator -= (const Angle & angle)
{
	radians -= angle.radians;
	Normalize();
	return *this;
}

/// Truncates so that the angle does not exceed this value in either positive or negative direction.
void Angle::Truncate(float minMaxValueInRadians)
{
	if (radians > minMaxValueInRadians)
		radians = minMaxValueInRadians;
	if (radians < -minMaxValueInRadians)
		radians = -minMaxValueInRadians;
}

/// Asserts angle being a decent number (not undefined).
bool Angle::IsGood()
{
	return (radians == radians);
}

void Angle::UnitTest()
{
	Angle angle = Angle::FromDegrees(30.f);
	Angle otherAngle = Angle::FromDegrees(150.f);

	Angle result1 = angle - otherAngle;
	std::cout<<"\n30 - 150 degrees = "<<result1.Degrees();
	Angle result2 = angle + otherAngle;
	std::cout<<"\n30 + 150 degrees = "<<result2.Degrees();
	Angle diff = result2 - result1;
	std::cout<<"\nFrom "<<result1.Degrees()<<" to "<<result2.Degrees()<<": "<<diff.Degrees();
}


/// Normalizes contents between the intervals of -PI to +PI
void Angle::Normalize()
{
	if (radians < -PI)
		radians += 2 * PI;
	else if (radians > PI)
		radians -= 2 * PI;
}
