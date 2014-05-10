/// Emil Hedemalm
/// 2014-04-09
/// Filter class for processing cv-based images or data related to them.

#include "CVFilter.h"
#include "CVPipeline.h"
#include <fstream>
#include "CVImageFilters.h"
#include "CVDataFilters.h"
#include "CVRenderFilters.h"

List<String> CVFilter::filterNames;
CVFilter * sampleFilters[CVFilterID::MAX_FILTERS];

/// Call once before trying to create/use any filters!
void CVFilter::CreateFilterNames()
{
	// Names already created, skipping.
	if (filterNames.Size())
		return;
	// Fill it with amount of strings corresponding to amount of known filters.
	for (int i = 0; i < CVFilterID::MAX_FILTERS; ++i)
	{
		filterNames.Add(String());
		sampleFilters[i] = NULL;
	}
	// And add the actual strings too.
	filterNames[CVFilterID::GREYSCALE] = "Greyscale";
	filterNames[CVFilterID::SCALE_UP] = "Scale up";
	filterNames[CVFilterID::SCALE_DOWN] = "Scale down";
	filterNames[CVFilterID::CANNY_EDGE] = "Canny Edge detection";
	filterNames[CVFilterID::INVERT] = "Invert";
	filterNames[CVFilterID::HARRIS_CORNER] = "Harris Corner detection";
	filterNames[CVFilterID::GAUSSIAN_BLUR] = "Gaussian blur";
	filterNames[CVFilterID::INVERT] = "Invert";
	filterNames[CVFilterID::THRESHOLD] = "Threshold";
	filterNames[CVFilterID::ABS] = "Abs";
	filterNames[CVFilterID::SATURATION_FILTER] = "Saturation filter";
	filterNames[CVFilterID::REMOVE_BACKGROUND] = "Background removal";
	filterNames[CVFilterID::ERODE] = "Erode";
	filterNames[CVFilterID::DILATE] = "Dilate";
	filterNames[CVFilterID::EXTRACT_CHANNELS] = "Extract channels";
	filterNames[CVFilterID::HUE_FILTER] = "Hue filter";
	filterNames[CVFilterID::VALUE_FILTER] = "Value filter";
	filterNames[CVFilterID::FILL_THREE] = "Fill three";

	// Data filters
	filterNames[CVFilterID::FIND_CONTOURS] = "Find contours";
	filterNames[CVFilterID::FIND_CONVEX_HULLS] = "Find convex hulls";
	filterNames[CVFilterID::SHI_TOMASI_CORNERS] = "ShiTomasi Corners / Good features to track";
	filterNames[CVFilterID::HOUGH_CIRCLES] = "Hough circles";
	filterNames[CVFilterID::HOUGH_LINES] = "Hough lines";
	filterNames[CVFilterID::FIND_QUADS] = "Find quads";
	filterNames[CVFilterID::MAX_BOX_PERSISTANCE] = "Max box persistance";
	filterNames[CVFilterID::HAND_DETECTOR] = "Hand detector";
	filterNames[CVFilterID::QUAD_ASPECT_RATIO_CONSTRAINT] = "Quad Aspect Ratio Constraint";
	filterNames[CVFilterID::HAND_PERSISTENCE] = "Hand persistence";
	filterNames[CVFilterID::HAND_DETECTOR_2] = "Hand detector (defects)";
	filterNames[CVFilterID::CALC_CONVEX_HULL] = "Calc Convex Hull";
	filterNames[CVFilterID::CALC_CONVEXITY_DEFECTS] = "Calc Convexity defects";
	filterNames[CVFilterID::FILTER_LINES_BY_ANGLE] = "Filter lines by angle";
	filterNames[CVFilterID::MERGE_LINES] = "Merge lines";
	filterNames[CVFilterID::APPROXIMATE_POLYGONS] = "Approximate polygons";

	// Render filters
	filterNames[CVFilterID::VIDEO_WRITER] = "Video writer";
	filterNames[CVFilterID::IMAGE_GALLERY_HAND] = "Image gallery, hand";
	filterNames[CVFilterID::MOVIE_PROJECTOR] = "Movie projector, polygon";

	// Own data filters <- Old stuff. Place in either Data or image above.
	filterNames[CVFilterID::LINE_GATHER] = "Line gather";
	filterNames[CVFilterID::MAX_BOUNDING_BOX] = "Max bounding box";
	filterNames[CVFilterID::TO_CHAR] = "Convert to single unsigned byte";
}

CVFilter * CreateFilterByID(int id)
{
	switch (id)
	{
		// Image filters
		case CVFilterID::CANNY_EDGE: return new CVCannyEdgeFilter();
		case CVFilterID::GAUSSIAN_BLUR: return new CVGaussianBlurFilter();
		case CVFilterID::GREYSCALE: return new CVGreyscaleFilter();
		case CVFilterID::SCALE_UP: return new CVScaleUpFilter();
		case CVFilterID::SCALE_DOWN: return new CVScaleDownFilter();
		case CVFilterID::HARRIS_CORNER: return new CVHarrisCornerFilter();
		case CVFilterID::INVERT: return new CVInvertFilter();
		case CVFilterID::THRESHOLD: return new CVThresholdFilter();
		case CVFilterID::ABS: return new CVAbsFilter();
		case CVFilterID::SATURATION_FILTER: return new CVSaturationFilter();
		case CVFilterID::REMOVE_BACKGROUND: return new CVRemoveBackgroundFilter();
		case CVFilterID::ERODE: return new CVErode();
		case CVFilterID::DILATE: return new CVDilate();
		case CVFilterID::EXTRACT_CHANNELS: return new CVExtractChannels();
		case CVFilterID::HUE_FILTER: return new CVHueFilter();
		case CVFilterID::VALUE_FILTER: return new CVValueFilter();
		case CVFilterID::FILL_THREE: return new CVFill3();

		// Data filters
		case CVFilterID::FIND_CONTOURS: return new CVFindContours();
		case CVFilterID::SHI_TOMASI_CORNERS: return new CVShiTomasiCorners();
		case CVFilterID::HOUGH_CIRCLES: return new CVHoughCircles();
		case CVFilterID::HOUGH_LINES: return new CVHoughLines();
		case CVFilterID::FIND_QUADS: return new CVFindQuads();
		case CVFilterID::MAX_BOX_PERSISTANCE: return new CVMaxBoxPersistance();
		case CVFilterID::HAND_DETECTOR: return new CVHandDetector();
		case CVFilterID::QUAD_ASPECT_RATIO_CONSTRAINT: return new CVQuadAspectRatioConstraint();
		case CVFilterID::HAND_PERSISTENCE: return new CVHandPersistance();
		case CVFilterID::HAND_DETECTOR_2: return new CVHandDetector2();
		case CVFilterID::CALC_CONVEX_HULL: return new CVConvexHull();
		case CVFilterID::CALC_CONVEXITY_DEFECTS: return new CVConvexityDefects();
		case CVFilterID::FILTER_LINES_BY_ANGLE: return new CVFilterLinesByAngle();
		case CVFilterID::MERGE_LINES: return new CVMergeLines();
		case CVFilterID::APPROXIMATE_POLYGONS: return new CVApproxPolygons();

		// Render-filters
		case CVFilterID::VIDEO_WRITER: return new CVVideoWriter();
		case CVFilterID::IMAGE_GALLERY_HAND: return new CVImageGalleryHand();
		case CVFilterID::MOVIE_PROJECTOR: return new CVMovieProjector();

		// Own <- Old stuff. Place in either Data or image above.
		case CVFilterID::LINE_GATHER: return new CVLineGatherFilter();
		case CVFilterID::MAX_BOUNDING_BOX: return new CVMaxBoundingBox();
		case CVFilterID::TO_CHAR: return new CVToChar();	
	}
	// And return it!
	return NULL;	
}

/// Returns sample filter of target type. These are static and should not be used directly.
CVFilter * GetSampleFilter(int id)
{
	if (!sampleFilters[id])
		sampleFilters[id] = CreateFilterByID(id);
	return sampleFilters[id];
}

// Default constructor only used for when loading from file.
CVFilterSetting::CVFilterSetting()
{
}

/// Creates a simple button!
CVFilterSetting::CVFilterSetting(String name)
	: name(name), type(CVSettingType::BUTTON)
{
	// Set boolean value, since it might function as a trigger-boolean until the filter has done something.. enough.
	bValue = false;
}	

CVFilterSetting::CVFilterSetting(String name, String value)
	: name(name), sValue(value), type(CVSettingType::STRING)
{
}

CVFilterSetting::CVFilterSetting(String name, bool value)
	: name(name), bValue(value), type(CVSettingType::BOOL)
{

}

CVFilterSetting::CVFilterSetting(String name, float initialValue)
	: name(name), fValue(initialValue), type(CVSettingType::FLOAT)
{
}

CVFilterSetting::CVFilterSetting(String name, int value)
	: name(name), iValue(value), type(CVSettingType::INT)
{
}
CVFilterSetting::CVFilterSetting(String name, Vector3f value)
	: name(name), vec3fData(value), type(CVSettingType::VECTOR_3F)
{
}

#define CV_FILTER_SETTING_VERSION_1	1
/// Save/load
bool CVFilterSetting::WriteTo(std::fstream & file)
{
	int version = CV_FILTER_SETTING_VERSION_1;
	file.write((char*)&version, sizeof(int));
	file.write((char*)&type, sizeof(int));
	name.WriteTo(file);
	switch(type)
	{
		case CVSettingType::INT:
			file.write((char*)&iValue, sizeof(int));
			break;
		case CVSettingType::FLOAT:
			file.write((char*)&fValue, sizeof(float));
			break;
		case CVSettingType::VECTOR_3F:
			vec3fData.WriteTo(file);
			break;
		case CVSettingType::BUTTON:
			break;
		case CVSettingType::BOOL:
			file.write((char*)&bValue, sizeof(bool));
			break;
		case CVSettingType::STRING:
			sValue.WriteTo(file);
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}
bool CVFilterSetting::ReadFrom(std::fstream & file)
{
	int version = CV_FILTER_SETTING_VERSION_1;
	file.read((char*)&version, sizeof(int));
	assert(version == CV_FILTER_SETTING_VERSION_1);
	file.read((char*)&type, sizeof(int));
	name.ReadFrom(file);
	switch(type)
	{
		case CVSettingType::INT:
			file.read((char*)&iValue, sizeof(int));
			break;
		case CVSettingType::FLOAT:
			file.read((char*)&fValue, sizeof(float));
			break;
		case CVSettingType::VECTOR_3F:
			vec3fData.ReadFrom(file);
			break;
		case CVSettingType::BUTTON:
			break;
		case CVSettingType::BOOL:
			file.read((char*)&bValue, sizeof(bool));
			break;
		case CVSettingType::STRING:
			sValue.ReadFrom(file);
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}


// Must specify name when creating! o.o
CVFilter::CVFilter(int id)
	: id(id)
{
	name = filterNames[id];
	enabled	= true;
	type = -1;
}

// Virtual destructor for proper deallocatoin when sub-classing.
CVFilter::~CVFilter()
{
	settings.ClearAndDelete();
}

/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVFilter::SetEnabled(bool newEnabledState)
{
	enabled = newEnabledState;
}

#define CV_FILTER_VERSION_1 1
/// Save all settings specific to this filter.
bool CVFilter::WriteTo(std::fstream & file)
{
	int version = CV_FILTER_VERSION_1;
	file.write((char*)&version, sizeof(int));
	name.WriteTo(file);
	/// Save amount of settings.
	int numSettings = settings.Size();
	file.write((char*)&numSettings, sizeof(int));
	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		setting->WriteTo(file);
	}
	return true;
}

bool CVFilter::ReadFrom(std::fstream & file)
{
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == CV_FILTER_VERSION_1);
	name.ReadFrom(file);
	int numSettings = settings.Size();
	file.read((char*)&numSettings, sizeof(int));
	/// Use a temporary setting in order to load settings.
	CVFilterSetting setting;
	for (int i = 0; i < numSettings; ++i)
	{
		setting.ReadFrom(file);
		// Compare it's name with our current settings.
		CVFilterSetting * currSetting = GetSetting(setting.name);
		// If we are currently using such given setting, copy over it's value.
		if (currSetting){
			currSetting->fValue = setting.fValue;
			currSetting->iValue = setting.iValue;
		}
	}
	return true;
}

int CVFilter::Process(CVPipeline * pipe)
{
	std::cout<<"\nDefault filter, does nothing.";
	return 0;
}

/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVFilter::Paint(CVPipeline * pipe)
{
	std::cout<<"\nDefault filter: Paint. Consider subclassing it to get proper output.";
}

/// Fetch specific setting.
CVFilterSetting * CVFilter::GetSetting(String byName)
{
	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		if (setting->name == byName)
			return setting;
	}
	return NULL;
}
	

/// Construction helper function. Used by pipeline's Save/Load.
CVFilter * CreateFilterByName(String filterName)
{
	/// Type to create.
	int id;
	for (int i = 0; i < CVFilter::filterNames.Size(); ++i)
	{
		String iName = CVFilter::filterNames[i];
		if (iName == filterName)
		{
			id = i;
			break;
		}
	}
	return CreateFilterByID(id);	
}
