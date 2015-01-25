// WRITTEN BY: Emil Hedemalm
// DATE: 2012-10-03
//
// FILE: Vector3d.h
// CLASS PROVIDED: Vector3d  (a three-dimensional vector class using doubles)

#ifndef Vector3d_H
#define Vector3d_H

#include <cmath>
#include "Vector4d.h"
#include "Vertex3d.h"
#include "Vector3f.h"

class Vector3f;
class Vector4d;

/** A three-dimensional vector class using doubles.
*/
class Vector3d {

public:	
	// CONSTRUCTORS
	/**	Default Constructor
		Postcondition: Initializes a NULL-3D-vector, having all variables (x, y, z) set to 0.
	*/
	Vector3d();			
	/**	Constructor
		Postcondition: Initializes a 3D vector to specified values.
	*/
	Vector3d(double x,  double y,  double z);
	/**	Constructor
		Postcondition: Initializes a 3D vector using v1 as start point and v2 as end point.
	*/
//	Vector3d(Vertex3d v1, Vertex3d v2);
	/**	Constructor
		Precondition: The arr-parameter has to be an at least 3 indices long array.
		Postcondition: Initializes a 3D vector to specified array values. 
	*/
	Vector3d(double arr[]);
	/**	Copy Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3d(const Vector3d& base);
	/**	Copy Conversion Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector.
	*/
	Vector3d(const Vector3f& base);
	/**	Conversion Constructor
		Postcondition: Initializes a 3D vector to have same values as the referenced vector. The w-value is discarded.
	*/
	Vector3d(const Vector4d& base);


	// Simple arithmetics
	/** Simple addition
		Postcondition: Adds the addend's coordinates to the calling vector's.
	*/
	void add(const Vector3d addend);
	/** Simple subtraction
		Postcondition: Subtracts the subtractor's coordinates from the calling vector's.
	*/
	void subtract(const Vector3d subtractor);
	/** Simple scaling
		Postcondition: Scales the vector's coordinates by the provided ratio.
	*/
	void scale(double ratio);
	/** Specific scaling
		Postcondition: Scales the vector's coordinates with the provided arguments.
	*/
	void scale(double x, double y, double z);

	// Operator overloading for the addition and subtraction operations
	/// Returns a summed vector based on this vector and the addend.
	Vector3d  operator + (const Vector3d addend) const;
	/// Returns a subtracted vector based on this vector and the subtractor.
	Vector3d  operator - (const Vector3d subtractor) const;
	/// Adds addend to this vector.
	void operator += (const Vector3d addend);
	/// Subtracts subtractor from this vector.
	void operator -= (const Vector3d subtractor);

	/// Internal element multiplication
	Vector3d operator * (const double &d) const;

	/// Operator overloading for the array-access operator []
	double & operator [](int index);
	/// Operator overloading for the array-access operator []
	const double operator [](int index) const;

	// Vector products
	/** Dot product
		Postcondition: Calculates the dot product of the vector. 
	*/
	double DotProduct(const Vector3d otherVector) const;	 
	
	/** 3D Cross product
		Postcondition: Calculates the cross product between the vectors. Returns a Vector3d.	*/
	Vector3d  CrossProduct(const Vector3d otherVector) const;

	/** Calculates the length of the vector. */
	double Length();
	/** Normalizes the vector coordinates so that the length becomes 1 */
	void Normalize();
	
	/// Returns the vector's x-coordinate.
	const double GetX() const {return x;};
	/// Returns the vector's y-coordinate.
	const double GetY() const {return y;};
	/// Returns the vector's z-coordinate.
	const double GetZ() const {return z;};

	/// x-coordinate
	double x;
	/// y-coordinate
	double y;
	/// z-coordinate
	double z;
public:
};

#endif