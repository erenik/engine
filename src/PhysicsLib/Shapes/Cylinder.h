/// Emil Hedemalm
/// 2014-08-06
/// Simple shapes

#include "MathLib.h"

class Cylinder 
{
public:
	Cylinder() { radius = 1.0f; length = 1.0f; };
	Vector3f position;
	Vector3f rotation;
	float radius;
	float length;
};

