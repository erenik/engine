/// Emil Hedemalm
/// 2014-04-11
/** Data-based filters that generate point- and/or blob-clouds.
*/

#ifndef CV_DATA_FILTERS_H
#define CV_DATA_FILTERS_H

#include "CV/CVFilter.h"
#include "CV/Data/CVData.h"
#include "CV/Data/CVHand.h"

#include "PhysicsLib.h"
#include "PhysicsLib/Shapes/Quad.h"
#include "PhysicsLib/Shapes/Line.h"

class AppWindow;
class Texture;

/// A dedicated texture used for painting contour-segment specific data onto (which is then displayed in a dedicated AppWindow)
extern Texture * contourSegmentRelativeAngleDistanceTexture;

// Render functions, since they are shared by multiple filter sub-classes
void RenderPolygons(CVPipeline * pipe);
void RenderContours(CVPipeline * pipe);
void RenderContourSegments(CVPipeline * pipe);
void RenderContourBounds(CVPipeline * pipe);
void RenderConvexHulls(CVPipeline * pipe);
void RenderConvexityDefects(CVPipeline * pipe);
void RenderHands(CVPipeline * pipe);
void RenderCircles(CVPipeline * pipe);
void RenderOpticalFlow(CVPipeline * pipe);

class CVDataFilter : public CVFilter
{
public:
	// Constructor that sets the filter-type to CVFilter::DATA
	CVDataFilter(int filterID);
	// Ensure that sub-classes may be deallocated correcty.
	virtual ~CVDataFilter();
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);	

};


class CVShiTomasiCorners : public CVDataFilter 
{
public:
	CVShiTomasiCorners();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);

private:
	CVFilterSetting * maxCorners, * qualityLevel, * minimumDistance, * blockSize, * k, * useHarrisDetector;
}; 


/* Old shit. Deprecate.
/// Filter that creates points of various "strengths" depending on what is most white in the image.
class CVLineGatherFilter : public CVDataFilter {
public:
	CVLineGatherFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * minimumInterestpointsPerLine;
};

class CVMaxBoundingBox : public CVDataFilter {
public:
	CVMaxBoundingBox();
	virtual int Process(CVPipeline * pipe);
private:
};
*/


class CVHoughCircles : public CVDataFilter 
{
public:
	CVHoughCircles();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	void Paint(CVPipeline * pipe);
private:
	CVFilterSetting * method, * dp, * minimumDistance, * param1, * param2, * minimumRadius, * maximumRadius;
};

class CVHoughLines : public CVDataFilter 
{
public:
	CVHoughLines();
	virtual ~CVHoughLines();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
//	void Paint(CVPipeline * pipe);
private:
	CVFilterSetting * rho, * theta, * threshold, * minLineLength, * maxLineGap;
	/// Here instead of being statically allocated inside as before, which was causing crashes on exit..
	std::vector<cv::Vec4i> lines;
};

/// Tries to retain only e.g. horizontal lines. Used to discard lines probably not interesting for when merging and finding quads later.
class CVFilterLinesByAngle : public CVDataFilter
{
public:
	CVFilterLinesByAngle();
	virtual int Process(CVPipeline * pipe);
private:
	/// 0 being horizontal. Allowed range being angle in degrees or radians.. probably radians?
	CVFilterSetting * angleToKeep, * allowedRange;
};

// Merges lines produced by e.g. HoughLines by comparing alignment and relative location to each other.
class CVMergeLines : public CVDataFilter 
{
public: 
	CVMergeLines();
	virtual int Process(CVPipeline * pipe);
private:
	/// Pew!
	CVFilterSetting * maxDistance, * minimumAngleRatio;
};

class CVLinePersistence : public CVDataFilter 
{
public:
	CVLinePersistence();
	virtual int Process(CVPipeline * pipe);
private:
	List<List<Line>> previousFramesLines;
	CVFilterSetting * maxDistance, * maxFrames;
};

// Approximates polygons out of contours
class CVApproxPolygons : public CVDataFilter 
{
public:
	CVApproxPolygons();
	virtual int Process(CVPipeline * pipe);
	virtual void Paint(CVPipeline * pipe);
private:
	CVFilterSetting * epsilon, * minimumArea;
	std::vector<cv::Point> approximatedPoly;
};

/// Custom filter to try and detect quads using points as input (e.g. 
class CVFindQuads : public CVDataFilter 
{
public:
	CVFindQuads();
	virtual int Process(CVPipeline * pipe);
private:
	// To filter out bad stuff.
	CVFilterSetting * minimumWidth, * maxLengthDiff, * maxCornerXDiff;
	Vector2f topLeft, topRight, bottomLeft, bottomRight;
	Vector2f topLine, bottomLine;
	bool good;
};

/// Removes/Filters away those quads which do not fit the desired constraints.
class CVQuadAspectRatioConstraint : public CVDataFilter 
{
public:
	CVQuadAspectRatioConstraint();
	virtual int Process(CVPipeline * pipe);
private:
	// To filter out bad stuff.
	CVFilterSetting * minimumRatio, * maximumRatio;
};

/// Filter that looks onto previous frames and tries to find an average Box for which to render. Based on output from FindQuads.
class CVMaxBoxPersistance : public CVDataFilter 
{
public:
	CVMaxBoxPersistance();
	virtual int Process(CVPipeline * pipe);
private:
	/// Main box of the previous frames.
	List<Quad> previousQuads;
	CVFilterSetting * maxFramesToConsider, * threshold;
	long long lastSeenQuad;
};

struct Circle 
{
	float radius;
	Vector2f position;
};

/// Image-analysis class related to circles, time-values to know if it should be kept or not.
class CVCircle 
{
public:
	/// Average of the previous circles. Use for rendering?
	Circle average;
	/// Last time it was seen.
	long long lastTimeSeen;
	/// The circle as it was seen the past X frames.
	List<Circle> previousCircles;
};

class CVCirclesPersistance : public CVDataFilter 
{
public:
	CVCirclesPersistance();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	void Paint(CVPipeline * pipe);
private:
	/// List of data pertaining to the previous circles.
	List<CVCircle> previousCircles;
};

/*
// Copy-pasted stuff from random website and russian's github. Works using angle-comparisons at certain steps along a contour.
class CVHandDetector : public CVDataFilter 
{
public:
	CVHandDetector();
	virtual int Process(CVPipeline * pipe);
private:
	// Checks if two points are considered equal, given set threshold value.
	bool IsEqual(double a, double b);
	// Computes angle between some points on a contour?
	double Angle(std::vector<cv::Point>& contour, int pt, int r);
	// Does.. something.
	signed int Rotation(std::vector<cv::Point>& contour, int pt, int r);
	
	CVFilterSetting * minimumArea, * cosThreshold, * equalThreshold, * r, * step;

};
*/

class CVConvexHull : public CVDataFilter 
{
public:
	CVConvexHull();
	virtual int Process(CVPipeline * pipe);
private:
};

class CVConvexityDefects : public CVDataFilter 
{
public:
	CVConvexityDefects();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * minimumDefectDepth;
	std::vector<cv::Vec4i> convexityDefects;
	List<cv::Vec4i> relevantDefects;	
};

// Detects "gestures" by comparing the latest detected hands to some extent. Only looks at the biggest/most relevant hand for now.
class CVGestureDetection : public CVDataFilter 
{
public:
	CVGestureDetection();
	virtual int Process(CVPipeline * pipe);
private:
	List<CVHand> pastFrames;
};

class CVAccumulate : public CVDataFilter 
{

};


struct FingerState 
{
	FingerState()
	{
		processed = 0;
	};
	int fingers;
	// Positions of the fingers, if needed.
	List<Vector3f> positions;
	int64 duration;
	int64 start;
	int64 stop;
	// Flag to be used by interactive applications. 0 upon creation. Suggest setting to 1 or using binary flags or something.
	int processed;
};

#include "Texture.h"

/// Class which records finger movements, storing them in a list of X recent finger movements in the pipeline.
class CVFingerActionFilter : public CVDataFilter
{
public:
	CVFingerActionFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * minimumDuration, * maxStatesStored;
	int fingersLastFrame;
	List<FingerState> fingerStates;

	// For generatic statistics
	// Save the data to a file?
	Texture * preFilter, * postFilter;
	/// Frames to track.. p-p
	int framesToTrack;
	int filesSaved;
	int framesPassed;

	int64 lastFingerStartTime;
};


// Divides contours into segments.
class CVContourSegmenter : public CVDataFilter 
{
public:
	CVContourSegmenter();
	virtual ~CVContourSegmenter();
	virtual int Process(CVPipeline * pipe);
	/// DOES WHAT?! Scales up somehting so that it becomes renderable on the dedicated texture? To be used for computing?
	void CreateAngleDistancePositionPairs(CVContour & forContour);
private:
	// Relative to the frame's circumference.
	CVFilterSetting * maximumEdgeLength, * showOutputWindow;
	AppWindow * handDataWindow;

};

/// Classifier for contour-segments
class CVContourSegmentClassifier : public CVDataFilter 
{
public:
	CVContourSegmentClassifier();
	virtual int Process(CVPipeline * pipe);
private:
	/// Threshold angles used for considering things as going up or down!
	CVFilterSetting * thresholdAngle, * blurRange;
};

/// Creates point clouds based on chosen input points and some division scheme.
class CVPointClouds : public CVDataFilter 
{
public:
	CVPointClouds();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * input, * divisionScheme;
};


/// Principal component analysis on point clouds
class CVPCA : public CVDataFilter 
{
public:
	CVPCA();
	virtual ~CVPCA();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * stuff;
};

/// Creates point clouds based on chosen input points and some division scheme.
class CVPointCloudPositionalFilter : public CVDataFilter 
{
public:
	CVPointCloudPositionalFilter();
	virtual ~CVPointCloudPositionalFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * filterType, * param1, * vectorParam1;
};

/// Approximates hands based on available point clouds.
class CVHandsFromPointClouds : public CVDataFilter 
{
public:
	CVHandsFromPointClouds();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * minimumPoints, * centerType;
};

class Template 
{
public:
	/// For logical handling.
	String name;
	/// Source file of the image.
	String source;
	/// o.o
	cv::Mat cvImage;
};

class TemplateMatch 
{
public:
	Template * t;
	/// Position in the original image that the match occurred. 
	Vector2i position;
	/// Relative probability, compared to the other approaches?
	float relativeProbability;
	/// Absolute probability as returned by the result using given method.
	float probability;
	/// Method used to calculate probability.
	int method;
};

/// Based on cv::matchTemplate http://docs.opencv.org/modules/imgproc/doc/object_detection.html?highlight=matchtemplate#matchtemplate
class CVTemplateMatcher : public CVDataFilter 
{
public:
	CVTemplateMatcher();
	virtual int Process(CVPipeline * pipe);
private:

	bool LoadTemplates(String fromDir);

	CVFilterSetting * templatesDirectory, * method, * imageFormat;
	List<Template> templates;
	/// Gathered each frame.
	List<TemplateMatch> results;
};

#endif

