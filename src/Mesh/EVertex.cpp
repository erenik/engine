/// Emil Hedemalm
/// 2014-07-28
/// An editable vertex. Related to the EMesh and other E* classes within the Mesh directory.

#include "EVertex.h"

EVertex::EVertex()
{
	uvCoord = NULL;
}

const EVertex & EVertex::operator = (const Vector3f & assign)
{
	data = assign.data;
	return *this;
}
	