// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector4f.h
// CLASS PROVIDED: Vector4f  (a four-dimensional vector class using floats)

#ifndef Vector4f_H
#define Vector4f_H

#include <cmath>
#include "Vector3f.h"
#include "Vector4d.h"

class Vector3f;
class Vector4d;

/** A four-dimensional vector class using floats.
*/
class Vector4f {

public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-4D-vector, having x, y & z set to 0 and w to 1.
	*/
	Vector4f();
	/**	Constructor
		Postcondition: Initializes a 4D vector to specified values.
	*/
	Vector4f(float x,  float y,  float z,  float w);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 4 indices long array.
		Postcondition: Initializes a 4D vector to specified array values.
	*/
	Vector4f(float arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 4D vector to have same values as the referenced vector.
	*/
	Vector4f(const Vector4f & base);
	/**	Copy Conversion Constructor
		Postcondition: Initializes a 4D vector to have same values as the referenced vector.
	*/
	Vector4f(const Vector4d & base);
	/**	Conversion Constructor
		Postcondition: Initializes a 4D vector to have same values as the referenced vector. w defaults to 1 unless the second parameter is sent.
	*/
	Vector4f(const Vector3f & base, float w = 1);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector3f& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector4f addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector4f subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(float ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(float x, float y, float z, float w);

	/** Scales the XYZ parts. */
	void MultiplyXYZ(float multiplier);

	// Unary - operator, switch signs of all sub-elements except w.
	Vector4f operator - () const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector4f  operator + (const Vector4f addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector4f  operator - (const Vector4f subtractor) const;

	/// Multiplication with float
	friend Vector4f operator * (float multiplier, const Vector4f& vector);

	/// Internal element multiplication
	Vector3f operator * (const float &f) const;
	/// Internal element division.
	Vector3f operator / (const float &f) const;

	/// Adds addend to this vector.
	void operator += (const Vector4f addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector4f addend);
	/// Multiplicator o-o
	void operator *= (const float floatur);

	/// Conversion equal-conversion operator
	Vector4f& operator = (const Vector4d &other);
	/// Conversion equal-conversion operator
	Vector4f& operator = (const Vector4f &other);

	/// Operator overloading for the array-access operator []
	float& operator[] (const unsigned int index);

	// Vector products
	/** Scalar product
		Postcondition: Calculates the scalar product of the vector. Same result as calling dotProduct()
	*/
	float ScalarProduct(const Vector4f otherVector);
	/** Dot product
		Postcondition: Calculates the dot product of the vector. Same result as calling scalarProduct()
	*/
	float DotProduct(const Vector4f otherVector);

	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors as if they were both Vector3fs. Returns a Vector3f.	*/
	Vector3f CrossProduct(const Vector3f otherVector);
	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors as if they were both Vector3fs. Returns a Vector3f.	*/
	Vector3f CrossProduct(const Vector4f otherVector);

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector4f ElementMultiplication(const Vector4f otherVector) const;

	/** Calculates the length of the vector, considering only {x y z}. */
	float Length3() const;
	/** Normalizes the vector coordinates so that the length of {x y z} becomes 1 */
	void Normalize3();
    /// Returns a normalized (x,y,z) copy of the given vector.
	Vector4f NormalizedCopy() const;

	/// Returns the vector's x-coordinate.
	const float GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const float GetY() const {return y;};
	/// Returns the vector's z-coordinate.
	const float GetZ() const {return z;};
	/// Returns the vector's w-coordinate.
	const float GetW() const {return w;};

public:
	/// x-coordinate
	float x;
	/// y-coordinate
	float y;
	/// z-coordinate
	float z;
	/// w-coordinate
	float w;
};

#endif
