/// Emil Hedemalm
/// 2014-08-14
/// Point clouds. Used for e.g. PCA analysis.

#ifndef CV_POINT_CLOUD_H
#define CV_POINT_CLOUD_H

#include "List/List.h"
#include "MathLib.h"
#include <vector>
#include <opencv2/opencv.hpp>

class CVPointCloud 
{
public:

	void CreateCVPointsIfNeeded();

	/// CV points
	std::vector<cv::Point2f> cvPoints;
	/// Own points for matrix transformations.
	List<Vector2f> points;
		

	Vector2f pcaCenter;
	List<Vector2f> eigenVectors;
	List<float> eigenValues;
};

#endif

