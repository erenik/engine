/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#include "OpticalFlow.h"


// Generates a radial color based on 2-directional vector. The coordinates should be in world-space and normalized.
Vector4f GetColorForDirection(Vector2f dir)
{
	Vector2f up(0,1), bottomLeft(-0.86602540378f,-0.5f), bottomRight(0.86602540378f, -0.5f);
	Vector4f color;
	color.x = dir.DotProduct(bottomRight);
	color.y = dir.DotProduct(up);
	color.z = dir.DotProduct(bottomLeft);
	color.w = 1.f;
	return color;
}

OpticalFlow::OpticalFlow()
{

}

OpticalFlow::~OpticalFlow()
{
	DeleteQuadrants();
}

void OpticalFlow::SetSize(Vector2i size)
{
	// Deallocate if needed..
	DeleteQuadrants();
	Matrix::SetSize(size);
	// De-allocate the flow quadrants as needed.
	for (int i = 0; i < arrLength; ++i)
	{
		arr[i] = new OpticalFlowQuadrant();
	}
}

// Clears all points from the flow grid.
void OpticalFlow::ClearPoints()
{
	// De-allocate the flow quadrants as needed.
	for (int i = 0; i < arrLength; ++i)
	{
		arr[i]->flowPoints.Clear();
		arr[i]->averageFlow = Vector3f();
	}
}	

void OpticalFlow::Add(List<OpticalFlowPoint> points)
{
	 /// o-o
	 for (int i = 0; i < points.Size(); ++i)
	 {
		 OpticalFlowPoint & point = points[i];
		 for (int j = 0; j < arrLength; ++j)
		 {
			 OpticalFlowQuadrant * quad = arr[j];
			 if (point.position.IsWithinMinMax(quad->min, quad->max))
			 {
				 quad->flowPoints.Add(point);
			 }
		 }
	 }
}

void OpticalFlow::DeleteQuadrants()
{
	// De-allocate the flow quadrants as needed.
	for (int i = 0; i < arrLength; ++i)
	{
		if (arr[i])
			delete arr[i];
		arr[i] = NULL;
	}
}

