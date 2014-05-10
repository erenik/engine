/// Emil Hedemalm
/// 2014-04-09
/// OpenCV Pipeline for handling input, filters, calculation-filters (working with points/blobs) and output.

#ifndef CV_IMAGE_H
#define CV_IMAGE_H

#include <opencv2/opencv.hpp>

/** Class for embedding an OpenCV image within. Contains some minor conversion functions as well.
	Stores a copy to the original image upon initial loading if needed for later. 
*/
class CVImage 
{
public:
	/// CV image data matrix
	cv::Mat mat;
	/// Source file from which this image was loaded.
	String source;
};


#endif
