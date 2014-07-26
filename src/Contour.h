/// Emil Hedemalm
/// 2014-06-26
/// An arbitrary contour. 

#ifndef CONTOUR_H
#define CONTOUR_H

#include "MathLib.h"
#include "List/CircularList.h"

class Contour 
{
public:
	CircularList<Vector3f> points;
	/// Size.
	float area;
	/// Center of mass assuming the contour makes up a solid 2D body.
	Vector3f centerOfMass;
	/// Raw average of all points.
	Vector3f averageContourVertexPosition;
};

#endif
