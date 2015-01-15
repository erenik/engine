/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#ifndef OPTICAL_FLOW_POINT_H
#define OPTICAL_FLOW_POINT_H

#include "MathLib.h"

struct OpticalFlowPoint 
{
	Vector2f position;
	// Position in the previous frame.
	Vector2f previousPosition;
	// Compared to last frame/initial detection.
	Vector2f offset;
};

#endif
