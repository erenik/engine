/// Emil Hedemalm
/// 2014-04-11
/** Data classes for handling image analysis.
*/

#ifndef CV_DATA_H
#define CV_DATA_H

// #include "CVFilter.h"
#include "MathLib.h"


/// Struct for own algorithm that just looks on a per-line basis. <- wat.
/*
struct CVLine 
{
public:
	int number;
	int interestPoints;
};
*/

struct CVBoundingBox 
{
	Vector2i min, max;
};


#endif
