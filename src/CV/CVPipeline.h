/// Emil Hedemalm
/// 2014-04-09
/// OpenCV Pipeline for handling input, filters, calculation-filters (working with points/blobs) and output.

#ifndef CV_PIPELINE_H
#define CV_PIPELINE_H

#include "Data/CVPoint.h"
#include <List/List.h>
#include <String/AEString.h>
// #include "PhysicsLib/Shapes/AABB.h"

#include "Data/CVData.h"
#include "Data/CVPointCloud.h"
#include "OpticalFlow/OpticalFlow.h"
#include "CVFilter.h"
#include "CVImage.h"
#include "DataFilters/CVDataFilters.h"

#include "PhysicsLib/Shapes/Line.h"

class AppWindow;

#define PIPELINE_CONFIG_FILE_ENDING ".pcfg"

class CVPipeline 
{
public:
	CVPipeline();
	virtual ~CVPipeline();

	/// Opens up an editor-AppWindow for this CVPipeline, assuming the existance of PipelineEditor GUI file. (gui/PipelineEditor.gui)
	AppWindow * OpenEditorWindow();
	/// Call every time after changing the pipeline. Updates the UI in the dedicated winodw.
	void OnPipelineUpdated();
	void OpenFilterSelectionMenu();
	/// Update filters available in the filter-selection menu (for appending/inserting new filters!)
	void OnFilterTypesUpdated();
	/// Loads target pipeline config, sends a message that it has been loaded upon completion.
	void LoadPipelineConfig(String fromFile);
	/// Updates the text in the Enable/Disable button
	void OnFilterEnabledUpdated(CVFilter * filter);
	
	/// Selects the filter, pushing it's ui onto the ui dedicated to handling filter edits.
	void SelectFilter(int byIndex);
	/// Returns the filter currently being edited.
	CVFilter * CurrentEditFilter(){return currentEditFilter;};


	/// Passes on the message to all filters.
	void ProcessMessage(Message * message);

	/// Save/load functions.
	void WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	/// If needed, for saving/loading, name.
	String name;	

	void Swap(int filterAtIndex, int withFilterAtIndex);
	void InsertFilter(CVFilter * newFilter, int atIndex);
	void AppendFilter(CVFilter * newFilter);

	// o-o
	void SetInitialInput(cv::Mat & mat);

	// Clears all filters, calling OnDelete on them so that they may do proper clean-up.
	void Clear();

	/// Current amount of filters.
	int NumFilters();
	List<CVFilter*> Filters();

	/// Deletes filter. Returns the now dead pointer (still pointing to the same address-space, for address-comparison) or NULL if it fails/invalid index.
	CVFilter * DeleteFilterByIndex(int index);

	/** Takes input image and processes it through all filters and calc-filters. 
		Returns an index specifying the type of output generated or -1 if an error occured.
	*/
	int Process();

	// o-o
	void PrintProcessingTime();
	
	// Below variables should only be accessed after the processing is complete, not in a separate thread!

	// Oh yeah. Full image flow. Co-ordinates probably in CV-space.
	OpticalFlow opticalFlow;


	/// Temporary input and output, as set by the filters.
	cv::Mat input, output;
	/// Separate variable for cannyInput/output as it is needed specifically for findContours
//	cv::Mat cannyOutput;
	/// Interesting columns and rows, as determined by filters that return LINES
//	List<CVLine> columns, rows;
	List<CVBoundingBox> boxes;
	
	/// Non-image based output std::vector<cv::Mat> and std::vector<std::vector<cv::Point>> interchangable.
	std::vector<std::vector<cv::Point> > cvContours;

	// List of contours as I want to structure them.
	List<CVContour> contours;

	//std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> contourHierarchy;

	/// Circles! x, y and radius inside each Vec3f.
	std::vector<cv::Vec3f> circles;
	/// Corners, for e.g. Shi-Tomasi goodFeaturesToTrack
	std::vector<cv::Point2f> corners;
	// Lines, (x1,y1,x2,y2) for each line.
	List<Line> lines;
	/// Hands! o.o
	List<CVHand> hands;
	// This assumes just one primary contour.. so use index 0 contour for it if so..?
	std::vector<int> convexHull;
	// Convexity defects calculated by contour's convex hull vs. the contour itself.
	List<cv::Vec4i> convexityDefects;
	

	/// Greyscaled initial input.
	cv::Mat greyscaled;
	/// Down-scaled version of the input.
	cv::Mat scaledDown;
	/// Blurred input image.
	cv::Mat blurred;

	/// o-o LK output
	List<OpticalFlowPoint> opticalFlowPoints;

	/// Current scale of the image (compared to initial input).
	Vector2f currentScale;
	/// Inverted scale of the image (compared to initial input). Used to multiply to co-ordinates to get everything in the same co-ordinate system.
	Vector2f currentScaleInv;

	/// List of previous finger states. The most recent one will be stored at the end, using .Last()
	List<FingerState> fingerStates;

	std::vector<std::vector<cv::Point> > approximatedPolygons;

	/// o-o PCA and stuff
	List<CVPointCloud> pointClouds;

	/// Quads found with e.g. custom FindQuads filter.
	List<Quad> quads;

	// Channels of relevance.
	cv::Mat hue, value, saturation;

	/// For motion-detection/filter. 
	cv::Mat motionHistoryImage;

	/// Initial input, as set by the calling function.
	cv::Mat initialInput;

	// As generated by the OpticalFlow filter when using Farneback method.
	cv::Mat opticalFlowFarneback;
	
	// Is this even in use..?
//	List<AABB*> boundingBoxes;
	List<CVPoint*> points;
//	List<CVBlob*> blobs;

	/// Fetches last error string, which is set if the Process function returns -1 (no result).
	String GetLastError() {return errorString;};

	/// Time the pipeline took from start to finish.
	int pipelineTimeConsumption;

	/// Last return type. Used to render appropriately.
	int returnType;

	/// Swipe-gestures! :D
	Vector2f swipeGestureDirection;
	/// See SwipeStates.. somewhere.
	int swipeState;


	/// Performance testing variables.
	int totalFilterProcessingTimeThisFrame;
	List<int64> totalProcessingTimes;
	int iterations;

	/// Used temporarily/internally. Used by e.g. the VideoWriter to paint before saving to file.
	CVFilter * filterToPaint;

	/// BS to get this info everywhere.
	Vector2i inputSize;
	// Any difference?
	Vector2f initialInputSize;

	/** Converts target input-space coordinate (CV image co-ordinates) into output-space (world-space?) coordinates, 
		as is handled in the game engine and rendered out.
	*/
#define InputSpaceCoordToOutputSpaceCoord InputSpaceToWorldSpace 
	Vector3f InputSpaceToWorldSpace(Vector3f coord);

	/** Coordinate data of the output (projection/projector) as detected in the input.
		This may be set either automatically or manually as pleased.

		These co-ordinates are in OpenCV image space. I.e. probably going from left to right in X, but maybe up to down in Y.
		By default these are set to the default max input image and must be adjusted in some filter if you want to use it.
	*/
	Vector2i outputTopLeftAsDetectedInInput,
		outputBottomRightAsDetectedInInput;

	Vector3f worldSpaceOrigoToImageSpaceOrigo;
	Vector3f imageSpaceOrigoToWorldSpaceOrigo;

	/// Used by render-filters in order to make optimal use of the area we have available for rendering.
	Vector3f outputProjectionRelativeSizeInInput;

	/// Resolution of the output projector or monitor
	Vector3f projectorResolution;

private:

	/// Set when selecting a feature and pushing its ui to the stack for editing.
	CVFilter * currentEditFilter;

	/// If we got a AppWindow, store it here!
	AppWindow * editorWindow;
	// Int, 0 - Image, 1 - Data, 2 - Render
	int filterSelectionFilter;

	List<CVFilter*> filters;


	String errorString;
};

#endif
