/// Emil Hedemalm
/// 2014-07-28
/// An editable UV-coordinate. Related to the EMesh and other E* classes within the Mesh directory.

#ifndef EDITABLE_UV_COORD_H
#define EDITABLE_UV_COORD_H

class EVertex;
class EFace;

#include "MathLib/Vector2f.h"
#include "List/List.h"

class EUV : public Vector2f 
{
public:
	EUV();
	EUV(const Vector2f & vec);
	EUV(float x, float y);


	/// If the UV coord is associated with only 1 vertex, then save it here.
	EVertex * vertex;

	/// All faces in which this UV-coordinate is included.
	List<EFace*> associatedFaces;
};

#endif
