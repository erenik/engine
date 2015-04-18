/// Emil Hedemalm
/// 2014-06-27
/// All filters concerning hands and finger detection.

#include "CVDataFilters.h"

class AppWindow;

class CVHandEmulator : public CVDataFilter 
{
public:
	/// Emulates one or more hands to be displayed on the screen.
	CVHandEmulator();
	virtual int Process(CVPipeline * pipe);
private:
	// Number of hands and relative size (0 to 1.0?)
	CVFilterSetting * numHands, * size;
};

// Own custom hand-detector, identifying fingers by first identifying relevant convexity defects (convex hull - contour)
class CVHandDetector : public CVDataFilter 
{
public:
	CVHandDetector();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * minimumArea, * minimumFingerDistance;
};

/** Filter which tries to persist some select data from hands detected in the past couple of frames (up to some max frames)
	Re-vamp of the old filter of same name which hasn't been used in a while.
*/
class CVHandPersistance : public CVDataFilter 
{
public:
	CVHandPersistance();
	virtual int Process(CVPipeline * pipe);
private:
	List<List<CVHand> > pastFramesHands;
	
	/// Max frames to look back at.
	CVFilterSetting * maxFrames, 
		// For smoothing velocity over X frames.
		* velocitySmoothingFactor,
		// Threshold for identifying this hand as one detected in the last frame. Default 1.0
		* areaThresh, 
		// Threshold for identifying this hand as one detected in the last frame. Compared to overall size of the contour. Default 1.0.
		* distanceThresh
		;
};


/// New finger extraction filter which works by comparing the contour co-ordinates to its center, creating a histogram based off of it.
class CVFingerExtractionFilter : public CVDataFilter
{
public:
	CVFingerExtractionFilter();
	virtual int Process(CVPipeline * pipe);
private:
	// Parse fingers based on the angle-distance-position pairs.
	void ParseFingers(CVHand & inHand);
	/// First texture painted to is in raw image co-ordinates, while the second one is in relative angle/distance coordinates .
	void PaintOutput(cv::Mat * pipelineTexture, Texture * relativeTexture);

	/// Scale used to multiply the normalized angles [0,1] in order to compare the properly with the variances in relative height.
	CVFilterSetting * xScale;
	/// Because filtering.
	CVFilterSetting * minimumFingerArea;
	/// Before change..
	CVFilterSetting * minimumClimbs;
//	CVFilterSetting * minimumFingerCurvature, * maximumFingerArea;

	// More filtering..
	CVFilterSetting * minParallelSegments, * parallelMinimum;

	CVHand * hand;
};


/** Filter which takes the output hand-data from both the convexity defects approach and the 
	contour segmentation and relative angle/position approach, yielding those fingers which 
	were detected in both approaches.

	It also tries to re-evaluate which finger is which, based on relative location, angle and size.
*/
class CVFingerIdentificationFilter : public CVDataFilter
{
public:
	CVFingerIdentificationFilter();
	virtual int Process(CVPipeline * pipe);
private:
	/// Attempts to determine the type of all fingers based on the fingers that were present in the last frame.
	void MatchFingersWithOldFingers(CVHand * inHand, CVHand * comparedToOldHand);

	// Add settings for the old-finger matcher?s

	CVFilterSetting * thumbToOtherFingersThreshold;
};

/// Filter which tries to associate optical flow quadrants with the existing hands/hand contours,
/// then approximate the velocity of the hand from those detected quadrants.
class CVApproximateHandVelocityByOpticalFlow : public CVDataFilter 
{
public:
	CVApproximateHandVelocityByOpticalFlow();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);	

private:
	/// Paint the filtered optical flow quadrants perhaps, or the direction of the hands.
	CVFilterSetting * toPaint, * checkType, * distance;
};

class CVFilterHandsByMovement : public CVDataFilter 
{
public:
	CVFilterHandsByMovement();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * type, * thresh;
};

