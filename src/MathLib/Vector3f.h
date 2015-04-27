// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector3f.h
// CLASS PROVIDED: Vector3f  (a three-dimensional vector class using floats)

#ifndef Vector3f_H
#define Vector3f_H

#include "AEMath.h"
#include <iostream>
#include <cmath>
#include "List/List.h"
#include "Vector4f.h"
#include "Vertex3f.h"
#include "System/Memory.h"

class Vector2i;
class Vector2f;
class Vector3i;
class Vector3d;
class Vector4f;
class Vector4d;
class Matrix4f;
class String;

#include "SSE.h" // Include our SSE macros where Vector is included, since those classes may require byte-alignment too, then.

#define ConstVec3fr const Vector3f & 

/** A three-dimensional vector class using floats.
*/
#ifdef USE_SSE
Align(16)
#endif
class Vector3f {
public:
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector3f();
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector3f(float x,  float y,  float z = 0);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values.
	*/
	Vector3f(float arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3f(const Vector2i& base, float z = 0);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3f(const Vector2f& base, float z = 0);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3f(const Vector3f& base);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3f(const Vector3d& base);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3f(const Vector3i& base);
	/**	Conversion Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector. The w-value is discarded.
	*/
	Vector3f(const Vector4f& base);
	/**	Conversion Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector. The w-value is discarded.
	*/
	Vector3f(const Vector4d& base);

	/// Virtual destructor so sub-classes get de-allocated appropriately.
	virtual ~Vector3f();

	/// o.o Create Vectors!
	static List<Vector3f> FromFloatList(List<float> floatList, int numMatricesToExtract);

	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const Vector3f& vec);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	
	/// Reads from file stream. 
	void ReadFrom(std::fstream & file);
	/// Reads from String. Expects space- or comma-separated values. E.g. 3 8.14 -15. To use another separator, specify the tokenizer. 
	void ReadFrom(const String & string, const char * tokenizer = " ,");
	/// Parses from string. Expects in the form of first declaring order "XY", "X Y" or "YX", then parses the space-separated values.
	void ParseFrom(const String & string);

	/// Returns abs-version.
	Vector3f Abs() const;
	/// Clamp to an interval.
	void Clamp(float min, float max);
	void Clamp(const Vector3f & min, const Vector3f & max);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector3f & addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector3f & subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(float ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(float x, float y, float z);

	/// o.o
	bool operator == (ConstVec3fr comparand) const;
	/// Comparison operators
	bool operator != (const Vector3f & comparand) const;
	/// This will return true if and only if all three components (x,y,z) are smaller than their corresponding comparands in the vector comparand.
	bool operator < (const Vector3f & comparand) const;
	/// This will return true if and only if all three components (x,y,z) are larger than their corresponding comparands in the vector comparand.
	bool operator > (const Vector3f & comparand) const;

	// Unary - operator (switch signs of all sub-elements)
	Vector3f operator - () const;

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector3f  operator + (const Vector3f & addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector3f  operator - (const Vector3f & subtractor) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector3f  operator * (const Vector3f & elementMultiplier) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector3f  operator / (const Vector3i & elementDivider) const;

	/// Multiplication with floats
	friend Vector3f operator * (float multiplier, const Vector3f & vector);

	/// Adds addend to this vector.
	void operator += (const Vector3f & addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector3f & addend);
	/// Internal element division
	void operator /= (const float &f);
	/// Internal element multiplication
	void operator *= (const float &f);
	/// Internal element multiplication
	void operator *= (const Vector3f &f);
	/// Internal element multiplication
	void operator *= (const Matrix4f &mat);

	/// Internal element multiplication
	Vector3f operator * (const float &f) const;
/// Internal element division.
	Vector3f operator / (const float &f) const;

	/// Operator overloading for the array-access operator []
	float & operator [](int index);
	/// Operator overloading for the array-access operator []
	const float & operator [](int index) const;

	// Vector products
	/** Scalar product
		Postcondition: Calculates the scalar product of the vector. Same result as calling dotProduct()
	*/
	float ScalarProduct(const Vector3f & otherVector) const;
	/** Dot product
		Postcondition: Calculates the dot product of the vector. Same result as calling scalarProduct()
	*/
	float DotProduct(const Vector3f & otherVector) const;

	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors. Returns a Vector3f.	*/
	Vector3f  CrossProduct(const Vector3f & otherVector) const;

	/// Multiplies the elements in the two vectors internally, returning the product.
	Vector3f ElementMultiplication(const Vector3f & otherVector) const;
	Vector3f ElementDivision(const Vector3f & otherVector) const;

	/// Calculates the length of the vector.
	float Length() const;
	/// Calculates the squared length of the vector.
	float LengthSquared() const;
	/** Normalizes the vector coordinates so that the length becomes 1 */
	void Normalize();
	/** Returns a normalized copy of this vector. */
	Vector3f NormalizedCopy() const;

	/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
	const float MaxPart() const;
	/// Returns the absolute value of the sub-component (x,y,z) of highest absolute value.
	const float MinPart() const;
	/// Returns the absolute value of the sub-component (x,y,z) of lowest absolute value.
	const float MinPartAbs() const;
	/// Utility functions
	static Vector3f Minimum(const Vector3f & vec1, const Vector3f & vec2);
	static Vector3f Maximum(const Vector3f & vec1, const Vector3f & vec2);

	// Rounds to nearest digit!
	void Round();
	/// Returns a copy with rounded values for each axis.
	Vector3i Rounded();
public:
#ifdef USE_SSE
	// Loads data into __m128 structure.
	void PrepareForSIMD();
	
	// Should be able to place in class!
	union {
		struct { 
			float x, y, z, w; 
		};
		__m128 data;
		float v[4];
	};

//	__m128 data;
	// If using SSE, don't use the standard XYX, use only the internal floats of the __m128 struct, preferably using the array index operator []
#else
	/// x-coordinate
	float x;
	/// y-coordinate
	float y;
	/// z-coordinate
	float z;
#endif
};

#endif
