/// Emil Hedemalm
/// 2014-07-28
/// An editable normal. Related to the EMesh and other E* classes within the Mesh directory.

#ifndef EDITABLE_NORMAL_H
#define EDITABLE_NORMAL_H

#include "List/List.h"

class EVertex;

class ENormal : public Vector3f
{
public:
	const EVertex & operator = (const Vector3f & assign){ x = assign.x; y = assign.y; z = assign.z; return *this;};

};

#endif
