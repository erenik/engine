// WRITTEN BY: Emil Hedemalm
// DATE: 2012-09-26
//
// FILE: Vertex3f.h
// CLASS PROVIDED: Vertex3f  (a three-dimensional vertex class)

#ifndef Vertex3f_H
#define Vertex3f_H

/** A three-dimensional vertex class using floats.
*/
struct Vertex3f {

public:	
	/** Default constructor
		Postcondition: Initializes a 3D vertex. Defaults the coordinates to 0 unless the parameters are specified.
	*/
	Vertex3f(float x = 0, float y = 0, float z = 0);

public:
	/// x-coordinate
	float x;
	/// y-coordinate
	float y;
	/// z-coordinate
	float z;
};


#endif