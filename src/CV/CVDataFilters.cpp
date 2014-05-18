/// Emil Hedemalm
/// 2014-04-11
/** Data-based filters that generate point- and/or blob-clouds.
*/

#include "CVDataFilters.h"
#include "CVPipeline.h"
#include "String/StringUtil.h"
cv::RNG rng(12345);


CVDataFilter::CVDataFilter(int id)
	: CVFilter(id)
{
	type = CVFilterType::DATA_FILTER;
};

void CVDataFilter::Paint(CVPipeline * pipe)
{
	if (pipe->returnType == CVReturnType::QUADS)
	{
		// Draw quads.
		if (!pipe->quads.Size())
			return;
		// Convert to color if needed.
		int channelsBefore = pipe->output.channels();
		if (channelsBefore == 1)
		{
			pipe->output.convertTo(pipe->output, CV_8UC3);
			cv::cvtColor(pipe->output, pipe->output, CV_GRAY2RGB);
		}
		// o.o Paste!
		for (int i = 0; i < pipe->quads.Size(); ++i)
		{
			Quad quad = pipe->quads[i];
	#define RGB(r,g,b) cv::Scalar(b,g,r)
			rng = cv::RNG(1344);
			cv::Scalar color = RGB(rng.next(),rng.next(),rng.next());
		//	cv::Mat rectImage = cv::Mat::zeros(pipe->output.size(), CV_8UC3);
			cv::rectangle(pipe->output, cv::Point(quad.point1.x, quad.point1.y), cv::Point(quad.point3.x, quad.point3.y), color, CV_FILLED);
			float alpha = 0.2f;
			float beta = 1 - alpha;
		//	cv::addWeighted(rectImage, alpha, pipe->output, beta, 0.0, pipe->output); 
		//	rectImage.copyTo(pipe->output);		
		}
	}
	else if (pipe->returnType == CVReturnType::APPROXIMATED_POLYGONS)
	{
		// Copy original input..
		pipe->initialInput->copyTo(pipe->output);
		RenderPolygons(pipe);
	}
	else if (pipe->returnType == CVReturnType::CV_IMAGE)
	{
		// Do nothing as the image should already be in the ouput matrix of the pipeline.
	}
	else if (pipe->returnType == CVReturnType::LINES ||
		pipe->returnType == CVReturnType::CV_LINES)
	{
		// Convert the color to colors again for visualization...
		pipe->initialInput->copyTo(pipe->output);
		for( size_t i = 0; i < pipe->lines.Size(); i++ )
		{
			line( pipe->output, cv::Point(pipe->lines[i].start.x, pipe->lines[i].start.y),
				cv::Point(pipe->lines[i].stop.x, pipe->lines[i].stop.y), cv::Scalar(0,0,255), 3, 8 );
		}
	}
	else if (pipe->returnType == CVReturnType::CV_CONTOURS)
	{
		pipe->initialInput->copyTo(pipe->output);
		RenderContours(pipe);
	}
	else if (pipe->returnType == CVReturnType::CV_CONVEX_HULLS)
	{
		pipe->initialInput->copyTo(pipe->output);
		RenderContours(pipe);
		RenderConvexHulls(pipe);
	}
	else if (pipe->returnType == CVReturnType::CV_CONVEXITY_DEFECTS)
	{
		pipe->initialInput->copyTo(pipe->output);
		RenderContours(pipe);
		RenderConvexHulls(pipe);
		RenderConvexityDefects(pipe);
	}
	else if (pipe->returnType == CVReturnType::HANDS)
	{
		// Convert image to RGB for easier display
		int channelsBefore = pipe->initialInput->channels();
	//	cv::cvtColor(*pipe->initialInput, pipe->output, CV_GRAY2RGB);
		pipe->initialInput->copyTo(pipe->output);
		int channelsAfter = pipe->output.channels();
		RenderContours(pipe);
		RenderHands(pipe);
	}
	else if (pipe->returnType == CVReturnType::FINGER_STATES)
	{
		// Render the last known one.
		pipe->initialInput->copyTo(pipe->output);
		FingerState & state = pipe->fingerStates.Last();
		cv::Scalar color(255,0,0,255);
		for (int i = 0; i < state.positions.Size(); ++i)
		{
			Vector3f pos = state.positions[i];
			cv::circle(pipe->output, cv::Point(pos.x, pos.y), 5, color, 3);
		}
	}
	else if (pipe->returnType == -1)
		// Nothing to render if error.
		;
	else if (pipe->returnType == CVReturnType::RENDER)
		// Nothing to render if render.
		;
	else
		std::cout<<"\nCVDataFilter::Paint called. Forgot to subclass the paint-method?";
}


void RenderPolygons(CVPipeline * pipe)
{
	/// Use same random seed every time to avoid rainbow hell..
	rng = cv::RNG(12345);
	for (int i = 0; i < pipe->approximatedPolygons.size(); ++i)
	{			
		cv::Scalar color = cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255));
		cv::drawContours(pipe->output, pipe->approximatedPolygons, i, color, 2, 8, pipe->contourHierarchy, 0, cv::Point());
	}
}

void RenderContours(CVPipeline * pipe)
{
	/// Use same random seed every time to avoid rainbow hell..
	rng = cv::RNG(12345);
	for (int i = 0; i < pipe->contours.size(); ++i)
	{			
		cv::Scalar color = cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255));
		cv::drawContours(pipe->output, pipe->contours, i, color, 2, 8, pipe->contourHierarchy, 0, cv::Point());
	}
}
void RenderConvexHulls(CVPipeline * pipe)
{
	if (pipe->contours.size() == 0)
		return;
	cv::Scalar color = cv::Scalar(rng.uniform(125,255), rng.uniform(125,255), rng.uniform(125,255));
//	cv::drawContours(pipe->output, pipe->convexHull, 0, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );
	std::vector<int> & convexHull = pipe->convexHull;
	std::vector<cv::Point> & contour = pipe->contours[0];
	for (int i = 0; i < convexHull.size(); ++i)
	{
		int index = convexHull[i];
		cv::Point point = contour[index];
		cv::circle(pipe->output, point, 15, color);
		int nextIndex = convexHull[(i+1) % convexHull.size()];
		cv::Point point2 = contour[nextIndex];
		// Line!
		cv::line(pipe->output, point, point2, color, 3);
	}
}
void RenderConvexityDefects(CVPipeline * pipe)
{
	cv::Scalar color = cv::Scalar(rng.uniform(155,255), rng.uniform(125,255), rng.uniform(0,200));
	List<cv::Vec4i> defects = pipe->convexityDefects;
	for (int i = 0; i < defects.Size(); ++i)
	{
		cv::Vec4i defect = defects[i];
		int farthestPointIndex = defect[2];
		cv::Point farthestPoint = pipe->contours[0][farthestPointIndex];
		// Render point furthest away?
		cv::circle(pipe->output, farthestPoint, 3, color, 5);
	}
}
void RenderHands(CVPipeline * pipe)
{
	List<Hand> & hands = pipe->hands;
	std::vector<std::vector<cv::Point> > c;
	for(int i = 0; i < hands.Size(); i++)
	{
		Hand & hand = hands[i];
		c.clear();
		c.push_back(hand.contour);
		cv::circle(pipe->output, cv::Point(hand.center.x, hand.center.y), 20, cv::Scalar(0, 0, 255), 2);
		int fingersSize = hand.fingers.Size();
		for(int j = 0; j < fingersSize; j++)
		{
#define VEC3FTOCVPOINT(a) (cv::Point(a.x,a.y))
			Vector3f fingerPoint = hand.fingers[j];
			cv::circle(pipe->output, cv::Point(fingerPoint.x, fingerPoint.y), 10, cv::Scalar(0, 0, 255), 2);
			cv::line(pipe->output, VEC3FTOCVPOINT(hand.center), VEC3FTOCVPOINT(fingerPoint), cv::Scalar(0, 0, 255), 4);
		}
//		std::cout<<"\nfingers drawn: "<<hand.fingers.Size();
	}
}


CVShiTomasiCorners::CVShiTomasiCorners()
	: CVDataFilter(CVFilterID::SHI_TOMASI_CORNERS)
{
	maxCorners = new CVFilterSetting("Max corners", 32);
	settings.Add(maxCorners);
	qualityLevel = new CVFilterSetting("Quality", 0.01f);
	settings.Add(qualityLevel);
	minimumDistance = new CVFilterSetting("Min distance", 10.f);
	settings.Add(minimumDistance);
	blockSize = new CVFilterSetting("Block size", 3);
	settings.Add(blockSize);
	k = new CVFilterSetting("k", 0.04f);
	settings.Add(k);
	useHarrisDetector = new CVFilterSetting("Use Harris detector", 0);
	settings.Add(useHarrisDetector);

//	CVFilterSetting * maxCorners, * qualityLevel, * minimumDistance, * blockSize, * k, * useHarrisDetector;
	
	List<String> strings;
	strings.Add("Quality denotes how to filter away lower quality corners depending");
	strings.Add("on the value of the corner with best quality. (percentage filtering)");
	about = MergeLines(strings, "\n");

}
int CVShiTomasiCorners::Process(CVPipeline * pipe)
{
	if (pipe->input.channels() != 1)
	{
		errorString = "Requires single-channel input image.";
		return -1;
	}
	try {
		//double qualityLevel = 0.01;
		//double minDistance = 10;
		//int blockSize = 3;
		//bool useHarrisDetector = false;
		//double k = 0.04;
	
		/// Apply corner detection
		goodFeaturesToTrack( pipe->input,
					pipe->corners,
					maxCorners->iValue,
					qualityLevel->fValue,
					minimumDistance->fValue,
					cv::Mat(),
					blockSize->iValue,
					useHarrisDetector->iValue,
					k->fValue);
	}
	catch (...)
	{
		errorString = ";_;";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_CORNERS;
}

/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVShiTomasiCorners::Paint(CVPipeline * pipe)
{
	/// Copy src image.
	/// Copy the source image
	cv::Mat copy;
	copy = pipe->input.clone();

	/// Draw corners detected
	std::cout<<"\n** Number of corners detected: "<<pipe->corners.size();
	int r = 4;
	for( int i = 0; i < pipe->corners.size(); ++i )
	{ 
		circle( copy, pipe->corners[i], r, cv::Scalar(rng.uniform(0,255), rng.uniform(0,255),
				rng.uniform(0,255)), -1, 8, 0 ); 
	}

	copy.copyTo(pipe->output);
}


CVFindContours::CVFindContours()
	: CVDataFilter(CVFilterID::FIND_CONTOURS)
{	
	// If positive, will check each contour against the minimum area to discard those not relevant.
	minimumArea = new CVFilterSetting("Minimum area", 50);
	settings.Add(minimumArea);
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
		pipe->contours.clear();
		pipe->contourHierarchy.clear();
		/// Find contours
		newContours.clear();

		cv::findContours( pipe->input, newContours, pipe->contourHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cv::Point(0,0));

		if (minimumArea->iValue > 0)
		{
			// Compute contour area-sizes and filter 'em.
			for (int i = 0; i < newContours.size(); ++i)
			{
				std::vector<cv::Point> contour = newContours[i];
				double area = cv::contourArea(contour);
				// Keep only big enough contours.

				
				if (area >= minimumArea->iValue)
				{
					pipe->contours.push_back(contour);
				}
				
			}
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

	return CVReturnType::CV_CONTOURS;
}


/// Own filter, do this later after testing existing CV filters?
CVLineGatherFilter::CVLineGatherFilter()
	: CVDataFilter(CVFilterID::LINE_GATHER)
{
	minimumInterestpointsPerLine = new CVFilterSetting("Minimum interest-points per line", 50);
	settings.Add(minimumInterestpointsPerLine);
//	settings.Add(
}


int CVLineGatherFilter::Process(CVPipeline * pipe)
{
	/// Expect a single-channel single-byte image.
	int channels = pipe->input.channels();
	if (channels != 1)
	{
		errorString = "Expects single-channel input.";
		return CVReturnType::NOTHING;
	}	

	/// Convert floating-point images to unsigned byte as needed.
	int type = pipe->input.type();
	switch(type)
	{
		case CV_32FC1: 
		{
			cv::Mat converted;
			pipe->input.convertTo(converted, CV_8UC1);
			pipe->input = converted;
			break;
		}
	}
	
	/// Do actual processing.
	try {
		cv::Mat * mat = &pipe->input;
		int yStep, xStep;
		unsigned char * data = mat->data;

		// Allocate rows and columns.
		CVLine * rows = new CVLine[mat->rows];
		for (int i = 0; i < mat->rows; ++i)
		{
			rows[i].number = i;
			rows[i].interestPoints = 0;
		}
		CVLine * columns = new CVLine[mat->cols];
		for (int i = 0; i < mat->cols; ++i)
		{
			columns[i].number = i;
			columns[i].interestPoints = 0;
		}
		
		// Look through the image.
		for (int y = 0; y < mat->rows; ++y)
		{
			yStep = y * mat->cols * channels;
			for (int x = 0; x < mat->cols; ++x)
			{
				xStep = x * channels;
				int index = yStep + xStep;
				// If the pixels is white (or non 0), increment relevant rows and columns.
				if (data[index] > 0)
				{
					rows[y].interestPoints++;
					columns[x].interestPoints++;
				}
			}
		}

		// Sort lines and columns by amount of "interesting" pixels within each.
		List<CVLine> interestingRows, interestingColumns;
		/// First compare with a minimum value. 
		int averageRowPoints = 0;
		for (int i = 0; i < mat->rows; ++i)
		{
			int interestPoints = rows[i].interestPoints;
			if (interestPoints > minimumInterestpointsPerLine->iValue)
			{
				interestingRows.Add(rows[i]);
				// Also calculate average of all added rows.
				averageRowPoints += interestPoints;
			}
		}
		if (interestingRows.Size())
			averageRowPoints /= interestingRows.Size();

		/// First compare with a minimum value.
		int averageColumnPoints = 0;
		for (int i = 0; i < mat->cols; ++i)
		{
			int interestPoints = columns[i].interestPoints;
			if (interestPoints > minimumInterestpointsPerLine->iValue)
			{
				interestingColumns.Add(columns[i]);
				// Also calculate average of all added rows.
				averageColumnPoints += interestPoints;
			}
		}
		if (interestingColumns.Size())
			averageColumnPoints /= interestingColumns.Size();

		std::cout<<"\nIntersting rows/columns: "<<interestingRows.Size()<<"/"<<interestingColumns.Size();

		
		/// Sort columns and rows by amount of interest-points. Remove all lines whose amount are below the average as needed
		/// TODO: Do it.



		// Create a new image and paint them over?
		pipe->output = cv::Mat::zeros(pipe->input.size(), pipe->input.type());
		// Paint lines?
		cv::Scalar color (255,255,255);
		for (int i = 0; i < interestingColumns.Size(); ++i)
		{
			CVLine line = interestingColumns[i];
			cv::line(pipe->output, cv::Point(line.number, 0), cv::Point(line.number, mat->rows), color);
		}
		for (int i = 0; i < interestingRows.Size(); ++i)
		{
			CVLine line = interestingRows[i];
			cv::line(pipe->output, cv::Point(0, line.number), cv::Point(mat->cols, line.number), color);
		}

		// Save interesting lines into pipe.
		pipe->columns = interestingColumns;
		pipe->rows = interestingRows;

		// Delete temporary data.
		delete[] rows;
		delete[] columns;
	} catch(...)
	{
		errorString = "Point-gather failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::LINES;
}



CVMaxBoundingBox::CVMaxBoundingBox()
	: CVDataFilter(CVFilterID::MAX_BOUNDING_BOX)
{
	
}

int CVMaxBoundingBox::Process(CVPipeline * pipe)
{
	// Check lines!
	if (!pipe->columns.Size())
	{
		errorString = "0 columns! Cannot make bounding box D:";
		return CVReturnType::NOTHING;
	}
	if (!pipe->rows.Size())
	{
		errorString = "0 rows! Cannot make bounding box D:";
		return CVReturnType::NOTHING;
	}

	CVBoundingBox box;
	// Initialize min/max
	box.min = box.max = Vector2i(pipe->columns[0].number, pipe->rows[0].number);
	for (int i = 0; i < pipe->columns.Size(); ++i)
	{
		CVLine line = pipe->columns[i];
		if (line.number > box.max.x)
			box.max.x = line.number;
		else if (line.number < box.min.x)
			box.min.x = line.number;
	}
	for (int i = 0; i < pipe->rows.Size(); ++i)
	{
		CVLine line = pipe->rows[i];
		if (line.number > box.max.y)
			box.max.y = line.number;
		else if (line.number < box.min.y)
			box.min.y = line.number;
	}
	// Clear existing list and add our new box!
	pipe->boxes.Clear();
	pipe->boxes.Add(box);
	std::cout<<"Box area: "<<(box.max - box.min).Length();

	// Render le box!
	pipe->output = cv::Mat::zeros(pipe->input.size(), pipe->input.type());

	cv::Scalar color(255,255,255);
	cv::rectangle(pipe->output, cv::Rect(cv::Point(box.min.x, box.min.y), cv::Point(box.max.x, box.max.y)), color);

	return CVReturnType::BOUNDING_BOX;
}


CVHoughCircles::CVHoughCircles()
	: CVDataFilter(CVFilterID::HOUGH_CIRCLES)
{
	//minimumDistance, param1, param2, minimumRadius, maximumRadius
	method = new CVFilterSetting("Method", 3);
	minimumDistance = new CVFilterSetting("Min distance", 10);
	param1 = new CVFilterSetting("param1", 50.f);
	param2 = new CVFilterSetting("param2", 50.f);
	minimumRadius = new CVFilterSetting("Min radius", 10.f);
	maximumRadius = new CVFilterSetting("Max radius", 100.f);

	settings.Add(method);
	settings.Add(minimumDistance);
	settings.Add(param1);
	settings.Add(param2);
	settings.Add(minimumRadius);
	settings.Add(maximumRadius);
	
	List<String> rows;
	rows += "Yo";
	rows += "CV_HOUGH_STANDARD =0,";
    rows += "CV_HOUGH_PROBABILISTIC =1,";
    rows += "CV_HOUGH_MULTI_SCALE =2,";
    rows += "CV_HOUGH_GRADIENT =3";
	about = MergeLines(rows, "\n");
}

int CVHoughCircles::Process(CVPipeline * pipe)
{
	if (pipe->input.channels() != 1){
		errorString = "Greyscale/Single-channel image required.";
		return -1;
	}
	try {
		cv::HoughCircles(pipe->input, pipe->circles, method->iValue, 2, minimumDistance->iValue, param1->fValue, param2->fValue, minimumRadius->fValue, maximumRadius->fValue);
	}
	catch(...){
		return -1;
	}
	return CVReturnType::CV_CIRCLES;
}
void CVHoughCircles::Paint(CVPipeline * pipe)
{
	// Convert to color!
	cv::Mat colorImage;
	int channels = pipe->output.channels();
	cv::cvtColor(pipe->output, colorImage, CV_GRAY2RGB);
//	pipe->output.convertTo(colorImage, CV_8UC3);
	int colorChannels = colorImage.channels();
	pipe->output = colorImage;
	int channelsPost = pipe->output.channels();
//	pipe->output = cv::Mat::zeros(pipe->input.size(), CV_8UC3);
	for( size_t i = 0; i < pipe->circles.size(); i++ )
	{
		cv::Point center(cvRound(pipe->circles[i][0]), cvRound(pipe->circles[i][1]));
		int radius = cvRound(pipe->circles[i][2]);
		// draw the circle center
		circle( pipe->output, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
		// draw the circle outline
		circle( pipe->output, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
	}
}

CVHoughLines::CVHoughLines()
	: CVDataFilter(CVFilterID::HOUGH_LINES)
{
	// CVFilterSetting * rho, * theta, * threshold, * minLineLEngth, * maxLineGap;
	rho = new CVFilterSetting("rho", 5.f);
	theta = new CVFilterSetting("theta", 0.1f);
	threshold = new CVFilterSetting("threshold", 100);
	minLineLength = new CVFilterSetting("min line length", 5.f);
	maxLineGap = new CVFilterSetting("max line gap", 5.f);

	settings.Add(rho);
	settings.Add(theta);
	settings.Add(threshold);
	settings.Add(minLineLength);
	settings.Add(maxLineGap);
}

int CVHoughLines::Process(CVPipeline * pipe)
{
	if (rho->fValue > 3000.f){
		errorString = "Use rho value below 3000 or it will crash, yo.";
		return -1;
	}
	if (pipe->input.channels() != 1)
	{
		errorString = "Input must be single-channel.";
		return -1;
	}
	int type = pipe->input.type();
	int depth = pipe->input.depth();
	switch(depth)
	{
		case CV_8U:
			break;
		case CV_32F:
			errorString = "Floating point (32-byte) image. Must be 8-byte.";
			return -1;
		default:
			errorString = "Bad depth, must be 8-byte or binary?";
			return -1;
	}
	try {
		// Use same list every time.
		static std::vector<cv::Vec4i> lines;
		cv::HoughLinesP(pipe->input, lines, rho->fValue, theta->fValue, threshold->iValue, minLineLength->fValue, maxLineGap->fValue);
		/// Convert to native type of lines.
		pipe->lines.Clear();
		for (int i = 0; i < lines.size(); ++i)
		{
			cv::Vec4i cvLine = lines[i];
			Line line(Vector3f(cvLine[0], cvLine[1], 0), Vector3f(cvLine[2], cvLine[3], 0));
			pipe->lines.Add(line);
		}
	}
	catch(cv::Exception & e)
	{
		errorString = e.what();
		List<String> tokens = errorString.Tokenize(":");
		errorString = tokens.Last();
		return -1;
	}
	catch(...){
		return -1;
	}
	return CVReturnType::LINES;
}

void CVHoughLines::Paint(CVPipeline * pipe)
{
	// Convert the color to colors again for visualization...
	cv::cvtColor(pipe->input, pipe->output, CV_GRAY2BGR);
	for( size_t i = 0; i < pipe->lines.Size(); i++ )
    {
		line( pipe->output, cv::Point(pipe->lines[i].start.x, pipe->lines[i].start.y),
			cv::Point(pipe->lines[i].stop.x, pipe->lines[i].stop.y), cv::Scalar(0,0,255), 3, 8 );
    }
}

CVFilterLinesByAngle::CVFilterLinesByAngle()
	: CVDataFilter(CVFilterID::FILTER_LINES_BY_ANGLE)
{
	angleToKeep = new CVFilterSetting("Angle to keep", 0.f);
	settings.Add(angleToKeep);
	allowedRange = new CVFilterSetting("Allowed range", 20.f);
	settings.Add(allowedRange);
	about = "Angles are in standard carteesian coordinates. I.e. 0 is horizontally right, 90 vertically up, 180 horizontally left.\nAngles are considered circular, meaning 0 and 180 are the same value.";
}
int CVFilterLinesByAngle::Process(CVPipeline * pipe)
{
	float i1Start, i1Stop, i2Start, i2Stop;
	i1Start = i2Start = angleToKeep->fValue - allowedRange->fValue;
	i1Stop = i2Stop = angleToKeep->fValue + allowedRange->fValue;
	if (i1Start < 0.f)
	{
		i2Start = i1Start + 180.f;
		i2Stop = 180.f;
	}
	else if (i1Stop > 180.f)
	{
		i2Start = 0.f;
		i2Stop = i1Stop - 180.f;
	}

	for (int i = 0; i < pipe->lines.Size(); ++i)
	{
		Line & line = pipe->lines[i];
		// Get angle, first radians.
		float radians = atan2(line.start.y - line.stop.y, line.start.x - line.stop.x);
		// Then to degrees.
		float degrees = radians * 180 / PI;
		if (degrees < 0)
			degrees += 180;
		else if (degrees > 180)
			degrees -= 180;

		// If outside both regions..
		if ((degrees > i2Stop || degrees < i2Start) && 
			(degrees > i1Stop || degrees < i1Start))
		{
			// Remove
			pipe->lines.RemoveIndex(i);
			--i;
			continue;
		}
		// Inside, keep it.
	}
	return CVReturnType::LINES;
}

CVMergeLines::CVMergeLines()
	: CVDataFilter(CVFilterID::MERGE_LINES)
{
	maxDistance = new CVFilterSetting("Max distance", 20.f);
	settings.Add(maxDistance);
	minimumAngleRatio = new CVFilterSetting("Minimum relative angle ratio", 0.95f);
	settings.Add(minimumAngleRatio);
}

int CVMergeLines::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->lines.Size(); ++i)
	{
		Line & line1 = pipe->lines[i];
		// Make sure it's normalized.
		Vector3f dir = line1.direction;
		for (int j = 0; j < pipe->lines.Size(); ++j)
		{
			if (i == j)
				continue;

			Line & line2 = pipe->lines[j];	
			// Check that the directions align.
			Vector3f dir2 = line2.direction;

			if (AbsoluteValue(dir.DotProduct(dir2)) < 0.95f)
				continue;
		
			// Check overlap distance. If not overlapping at all, skip 'em.
			if (line1.stop.x < line2.start.x ||
				line1.start.x > line2.stop.x)
				continue;

			// Check the distance between the lines.
			float distance = AbsoluteValue(line1.start.y - line2.start.y);
			if (distance < maxDistance->fValue)
			{
				// Merge 'em
				line1.MergeYExpandX(line2);
				pipe->lines.RemoveIndex(j);
				--j;
			}
		}
	}
	return CVReturnType::LINES;
}

// Approximates polygons out of contours
CVApproxPolygons::CVApproxPolygons()
	: CVDataFilter(CVFilterID::APPROXIMATE_POLYGONS)
{
	epsilon = new CVFilterSetting("Epsilon", 10.f);
	settings.Add(epsilon);
	minimumArea = new CVFilterSetting("Minimum area", 1000);
	settings.Add(minimumArea);
}
int CVApproxPolygons::Process(CVPipeline * pipe)
{			
	pipe->approximatedPolygons.clear();
	for (int i = 0; i < pipe->contours.size(); ++i)
	{
		try {
			std::vector<cv::Point> & contour = pipe->contours[i];
			cv::approxPolyDP(contour, approximatedPoly, epsilon->fValue, true);

			// Just copy over the contours for now?
//			std::cout<<"\nContour vertices: "<<approximatedPoly.size();
			if (approximatedPoly.size() > 4)
				continue;

			// Minimum area?
			float area = cv::contourArea(approximatedPoly);
			if (area < minimumArea->iValue)
				continue;
			pipe->approximatedPolygons.push_back(approximatedPoly);

		} catch(...)
		{
			return -1;
		}
	}
	return CVReturnType::APPROXIMATED_POLYGONS;
}

void CVApproxPolygons::Paint(CVPipeline * pipe)
{
	// Copy original input..
	pipe->initialInput->copyTo(pipe->output);
	RenderPolygons(pipe);
}

CVFindQuads::CVFindQuads()
	: CVDataFilter(CVFilterID::FIND_QUADS)
{
	// To filter out bad stuff.
	minimumWidth = new CVFilterSetting("Minimum width", 40);
	settings.Add(minimumWidth);
	maxLengthDiff = new CVFilterSetting("Max length difference", 40.f);
	settings.Add(maxLengthDiff);
	maxCornerXDiff = new CVFilterSetting("Max corner X diff", 40.f);
	settings.Add(maxCornerXDiff);
}

int CVFindQuads::Process(CVPipeline * pipe)
{
	pipe->quads.Clear();
	good = false;
	/// Check for ... in pipeline
	if (pipe->corners.size() == 0 &&
		pipe->lines.Size() == 0)
	{
		errorString = "No corners or lines in pipeline.";
		return -1;
	}
	float minimumWidthSquared = minimumWidth->iValue * minimumWidth->iValue;
	// Line-style!
	if (pipe->lines.Size())
	{
		int linesAtStart = pipe->lines.Size();
		std::cout<<"\nLines: "<<linesAtStart;
		
		/// Then try to locate at least 2 lines that are parallel first?
		// Approximate, assume 2 horizontal lines, and pick those two which are extremes (highest and lowest).
		List<Line> singleLines;
		for (int i = 0; i < pipe->lines.Size(); ++i)
		{
			Line line = pipe->lines[i];
			Line line2;
			bool foundPair = false;
			// Compare with list of lines.
			for (int j = 0; j < singleLines.Size(); ++j)
			{
				line2 = singleLines[j];
				// Any suitable? 
				// First check length.
				if (AbsoluteValue(line.Length() - line2.Length()) > maxLengthDiff->fValue)
					continue;

				// Ensure both the start and end points are pretty parallel
				if (AbsoluteValue(line.start.x - line2.start.x) > maxCornerXDiff->fValue)
					continue;
				if (AbsoluteValue(line.stop.x - line2.stop.x) > maxCornerXDiff->fValue)
					continue;

				foundPair = true;
				break;
			}

			if (foundPair)
			{
		// Limit the range in X for both lines so that they work "together" (overlapping)
#define Minimum(a,b) ( (a < b) ? a : b)
#define Maximum(a,b) ( (a > b) ? a : b)
				Line min, max;
				if (line.start.y < line2.start.y){
					min = line;
					max = line2;
				}
				else {
					min = line2;
					max = line;
				}
				min.stop.x = max.stop.x = Minimum(min.stop.x, max.stop.x);
				min.start.x = max.start.x = Maximum(min.start.x, max.start.x);

				// Create rectangle using these two lines! o-o
				Quad quad;
				quad.Set4Points(min.start, min.stop, max.stop, max.start);
				pipe->quads.Add(quad);
				good = true;
			}
			// If not, add this line to the list.
			else
				singleLines.Add(line);
		}
	
	}
	// Use corners if we have any.
	if (pipe->corners.size()){
		// Constraint for now...
		if (pipe->corners.size() != 4){
			errorString = "Skipping. Simple algorithm requires exactly 4 corners.";
			return -1;
		}
		// Look through all corners. Find the top 2 and bottom 2.
		List<cv::Point2f> sortedList, unsortedList;
		for (int i = 0; i < pipe->corners.size(); ++i)
			unsortedList.Add(pipe->corners[i]);
		/// Sort by Y-position.
		while (unsortedList.Size())
		{
			cv::Point2f max = unsortedList[0];
			int maxIndex = 0;
			for (int i = 1; i < unsortedList.Size(); ++i)
			{
				cv::Point2f point = pipe->corners[i];
				if (point.y > max.y){
					max = point;
					maxIndex = i;
				}
			}
			// Add the max to the sortedList and remove it from the list to be sorted.
			sortedList.Add(max);
			unsortedList.RemoveIndex(maxIndex);
			std::cout<<"\nAdded item"<<sortedList.Size()<<": "<<max;
		}
		/// Now, store top and bottom.
		List<cv::Point2f> top, bottom;
		top.Add(sortedList[0]);
		top.Add(sortedList[1]);
		bottom.Add(sortedList[2]);
		bottom.Add(sortedList[3]);
	
	#define ToVec3f(cvVec) (Vector3f(cvVec.x, cvVec.y, 0))
	
		if (top[0].x < top[1].x)
		{
			topLeft = ToVec3f(top[0]);
			topRight = ToVec3f(top[1]);
		}
		else {
			topLeft = ToVec3f(top[1]);
			topRight = ToVec3f(top[0]);
		}
		if (bottom[0].x < bottom[1].x)
		{
			bottomLeft = ToVec3f(bottom[0]);
			bottomRight = ToVec3f(bottom[1]);
		}
		else 
		{
			bottomLeft = ToVec3f(bottom[1]);
			bottomRight = ToVec3f(bottom[0]);
		}

		// Fetch vectors for lines between the top and bottom points.
		// From left to right.
		topLine = Vector2f(topRight.x - topLeft.x, topRight.y - topLeft.y);
		bottomLine = Vector2f(bottomRight.x - bottomLeft.x, bottomRight.y - bottomLeft.y);
	
		// Normalize them before dot-product
		topLine.Normalize();
		bottomLine.Normalize();

		// Check that they are parralel
		float dotProduct = topLine.DotProduct(bottomLine);
		if (AbsoluteValue(dotProduct) < 0.9f)
		{
			errorString = "Bottom and top lines not parralell enough.";
			return -1;
		}

		// Fetch vector between left top and left bottom.
		Vector2f topLeftToBottomLeft(bottomLeft.x - topLeft.x, bottomLeft.y - topLeft.y);

		// Check that it is perpendicular to either top or bottom lines.
		topLeftToBottomLeft.Normalize();
		dotProduct = topLine.DotProduct(topLeftToBottomLeft);
		if (AbsoluteValue(dotProduct) > 0.1f)
		{
			errorString = "Left and top lines not perpendicular enough.";
			return -1;
		}


		// We have a quad if we reach here!
		Quad quad;
		quad.Set4Points(ToVec3f(bottomLeft), ToVec3f(bottomRight), ToVec3f(topRight), ToVec3f(topLeft));
		pipe->quads.Add(quad);
	
		good = true;
	}
	return CVReturnType::QUADS;
}

CVQuadAspectRatioConstraint::CVQuadAspectRatioConstraint()
	: CVDataFilter(CVFilterID::QUAD_ASPECT_RATIO_CONSTRAINT)
{
	minimumRatio = new CVFilterSetting("Minimum ratio", 1.2f);
	maximumRatio = new CVFilterSetting("Maximum ratio", 3.35f);
	settings.Add(minimumRatio);
	settings.Add(maximumRatio);
}

int CVQuadAspectRatioConstraint::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->quads.Size(); ++i)
	{
		Quad & quad = pipe->quads[i];
		Vector3f min = quad.point1;
		Vector3f max = quad.point3;
		// OK, max and min lines found.
		// Compare with acceptable aspect ratios.
		float width = max.x - min.x;
		float height = max.y - min.y;
		float ratio = width / height;
		bool ratioGood = true;
		if (ratio < minimumRatio->fValue)
			ratioGood = false;
		else if (ratio > maximumRatio->fValue)
			ratioGood = false;
		if (ratioGood)
			continue;
		// Delete if bad.
		pipe->quads.RemoveIndex(i);
		--i;
	}
	return CVReturnType::QUADS;
}

CVMaxBoxPersistance::CVMaxBoxPersistance()
	: CVDataFilter(CVFilterID::MAX_BOX_PERSISTANCE)
{
	maxFramesToConsider = new CVFilterSetting("Max frames", 30);
	settings.Add(maxFramesToConsider);
	threshold = new CVFilterSetting("Treshold", 30);
	settings.Add(threshold);
	lastSeenQuad = Timer::GetCurrentTimeMs();
}

int CVMaxBoxPersistance::Process(CVPipeline * pipe)
{
	if (pipe->quads.Size())
	{
		/// Take quads from pipeline and insert into our array.
		previousQuads.Add(pipe->quads[0]);
		/// If array is long enough, delete oldest one.
		if (previousQuads.Size() > maxFramesToConsider->iValue)
			previousQuads.RemoveIndex(0, ListOption::RETAIN_ORDER);
		// Take note of the time
		this->lastSeenQuad = lastSeenQuad = Timer::GetCurrentTimeMs();
	}
	else if (this->lastSeenQuad < Timer::GetCurrentTimeMs() - 2000)
	{
		errorString = "No valid quad seen in the past 2 seconds.";
		return -1;
	}
	if (previousQuads.Size() <= 0)
	{
		errorString = "No quads to calculate on";
		return -1;
	}

	// First fetch biggest box.
	Quad biggest = previousQuads[0];
	for (int i = 1; i < previousQuads.Size(); ++i)
	{
		Quad & quad = previousQuads[i];
		if (quad.ManhattanSize() > biggest.ManhattanSize())
			biggest = quad;
	}

	// Then filter out those boxes not within threshold value of biggest one.
	List<Quad> quadsToConsider;
	for (int i = 0; i < previousQuads.Size(); ++i)
	{
		Quad & quad = previousQuads[i];
		if (quad.ManhattanSize() > biggest.ManhattanSize() - threshold->iValue)
			quadsToConsider.Add(quad);
	}
	/// Calculate an "average" quad based on our known array, skipping 
	/// those which are not within a current threshold of total 
	/// manhattan distance differing from the biggest quad.
	Quad average;
	Vector2i min, max;
	for (int i = 0; i < quadsToConsider.Size(); ++i)
	{
		Quad & quad = quadsToConsider[i];
		min += quad.point1;
		max += quad.point3;
	}
	std::cout<<"\nQuads considered: "<<quadsToConsider.Size();

	min /= quadsToConsider.Size();
	max /= quadsToConsider.Size();
	average.Set2Points(min, max);
	pipe->quads.Clear();
	pipe->quads.Add(average);

	return CVReturnType::QUADS;
}

/*
void CVMaxBoxPersistance::Paint(CVPipeline * pipe)
{
	if (!pipe->quads.Size())
		return;
	// Convert to color if needed.
	int channelsBefore = pipe->output.channels();
	if (channelsBefore == 1)
	{
		pipe->output.convertTo(pipe->output, CV_8UC3);
		cv::cvtColor(pipe->output, pipe->output, CV_GRAY2BGR);
	}
	// o.o Paste!
	Quad quad = pipe->quads[0];
	cv::Scalar color = RGB(255,0,0);
	cv::Mat rectImage = cv::Mat::zeros(pipe->output.size(), CV_8UC3);
	cv::rectangle(rectImage, cv::Point(quad.point1.x, quad.point1.y), cv::Point(quad.point3.x, quad.point3.y), color, CV_FILLED);
	float alpha = 0.5f;
	float beta = 1 - alpha;
	cv::addWeighted(rectImage, alpha, pipe->output, beta, 0.0, pipe->output); 
//	rectImage.copyTo(pipe->output);

}*/


// Copy-pasted stuff from random website and russian's github. Works using angle-comparisons at certain steps along a contour.
CVHandDetector::CVHandDetector()
	: CVDataFilter(CVFilterID::HAND_DETECTOR)
{
	minimumArea = new CVFilterSetting("Minimum area", 1000);
	cosThreshold = new CVFilterSetting("Cosinus threshold", 0.5f);
	equalThreshold = new CVFilterSetting("Equal threshold", 1e-7f);
	r = new CVFilterSetting("r", 40);
	step = new CVFilterSetting("step", 16);

	settings.Add(minimumArea);
	settings.Add(cosThreshold);
	settings.Add(equalThreshold);
	settings.Add(r);
	settings.Add(step);

	about = "Hand-detector utilizing angles along the contours.";
}

// Checks if two points are considered equal, given set threshold value.
bool CVHandDetector::IsEqual(double a, double b)
{
	return fabs(a - b) <= equalThreshold->fValue;
}

// Computes angle between some points on a contour?
double CVHandDetector::Angle(std::vector<cv::Point>& contour, int pt, int r)
{
	int size = contour.size();
	// Fetch point, using the contour as a circular array.
	cv::Point p0 = (pt > 0) ? contour[pt % size] : contour[size - 1 + pt];
	// Fetch point r steps forward.
	cv::Point p1 = contour[(pt + r) % size];
	// Fetch point r steps behind.
	cv::Point p2 = (pt > r) ? contour[pt - r] : contour[size - 1 - r];

	// Compute angle 
	double ux = p0.x-p1.x;
	double uy = p0.y-p1.y;
	double vx = p0.x-p2.x;
	double vy = p0.y-p2.y;
	return (ux*vx + uy*vy) / sqrt((ux*ux + uy*uy)*(vx*vx + vy*vy));
}

signed int CVHandDetector::Rotation(std::vector<cv::Point>& contour, int pt, int r)
{
	int size = contour.size();
	cv::Point p0=(pt>0)?contour[pt%size]:contour[size-1+pt];
	cv::Point p1=contour[(pt+r)%size];
	cv::Point p2=(pt>r)?contour[pt-r]:contour[size-1-r];

	double ux=p0.x-p1.x;
	double uy=p0.y-p1.y;
	double vx=p0.x-p2.x;
	double vy=p0.y-p2.y;
	return (ux*vy - vx*uy);
}

// Tries to detect hands using given contours.
int CVHandDetector::Process(CVPipeline * pipe)
{
	// Make sure we have contours to work with.
	if (!pipe->contours.size())
	{
		errorString = "No contours to work with.";
		return CVReturnType::HANDS;
	}

	std::vector<std::vector<cv::Point> > & contours = pipe->contours;

	try {
		// Delete old hands
		pipe->hands.Clear();
		// Find the hands.
		for(int i = 0; i < contours.size(); i++)
		{
			std::vector<cv::Point> & contour = contours[i];
			// Compare with minimum area for contour to be considered.
			int contourArea = cv::contourArea(contour);
			if(contourArea > minimumArea->iValue)
			{
				Hand tmp;
				cv::Moments m = cv::moments(contour);
				tmp.center.x = m.m10 / m.m00;
				tmp.center.y = m.m01 / m.m00;
				tmp.contourAreaSize = contourArea;

				// For each point in the contour.. do something..
				for(int j = 0; j < contour.size(); j += step->iValue )
				{
					// Check current angle/velocity change at point j, compared to points j+r and j-r
					double cos0 = Angle(contour, j, r->iValue);

					// If the angle is big enough, and not at the end of the array...?
					if ((cos0 > 0.5) && (j + step->iValue < contours[i].size()))
					{
						// Do 2 more checks, for some reason, [step] behind and in front of j.
						double cos1 = Angle(contour, j - step->iValue, r->iValue);
						double cos2 = Angle(contour, j + step->iValue, r->iValue);
						// Fetch maximum change of all 3.
						double maxCos = Maximum(Maximum(cos0, cos1), cos2);
						bool equal = IsEqual(maxCos, cos0);
						signed int z = Rotation (contour, j, r->iValue);
						// Add finger.. o.o
						if (equal == 1 && z < 0)
						{
							cv::Point contourLocation = contour[j];
							tmp.fingers.Add(Vector2f(contourLocation.x, contourLocation.y));
						}
					}
				}
				tmp.contour=contours[i];
				pipe->hands.Add(tmp);
			}
		}
	} catch (...)
	{
		errorString = "Error D:";
		return -1;
	}
	return CVReturnType::HANDS;
}

CVConvexHull::CVConvexHull()
	: CVDataFilter(CVFilterID::CALC_CONVEX_HULL)
{
}
int CVConvexHull::Process(CVPipeline * pipe)
{
	// Calculate convex hull of the contour.
	if (!pipe->contours.size()){
		errorString = "No contours to process";
		return CVReturnType::CV_CONVEX_HULLS;
	}
	cv::convexHull(pipe->contours[0], pipe->convexHull);
	return CVReturnType::CV_CONVEX_HULLS;
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
	if (pipe->contours.size() == 0)
	{
		return CVReturnType::CV_CONVEXITY_DEFECTS;
	}
	cv::convexityDefects(pipe->contours[0], pipe->convexHull, convexityDefects);
	for (int j = 0; j < convexityDefects.size(); ++j)
	{
		cv::Vec4i & defect = convexityDefects[j];
		if (defect[3] > minimumDefectDepth->fValue)
			pipe->convexityDefects.Add(defect);
	}
	return CVReturnType::CV_CONVEXITY_DEFECTS;
}


// Own custom hand-detector, identifying fingers by first identifying relevant convexity defects (convex hull - contour)
CVHandDetector2::CVHandDetector2()
	: CVDataFilter(CVFilterID::HAND_DETECTOR_2)
{
	minimumArea = new CVFilterSetting("Minimum contour area", 1000);
	minimumFingerDistance = new CVFilterSetting("Minimum finger distance", 200);
	settings.Add(minimumArea);
	settings.Add(minimumFingerDistance);
	about = "Hand-detector utilizing contour-convex-hull defects.";
}

using namespace std;

int CVHandDetector2::Process(CVPipeline * pipe)
{
	// Clear hands if any.
	pipe->hands.Clear();
	for (int i = 0; i < pipe->contours.size() && i < 1; ++i)
	{
		std::vector<cv::Point> contour = pipe->contours[i];
		float contourArea = cv::contourArea(contour);
		// Skip areas below minimum size threshold
		if (contourArea < minimumArea->iValue)
			continue;
		Hand newHand;
		
		cv::Moments m = cv::moments(contour);
		newHand.center.x = m.m10 / m.m00;
		newHand.center.y = m.m01 / m.m00;
		newHand.contourAreaSize = contourArea;
		newHand.boundingRect = cv::boundingRect(contour);
		// Calculate rect too.

		// Creat fingerrrrs ovo
		if (pipe->convexityDefects.Size())
		{
			// Create a "finger" at the start and end of each defect.
			for (int j = 0; j < pipe->convexityDefects.Size(); ++j)
			{
				cv::Vec4i defect = pipe->convexityDefects[j];
				if (defect[0] >= contour.size() || defect[1] >= contour.size())
					continue;
				cv::Point point = contour[defect[0]];
				Vector2f finger = Vector2f(point.x, point.y);
				newHand.fingers.Add(finger);
				point = contour[defect[1]];
				finger = Vector2f(point.x, point.y);
				newHand.fingers.Add(finger);
			}
			
			// Merge duplicate fingers
			for (int j = 0; j < newHand.fingers.Size(); ++j)
			{
				Vector2f finger1 = newHand.fingers[j];
				for (int k = j + 1; k < newHand.fingers.Size(); ++k)
				{
					Vector2f finger2 = newHand.fingers[k];
					float distance = (finger2 - finger1).Length();
					if (distance < minimumFingerDistance->iValue)
					{
						// Erase finger index k if they are considered equal.
						newHand.fingers.RemoveIndex(k);
						--k;
					}
				}
			}
			
		}
	
		pipe->hands.Add(newHand);
	}

	return CVReturnType::HANDS;
}




CVHandPersistance::CVHandPersistance()
	: CVDataFilter(CVFilterID::HAND_PERSISTENCE)
{
	// Stuff.
	distanceThreshold = new CVFilterSetting("Distance threshold", 20.f);
}

int CVHandPersistance::Process(CVPipeline * pipe)
{
	// Remove all but the largest hand..? 
	int largestIndex = -1;
	int largestSize = 0;
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		Hand & hand = pipe->hands[i];
		if (hand.contourAreaSize > largestSize)
		{
			largestIndex = i;
			largestSize = hand.contourAreaSize;
		}
	}
	if (largestIndex > 0)
	{
		Hand biggest = pipe->hands[largestIndex];
		pipe->hands.Clear();
		pipe->hands.Add(biggest);
	}

	// Do an initial check 
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		Hand & hand = pipe->hands[i];
		Vector3f center(hand.center.x, hand.center.y, 0);
		Vector3f generalDirection;
		// Removes fingers not aligned with the general direction of the hands.
		for (int i = 0; i < hand.fingers.Size(); ++i)
		{
			Vector3f point = hand.fingers[i];
			Vector3f fingerDirection = point - center;
			generalDirection += fingerDirection;
		}
		// Normalize the direction.
		generalDirection.Normalize();
		// Go through the fingers again, this time removing erroneous ones.
		List<Vector2f> culledFingers;
		for (int i = 0; i < hand.fingers.Size(); ++i)
		{
			Vector3f point = hand.fingers[i];
			Vector3f fingerDirection = point - center;
			if (fingerDirection.DotProduct(generalDirection) > 0)
			{
				culledFingers.Add(point);
			}
		}
		hand.fingers = culledFingers;
		/*
		if (hand.contour.size() < contourThreshold->iValue)
		{
			pipe->hands.RemoveIndex(i);
			--i;
		}
		*/
	}
	
//	List<Hand> newHands = pipe->hands;
	
	/// Only do time comparison after we have at least a frame.
	/*
	if (pastFramesHands.Size())
	{
		// Fetch Past frame
		List<Hand> lastFramesHands = pastFramesHands.Last();

		// Compare with previous' frames hands. Are they the same?
		for (int i = 0; i < lastFramesHands.Size(); ++ i)
		{
			for (int i = 0; i < pipe->hands.Size(); ++i)
			{
				// Just check distance of the centers
				Hand & pastFrame = lastFramesHands[i];
				Hand & newHand = pipe->hands[i];
				Vector3f pointDistance = pastFrame.center - newHand.center;
				float distance = hypot(pointDistance.x, pointDistance.y);
				if (distance < distanceThreshold->fValue)
				{
					// Keep it.

				}
				else 
				{
					// Discard it.
					pipe->hands.RemoveIndex(i);
					--i;
				}
			
			}
		}
	}

	// Add the current frame's hands to the history
	pastFramesHands.Add(pipe->hands);
	*/
	// Return all the hands o-o
	return CVReturnType::HANDS;
}

CVFingerActionFilter::CVFingerActionFilter()
	: CVDataFilter(CVFilterID::FINGER_ACTION)
{
	minimumDuration = new CVFilterSetting("Minimum state duration", 500);
	settings.Add(minimumDuration);
	maxStatesStored = new CVFilterSetting("Max states stored", 20);
	settings.Add(maxStatesStored);
}
int CVFingerActionFilter::Process(CVPipeline * pipe)
{
	Hand hand;
	if (pipe->hands.Size())
		hand = pipe->hands[0];

	// Check finger now, but update it to the last finger after all processing is done.
	int fingersNow = hand.fingers.Size();
	
	int64 now = Timer::GetCurrentTimeMs();

	// When finger state has changed.
	if (fingersNow != lastFinger)
	{
		lastFinger = fingersNow;
		lastFingerStart = Timer::GetCurrentTimeMs();
		// Adjust the current/last state as it exits scope.
		if (fingerStates.Size())
		{
			FingerState & lastFingerState = fingerStates.Last();
			lastFingerState.stop = now;
			lastFingerState.duration = lastFingerState.stop - lastFingerState.start;
		}
		// Create new state for the new finger amount.
		FingerState newState;
		newState.start = now;
		newState.fingers = fingersNow;
		for (int i = 0; i < hand.fingers.Size(); ++i)
		{
			newState.positions.Add(hand.fingers[i]);
		}
		fingerStates.Add(newState);
		// If we pass 20 or something, start deleting old states.
		if (fingerStates.Size() >= maxStatesStored->iValue)
			fingerStates.RemoveIndex(0, ListOption::RETAIN_ORDER);
	}
	else {
		lastFingerDuration = Timer::GetCurrentTimeMs() - lastFingerStart;
		if (fingerStates.Size())
		{
			FingerState & lastFingerState = fingerStates.Last();
			lastFingerState.duration = lastFingerDuration;
		}
	}	
	
	// Synchronize the pipeline list of finger states with our own.
	int lastAddedStateIndex = 0;
	for (int i = 0; i < fingerStates.Size(); ++i)
	{
		FingerState & state = fingerStates[i];
		for (int j = 0; j < pipe->fingerStates.Size(); ++j)
		{
			FingerState & pipeState = pipe->fingerStates[j];
			if (pipeState.start == state.start)
			{
				// Values that could have been updated from outside.
				state.processed = pipeState.processed;
				lastAddedStateIndex = i;
			}
		}
	}

	// Remove any finger-states (except for the MOST RECENT one) whose duration is below the threshold. Merge the states beside if they have the same amount of fingers too!
	FingerState * lastState = NULL;
	for (int i = Maximum(0, fingerStates.Size() - 3); i < fingerStates.Size() - 1; )
	{
		FingerState * state = &fingerStates[i];
		if (state->duration > minimumDuration->iValue)
		{
			// Set last finger state for upcoming merges.
			lastState = &fingerStates[i];
			++i;
			continue;
		}
		std::cout<<"\nRemoving start with start time: "<<fingerStates[i].start<<" and fingers: "<<fingerStates[i].fingers;

		// Ok, we are to remove this one. So, do it!
		fingerStates.RemoveIndex(i, ListOption::RETAIN_ORDER);
		
		// Now check if we have a last state.
		if (!lastState)
		{
			// If not, no worries. 
			continue;
		}
		// But if we do, check the next state too.
		state = &fingerStates[i];
		
		// Because if they are the same number of fingers we'll want to merge them, 
		// considering them the same "state" with the one in-between being considered as noise,
		// or unwanted stuff anyway.
		if (state->fingers == lastState->fingers)
		{
			// They are the same! Mergy bergy!
			lastState->stop = state->stop;
			lastState->duration = now - lastState->start;
			// Remove this state, since the older one takes precedence
			fingerStates.RemoveIndex(i, ListOption::RETAIN_ORDER);
			continue;
		}	
		// Set pointers
		lastState = &fingerStates[i];
	}


	// Clear their list.
	pipe->fingerStates.Clear();
	// Plus 1 of the last added state or we will accidentally add it again!
	for (int i = 0; i < fingerStates.Size(); ++i)
	{
		FingerState & state = fingerStates[i];
		if (state.duration < minimumDuration->iValue)
			continue;
		// Copy-bopy!
		pipe->fingerStates.Add(state);
	}
	while(pipe->fingerStates.Size() > maxStatesStored->iValue)
		pipe->fingerStates.RemoveIndex(0, ListOption::RETAIN_ORDER);

	// Should replace with "fingers" type and render them somewhere...
	return CVReturnType::HANDS;
}



