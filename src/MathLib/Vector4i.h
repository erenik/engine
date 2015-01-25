/// Emil Hedemalm
/// 2014-09-29
/// Integer-based 4-element vector.

#ifndef VECTOR_4I_H
#define VECTOR_4I_H

#include "AEMath.h"
#include <iostream>
#include <cmath>

class Vector2i;
class Vector3i;
class Vector3f;
class Vector4f;

/** A three-dimensional vector class using ints.
*/
class Vector4i {
public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector4i();
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector4i(int x,  int y, int z, int w = 1);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values.
	*/
	Vector4i(int arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector4i(const Vector3i& base, int w = 1);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector4i(const Vector2i& base, int z, int w = 1);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector4i(const Vector3f& base, int w);
	/** Constructor, based on Vector3f equivalent
	*/
	Vector4i(const Vector4f& base);
	/// Copy constructor.
	Vector4i(const Vector4i& base);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector4i& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void Add(const Vector4i addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void Subtract(const Vector4i subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void Scale(float ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void Scale(float x, float y, float z);

	// Unary - operator (switch signs of all sub-elements)
	Vector4i operator - () const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector4i  operator + (const Vector4i addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector4i  operator - (const Vector4i subtractor) const;

	/// Multiplication with ints
	friend Vector4i operator * (int multiplier, Vector4i& vector);

	/// Adds addend to this vector.
	void operator += (const Vector4i addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector4i addend);
	/// Internal element division
	void operator /= (const float &f);
	/// Internal element multiplication
	void operator *= (const float &f);

	/// Comparison operators
	bool operator == (const Vector4i other) const;
	/// Comparison operators
	bool operator != (const Vector4i other) const;

	/// Internal element multiplication
	Vector4i operator * (const int &f) const;
	/// Internal element division.
	Vector4i operator / (const int &f) const;

	/// Operator overloading for the array-access operator []
	int & operator [](int index);
	/// Operator overloading for the array-access operator []
	const int & operator [](int index) const;

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector4i ElementMultiplication(const Vector4i otherVector) const;

	/// Calculates the length of the vector.
	float Length() const;
	/// Calculates the squared length of the vector.
	int LengthSquared() const;
	/** Normalizes the vector coordinates so that the length becomes 1 */
	Vector4i Normalize();
	/** Returns a normalized copy of this vector. */
	Vector4i NormalizedCopy() const;

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
	static Vector4i Minimum(const Vector4i & vec1, const Vector4i & vec2);
	static Vector4i Maximum(const Vector4i & vec1, const Vector4i & vec2);

public:
	/// x-coordinate
	int x;
	/// y-coordinate
	int y;
	/// z-coordinate
	int z;
	int w;
};

#endif

