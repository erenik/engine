///// Emil Hedemalm
/// 2014-08-14
/// Point clouds. Used for e.g. PCA analysis.

#include "CVPointCloud.h"

void CVPointCloud::CreateCVPointsIfNeeded()
{
	if (cvPoints.size() != points.Size())
	{
		cvPoints.clear();
		for (int i = 0; i < points.Size(); ++i)
		{
			Vector2f & vec = points[i];
			cvPoints.push_back(cv::Vec2f(vec.x, vec.y));
		}
	}
}
