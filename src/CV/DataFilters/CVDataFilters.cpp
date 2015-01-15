/// Emil Hedemalm
/// 2014-04-11
/** Data-based filters that generate point- and/or blob-clouds.
*/

#include "CVDataFilters.h"
#include "CV/CVPipeline.h"
#include "String/StringUtil.h"
#include "Texture.h"
#include "TextureManager.h"
cv::RNG rng(12345);

#include "Time/Time.h"


CVDataFilter::CVDataFilter(int id)
	: CVFilter(id)
{
	type = CVFilterType::DATA_FILTER;
	returnType = CVReturnType::NOTHING;
};

// Ensure that sub-classes may be deallocated correcty.
CVDataFilter::~CVDataFilter()
{
}


void CVDataFilter::Paint(CVPipeline * pipe)
{
	if (returnType == CVReturnType::QUADS)
	{
		// Draw quads.
		if (!pipe->quads.Size())
			return;
		// Copy original input
		pipe->initialInput.copyTo(pipe->output);
		// Convert to color if needed.
		int channelsBefore = pipe->output.channels();
		if (channelsBefore == 1)
		{
		//	pipe->output.convertTo(pipe->output, CV_8UC3);
			cv::cvtColor(pipe->output, pipe->output, CV_GRAY2RGB);
		}
		int channelsAfter = pipe->output.channels();
		// o.o Paste!
		for (int i = 0; i < pipe->quads.Size(); ++i)
		{
			Quad quad = pipe->quads[i];
	#define RGB(r,g,b) cv::Scalar(b,g,r)
			rng = cv::RNG(1344);
			cv::Scalar color = RGB(rng.uniform(0, 255),rng.uniform(0, 255),rng.uniform(0, 255));
		//	cv::Mat rectImage = cv::Mat::zeros(pipe->output.size(), CV_8UC3);
			cv::rectangle(pipe->output, cv::Point(quad.point1.x, quad.point1.y), cv::Point(quad.point3.x, quad.point3.y), color, CV_FILLED);
			float alpha = 0.2f;
			float beta = 1 - alpha;
		//	cv::addWeighted(rectImage, alpha, pipe->output, beta, 0.0, pipe->output); 
		//	rectImage.copyTo(pipe->output);		
		}
	}
	else if (returnType == CVReturnType::APPROXIMATED_POLYGONS)
	{
		// Copy original input..
		pipe->initialInput.copyTo(pipe->output);
		RenderPolygons(pipe);
	}
	else if (returnType == CVReturnType::CV_IMAGE)
	{
		// Do nothing as the image should already be in the ouput matrix of the pipeline.
	}
	else if (returnType == CVReturnType::LINES ||
		returnType == CVReturnType::CV_LINES)
	{
		// Convert the color to colors again for visualization...
		pipe->initialInput.copyTo(pipe->output);
		for( size_t i = 0; i < pipe->lines.Size(); i++ )
		{
			Line & line = pipe->lines[i];
			// Multiply the line coordinates with the last used inverted scale.
			line.start *= pipe->currentScaleInv;
			line.stop *= pipe->currentScaleInv;

			cv::line( pipe->output, cv::Point(line.start.x, line.start.y),
				cv::Point(line.stop.x, line.stop.y), cv::Scalar(0,0,255), 3, 8 );
		}
	}
	else if (returnType == CVReturnType::CV_CONTOURS)
	{
		pipe->initialInput.copyTo(pipe->output);
		RenderContours(pipe);
	}

	switch(returnType)
	{
		case CVReturnType::CV_CONTOUR_SEGMENTS:
		{
			// Render shit!
			pipe->initialInput.copyTo(pipe->output);
			RenderContourSegments(pipe);
			break;
		}
	}

	if (returnType == CVReturnType::CV_CONTOUR_ELLIPSES)
	{
		pipe->initialInput.copyTo(pipe->output);
		RenderContours(pipe);
		RenderContourBounds(pipe);
	}
	else if (returnType == CVReturnType::CV_CONVEX_HULLS)
	{
		pipe->initialInput.copyTo(pipe->output);
		RenderContours(pipe);
		RenderConvexHulls(pipe);
	}
	else if (returnType == CVReturnType::CV_CONVEXITY_DEFECTS)
	{
		pipe->initialInput.copyTo(pipe->output);
		RenderContours(pipe);
		RenderConvexHulls(pipe);
		RenderConvexityDefects(pipe);
	}
	else if (returnType == CVReturnType::HANDS)
	{
		// Convert image to RGB for easier display
		int channelsBefore = pipe->initialInput.channels();
	//	cv::cvtColor(*pipe->initialInput, pipe->output, CV_GRAY2RGB);
		pipe->initialInput.copyTo(pipe->output);
		int channelsAfter = pipe->output.channels();
		// Check if we got contour-segments, if so prioritize them.
		RenderHands(pipe);
	}
	else if (returnType == CVReturnType::FINGER_STATES)
	{
		// Render the last known one.
		pipe->initialInput.copyTo(pipe->output);
		FingerState & state = pipe->fingerStates.Last();
		cv::Scalar color(255,0,0,255);
		for (int i = 0; i < state.positions.Size(); ++i)
		{
			Vector3f pos = state.positions[i];
			cv::circle(pipe->output, cv::Point(pos.x, pos.y), 5, color, 3);
		}
	}
	else if (returnType == CVReturnType::POINT_CLOUDS)
	{
		pipe->initialInput.copyTo(pipe->output);
		for (int i = 0; i < pipe->pointClouds.Size(); ++i)
		{
			int r,g,b,a;
			r = g = b = a = 255;
			if (i % 2 == 0)
				g = 0;
			if (i % 4 == 0)
				b = 0;
			cv::Scalar color(r,g,b,a);
			
			CVPointCloud & pc = pipe->pointClouds[i];
			for (int j = 0; j < pc.points.Size(); ++j)
			{
				Vector2f & pos = pc.points[j];
				cv::circle(pipe->output, cv::Point(pos.x, pos.y), 2, color, 3);
			}

			/// Render PCA data if applicable.
			for (int j = 0; j < pc.eigenVectors.Size(); ++j)
			{
				cv::Scalar color = j == 0? CV_RGB(255, 255, 0) : CV_RGB(0, 255, 255);
				cv::Point2f eigenVector(pc.eigenVectors[j].x, pc.eigenVectors[j].y);
				float eigenValue = pc.eigenValues[j];

				cv::Point2f scaledEigenVector = eigenVector * eigenValue * 0.02;

				cv::Point2f pcaCenter(pc.pcaCenter.x, pc.pcaCenter.y);
				circle(pipe->output, pcaCenter, 3, CV_RGB(255, 0, 255), 2);
				line(pipe->output, pcaCenter, pcaCenter + scaledEigenVector, color);
			}
		}
	}
	else if (returnType == CVReturnType::CV_OPTICAL_FLOW)
	{
		// http://stackoverflow.com/questions/7693561/opencv-displaying-a-2-channel-image-optical-flow
		// Split the coordinates.
		/*
		cv::Mat xy[2];
		cv::split(pipe->opticalFlow, xy);

		cv::Mat magnitude, angle;

		// Fetch matrix from pipeline.
		cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);

		double magMax;
		cv::minMaxLoc(magnitude, 0, &magMax);
		magnitude.convertTo(magnitude, -1, 1.0 / magMax);
		
		// Build HSV image?
		cv::Mat hsvChannels[3], hsv;
		hsvChannels[0] = angle;
		hsvChannels[1] = cv::Mat::ones(angle.size(), CV_32F);
		hsvChannels[2] = magnitude;
		cv::merge(hsvChannels, 3, hsv);

		// Convert to rgb and send to output.
		cv::cvtColor(hsv, pipe->output, cv::COLOR_HSV2RGB);
		*/
	}
	else if (returnType == CVReturnType::NOTHING)
	{
		// Nothing to render...
	}
	else if (returnType == -1)
	{
		// Nothing to render if error.
		std::cout<<"\nreturnType variable in filter "<<name<<" not set!";
	}
	else if (returnType == CVReturnType::RENDER)
		// Nothing to render if render.
		;
	else
		; // std::cout<<"\nCVDataFilter::Paint called. Forgot to subclass the paint-method?";
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

void RenderContourSegments(CVContour * contour, CVPipeline * pipe)
{
	Vector3f center = contour->largestInnerCircleCenter;
	if (contour->largestInnerCircleRadius < 0)
		return;
	// Paint center of the contour.
	Vector4f handColor(255.f,200.f,155.f,1.f);
#define V_TO_CVV(v) cv::Scalar(v.z,v.y,v.x)
	cv::circle(pipe->output, cv::Point(center.x, center.y), contour->largestInnerCircleRadius, V_TO_CVV(handColor), 2);
	cv::circle(pipe->output, cv::Point(center.x, center.y), 2, cv::Scalar(255,255,255), 2);
	
	// Set color of the segment appropriately.
	for (int i = 0; i < contour->segments.Size(); ++i)
	{
		CVContourSegment & seg = contour->segments[i];

		Vector4f color = Vector4f(1,1,1,1);
		switch(seg.type)
		{
			case SegmentType::EDGE:
				color = Vector4f(0.3f, 0.2f, 0.4f, 1.f);
				break;
			default:
			case SegmentType::NONE:
				color = Vector4f(0.6f,0.6f,0.5f,1);
				break;
			case SegmentType::EXTENDED_FINGER_START:
				color = Vector4f(1,0,0,1);
				break;
			case SegmentType::EXTENDED_FINGER:
				color = Vector4f(0,1,0,1);
				break;
			case SegmentType::EXTENDED_FINGER_STOP:
				color = Vector4f(0,0,1,1);
				break;
		}
		if (seg.start)
			color += Vector4f(0.5f,0.5f,0.5f,0.5f);
		// Paint in both textures.
		Vector2i coord = Vector2i(int(seg.angleDistanceStart.x * contourSegmentRelativeAngleDistanceTexture->width) % contourSegmentRelativeAngleDistanceTexture->width, 
			int(seg.angleDistanceStart.y * contourSegmentRelativeAngleDistanceTexture->height * 0.1f) % contourSegmentRelativeAngleDistanceTexture->height);
		contourSegmentRelativeAngleDistanceTexture->SetPixel(coord, color, 3);
		color *= 255.f;
		Vector3f rawStart = seg.rawInputStart;
		cv::Point point(rawStart.x, rawStart.y);
		cv::circle(pipe->output, point, 3, cv::Scalar(color.z, color.y, color.x), 1);
	}
}

/// Yeah.
void RenderContourSegments(CVPipeline * pipe)
{
	cv::Mat * pipelineTexture = &pipe->output;

	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour * contour = &pipe->contours[i];
		RenderContourSegments(contour, pipe);
	}
}


void RenderContour(CVContour * contour, CVPipeline * pipe)
{
	assert(false);
}

void RenderContours(CVPipeline * pipe)
{
	/// Use same random seed every time to avoid rainbow hell..
	rng = cv::RNG(12345);
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{			
		CVContour * contour = &pipe->contours[i];
		if (contour->segments.Size())
			RenderContourSegments(contour, pipe);
		else 
		{
			cv::Scalar color = cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255));
			cv::drawContours(pipe->output, pipe->cvContours, i, color, 2, 8, pipe->contourHierarchy, 0, cv::Point());
		}
	}
}

void RenderContourBounds(CVPipeline * pipe)
{
	rng = cv::RNG(5553);
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour * contour = &pipe->contours[i];
		cv::Scalar color;
		switch(contour->contourClass)
		{
			case ContourClass::FINGERTIP: 
				color = cv::Scalar(255, 0, 0);	
				break;
			case ContourClass::FINGER: 
				color = cv::Scalar(0, 255, 0);	
				break;
			default: color = cv::Scalar(rng.uniform(0,225), rng.uniform(0,225), rng.uniform(0,255));
		}
		switch(contour->boundingType)
		{
			case BoundingType::ELLIPSE:
				// ellipse
				ellipse(pipe->output, contour->boundingEllipse, color, 2, 8 );
				break;
			case BoundingType::RECT: 
			{
				// rotated rectangle
				cv::Point2f rect_points[4]; 
				contour->boundingRect.points( rect_points );
				for( int j = 0; j < 4; j++ )
					line(pipe->output, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
				break;
			}
		}
	}
}


void RenderConvexHulls(CVPipeline * pipe)
{
	if (pipe->cvContours.size() == 0)
		return;
	cv::Scalar color = cv::Scalar(rng.uniform(125,255), rng.uniform(125,255), rng.uniform(125,255));
//	cv::drawContours(pipe->output, pipe->convexHull, 0, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );
	std::vector<int> & convexHull = pipe->convexHull;
	std::vector<cv::Point> & contour = pipe->cvContours[0];
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
		cv::Point farthestPoint = pipe->cvContours[0][farthestPointIndex];
		// Render point furthest away?
		cv::circle(pipe->output, farthestPoint, 3, color, 5);
	}
}

// Wat..
void RenderHands(CVPipeline * pipe)
{
	List<CVHand> & hands = pipe->hands;
//	std::vector<std::vector<cv::Point> > c;
	for(int i = 0; i < hands.Size(); i++)
	{
		CVHand & hand = hands[i];

		if (hand.contour && hand.contour->segments.Size())
			RenderContourSegments(hand.contour, pipe);
		else
			RenderContours(pipe);
	

	//	assert(hand.contour);
	//	c.clear();
	//	c.push_back(hand.contour->cvPointList);
		cv::circle(pipe->output, cv::Point(hand.center.x, hand.center.y), 20, cv::Scalar(0, 0, 255), 2);
		int fingersSize = hand.fingers.Size();
		for(int j = 0; j < fingersSize; j++)
		{
#define VEC3FTOCVPOINT(a) (cv::Point(a.x,a.y))
			CVFinger & finger = hand.fingers[j];
			Vector3f fingerPoint = finger.point;
			Vector4f fingerColor = finger.GetColor();
#define VECTOCVSCALAR(a) (cv::Scalar(a.z,a.y,a.x))
			cv::circle(pipe->output, cv::Point(fingerPoint.x, fingerPoint.y), 10, VECTOCVSCALAR(fingerColor), 2);
			cv::line(pipe->output, VEC3FTOCVPOINT(hand.center), VEC3FTOCVPOINT(fingerPoint), VECTOCVSCALAR(fingerColor), 4);

			Vector3f start = finger.start;
			if (start.MaxPart())
				cv::circle(pipe->output, cv::Point(start.x, start.y), 5, cv::Scalar(255,0,125), 2);
			Vector3f stop = finger.stop;
			if (stop.MaxPart())
				cv::circle(pipe->output, cv::Point(stop.x, stop.y), 5, cv::Scalar(0,255,255), 2);

			// Render each parallel pair in some colored circle or line?
			for (int i = 0; i < finger.parallelPairs.Size(); ++i)
			{
				CVContourSegmentPair & csp = finger.parallelPairs[i];
				// Parallel point.
				Vector3f pp = csp.one.rawInputStart, pp2 = csp.two.rawInputStart;
				cv::circle(pipe->output, cv::Point(pp.x, pp.y), 5, cv::Scalar(155,255,155), 2);
				cv::circle(pipe->output, cv::Point(pp2.x, pp2.y), 5, cv::Scalar(155,255,255), 2);
			}

		}

		// Render approximate velocity.
		Vector3f vel = hand.approximateVelocityFromOpticalFlow;
		Vector3f colorVel = vel;
		colorVel.y *= -1;
		Vector4f color = GetColorForDirection(colorVel.NormalizedCopy());
		// Paint it.
		cv::line(pipe->output, VectorToCVPoint(hand.center), VectorToCVPoint(hand.center + vel * 3), 
			VectorToCVScalarColor(color), 5);

		//		std::cout<<"\nfingers drawn: "<<hand.fingers.Size();
	}
}


void RenderCircles(CVPipeline * pipe)
{
	if (pipe->output.channels() != 1)
		return;
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

void RenderOpticalFlow(CVPipeline * pipe)
{
	cv::Mat & mat = pipe->output;
	// Paint the points
	for (int i = 0; i < pipe->opticalFlowPoints.Size(); ++i)
	{
		OpticalFlowPoint & point = pipe->opticalFlowPoints[i];
		float intensity = point.offset.Length();
//		std::cout<<"\nPoint at position: "<<point.position;

//		if (intensity < intensityThreshold->GetFloat())
	//		continue;

		Vector2f dir = point.offset.NormalizedCopy();
		dir.y *= -1;
		Vector4f color = GetColorForDirection(dir);
	//	color *= intensity * 0.05f;
		cv::Scalar cvColor = VectorToCVScalarColor(color);
		// points[i] wat..
		cv::circle(mat, VectorToCVPoint(point.previousPosition), 3, cvColor, -1, 8);
		// Draw a line from start to stop.
		cv::line(mat, VectorToCVPoint(point.previousPosition), VectorToCVPoint(point.position), cvColor);
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
					maxCorners->GetInt(),
					qualityLevel->GetFloat(),
					minimumDistance->GetFloat(),
					cv::Mat(),
					blockSize->GetInt(),
					useHarrisDetector->GetInt(),
					k->GetFloat());
	}
	catch (...)
	{
		errorString = ";_;";
		return CVReturnType::NOTHING;
	}
	returnType = CVReturnType::CV_CORNERS;
	return returnType;
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
	int radius = 1;
	cv::Scalar color(255,255,255);
//	cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255))
	for( int i = 0; i < pipe->corners.size(); ++i )
	{ 
		circle( copy, pipe->corners[i], radius, color, -1, 8, 0 ); 
	}

	copy.copyTo(pipe->output);
}



/* Old shit. Deprecate.
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
			if (interestPoints > minimumInterestpointsPerLine->GetInt())
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
			if (interestPoints > minimumInterestpointsPerLine->GetInt())
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
	returnType = CVReturnType::LINES;
	return returnType;
}
*/

	/*
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

	returnType = CVReturnType::BOUNDING_BOX;
	return returnType;
}
*/


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
	dp = new CVFilterSetting("dp", 1.f);

	settings.Add(method);
	settings.Add(dp);
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
		cv::HoughCircles(pipe->input, pipe->circles, method->GetInt(), dp->GetFloat(), minimumDistance->GetInt(), param1->GetFloat(), param2->GetFloat(), minimumRadius->GetFloat(), maximumRadius->GetFloat());
	}
	catch(...){
		return -1;
	}
	returnType = CVReturnType::CV_CIRCLES;
	return returnType;
}
void CVHoughCircles::Paint(CVPipeline * pipe)
{
	if (pipe->output.channels() != 1)
		return;
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

CVHoughLines::~CVHoughLines()
{
	
}

int CVHoughLines::Process(CVPipeline * pipe)
{
	if (rho->GetFloat() > 3000.f){
		errorString = "Use rho value below 3000 or it will crash.";
		return -1;
	}
	if (theta->GetFloat() >= 6.f)
	{
		errorString = "Use theta value 6 or below or it will crash.";
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
		cv::HoughLinesP(pipe->input, lines, rho->GetFloat(), theta->GetFloat(), threshold->GetInt(), minLineLength->GetFloat(), maxLineGap->GetFloat());
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
	returnType = CVReturnType::LINES;
	return returnType;
}

/*
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
*/

CVFilterLinesByAngle::CVFilterLinesByAngle()
	: CVDataFilter(CVFilterID::FILTER_LINES_BY_ANGLE)
{
	angleToKeep = new CVFilterSetting("Angle to keep", 0.f);
	settings.Add(angleToKeep);
	allowedRange = new CVFilterSetting("Allowed range", 20.f);
	settings.Add(allowedRange);
	about = "Angles are in standard carteesian coordinates.\nI.e. 0 is horizontally right, 90 vertically up,\n\180 horizontally left. \n\nAngles are considered circular, meaning 0 and \n180 are the same value.";
}
int CVFilterLinesByAngle::Process(CVPipeline * pipe)
{
	float i1Start, i1Stop, i2Start, i2Stop;
	i1Start = i2Start = angleToKeep->GetFloat() - allowedRange->GetFloat();
	i1Stop = i2Stop = angleToKeep->GetFloat() + allowedRange->GetFloat();
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
	returnType = CVReturnType::LINES;
	return returnType;
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
		Vector3f dir = line1.direction.NormalizedCopy();
		for (int j = 0; j < pipe->lines.Size(); ++j)
		{
			if (i == j)
				continue;

			Line & line2 = pipe->lines[j];	
			// Check that the directions align.
			Vector3f dir2 = line2.direction.NormalizedCopy();

			if (AbsoluteValue(dir.DotProduct(dir2)) < 0.95f)
				continue;
		
			// Check overlap distance. If not overlapping at all, skip 'em.
	//		if (line1.stop.x < line2.start.x ||
		//		line1.start.x > line2.stop.x)
		//		continue;

			// Check the distance between the lines.
			float distance = (line1.start - line2.start).Length();
			if (distance < maxDistance->GetFloat())
			{
				// Merge 'em
				line1.Merge(line2);
				pipe->lines.RemoveIndex(j);
				--j;
			}
		}
	}
	returnType = CVReturnType::LINES;
	return returnType;
}


CVLinePersistence::CVLinePersistence()
	: CVDataFilter(CVFilterID::LINE_PERSISTENCE)
{
	maxDistance = new CVFilterSetting("Max dist", 10.f);
	maxFrames = new CVFilterSetting("Max frames", 5);
	settings.Add(2, maxDistance, maxFrames);
}
int CVLinePersistence::Process(CVPipeline * pipe)
{
	previousFramesLines.Add(pipe->lines);
	if (previousFramesLines.Size() > maxFrames->GetInt())
	{
		previousFramesLines.RemoveIndex(0, ListOption::RETAIN_ORDER);
	}

	/// Temporary lines list.
	List<Line> lines;

	/// For each frame..
	for (int i = 0; i < pipe->lines.Size(); ++i)
	{
		List<Line> & frameLines = previousFramesLines[i];	

		/// Look at each line...
		for (int j = 0; j < frameLines.Size(); ++j)
		{
			Line & line = frameLines[j];
	
			/// And compare it to identified lines within a temporary list.
			bool foundIt = false;
			for (int k = 0; k < lines.Size(); ++k)
			{
				Line & mergedLine = lines[k];
				/// Is it there?
				bool equal = false;
				float dist = (line.start - mergedLine.start).Length();	
				if (dist < maxDistance->GetFloat())
					equal = true;
				if (equal)
				{
					/// Then merge it.
					foundIt = true;
					mergedLine.Merge(line);
				}
			}
			
			/// If not?
			if (!foundIt)
			{
				/// Then add it to the list of identified lines.
				lines.Add(line);
			}
		}
	}
	/// At the end, all lines should have been either added or merged together with other lines.

	/// Now a final filtering can be made to remove those lines with least weight (least merges), if wanted.

	/// Output the smoooothed liiines to the pipeline.
	pipe->lines = lines;

	returnType = CVReturnType::CV_LINES;
	return returnType;
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
	for (int i = 0; i < pipe->cvContours.size(); ++i)
	{
		try {
			std::vector<cv::Point> & contour = pipe->cvContours[i];
			cv::approxPolyDP(contour, approximatedPoly, epsilon->GetFloat(), true);

			// Replace the contour with the polygon?
			contour = approximatedPoly;

			// Just copy over the contours for now?
//			std::cout<<"\nContour vertices: "<<approximatedPoly.size();
			if (approximatedPoly.size() > 4)
				continue;

			// Minimum area?
			float area = cv::contourArea(approximatedPoly);
			if (area < minimumArea->GetInt())
				continue;
			pipe->approximatedPolygons.push_back(approximatedPoly);

		} catch(...)
		{
			return -1;
		}
	}
	returnType = CVReturnType::APPROXIMATED_POLYGONS;
	return returnType;
}

void CVApproxPolygons::Paint(CVPipeline * pipe)
{
	// Copy original input..
	pipe->initialInput.copyTo(pipe->output);
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
	float minimumWidthSquared = minimumWidth->GetInt() * minimumWidth->GetInt();
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
				if (AbsoluteValue(line.Length() - line2.Length()) > maxLengthDiff->GetFloat())
					continue;

				// Ensure both the start and end points are pretty parallel
				if (AbsoluteValue(line.start.x - line2.start.x) > maxCornerXDiff->GetFloat())
					continue;
				if (AbsoluteValue(line.stop.x - line2.stop.x) > maxCornerXDiff->GetFloat())
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
	returnType = CVReturnType::QUADS;
	return returnType;
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
		if (ratio < minimumRatio->GetFloat())
			ratioGood = false;
		else if (ratio > maximumRatio->GetFloat())
			ratioGood = false;
		if (ratioGood)
			continue;
		// Delete if bad.
		pipe->quads.RemoveIndex(i);
		--i;
	}
	returnType = CVReturnType::QUADS;
	return returnType;
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
		if (previousQuads.Size() > maxFramesToConsider->GetInt())
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
		if (quad.ManhattanSize() > biggest.ManhattanSize() - threshold->GetInt())
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

	returnType = CVReturnType::QUADS;
	return returnType;
}

CVConvexHull::CVConvexHull()
	: CVDataFilter(CVFilterID::CALC_CONVEX_HULL)
{
}
int CVConvexHull::Process(CVPipeline * pipe)
{
	// Calculate convex hull of the contour.
	if (!pipe->cvContours.size()){
		errorString = "No contours to process";
		return CVReturnType::CV_CONVEX_HULLS;
	}
	cv::convexHull(pipe->cvContours[0], pipe->convexHull);
	returnType = CVReturnType::CV_CONVEX_HULLS;
	return returnType;
}
