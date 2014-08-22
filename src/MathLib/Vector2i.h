// WRITTEN BY: Emil Hedemalm
// DATE: 2013-07-14
//
// FILE: Vector2i.h
// CLASS PROVIDED: Vector2i  (a two-dimensional vector class using integers)

#ifndef Vector2i_H
#define Vector2i_H

#include "AEMath.h"
#include <iostream>
#include <cmath>

class Vector2f;
class Vector3i;
class Vector3f;

/** A two-dimensional vector class using ints.
*/
class Vector2i {
public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector2i();
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector2i(int x,  int y);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values.
	*/
	Vector2i(int arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector2i(const Vector2i& base);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector2i(const Vector2f& base);
	/** Constructor, based on Vector3i equivalent
	*/
	Vector2i(const Vector3i& base);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector2i(const Vector3f& base);

	/// Similar to clamp, ensures that this vector's values are within the given range (including the limits)
	void Limit(Vector2i min, Vector2i max);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector2i& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector2i addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector2i subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(int ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(int x, int y);

	// Unary - operator (switch signs of all sub-elements)
	Vector2i operator - () const;

	/// Binary operator.
	bool operator == (const Vector2i other) const;
	/// Binary operator.
	bool operator != (const Vector2i other) const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector2i  operator + (const Vector2i addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector2i  operator - (const Vector2i subtractor) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector2i  operator * (const Vector2i elementMultiplier) const;

	/// Multiplication with ints
	friend Vector2i operator * (int multiplier, Vector2i& vector);

	/// Adds addend to this vector.
	void operator += (const Vector2i addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector2i addend);
	/// Internal element division
	void operator /= (const int &f);
	/// Internal element multiplication
	void operator *= (const int &f);

	/// Internal element multiplication
	Vector2i operator * (const float &f) const;
	/// Internal element multiplication
	Vector2i operator * (const int &f) const;
	/// Internal element division.
	Vector2i operator / (const int &f) const;

	/// Operator overloading for the array-access operator []
	int operator [](int index);

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector2i ElementMultiplication(const Vector2i otherVector) const;

	/// Calculates the length of the vector.
	float Length() const;
	/// Calculates the squared length of the vector.
	int LengthSquared() const;

	/// Corresponds to the area (x*y).
	int GeometricSum();

	/** Normalizes the vector coordinates so that the length becomes 1 */
	Vector2i Normalize();
	/** Returns a normalized copy of this vector. */
	Vector2i NormalizedCopy() const;

	/// Returns the vector's x-coordinate.
	const int GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const int GetY() const {return y;};

	/// Absolute values version.
	Vector2i AbsoluteValues();

	/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
	const int MaxPart() const {
		if (AbsoluteValue(x) > AbsoluteValue(y)){
			return AbsoluteValue(x);
		}
		return AbsoluteValue(y);
	};
	/// Utility functions
	static Vector2i Minimum(const Vector2i & vec1, const Vector2i & vec2);
	static Vector2i Maximum(const Vector2i & vec1, const Vector2i & vec2);

public:
	/// x-coordinate
	int x;
	/// y-coordinate
	int y;
};

#endif
