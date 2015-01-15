/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#include "OpticalFlowQuadrant.h"

void OpticalFlowQuadrant::CalculateFlow()
{
	averageFlow = Vector3f();
	for (int i = 0; i < flowPoints.Size(); ++i)
	{
		OpticalFlowPoint & point = flowPoints[i];
		averageFlow += point.offset;
	}
	if (flowPoints.Size())
		averageFlow /= flowPoints.Size();

	/// Calculate center too.
	center = (max + min) * 0.5f;
}
