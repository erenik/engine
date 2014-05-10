/// Emil Hedemalm
/// 2014-04-09
/// Computer vision processing point-class.

#ifndef CV_POINT_H
#define CV_POINT_H

#include "MathLib.h"

// A point-class for computer vision processing.
class CVPoint 
{
public:
	/// Position in the image.
	Vector2i position;
};

#endif