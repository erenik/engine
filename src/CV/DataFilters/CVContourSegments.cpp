/// Emil Hedemalm
/// 2014-07-14
/// Classifier for contour-segments

#include "CVDataFilters.h"
#include "CV/CVPipeline.h"
#include "TextureManager.h"

#include "Window/AppWindowManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "UI/UserInterface.h"
#include "UI/UIImage.h"

#undef min
#undef max

// Length of segments? 
Texture * contourSegmentRelativeAngleDistanceTexture = NULL;

void RenderToTexture(Texture * tex, CVContour * contour)
{
	Vector3f center = contour->largestInnerCircleCenter;
	if (contour->largestInnerCircleRadius < 0)
		return;
	// Paint center of the contour.
	Vector4f handColor(255.f,200.f,155.f,1.f);
	
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
	}
}


CVContourSegmenter::CVContourSegmenter()
	: CVDataFilter(CVFilterID::CONTOUR_SEGMENTER)
{
	handDataWindow = NULL;	
	returnType = CVReturnType::CV_CONTOUR_SEGMENTS;

	// Relative to the frame's circumference.
	maximumEdgeLength = new CVFilterSetting("Max edge length", 0.25f);
	showOutputWindow = new CVFilterSetting("Show output AppWindow", true);
	settings.Add(2, maximumEdgeLength, showOutputWindow);

}

CVContourSegmenter::~CVContourSegmenter()
{
	std::cout<<"\nContour segmenter destructorrrr";
}

int CVContourSegmenter::Process(CVPipeline * pipe)
{
	// Debuhhgging.
	if (!contourSegmentRelativeAngleDistanceTexture)
		contourSegmentRelativeAngleDistanceTexture = TexMan.NewDynamic();
	int imageResolution = 512;
	contourSegmentRelativeAngleDistanceTexture->Resize(Vector2i(imageResolution, imageResolution));
	contourSegmentRelativeAngleDistanceTexture->SetColor(Vector4f(0,0,0,1));


	/// Create angleDistancePos co-ordinate pairs based on the relative angles and distances.
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour & contour = pipe->contours[i];
		// Length?
		contour.ExtractSegments();

		// Detect edges by providing minimum and maximum assumable XY-values.
		Vector2i min(1,1), max(pipe->input.cols - 2, pipe->input.rows - 2);
		contour.FindEdges(min,max);

		float inputCircumference = pipe->input.cols * 2 + pipe->input.rows * 2;
		if (contour.edgeLength > inputCircumference * maximumEdgeLength->GetFloat())
		{
			pipe->contours.RemoveIndex(i);
			--i;
			continue;
		}
		
		/// Approximate inner circle if possible.
		contour.ApproximateInnerCircle();

		contour.CalculateRelativeAngles(contour.largestInnerCircleCenter);
		// Calculate relative distance between the segments and the chosen center.
		contour.CalculateRelativeDistances(contour.largestInnerCircleCenter, contour.largestInnerCircleRadius);

		/// Do something o.o
		contour.GatherRelativeIntoJointVector();

		// THEN calculate the angle/distance, as this requires the center to have been finalized.
		CreateAngleDistancePositionPairs(contour);

		contour.CalculateContourSegmentEnergies();
		
	}


	if (!handDataWindow)
	{
		handDataWindow = WindowMan.NewWindow("HandData", "Hand data");
		handDataWindow->ui = new UserInterface();
		handDataWindow->ui->Load("gui/HandData.gui");
		handDataWindow->CreateGlobalUI();
		handDataWindow->SetRequestedSize(Vector2i(400, 400));
		handDataWindow->DisableAllRenders();
		handDataWindow->renderUI = true;
		handDataWindow->Create();
			
	}
		
	if (!handDataWindow->IsVisible() && showOutputWindow->GetBool())
	{
		handDataWindow->Show();
	}
	else if (handDataWindow->IsVisible() && !showOutputWindow->GetBool())
		handDataWindow->Hide();
	
	// Pass output to the texture in the hand-data AppWindow
	Graphics.QueueMessage(new GMSetUIp("HandData", GMUI::TEXTURE, contourSegmentRelativeAngleDistanceTexture, handDataWindow->ui));
	Graphics.QueueMessage(new GMBufferTexture(contourSegmentRelativeAngleDistanceTexture));		

	return returnType;
}


// Does what?
void CVContourSegmenter::CreateAngleDistancePositionPairs(CVContour & forContour)
{
	float invMinDist = 1 / forContour.largestInnerCircleRadius;
	bool fixX = false;
	float xMultiplier = 0.1f;
	for (int i = 0; i < forContour.angleDistancePositions.Size(); ++i)
	{

		Vector2f & angleDistancePos = forContour.angleDistancePositions[i];
		// Scale Y to a unit distance based on the minimum circle of the palm of the hand.
		angleDistancePos.y *= invMinDist;
		// Multiply X so it becomes easier to analyse.
		if (fixX)
		{
			// Move it to center [-0.5,+0.5]
			angleDistancePos.x -= contourSegmentRelativeAngleDistanceTexture->width * 0.5f;
			// Multiply scale to try and compensate for the radial stuff impose by the algorithm
			angleDistancePos.x *= angleDistancePos.y * xMultiplier;
			// Move it back to texture space again [0,1]
			angleDistancePos.x += contourSegmentRelativeAngleDistanceTexture->width * 0.5f;
		}

		// Adjust Y to texture space again too.
		angleDistancePos.y *= contourSegmentRelativeAngleDistanceTexture->height * 0.1f;

		angleDistancePos.Clamp(0, contourSegmentRelativeAngleDistanceTexture->height);
	}
}

CVContourSegmentClassifier::CVContourSegmentClassifier()
	: CVDataFilter(CVFilterID::CONTOUR_SEGMENT_CLASSIFIER)
{
	thresholdAngle = new CVFilterSetting("ThreshAngle", 0.5f);
	settings.Add(thresholdAngle);
	blurRange = new CVFilterSetting("Blur range", 3);
	settings.Add(blurRange);
	returnType = CVReturnType::CV_CONTOUR_SEGMENTS;
}
int CVContourSegmentClassifier::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour * inContour = &pipe->contours[i];
		// For finding the end of the finger.
		int endIndex = -1;
		// Probably use this for finger end-points.
		int firstEndIndex = -1;
		float distanceTraveled = 0;
		Vector3f lastPoint;
		int state;
		enum {
			NONE,
			FINGER_STARTED,
			FINGER_PLATEAU, // At the point!
			FINGER_ENDING,
			FINGER_ENDED,
			EVALUATING_CLIMB,
		};
		state = NONE;
		int fingersStarted = 0;
		int fingersStopped = 0;
		int startIndex = -1, stopIndex = -1;
		/// To make it work for high-resolution contours...
		int positives = 0;

		bool useMinimumDistance = false;
		int negatives = 0;

		float minimumSegmentLength = 5.f;

		CircularList<CVContourSegment> & segments = inContour->segments;
		if (segments.Size() == 0)
			return returnType;

		// ...

		// Fetch random edge-piece if possible

		int leastEnergyIndex = inContour->leastEnergySegmentIndex;
		if (leastEnergyIndex < 0)
		{
			for (int i = 0; i < segments.Size(); ++i)
			{
				CVContourSegment & seg = segments[i];
				if (seg.edge ||
					seg.type == SegmentType::EDGE)
				{
					leastEnergyIndex = i;
					break;
				}
			}
		}
		if (leastEnergyIndex < 0)
		{
			leastEnergyIndex = 0;
//			std::cout<<"\nLeast energy index defaulted to 0.!";
		}
		assert(leastEnergyIndex >= 0);


		// Lists of those segments which have only received a preliminary type, but may change if the current inclination continues as is.
		List<CVContourSegment*> suggestedStarts, suggestedDescents, suggestedPlateaus;

		// Start the real hand-parsing starting with the segments of least energy so as to avoid missing any relevant fingers. Go one loop!
		int parsingState = NONE;;
		segments[leastEnergyIndex].start = true;
	
		/// Values for the relative climb or descent during the past segments.
		int climb = 0, plateau = 0, descent = 0;
		float anti = 0.3f;
		float close = 0.7f;
		
		for (int i = leastEnergyIndex; i < leastEnergyIndex + segments.Size(); ++i)
		{
			CVContourSegment & seg = segments[i];
			Vector2f angleDistancePos = seg.angleDistanceStart;
			Vector3f normalizedDirection = (seg.angleDistanceStop - seg.angleDistanceStart).NormalizedCopy();
			seg.angleDistanceNormalizedDirection = normalizedDirection;
		
			// Special cases before the general switch.
			if (seg.edge == true)
			{
				seg.type = SegmentType::EDGE;
				continue;
			}
			// Thumb. Only one that can go back far in the normalized direction.
			else if (normalizedDirection.x > 0.2f)
			{
				++climb;
				plateau *= close;
				descent *= anti;
			}
			else 
			{
				/// Climb segment.
				if (normalizedDirection.y > thresholdAngle->GetFloat())
				{
					++climb;
					plateau *= close;
					descent *= anti;
				//	suggestedStarts.Add(&seg);
				}
				/// Plateau segment
				else if (normalizedDirection.y > -thresholdAngle->GetFloat() && normalizedDirection.y < thresholdAngle->GetFloat())
				{
					++plateau;
					climb *= close;
					descent *= close;
				}
				/// Descent segment
				else if (normalizedDirection.y < -thresholdAngle->GetFloat())
				{	
					++descent;
					climb *= anti;
					plateau *= close;
				}
			}
			/// Check which type has the highest current value.
			int highestType;
			if (climb > plateau)
			{
				if (climb > descent)
					highestType = SegmentType::CLIMB;
				else
					highestType = SegmentType::DESCENT;
			}
			else {
				if (plateau > descent)
					highestType = SegmentType::PLATEAU;
				else
					highestType = SegmentType::DESCENT;
			}
			/// Set the segment type to the highest determined type (most probable).
			seg.type = highestType;
		}	

		
		// Alright, so initial analysis is done. Now propagate these results onto surrounding segments and re-evaluate what type each segment really should have (in order to stay similar to its neighbours)
		int intervalRange = blurRange->GetInt() - 1;
		if (intervalRange >= 0)
		{
			for (int i = 0; i < segments.Size(); ++i)
			{
				CVContourSegment & cSeg = segments[i];
				for (int j = i - intervalRange; j <= i + intervalRange; ++j)
				{
					CVContourSegment & cSeg2 = segments[j];
					switch(cSeg.type)
					{
						case SegmentType::CLIMB:
							++cSeg2.climb;
							break;
						case SegmentType::DESCENT:
							++cSeg2.descent;
							break;
						case SegmentType::PLATEAU:
							++cSeg2.plateau;
							break;
					}
				}
			}
			// All have gotten values, re-assign them.
			for (int i = 0; i < segments.Size(); ++i)
			{
				CVContourSegment & cSeg = segments[i];
				if (cSeg.type == SegmentType::EDGE)
					continue;
				if (cSeg.climb > cSeg.descent)
				{
					if (cSeg.climb > cSeg.plateau)
						cSeg.type = SegmentType::CLIMB;
					else
						cSeg.type = SegmentType::PLATEAU;
				}
				else {
					if (cSeg.plateau > cSeg.descent)
						cSeg.type = SegmentType::PLATEAU;
					else
						cSeg.type = SegmentType::DESCENT;
				}
			}
		}

		// Always render to dedicated texture.
		RenderToTexture(contourSegmentRelativeAngleDistanceTexture, inContour);
	}
	return returnType;
}
