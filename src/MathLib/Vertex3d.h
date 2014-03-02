// WRITTEN BY: Emil Hedemalm
// DATE: 2012-09-26
//
// FILE: Vertex3d.h
// CLASS PROVIDED: Vertex3d  (a three-dimensional vertex class)

#ifndef Vertex3d_H
#define Vertex3d_H

/** A three-dimensional vertex class using doubles.
*/
class Vertex3d {

public:	
	/** Default constructor
		Postcondition: Initializes a 3D vertex. Defaults the coordinates to 0 unless the parameters are specified.
	*/
	Vertex3d(double x = 0, double y = 0, double z = 0);

public:
	/// x-coordinate
	double x;
	/// y-coordinate
	double y;
	/// z-coordinate
	double z;
};


#endif