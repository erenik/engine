/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#include "CVOpticalFlowFilters.h"
#include "CV/CVPipeline.h"
#include "Time/Time.h"

#include "Model/ModelManager.h"
#include "Maps/MapManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "TextureManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

CVOpticalFlowFarneback::CVOpticalFlowFarneback()
	: CVDataFilter(CVFilterID::OPTICAL_FLOW_FARNEBACK)
{

	pyramidScale = new CVFilterSetting("Pyramid scale", 0.5f);
	pyramidLayers = new CVFilterSetting("Pyramid layers", 1);
	averagingWindowSize = new CVFilterSetting("Averaging window size", 3);
	iterations = new CVFilterSetting("Iterations", 1);
	polynomialNeighbourhoodSize = new CVFilterSetting("Poly Neighbourhood size", 5);
	polynomialSmoothingSigma = new CVFilterSetting("Poly Smoothing sigma", 1.1f);
	outputType = new CVFilterSetting("Output", 0);
	settings.Add(7, pyramidScale, pyramidLayers, averagingWindowSize, 
		iterations, polynomialNeighbourhoodSize, polynomialSmoothingSigma,
		outputType);
	about = "Poly N size usually 5 or 7.\nPoly sigma typical 1.1 for Poly N 5 or 1.5 for Poly N 7";
}
int CVOpticalFlowFarneback::Process(CVPipeline * pipe)
{
	// Require greyscale.
	if (pipe->input.channels() != 1)
	{
// Stupid windows global defines.
#undef ERROR
		errorString = "Requires single-channel input.";
		returnType = CVReturnType::ERROR;
		return returnType;
	}
	if (lastFrame.cols != pipe->input.cols ||
		lastFrame.rows != pipe->input.rows ||
		lastFrame.type() != pipe->input.type())
	{
		// Not same format in previous and current image? skip one frame
		pipe->input.copyTo(lastFrame);

		// No output this frame.
		returnType = CVReturnType::NOTHING;
		return returnType;
	}

	// Create optical flow image as needed.
	cv::Mat & opticalFlow = pipe->opticalFlowFarneback;
	opticalFlow = cv::Mat(pipe->input.rows, pipe->input.cols, CV_32FC2);

	try 
	{
		cv::calcOpticalFlowFarneback(lastFrame, 
			pipe->input, 
			opticalFlow, 
			pyramidScale->GetFloat(), 
			pyramidLayers->GetInt(), 
			averagingWindowSize->GetInt(), 
			iterations->GetInt(), 
			polynomialNeighbourhoodSize->GetInt(), 
			polynomialSmoothingSigma->GetFloat(), 
			0);

		// For each pixel, calculate flow.
		int output = outputType->GetInt();
		if (output == 1)
		{
			// Paint output black.
			pipe->output = cv::Mat::zeros(pipe->input.size(), CV_8UC3);

			static float maxFlow = 1;

			// Get pointer
			unsigned char * buf = pipe->output.data;
			float * flowBuf = (float*)opticalFlow.data;

			float scalingFactor = 1.f;


			
			Vector3f color;
					
			// Set pixel black or white depending on stuff?
			for (int x = 0; x < opticalFlow.cols; ++x)
			{
				for (int y = 0; y < opticalFlow.rows; ++y)
				{

					int pixelIndex = x * opticalFlow.rows + y;
				
					float flowX = flowBuf[pixelIndex * 2];
					float flowY = flowBuf[pixelIndex * 2+1];
				
					if (flowX <= 0 && flowY <= 0)
						continue;

					Vector2f flow(flowX, flowY);
					// Use direction to determine colors.


					float magnitude = flow.Length() * scalingFactor;
					if (magnitude > maxFlow)
						maxFlow = magnitude;

					magnitude = MinimumFloat(magnitude, 255.f);

					// o-o
					flow.Normalize();

					color = GetColorForDirection(flow);
					
					color *= magnitude;

					
					buf[pixelIndex*3] = color.x;
					buf[pixelIndex*3+1] = color.y;
					buf[pixelIndex*3+2] = color.z;

				}
			}
		}
		// Greyscale abs-output.
		else if (output == 0)
		{
			// Paint output black.
			pipe->output = cv::Mat::zeros(pipe->input.size(), CV_8UC1);

			static float maxFlow = 1;

			// Get pointer
			unsigned char * buf = pipe->output.data;
			float * flowBuf = (float*)opticalFlow.data;

			float scalingFactor = 1.f;

			// Set pixel black or white depending on stuff?
			for (int x = 0; x < opticalFlow.cols; ++x)
			{
				for (int y = 0; y < opticalFlow.rows; ++y)
				{

					int pixelIndex = x * opticalFlow.rows + y;
				
					float flowX = flowBuf[pixelIndex * 2];
					float flowY = flowBuf[pixelIndex * 2];
				
					Vector2f flow(flowX, flowY);
					float magnitude = flow.Length() * scalingFactor;
					if (magnitude > maxFlow)
						maxFlow = magnitude;
				
					buf[pixelIndex] = MinimumFloat(magnitude, 255.f);

				}
			}
		}
	}
	catch(...)
	{
		return CVReturnType::ERROR;
	}

	// Save current image as previous one.
	pipe->input.copyTo(lastFrame);

	returnType = CVReturnType::CV_IMAGE;
	return returnType;
}


CVOpticalFlowLucasKanade::CVOpticalFlowLucasKanade()
	: CVDataFilter(CVFilterID::OPTICAL_FLOW_LUCAS_KANADE)
{
	searchWindowSize = new CVFilterSetting("Search window size", 21);
	maxLevel = new CVFilterSetting("Max pyr level", 0);
	minEigenThreshold = new CVFilterSetting("Min eigen threshold", 1e-4f);
	maxIterations = new CVFilterSetting("Criteria - Max iterations", 30);
	epsilon = new CVFilterSetting("Criteria - Epsilon", 0.01f);
	//intensityThreshold = new CVFilterSetting("Intensity threshold", 1.f);
	minimumDistance = new CVFilterSetting("Minimum distance", 1.f);
	maxDistance = new CVFilterSetting("Max distance", 20.f);
	quadrantFilter = new CVFilterSetting("Quadrant filter", Vector4f(0,0,0,0));

	renderOutputAsDedicatedEntity = new CVFilterSetting("Render output as dedicated entity", false);

	paintPointSize = new CVFilterSetting("Point size", 3);
	directionVectorThickness = new CVFilterSetting("Direction vector thickness", 2);


	settings.Add(11, 
		searchWindowSize, maxLevel, minEigenThreshold,
		maxIterations, epsilon, minimumDistance, 
		maxDistance, renderOutputAsDedicatedEntity, quadrantFilter,
		paintPointSize, directionVectorThickness);

	outputRenderTexture = NULL;
	outputRenderEntity = NULL;
}

CVOpticalFlowLucasKanade::~CVOpticalFlowLucasKanade()
{
	if (MapManager::Instance())
		MapMan.DeleteEntity(outputRenderEntity);
}

int CVOpticalFlowLucasKanade::Process(CVPipeline * pipe)
{

	pipe->opticalFlowPoints.Clear();

	// Require greyscale.
	if (pipe->input.channels() != 1)
	{
// Stupid windows global defines.
#undef ERROR
		errorString = "Requires single-channel input.";
		returnType = CVReturnType::ERROR;
		return returnType;
	}
	if (lastFrame.cols != pipe->input.cols ||
		lastFrame.rows != pipe->input.rows ||
		lastFrame.type() != pipe->input.type())
	{
		// Not same format in previous and current image? skip one frame
		pipe->input.copyTo(lastFrame);

		// No output this frame.
		returnType = CVReturnType::NOTHING;
		return returnType;
	}

	returnType = CVReturnType::CV_OPTICAL_FLOW;
	float maxDist = maxDistance->GetFloat();
	try {
		int windowSize = searchWindowSize->GetInt();
		cv::Size searchWindow(windowSize, windowSize);

		CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, maxIterations->GetInt(), epsilon->GetFloat());

		/// Fetch relevants from where possible.
	//	lastPoints = pipe->goodFeatures;

	
		// Create requested number of pyramid-levels!!
		int pyramidLevels = maxLevel->GetInt();
		// Build image pyramids of the input images and the last frame! o-o
		
		// Copy pyramids from last frame? Nah.		
		cv::buildOpticalFlowPyramid(lastFrame, pastPyramids, searchWindow, pyramidLevels);
		cv::buildOpticalFlowPyramid(pipe->input, currentPyramids, searchWindow, pyramidLevels);

		// Grab detected points.
		// Set last paints for next iteration
		lastPoints = pipe->corners;
		points.clear();
//		points.resize(lastPoints.size());
		    
		cv::calcOpticalFlowPyrLK(pastPyramids, 
			currentPyramids, 
			lastPoints, 
			points, 
			status, 
			err, 
			searchWindow, 
			maxLevel->GetInt(),
			criteria, 
			// OPTFLOW_USE_INITIAL_FLOW uses initial estimations, stored in nextPts; if the flag is not set, then prevPts is copied to nextPts and is considered the initial estimate.
			// OPTFLOW_LK_GET_MIN_EIGENVALS use minimum eigen values as an error measure (see minEigThreshold description); if the flag is not set, then L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.
			cv::OPTFLOW_LK_GET_MIN_EIGENVALS,
			minEigenThreshold->GetFloat()
			);

		// Do something 
		bool addRemovePt = false;
		size_t i, k;
	    for( i = k = 0; i < points.size(); i++ )
        {
			// For click-removing points... wat
			/*
            if( addRemovePt )
            {
                if( cv::norm(point - points[i]) <= 5 )
                {
                    addRemovePt = false;
                    continue;
                }
            }*/

			uchar pointStatus = status[i];
            if( !status[i] )
                continue;

			cv::Point2f & cvPoint = points[i];
			cv::Point2f & cvOriginalPoint = lastPoints[i];
			OpticalFlowPoint point;
			point.position = Vector2f(cvPoint.x, cvPoint.y);
			point.previousPosition = Vector2f(cvOriginalPoint.x, cvOriginalPoint.y);
			// Convert to initial input space as needed.
			point.position *= pipe->currentScaleInv;
			point.previousPosition *= pipe->currentScaleInv;
			point.offset = point.position - point.previousPosition;
			float length = point.offset.Length();
			if (length < minimumDistance->GetFloat())
				continue;
			else if (maxDist > 0 && length > maxDist)
				continue;

			Vector4f quad = quadrantFilter->GetVec4f();
			if (quad.Length3Squared())
			{
				// Check if within bounds.
				Vector3f p = point.position;
				if (p.x < quad.x ||
					p.y < quad.y ||
					p.x > quad.z ||
					p.y > quad.w) 
					continue;
			}
			pipe->opticalFlowPoints.Add(point);
			/*
            points[k++] = points[i];
			*/

		
        }


		// Resize it according to how many good points actually remain..?
	//	points.resize(k);
	}
	catch(...)
	{
		return returnType;
	}

	// Make output in color for visualization.
	int channelsPre = pipe->initialInput.channels();
	if (channelsPre == 1)
	{
		cv::cvtColor(pipe->initialInput, pipe->output, CV_GRAY2RGB);
	}
	else 
		pipe->initialInput.copyTo(pipe->output);
	int channelsPost = pipe->output.channels();
		// pipe->pooutput = cv::Mat::zeros(pipe->input.size(), CV_8UC3);

	// Paint in output.
	PaintOFPoints(pipe->output, pipe);

	// Save current image as previous one.
	pipe->input.copyTo(lastFrame);
	
	// copy it? 
	if (!outputRenderTexture)
	{
		outputRenderTexture = TexMan.NewDynamic();
		outputRenderTexture->SetSource("CVOpticalFlowLucasKanade render output");
	}
		
	if (!outputRenderEntity)
	{
		outputRenderEntity = MapMan.CreateEntity("Output render entity", ModelMan.GetModel("Sprite.obj"), outputRenderTexture, Vector3f(0,0,-20));
		// Unregister it from physics!
		Physics.QueueMessage(new PMUnregisterEntity(outputRenderEntity));
		// Set it to render additively?
		Graphics.QueueMessage(new GMSetEntityi(outputRenderEntity, GT_BLEND_MODE_SRC, GL_ONE));
	}
		
	if (renderOutputAsDedicatedEntity->GetBool())
	{
		// o.o
		if (outputRender.rows != pipe->output.rows || outputRender.cols != pipe->output.cols ||
			outputRender.type() != CV_8UC3)
			outputRender = cv::Mat::zeros(pipe->output.rows, pipe->output.cols, CV_8UC3);
		else 
			outputRender = cv::Scalar(0,0,0);
		// Paint shit.
		PaintOFPoints(outputRender, pipe);
		outputRenderTexture->LoadFromCVMat(outputRender);
		// Scale it to size.
		Physics.QueueMessage(new PMSetEntity(outputRenderEntity, PT_SET_SCALE, Vector3f(outputRenderTexture->width, outputRenderTexture->height, 1)));
	}
	if (renderOutputAsDedicatedEntity->HasChanged())
		Graphics.QueueMessage(new GMSetEntityb(outputRenderEntity, GT_VISIBILITY, renderOutputAsDedicatedEntity->GetBool()));
	return returnType;
}

void CVOpticalFlowLucasKanade::PaintOFPoints(cv::Mat & mat, CVPipeline * pipe)
{
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
		cv::circle(mat, VectorToCVPoint(point.previousPosition), paintPointSize->GetInt(), cvColor, -1, 8);
		// Draw a line from start to stop.
		cv::line(mat, VectorToCVPoint(point.previousPosition), VectorToCVPoint(point.position), cvColor, directionVectorThickness->GetInt());
	}
}


CVOpticalFlowLKSubdivided::CVOpticalFlowLKSubdivided()
	: CVDataFilter(CVFilterID::OPTICAL_FLOW_LK_SUBDIVIDED)
{
	searchWindowSize = new CVFilterSetting("Search window size", 21);
	maxLevel = new CVFilterSetting("Max pyr level", 0);
	minEigenThreshold = new CVFilterSetting("Min eigen threshold", 1e-4f);
	maxIterations = new CVFilterSetting("Criteria - Max iterations", 30);
	epsilon = new CVFilterSetting("Criteria - Epsilon", 0.01f);
	intensityThreshold = new CVFilterSetting("Intensity threshold", 1.f);
	elementsPerRow = new CVFilterSetting("Elements per row", 5);
	settings.Add(7, searchWindowSize, maxLevel, minEigenThreshold,
		maxIterations, epsilon, intensityThreshold,
		elementsPerRow);
}

int CVOpticalFlowLKSubdivided::Process(CVPipeline * pipe)
{
	/*
	struct OpticalFlowPoint 
	{
		Vector2f position;
		Vector2f lastPosition;
		// Compared to last frame/initial detection.
		Vector2f offset;
	};
	List<OpticalFlowPoint> flowPoints;

	// Require greyscale.
	if (pipe->input.channels() != 1)
	{
// Stupid windows global defines.
#undef ERROR
		errorString = "Requires single-channel input.";
		returnType = CVReturnType::ERROR;
		return returnType;
	}
	if (lastFrame.cols != pipe->input.cols ||
		lastFrame.rows != pipe->input.rows ||
		lastFrame.type() != pipe->input.type())
	{
		// Not same format in previous and current image? skip one frame
		pipe->input.copyTo(lastFrame);

		// No output this frame.
		returnType = CVReturnType::NOTHING;
		return returnType;
	}

	/// Prepare the for-loop for calculating the Optical flow
	returnType = CVReturnType::CV_OPTICAL_FLOW;
	struct Box2f {
		Vector2f min, max;
	};
	List<Box2f> boxes;
	Vector2f elementSize = pipe->inputSize;
	elementSize /= elementsPerRow->GetInt();
	for (int i = 0; i < elementsPerRow->GetInt(); ++i)
	{
		Box2f box;
		box.min = Vector2f(elementSize.x * i, 0);
		box.max = Vector2f(elementSize.x * (i + 1), elementSize.y);
	}



	try {
		int windowSize = searchWindowSize->GetInt();
		cv::Size searchWindow(windowSize, windowSize);

		CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, maxIterations->GetInt(), epsilon->GetFloat());

		/// Fetch relevants from where possible.
	//	lastPoints = pipe->goodFeatures;

	
		// Create requested number of pyramid-levels!!
		int pyramidLevels = maxLevel->GetInt();
		// Build image pyramids of the input images and the last frame! o-o
		cv::buildOpticalFlowPyramid(lastFrame, pastPyramids, searchWindow, pyramidLevels);
		cv::buildOpticalFlowPyramid(pipe->input, currentPyramids, searchWindow, pyramidLevels);

		// Grab detected points.
		lastPoints = pipe->corners;
		points.resize(lastPoints.size());
		    
		cv::calcOpticalFlowPyrLK(pastPyramids, 
			currentPyramids, 
			lastPoints, 
			points, 
			status, 
			err, 
			searchWindow, 
			maxLevel->GetInt(),
			criteria, 
			// OPTFLOW_USE_INITIAL_FLOW uses initial estimations, stored in nextPts; if the flag is not set, then prevPts is copied to nextPts and is considered the initial estimate.
			// OPTFLOW_LK_GET_MIN_EIGENVALS use minimum eigen values as an error measure (see minEigThreshold description); if the flag is not set, then L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.
			0,
			minEigenThreshold->GetFloat()
			);

		// Do something 
		bool addRemovePt = false;
		size_t i, k;
	    for( i = k = 0; i < points.size(); i++ )
        {
	
            if( !status[i] )
                continue;

			cv::Point2f & cvPoint = points[i];
			cv::Point2f & cvOriginalPoint = lastPoints[i];
			OpticalFlowPoint point;
			point.position = Vector2f(cvPoint.x, cvPoint.y);
			point.lastPosition = Vector2f(cvOriginalPoint.x, cvOriginalPoint.y);
			point.offset = point.position - point.lastPosition;
			flowPoints.Add(point);
	    }
		// Resize it according to how many good points actually remain..?
	//	points.resize(k);
	}
	catch(...)
	{
		return returnType;
	}

	// Make output in color for visualization.
	pipe->initialInput.copyTo(pipe->output);
		// pipe->pooutput = cv::Mat::zeros(pipe->input.size(), CV_8UC3);

	// Paint the points
	for (int i = 0; i < flowPoints.Size(); ++i)
	{
		OpticalFlowPoint & point = flowPoints[i];
		float intensity = point.offset.Length();

		if (intensity < intensityThreshold->GetFloat())
			continue;

		Vector2f dir = point.offset.NormalizedCopy();
		dir.y *= -1;
		Vector4f color = GetColorForDirection(dir);
		color *= intensity * 0.05f;
		cv::Scalar cvColor = VectorToCVScalarColor(color);
		cv::circle(pipe->output, points[i], 3, cvColor, -1, 8);
	}

	// Save current image as previous one.
	pipe->input.copyTo(lastFrame);
	*/
		returnType = 0;
	return returnType;
}



CVOpticalFlowSimpleFlow::CVOpticalFlowSimpleFlow()
	: CVDataFilter(CVFilterID::OPTICAL_FLOW_SIMPLE_FLOW)
{
	averagingBlockSize = new CVFilterSetting("Averaging block size", 5);
	maxFlow = new CVFilterSetting("Max flow", 5);
	settings.Add(2, averagingBlockSize, maxFlow);
}
int CVOpticalFlowSimpleFlow::Process(CVPipeline * pipe)
{
	// Shrink the inpit to infinity? lol
	int size = 32;
	pipe->input = cv::Mat(size, size, CV_8UC3);

	if (pipe->input.type() != CV_8UC3) 
	{
		errorString = "Input has to be 3-color single byte.";
		return 0;
	}
	if (lastFrame.cols != pipe->input.cols || lastFrame.rows != pipe->input.rows ||
		lastFrame.type() != pipe->input.type())
	{
		pipe->input.copyTo(lastFrame);
		return 0;
	}

	try {
		Time startTime = Time::Now();
		cv::calcOpticalFlowSF(lastFrame, pipe->input,
			pipe->output,
                    3, 2, 4, 4.1, 25.5, 18, 55.0, 25.5, 0.35, 18, 55.0, 25.5, 10);
		Time stopTime = Time::Now();
		Time timeTaken = stopTime - startTime;
		int milliseconds = timeTaken.Milliseconds();
		std::cout<<"\nMilliseconds taken: "<<milliseconds;
  
	//	cv::calcOpticalFlowSF(lastFrame, pipe->input, pipe->output, 3, averagingBlockSize->GetInt(), maxFlow->GetInt());
	} catch(...)
	{
		
	}

	pipe->input.copyTo(lastFrame);
	returnType = CVReturnType::CV_IMAGE;
	return returnType;
}

CVOpticalFlowDualTVL1::CVOpticalFlowDualTVL1()
	: CVDataFilter(CVFilterID::OPTICAL_FLOW_DUAL_TV_L1)
{
	dualTVL1 = NULL;
//	CVFilterSetting * tau, * lambda, * theta, * nscales, * warps, * epsilon, * iterations;
	tau = new CVFilterSetting("Tau", 1.f);
	lambda = new CVFilterSetting("Lambda", 1.f);
	settings.Add(2, tau, lambda);
	
}

CVOpticalFlowDualTVL1::~CVOpticalFlowDualTVL1()
{
	if (dualTVL1)
	{
		dualTVL1->collectGarbage();
		delete dualTVL1;
	}
}

int CVOpticalFlowDualTVL1::Process(CVPipeline * pipe)
{
	if (!dualTVL1)
		dualTVL1 = cv::createOptFlow_DualTVL1();



	//std::vector<cv::String> params;
	//dualTVL1->getParams(params);

	// Set variables as needed.
	if (tau->HasChanged())
	{
		dualTVL1->setDouble("tau", tau->GetFloat());
	}
	if (lambda->HasChanged())
	{
		dualTVL1->setDouble("lambda", lambda->GetFloat());
	}

	// Demand greyscale input.
	if (pipe->input.type() != CV_8UC1)
	{
		errorString = "Must be single channel 8-bits per channel.";
		return -1;
	}

	if (pipe->input.cols != lastFrame.cols ||
		pipe->input.rows != lastFrame.rows ||
		pipe->input.type() != lastFrame.type())
	{
		pipe->input.copyTo(lastFrame);
		return CVReturnType::NOTHING;
	}
	
	try {
		dualTVL1->calc(lastFrame, pipe->input, flow);
		flow.copyTo(pipe->output);
	}
	catch(...)
	{
		return -1;
	}
	// Copy for next iteration
	pipe->input.copyTo(lastFrame);
	returnType = CVReturnType::CV_IMAGE;
	return returnType;
}



CVGenerateOpticalFlowField::CVGenerateOpticalFlowField()
	: CVDataFilter(CVFilterID::GENERATE_OPTICAL_FLOW_FIELD)
{
	segments = new CVFilterSetting("Segments", 5);
	toPaint = new CVFilterSetting("To paint", 0);
	settings.Add(2, segments, toPaint);
	about = "To paint: 0 - dir, 1 - average velocity\n2 - Number of points";
}

int CVGenerateOpticalFlowField::Process(CVPipeline * pipe)
{

	 Vector2f quadrantSize = Vector2f(pipe->inputSize) / segments->GetInt();
	 Vector2i flowSize = Vector2i(segments->GetInt(), segments->GetInt());
	 OpticalFlow & flow = pipe->opticalFlow;
	 if (flow.Size() != flowSize)
		flow.SetSize(flowSize);
	 // Assign all quadrants min/max points.
	 for (int i = 0; i < flow.Elements(); ++i)
	 {
		 Vector2i loc(i / flow.Size().x, i % flow.Size().y);
		 OpticalFlowQuadrant * quad = flow.Element(i);
		 quad->min = loc * quadrantSize;
		 quad->max = quad->min + quadrantSize;
	 }

	 flow.ClearPoints();
	 flow.Add(pipe->opticalFlowPoints);

	 // Calculate average n stuff
	 for (int i = 0; i < flow.Elements(); ++i)
	 {
		 OpticalFlowQuadrant * quad = flow.Element(i);
		 quad->CalculateFlow();
	 }

	 // Paint it?
	 returnType = CVReturnType::CV_IMAGE;
	 return returnType;
}


/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVGenerateOpticalFlowField::Paint(CVPipeline * pipe)
{
	OpticalFlow & flow = pipe->opticalFlow;
	// Paint it!
	pipe->initialInput.copyTo(pipe->output);
	for (int i = 0; i < flow.Elements(); ++i)
	{
		OpticalFlowQuadrant * quad = flow.Element(i);
		Vector4f color;
		switch(toPaint->GetInt())
		{
			case 0:
			{
				Vector3f dir = quad->averageFlow;
				dir.y *= -1.f;
				color = GetColorForDirection(dir);
				break;
			}
			case 1:
				color = Vector3f(1,1,1) * quad->averageFlow.Length();
				break;
			case 2:
				color = Vector3f(1,1,1) * quad->flowPoints.Size() * 0.1f;
				break;
		}
		cv::rectangle(pipe->output, VectorToCVPoint(quad->min), VectorToCVPoint(quad->max), VectorToCVScalarColor(color), -1);
	}
}

