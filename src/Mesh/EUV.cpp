/// Emil Hedemalm
/// 2014-07-28
/// An editable UV-coordinate. Related to the EMesh and other E* classes within the Mesh directory.

#include "EUV.h"

EUV::EUV()
{
	vertex = NULL;
}

EUV::EUV(const Vector2f & vec)
{
	x = vec.x;
	y = vec.y;
}

EUV::EUV(float ix, float iy)
{
	vertex = 0;
	x = ix;
	y = iy;
}

