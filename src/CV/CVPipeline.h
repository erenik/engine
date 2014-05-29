/// Emil Hedemalm
/// 2014-04-09
/// OpenCV Pipeline for handling input, filters, calculation-filters (working with points/blobs) and output.

#ifndef CV_PIPELINE_H
#define CV_PIPELINE_H

#include <List/List.h>
#include <String/AEString.h>
#include "PhysicsLib/AxisAlignedBoundingBox.h"

#include "CVData.h"
#include "CVFilter.h"
#include "CVImage.h"
#include "PhysicsLib/Shapes.h"
#include "CVDataFilters.h"

class CVPipeline 
{
public:
	CVPipeline();
	virtual ~CVPipeline();

	/// Save/load functions.
	void WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	/// If needed, for saving/loading, name.
	String name;	

	// Clears all filters, calling OnDelete on them so that they may do proper clean-up.
	void Clear();

	/// Current amount of filters.
	int Filters();
	/// Deletes filter. Returns the now dead pointer (still pointing to the same address-space, for address-comparison) or NULL if it fails/invalid index.
	CVFilter * DeleteFilterByIndex(int index);

	/** Takes input image and processes it through all filters and calc-filters. 
		Returns an index specifying the type of output generated or -1 if an error occured.
	*/
	int Process(cv::Mat * initialInput);

	// o-o
	void PrintProcessingTime();
	
	// Below variables should only be accessed after the processing is complete, not in a separate thread!

	/// Temporary input and output, as set by the filters.
	cv::Mat input, output;
	/// Separate variable for cannyInput/output as it is needed specifically for findContours
//	cv::Mat cannyOutput;
	/// Interesting columns and rows, as determined by filters that return LINES
	List<CVLine> columns, rows;
	List<CVBoundingBox> boxes;
	/// Non-image based output std::vector<cv::Mat> and std::vector<std::vector<cv::Point>> interchangable.
	std::vector<std::vector<cv::Point> > contours;
	//std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> contourHierarchy;
	/// Circles! x, y and radius inside each Vec3f.
	std::vector<cv::Vec3f> circles;
	/// Corners, for e.g. Shi-Tomasi goodFeaturesToTrack
	std::vector<cv::Point2f> corners;
	// Lines, (x1,y1,x2,y2) for each line.
	List<Line> lines;
	/// Hands! o.o
	List<Hand> hands;
	// This assumes just one primary contour.. so use index 0 contour for it if so..?
	std::vector<int> convexHull;
	// Convexity defects calculated by contour's convex hull vs. the contour itself.
	List<cv::Vec4i> convexityDefects;
	
	/// List of previous finger states. The most recent one will be stored at the end, using .Last()
	List<FingerState> fingerStates;

	std::vector<std::vector<cv::Point> > approximatedPolygons;

	/// Quads found with e.g. custom FindQuads filter.
	List<Quad> quads;

	// Channels of relevance.
	cv::Mat hue, value, saturation;

	/// For motion-detection/filter. 
	cv::Mat motionHistoryImage;

	/// Initial input, as set by the calling function.
	cv::Mat * initialInput;
	List<CVFilter*> filters;
	
	List<AxisAlignedBoundingBox*> boundingBoxes;
	List<CVPoint*> points;
	List<CVBlob*> blobs;

	/// Fetches last error string, which is set if the Process function returns -1 (no result).
	String GetLastError() {return errorString;};

	/// Time the pipeline took from start to finish.
	int pipelineTimeConsumption;

	/// Last return type. Used to render appropriately.
	int returnType;


	/// Performance testing variables.
	int totalFilterProcessingTimeThisFrame;
	List<int64*> totalProcessingTimes;
	int iterations;
private:
	String errorString;
};

#endif
