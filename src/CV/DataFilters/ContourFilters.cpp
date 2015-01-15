/// Emil Hedemalm
/// 2014-06-27
/// All filters concerning contours and their data.

#include "ContourFilters.h"
#include "CV/CVPipeline.h"

CVFindContours::CVFindContours()
: CVDataFilter(CVFilterID::FIND_CONTOURS)
{	
	// If positive, will check each contour against the minimum area to discard those not relevant.
	minimumArea = new CVFilterSetting("Minimum area", 500);
	settings.Add(minimumArea);
	maximumArea = new CVFilterSetting("Maximum area", 50000);
	settings.Add(maximumArea);
	removeEdges = new CVFilterSetting("Remove edges", true);
	settings.Add(removeEdges);
}

CVFindContours::~CVFindContours()
{
}

int CVFindContours::Process(CVPipeline * pipe)
{
	// Get channels of input.
	int channels = pipe->input.channels();
	if (channels != 1)
	{
		errorString = "Must be 8-bit single channel input.";
		return CVReturnType::NOTHING;
	}
	if(cv::sum(pipe->input).val[0] < 0.0)
	{
		errorString = "Black image. Returning.";
		return CVReturnType::NOTHING;
	}
	int type = pipe->input.type();
	switch(type)
	{
	case CV_8UC1:
		break;
	default:
		errorString = "Bad image format/datatypes. Must be 1 char.";
		return -1;
	}
	
//	std::vector< std::vector< cv::Point > > newContours;
//	newContours.clear();
		

	try {
		/// Delete old contours and hierarchy
		pipe->cvContours.clear();
		pipe->contourHierarchy.clear();

		// Clear old custom contours.
		pipe->contours.Clear();

		/// Find contours
		newCVContours.clear();

		cv::findContours( pipe->input, newCVContours, pipe->contourHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cv::Point(0,0));

//		List<int> edgesRemoved;
		// Remove edges if wanted
		/*
		if (removeEdges->GetBool())
		{
			for (int i = 0; i < newCVContours.size(); ++i)
			{
				int edgesRem = 0;
				std::vector<cv::Point> & contour = newCVContours[i];
				for (int j = 0; j < contour.size(); ++j)
				{
					cv::Point & point = contour[j];
					if (point.x <= 2 || point.x >= pipe->input.cols - 2 ||
						point.y <= 2 || point.y >= pipe->input.rows -2)
					{
						// Remove!
						contour.erase(contour.begin() + j);
						--j;
						++edgesRem;
					}
				}
				edgesRemoved.Add(edgesRem);
			}
		}
		*/
		List<CVContour> newContours;

		// Copy data from the cv format to our own.
		for (int i = 0; i < newCVContours.size(); ++i)
		{
			CVContour contour;
			std::vector<cv::Point> & cvContour = newCVContours[i];
			// Copy over from cv to our custom aggregate type.
			contour.cvPointList = cvContour;
			// Also create native form of the contour data.
			for (int j = 0; j < cvContour.size(); ++j)
			{
				cv::Point & p = cvContour[j];
				Vector3f point(p.x,p.y,0);
				contour.points.Add(point);
			}
			// If any edges were removed earlier, mark it as such.
		//	if (edgesRemoved[i])
		//		contour.broken = true;

			newContours.Add(contour);		
		}


		assert(newContours.Size() == newCVContours.size());

		if (minimumArea->GetInt() > 0)
		{
			// Compute contour area-sizes and filter 'em.
			for (int i = 0; i < newCVContours.size(); ++i)
			{
				std::vector<cv::Point> newCVContour = newCVContours[i];
				double area = cv::contourArea(newCVContour);
				// Keep only big enough contours.

				CVContour & newContour = newContours[i];
				newContour.area = area;
				
				if (area >= minimumArea->GetInt()  && area <= maximumArea->GetInt())
				{
					pipe->cvContours.push_back(newCVContour);
					pipe->contours.Add(newContour);
				}
				
			}
		}

		// Calculate the moments (centers) of each now known contour.
		for (int i = 0; i < pipe->contours.Size(); ++i)
		{
			CVContour * contour = &pipe->contours[i];
			cv::Moments m = cv::moments(contour->cvPointList);
			contour->centerOfMass.x = m.m10 / m.m00;
			contour->centerOfMass.y = m.m01 / m.m00;
		}
	}
	catch(cv::Exception & e)
	{
		const char * errMsg = e.what();
		errorString = errMsg;
		return -1;
	}
	catch(...)
	{
		errorString = "Unknown error";
		return CVReturnType::NOTHING;
	}

	// Clear the array..?
	// newContours.clear();
	returnType = CVReturnType::CV_CONTOURS;
	return returnType;
}


CVContourClassification::CVContourClassification()
	: CVDataFilter(CVFilterID::CONTOUR_CLASSIFICATION)
{
}
int CVContourClassification::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour * contour = &pipe->contours[i];
		std::vector<cv::Point> & cvContour = contour->cvPointList;
		contour->boundingEllipse = cv::fitEllipse(cvContour);
		contour->boundingRect = cv::minAreaRect(cvContour);
	}

	// Ok, all found. Determine class.
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour * contour = &pipe->contours[i];
		std::vector<cv::Point> & cvContour = contour->cvPointList;
		contour->boundingEllipse = cv::fitEllipse(cvContour);

		cv::RotatedRect & ellipse = contour->boundingEllipse;
		cv::RotatedRect & rect = contour->boundingRect;

		contour->boundingType = BoundingType::RECT;

		cv::RotatedRect & relevant = rect;

		/// Small small size. Must be finger-tip! o.o
		if (relevant.size.width < 25.f && 
			relevant.size.height < 25.f)
			contour->contourClass = ContourClass::FINGERTIP;
		// o-o
		else if (relevant.size.width < 50.f)
		{
			contour->contourClass = ContourClass::FINGER;
		}
	}


	returnType = CVReturnType::CV_CONTOUR_ELLIPSES;
	return returnType;
}



CVConvexityDefects::CVConvexityDefects()
	: CVDataFilter(CVFilterID::CALC_CONVEXITY_DEFECTS)
{
	minimumDefectDepth = new CVFilterSetting("Minimum defect depth", 100.f);
	settings.Add(minimumDefectDepth);
		
}
int CVConvexityDefects::Process(CVPipeline * pipe)
{
	// Calculate defects (convex hull vs. contour diff)
//	std::cout<<"\ndefects "<<&convexityDefects<<" "<<&relevantDefects;
	relevantDefects.Clear();
	pipe->convexityDefects.Clear();
	if (pipe->cvContours.size() == 0)
	{
		return CVReturnType::CV_CONVEXITY_DEFECTS;
	}
	cv::convexityDefects(pipe->cvContours[0], pipe->convexHull, convexityDefects);
	for (int j = 0; j < convexityDefects.size(); ++j)
	{
		cv::Vec4i & defect = convexityDefects[j];
		if (defect[3] > minimumDefectDepth->GetFloat())
			pipe->convexityDefects.Add(defect);
	}
	returnType = CVReturnType::CV_CONVEXITY_DEFECTS;
	return returnType;
}

