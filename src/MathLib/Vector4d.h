// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector4d.h
// CLASS PROVIDED: Vector4d  (a four-dimensional vector class using doubles)

#ifndef Vector4d_H
#define Vector4d_H

#include <cmath>
#include "Vector3d.h"
#include "Vector4f.h"

class Vector3d;
class Vector4f;

/** A four-dimensional vector class using doubles.
*/
class Vector4d {
	friend class Vector4f;
public:	
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-4D-vector, having all variables (x, y, z, w) set to 0.
	*/
	Vector4d();			
	/**	Constructor
		Postcondition: Initializes a 4D vector to specified values.
	*/
	Vector4d(double x,  double y,  double z,  double w);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 4 indices long array.
		Postcondition: Initializes a 4D vector to specified array values.
	*/
	Vector4d(double arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 4D vector to have same values as the referenced vector.
	*/
	Vector4d(const Vector4d & base);
	/**	Conversion Constructor
		Postcondition: Initializes a 4D vector to have same values as the referenced vector. w defaults to 0 unless the second parameter is sent.
	*/
	Vector4d(const Vector3d & base, double w = 0);

	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector4d addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector4d subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(double ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(double x, double y, double z, double w);

	// Unary - operator (switch signs of all sub-elements)
	Vector4d operator - () const;


	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector4d  operator + (const Vector4d addend);
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector4d  operator - (const Vector4d subtractor);
	/// Adds addend to this vector.
	void operator += (const Vector4d addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector4d addend);

	/// Internal element multiplication
	Vector4d operator * (const float &f) const;

	/// Operator overloading for the array-access operator []
	double& operator[] (const unsigned int index);

	// Vector products
	/** Scalar product
		Postcondition: Calculates the scalar product of the vector. Same result as calling dotProduct()
	*/
	double ScalarProduct(const Vector4d otherVector);
	/** Dot product
		Postcondition: Calculates the dot product of the vector. Same result as calling scalarProduct()
	*/
	double DotProduct(const Vector4d otherVector);	 

	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors as if they were both Vector3ds. Returns a Vector3d.	*/
	Vector3d CrossProduct(const Vector3d otherVector);
	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors as if they were both Vector3ds. Returns a Vector3d.	*/
	Vector3d CrossProduct(const Vector4d otherVector);

	/** Calculates the length of the vector, considering only {x y z}. */
	double Length3() const;
	/** Normalizes the vector coordinates so that the length of {x y z} becomes 1 */
	void Normalize3();
	/// Returns a normalized (xyz) copy of the current vector.
	Vector4d NormalizedCopy() const;

	/// Returns the vector's x-coordinate.
	const double GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const double GetY() const {return y;};
	/// Returns the vector's z-coordinate.
	const double GetZ() const {return z;};
	/// Returns the vector's w-coordinate.
	const double GetW() const {return w;};

	
	/// x-coordinate
	double x;
	/// y-coordinate
	double y;
	/// z-coordinate
	double z;
	/// w-coordinate
	double w;
public:
};

#endif