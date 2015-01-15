/// Emil Hedemalm
/// 2014-04-09
/** Filter class for processing cv-based images or data related to them.
	The classes rely on defining a new filter type in the CVFilter enum, 
	and then adding a corresponding name and constructor call in the 
	CreateFilterNames and CreateFilterByName functions.
*/

#ifndef CV_FILTER_H
#define CV_FILTER_H


#include "CVFilterTypes.h"
#include "CVFilterSetting.h"

#define CATCH_EXCEPTION(inFunction) {std::cout<<"\nCaught exception in function: "<<inFunction;}

class CVPipeline;
class UserInterface;

#undef ERROR

#define CVResult CVOutputType
#define CVOutputType CVReturnType
namespace CVReturnType {
	enum {
		ERROR = -2, CV_ERROR = ERROR,
		NO_OUTPUT = -1,
		NOTHING = NO_OUTPUT,
		// Undefined output?
		UNDEFINED_OUTPUT,
		// OpenCV-based output.
		CV_IMAGE,
		CV_CONTOURS,
		CV_CONTOUR_SEGMENTS,
		CV_CONVEX_HULLS,
		CV_CONVEXITY_DEFECTS,
		CV_CIRCLES,
		CV_CORNERS,
		CV_LINES, // std::vector<cv::Vec4i>
		CV_CHANNELS,
		APPROXIMATED_POLYGONS,
		CV_CONTOUR_ELLIPSES,
		CV_OPTICAL_FLOW,
		
		CV_TEMPLATE_MATCHES,

		// Custom output
		LINES, // Columns and rows mainly
		BOUNDING_BOX,
		POINTS,
		BLOBS,
		QUADS,
		HANDS, // See CVHandDetector
		VIDEO,
		RENDER, // Any arbitrary render-output/visualizations
		FINGER_STATES,
		POINT_CLOUDS,
		SWIPES_GESTURES, // As delivered by CVSwipeGesture
	};
};


class Message;

/// Filter class for processing cv-based images.
class CVFilter 
{
	friend class CVPipeline;
public:
	// Must specify name when creating! o.o
	CVFilter(int id);
	// Virtual destructor for proper deallocatoin when sub-classing.
	virtual ~CVFilter();

	/// Called upon adding the filter to the active pipeline.
	virtual void OnAdd();
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
	// Name of the filter.
	String name;

	/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
	virtual void ProcessMessage(Message * message);

	/// Save/load
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);

	/// Fetches last error string, which is set if the Process function returns -1 (no result).
	String GetLastError() { return errorString; };

	/// Fetch specific setting.
	CVFilterSetting * GetSetting(String byName);
	/// Returns the settings in their entirety
	List<CVFilterSetting*> GetSettings() {return settings;};

	/// Creates UI for editing this filter. Assumes an element named FilterEditor is present which will contain all created elements.
	void UpdateEditUI(UserInterface * inUI);

	/// If currently enabled. Default is true.
	bool enabled;

	/// Call once before trying to create/use any filters!
	static void CreateFilterNames();
	/// List of all filter-names. Created by calling the CreateFilterNames() function.
	static List<String> filterNames;

	int Type() {return type;};
	int ID() {return id;};

	/// Text displayed in the filter-editor in order to give hints for how it can work correctly.
	String about;
	/// Text displayed in the filter-editor below the about-text?
	String status;

	/// Time in milliseconds consumed by the filter for calculation.
	int64 processingTime, renderTime;

	/// Temporary thing. Set each iteration.
	CVFilter * previousFilter;
protected:

	
	/// Return-type of this filter. Set automatically using the result it returns~..!
	int returnType;

	/// Type, as defined in the enum CVFilterID.
	int id;
	/// Type of filter, as in CVFilterType
	int type;
	/// Settings for this filter.
	List<CVFilterSetting*> settings;
	/// Last error string. This is set if the Process function returns -1.
	String errorString;
};

/// Construction helper function. Used by pipeline's Save/Load.
CVFilter * CreateFilterByName(String filterName);
CVFilter * CreateFilterByID(int id);
/// Returns sample filter of target type. These are static and should not be used directly.
CVFilter * GetSampleFilter(int id);
/// Call once upon exit.
void DeleteSampleFilters();

#endif
