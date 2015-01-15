/// Emil Hedemalm
/// 2014-06-27
/// All filters concerning contours and their data.

#include "CVDataFilters.h"


/// Finds contours in the image.
class CVFindContours : public CVDataFilter {
public:
	CVFindContours();
	virtual ~CVFindContours();
	virtual int Process(CVPipeline * pipe);
private:
	// If positive, will check each contour against the minimum area to discard those not relevant.
	CVFilterSetting * minimumArea, * maximumArea;
	/// Removes all contour parts which lie within 1-2 pixels of the image's edges.
	CVFilterSetting * removeEdges;
	// Store here since it's failing when having it on the regular stack..?
	std::vector< std::vector< cv::Point > > newCVContours;

};

class CVContourClassification : public CVDataFilter 
{
public:
	CVContourClassification();
	virtual int Process(CVPipeline * pipe);
private:

};
