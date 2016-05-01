/// Emil Hedemalm
/// 2014-07-28
/// An editable vertex. Related to the EMesh and other E* classes within the Mesh directory.

#ifndef EDITABLE_VERTEX_H
#define EDITABLE_VERTEX_H

#include "MathLib/Vector3f.h"
#include "List/List.h"

class EFace;
class EUV;

// Lol, just sub-class the vector3f to get x,y,z straight away? :D
class EVertex : public Vector3f
{
public:
	EVertex();
	EVertex(ConstVec3fr vec);
	const EVertex & operator = (const Vector3f & assign);
	/// o-o
//	Vector3f position;
	/// Faces this vertex is part of.
	List<EFace*> faces;

	/// If this vertex is associated with one unique uv-coordinate (non-per-face UV-coordinates), then save it here.
	EUV * uvCoord;
};

#endif
