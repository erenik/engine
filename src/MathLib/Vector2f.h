// WRITTEN BY: Emil Hedemalm
// DATE: 2014-02-27
//
// FILE: Vector2f.h
// CLASS PROVIDED: Vector2f  (a two-dimensional vector class using integers)

#ifndef Vector2f_H
#define Vector2f_H

#include "AEMath.h"
#include <iostream>
#include <cmath>

class Vector2i;
class Vector3i;
class Vector3f;

/** A three-dimensional vector class using ints.
*/
class Vector2f {
public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector2f();
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector2f(float x,  float y);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values.
	*/
	Vector2f(float arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector2f(const Vector2i& base);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector2f(const Vector2f& base);
	/** Constructor, based on Vector3i equivalent
	*/
	Vector2f(const Vector3i& base);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector2f(const Vector3f& base);


	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector2f& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	/// Clamp to an interval.
	void Clamp(float min, float max);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector2f addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector2f subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(float ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(float x, float y);

	// Unary - operator (switch signs of all sub-elements)
	Vector2f operator - () const;

	/// Binary operator.
	bool operator == (const Vector2f other) const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector2f  operator + (const Vector2f addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector2f  operator - (const Vector2f subtractor) const;

	/// Multiplication with ints
	friend Vector2f operator * (float multiplier, Vector2f& vector);

	/// Adds addend to this vector.
	void operator += (const Vector2f addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector2f addend);
	/// Internal element division
	void operator /= (const float &f);
	/// Internal element multiplication
	void operator *= (const float &f);
	/// Per-element multiplication
	void operator *= (const Vector2f &vec);

	/// Internal element multiplication
	Vector2f operator * (const float &f) const;
	/// Internal element division.
	Vector2f operator / (const float &f) const;

	/// Operator overloading for the array-access operator []
	float operator [](int index);

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector2f ElementMultiplication(const Vector2f otherVector) const;
	/// Make sure all elements are non-0 before calling this...
	Vector2f ElementDivision(const Vector2f dividend) const;

	// Dot product.
	float DotProduct(const Vector2f otherVector) const;

	/// Calculates the length of the vector.
	float Length() const;
	/// Calculates the squared length of the vector.
	float LengthSquared() const;
	/** Normalizes the vector coordinates so that the length becomes 1 */
	Vector2f Normalize();
	/** Returns a normalized copy of this vector. */
	Vector2f NormalizedCopy() const;

	/// Returns the vector's x-coordinate.
	const float GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const float GetY() const {return y;};

	/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
	const float MaxPart() const {
		if (AbsoluteValue(x) > AbsoluteValue(y)){
			return AbsoluteValue(x);
		}
		return AbsoluteValue(y);
	};
	/// Utility functions
	static Vector2f Minimum(const Vector2f & vec1, const Vector2f & vec2);
	static Vector2f Maximum(const Vector2f & vec1, const Vector2f & vec2);

	/// Comparison.
	bool IsWithinMinMax(Vector2f min, Vector2f max);

public:
	/// x-coordinate
	float x;
	/// y-coordinate
	float y;
};

#endif
