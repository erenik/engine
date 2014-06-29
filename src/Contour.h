/// Emil Hedemalm
/// 2014-06-26
/// An arbitrary contour. 

#ifndef CONTOUR_H
#define CONTOUR_H

#include "MathLib.h"
#include "List/List.h"

class Contour 
{
public:
	List<Vector3f> points;
	/// Size.
	float area;
	Vector3f centerOfMass;
};

#endif
