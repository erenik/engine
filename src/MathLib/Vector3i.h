// WRITTEN BY: Emil Hedemalm
// DATE: 2013-12-22
//
// FILE: Vector3i.h
// CLASS PROVIDED: Vector3i  (a three-dimensional vector class using integers)

#ifndef Vector3i_H
#define Vector3i_H

#include "AEMath.h"
#include <iostream>
#include <cmath>

class Vector2i;
class Vector3f;

/** A three-dimensional vector class using ints.
*/
class Vector3i {
public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector3i();
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector3i(int x,  int y, int z);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values.
	*/
	Vector3i(int arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3i(const Vector3i& base);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector3i(const Vector2i& base);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector3i(const Vector3f& base);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector3i& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	///
	int DotProduct(Vector3i otherVec);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector3i addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector3i subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(int ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(int x, int y, int z);

	// Unary - operator (switch signs of all sub-elements)
	Vector3i operator - () const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector3i  operator + (const Vector3i addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector3i  operator - (const Vector3i subtractor) const;

	/// Multiplication with ints
	friend Vector3i operator * (int multiplier, Vector3i& vector);

	/// Adds addend to this vector.
	void operator += (const Vector3i addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector3i addend);
	/// Internal element division
	void operator /= (const float &f);
	/// Internal element multiplication
	void operator *= (const float &f);

	/// Comparison operators
	bool operator == (const Vector3i other);
	/// Comparison operators
	bool operator != (const Vector3i other);

	/// Internal element multiplication
	Vector3f operator * (const float &f) const;
	/// Internal element division.
	Vector3f operator / (const float &f) const;

	/// Operator overloading for the array-access operator []
	int operator [](int index);

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector3i ElementMultiplication(const Vector3i otherVector) const;

	/// Calculates the length of the vector.
	float Length() const;
	/// Calculates the squared length of the vector.
	int LengthSquared() const;
	/** Normalizes the vector coordinates so that the length becomes 1 */
	Vector3i Normalize();
	/** Returns a normalized copy of this vector. */
	Vector3i NormalizedCopy() const;

	/// Returns the vector's x-coordinate.
	const int GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const int GetY() const {return y;};

	/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
	const int MaxPart() const {
		if (AbsoluteValue(x) > AbsoluteValue(y)){
			return AbsoluteValue(x);
		}
		return AbsoluteValue(y);
	};
	/// Utility functions
	static Vector3i Minimum(const Vector3i & vec1, const Vector3i & vec2);
	static Vector3i Maximum(const Vector3i & vec1, const Vector3i & vec2);

public:
	/// x-coordinate
	int x;
	/// y-coordinate
	int y;
	/// z-coordinate
	int z;
};

#endif
