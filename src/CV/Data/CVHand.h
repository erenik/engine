/// Emil Hedemalm
/// 2014-06-27
/// Computer Vision CVHand class.

#ifndef CV_HAND_H
#define CV_HAND_H

#include "CVContour.h"
#include "CVFinger.h"

class OpticalFlow;
class OpticalFlowQuadrant;

class CVHand
{
public:
	/// Contour it's based on.
	CVHand();
	CVHand(CVContour * contour);
	virtual ~CVHand();

	// .
	List<OpticalFlowQuadrant*> GetQuadrantsWithinContour(OpticalFlow * fromOpticalFlow, float byDistanceToCenter = 100.f);
	/// If fromSegments is true, it will compare using the available contour segments instead of all contour points.
	List<OpticalFlowQuadrant*> GetQuadrantsWithinContour(OpticalFlow * fromOpticalFlow, bool fromSegments, Vector3f byPointInContourComparisonUsingGivenVector = Vector3f(0,1,0));

	/** Returns 0 if less than 5 fingers were found, in which case some custom matching algorithm should be run based on the last analysis. 
		thumbToOtherFingersThreshold defines how much the dot product of the thumb must differ from the nearest finger (in terms of dot product) compared to the general direction of all fingers.
	*/
	int IdentifyFingers(float thumbToOtherFingerThreshold = 0.2f);

	// Add functions?

	/// If true, should not be considered for applications. Set during filters, e.g. the finger identification when the finger count exceeds 5.
	bool bad;


	/// Gathered from optical flow analysis.
	Vector3f approximateVelocityFromOpticalFlow;

	/// Similar to the averaged velocity, but does not care about direction.
	float averagedVelocityMagnitude;

	float averageFingerLength;

	// Data.
	List<CVFinger> fingers;
	// In raw-image co-ordinates?
	Vector3f center;
	// Associated contour. 
	CVContour * contour;

//	std::vector<cv::Point> contour;
	// Bounding rectangle p-p
	cv::Rect boundingRect;
	// Area in pixels squared?
	float contourAreaSize;

	/// Smoothed bounding rectangle limits/size.
	Vector3f min, max, size;
};

#endif