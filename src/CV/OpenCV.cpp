/// Emil Hedemalm
/// 2014-07-30
/// Some general openCV functions.


#include "OpenCV.h"


cv::Point VectorToCVPoint(Vector3f vec)
{
	return cv::Point(vec.x, vec.y);
}

cv::Scalar VectorToCVScalarColor(Vector4f vec)
{
	vec.Clamp(0, 1);
	// Scale to 255.
	vec *= 255.f;
	return cv::Scalar(vec.z, vec.y, vec.x, vec.w);
}
