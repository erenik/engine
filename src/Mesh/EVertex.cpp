/// Emil Hedemalm
/// 2014-07-28
/// An editable vertex. Related to the EMesh and other E* classes within the Mesh directory.

#include "EVertex.h"

EVertex::EVertex()
{
	uvCoord = NULL;
}

const EVertex & EVertex::operator = (Vector3f assign)
{
	x = assign.x;
	y = assign.y;
	z = assign.z;
	return *this;
}
	