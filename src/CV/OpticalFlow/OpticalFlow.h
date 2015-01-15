/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#ifndef OPTICAL_FLOW_H
#define OPTICAL_FLOW_H

#include "OpticalFlowQuadrant.h"
#include "Matrix/Matrix.h"

// Generates a radial color based on 2-directional vector. The coordinates should be in world-space and normalized.
Vector4f GetColorForDirection(Vector2f dir);

class OpticalFlow : public Matrix<OpticalFlowQuadrant*>
{
public:
	OpticalFlow();
	~OpticalFlow();
	virtual void SetSize(Vector2i size);
	// Clears all points from the flow grid.
	void ClearPoints();
	// Adds points to the flow's quadrants.
	void Add(List<OpticalFlowPoint> points);
private:
	void DeleteQuadrants();
};

#endif
