/// Emil Hedemalm
/// 2014-04-09
/** Filter class for processing cv-based images or data related to them.
	The classes rely on defining a new filter type in the CVFilter enum, 
	and then adding a corresponding name and constructor call in the 
	CreateFilterNames and CreateFilterByName functions.
*/

#ifndef CV_FILTER_H
#define CV_FILTER_H

#include "CVBlob.h"
#include "String/AEString.h"
#include <opencv2/opencv.hpp>
#include "CVFilterTypes.h"

class CVPipeline;

#define CVResult CVOutputType
#define CVOutputType CVReturnType
namespace CVReturnType {
	enum {
		NO_OUTPUT = -1,
		NOTHING = NO_OUTPUT,
		// Undefined output?
		UNDEFINED_OUTPUT,
		// OpenCV-based output.
		CV_IMAGE,
		CV_CONTOURS,
		CV_CONVEX_HULLS,
		CV_CONVEXITY_DEFECTS,
		CV_CIRCLES,
		CV_CORNERS,
		CV_LINES, // std::vector<cv::Vec4i>
		CV_CHANNELS,
		APPROXIMATED_POLYGONS,

		// Custom output
		LINES, // Columns and rows mainly
		BOUNDING_BOX,
		POINTS,
		BLOBS,
		QUADS,
		HANDS, // See CVHandDetector
		VIDEO,
		RENDER, // Any arbitrary render-output/visualizations
	};
};

/// Possible value types of settings.
namespace CVSettingType {
enum settingTypes
{
    INT = 1,
    FLOAT,
    STRING,
	BOOL,
	VECTOR_3F,
	BUTTON, // A bool that is only activated once at a time, not saved.
};};

/// Structure for a saved setting.
struct CVFilterSetting
{
	// Default constructor only used for when loading from file.
	CVFilterSetting();
	/// Creates a simple button!
	CVFilterSetting(String name);
	// Regular constructors.
	CVFilterSetting(String name, String value);
	CVFilterSetting(String name, bool value);
	CVFilterSetting(String name, float initialValue);
	CVFilterSetting(String name, int value);
	CVFilterSetting(String name, Vector3f value);

	/// Save/load
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

    int type;
    String name;
    
    String sValue;
    float fValue;
    int iValue;
	bool bValue;
	Vector3f vec3fData;
};


/// Filter class for processing cv-based images.
class CVFilter 
{
public:
	// Must specify name when creating! o.o
	CVFilter(int id);
	// Virtual destructor for proper deallocatoin when sub-classing.
	virtual ~CVFilter();
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
	// Name of the filter.
	String name;

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
protected:
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

#endif
