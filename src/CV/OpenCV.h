/// Emil Hedemalm
/// 2014-07-27
/// Includes the OpenCV headers.

#ifndef OPEN_CV_H
#define OPEN_CV_H

#include <opencv2/opencv.hpp>

#include "MathLib.h"

cv::Point VectorToCVPoint(Vector3f vec);
cv::Scalar VectorToCVScalarColor(Vector4f vec);

#endif