/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#ifndef OPTICAL_FLOW_QUADRANT_H
#define OPTICAL_FLOW_QUADRANT_H

#include "OpticalFlowPoint.h"
#include "List/List.h"

// Create a grid.. lol
class OpticalFlowQuadrant 
{ 
public:
	void CalculateFlow();
	// Limits.
	Vector3f min, max, center;
	// Average, not normalized.
	Vector3f averageFlow;
	// Copy of all points within this quadrant.
	List<OpticalFlowPoint> flowPoints;
};

#endif
