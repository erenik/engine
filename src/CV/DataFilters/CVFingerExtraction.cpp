/// Emil Hedemalm
/// 2014-06-27
/// All filters concerning hands and finger detection.

#include "CVHandFilters.h"
#include "CV/CVPipeline.h"
#include "Window/AppWindowManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "UI/UserInterface.h"
#include "UI/UIImage.h"

using namespace std;

/// New finger extraction filter which works by comparing the contour co-ordinates to its center, creating a histogram based off of it.
CVFingerExtractionFilter::CVFingerExtractionFilter()
	: CVDataFilter(CVFilterID::FINGER_EXTRACTION)
{
	xScale = new CVFilterSetting("X scale", 25.f);
	settings.Add(xScale);
	minimumFingerArea = new CVFilterSetting("Min finger area", 50.f);
	settings.Add(minimumFingerArea);
	minimumClimbs = new CVFilterSetting("Minimum climbs", 5);
	settings.Add(minimumClimbs);

//	minimumFingerCurvature = new CVFilterSetting("Minimum curvature", 0.5f);
//	settings.Add(minimumFingerCurvature);


//	maximumFingerArea = new CVFilterSetting("Max finger area", 15000.f);
//	settings.Add(maximumFingerArea);
	minParallelSegments = new CVFilterSetting("Minimum parallel segments", 3);
	settings.Add(minParallelSegments);
	
	parallelMinimum = new CVFilterSetting("Parallel minimum", 0.9f);
	settings.Add(parallelMinimum);


	returnType = CVReturnType::HANDS;

}
int CVFingerExtractionFilter::Process(CVPipeline * pipe)
{
	// Clear old hands.
	pipe->hands.Clear();
	float minDist = 1000000.f, maxDist = 0.f;

	bool saveOutput = true;

	
	// Reset pointer each frame.
	hand = NULL;

	// Create new hands based on the contours in the pipeline.
	for (int i = 0; i < pipe->contours.Size(); ++i)
	{
		CVContour & contour = pipe->contours[i];
		// Create a new hand?
		CVHand newHand;
		newHand.contour = &contour;
		hand = &newHand;
		
		if (contour.largestInnerCircleRadius < 0)
		{
			std::cout<<"\nExpected a decent value for the contour's largestInnerCircleRadius. Skipping contour.";
			continue;
		}
		// Use approximated inner circle center as base of hand.
		hand->center = contour.largestInnerCircleCenter;
		
		// Scaling for output image. Should be moved to CVFingerExtraction
		CircularList<float> & angles = contour.relativeAngles;
		CircularList<float> & distances = contour.relativesDistances;

		// Parse fingers based on the angle-distance-position pairs.
		ParseFingers(newHand);

		// Re-calculate finger-points based on their given contour-segments.
		for (int j = 0; j < newHand.fingers.Size(); ++j)
		{
			CVFinger & finger = newHand.fingers[j];
			finger.RecalculatePoint();
			finger.DetermineType(1.3f);
		}

		// Render shit!
		pipe->initialInput.convertTo(pipe->output, CV_8UC3);
//		this->PaintOutput(&pipe->output, outputTexture);

		// Always wrong side.. NOT! Only when saving to file.
	//	output->FlipY();

	
	//	output->Save("output/FingerExtractionContourDistance.png", true);

//		hand.fingers = newFingers;
	
		// Add the new hand.
		pipe->hands.Add(newHand);

	}
	returnType = CVReturnType::HANDS;
	return returnType;
}

// Parse fingers based on the angle-distance-position pairs.
void CVFingerExtractionFilter::ParseFingers(CVHand & inHand)
{
	List<CVFinger> suggestedFingersToAdd;
	
	List<CVContourSegment> & segments = inHand.contour->segments;
	CVFinger newFinger;
	int pastSegmentType = -1;
	
	// Fetch a good starting segment for the finger-parsing.
	int leastEnergyIndex = inHand.contour->leastEnergySegmentIndex;
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
	//	std::cout<<"\nLeast energy index defaulted to 0.!";
	}
	assert(leastEnergyIndex >= 0);

	segments[leastEnergyIndex].start = true;

	/// Since many plateaus = flat -> reset finger calculation.
	int plateausInARow = 0;

	for (int i = leastEnergyIndex; i < leastEnergyIndex + segments.Size() + 1; ++i)
	{
		CVContourSegment & seg = segments[i];
		// Check if we should end the current finger.
		bool shouldEnd = false;
		// First by current type.
		switch(seg.type)
		{
			case SegmentType::NONE:
			case SegmentType::EDGE:
				shouldEnd = true;
				break;
		}
		/// But also if there is a transition from a descent to any other segment type, as this is how the algorithm is defined.
		/// Finger = (plateau >) climb > (plateau >) descent
		if (!shouldEnd)
		{
			shouldEnd = pastSegmentType == SegmentType::DESCENT && pastSegmentType != seg.type;
			if (shouldEnd)
			{
				switch(seg.type)
				{
					// No unless!
					// Unless.. that segment is a plateau..?
					case SegmentType::PLATEAU:
						shouldEnd = false;
						break;
					default:
						shouldEnd = true;
						break;
				}
			}
		}
		if (shouldEnd)
		{
			if (newFinger.contourSegments.Size() > 2)
				suggestedFingersToAdd.Add(newFinger);
			newFinger = CVFinger();
		}
		// Depending on type, count up stuff.
		switch(seg.type)
		{
			case SegmentType::NONE:
			case SegmentType::EDGE:
			{
				pastSegmentType = seg.type;
				continue;
			}
			case SegmentType::CLIMB:
			{
				/// If the finger had descent segments since before, save it away before starting the new one!
				if (newFinger.descentSegments.Size())
				{
					if (newFinger.climbSegments.Size() > 0)
						suggestedFingersToAdd.Add(newFinger);
					newFinger = CVFinger();
				}
				newFinger.climbSegments.Add(seg);
				break;
			}
			case SegmentType::DESCENT:
				// Must ascend before reciving other stuff
				if (newFinger.climbSegments.Size() == 0)
					continue;
				newFinger.descentSegments.Add(seg);
				break;
			case SegmentType::PLATEAU:
				// Must ascend before reciving other stuff <- Will not work if the hand is mirrored.
	//			if (newFinger.climbSegments.Size() == 0)
	//				break;
				++plateausInARow;
				if (plateausInARow > 5)
				{
					suggestedFingersToAdd.Add(newFinger);
					newFinger = CVFinger();
				}
				newFinger.plateauSegments.Add(seg);
				break;
		}
		if (seg.type != SegmentType::PLATEAU)
		{
			plateausInARow = 0;
		}
		// Add tis segment. If it was bad a continue should have been placed earlier.
		newFinger.contourSegments.Add(seg);
		pastSegmentType = seg.type;
	}
	
	// Add final finger as needed.
	if (newFinger.climbSegments.Size() > 0)
		suggestedFingersToAdd.Add(newFinger);
	newFinger = CVFinger();

	/// Perform filtering of the suggested fingers
	for (int i = 0; i < suggestedFingersToAdd.Size(); ++i)
	{
		CVFinger & finger = suggestedFingersToAdd[i];
		/// See if it has enough end-segments to be considered a real-finger... (at least 0.5 of begin-segments?)
		bool good = true; //= finger.descentSegments.Size() > finger.climbSegments.Size() * 0.3f;
		// Ensure minimum climbs
	//	good &= finger.descentSegments.Size() * 0.3f < finger.climbSegments.Size();
		if (finger.contourSegments.Size() < 5)
			good = false;
		if (good)
			good &= finger.climbSegments.Size() >= minimumClimbs->GetInt();
		if (good)
		{
			// Also calculate its contour area. We don't want small finger.
			float area = finger.ContourArea();
			if (area < minimumFingerArea->GetFloat())
				good = false;
			// Check maximum area too..
//			else if (area > maximumFingerArea->GetFloat())
	//			good = false;
		}
		// Check amount of parralel segments within the finger. A finger should have several!
		if (good){
			int parallelSegments = finger.ParallelSegments(parallelMinimum->GetFloat());
			if (parallelSegments < minParallelSegments->GetInt())
				good = false;
		}
		if (good)
		{

//			float curvature = finger.MaximumCurvature();
	//		if (curvature <= minimumFingerCurvature->GetFloat())
		//		good = false;
		}
		if (good)
		{
			// Save it.
			finger.lengthToCenter = (finger.point - hand->center).Length();
			hand->fingers.Add(finger);
						
			hand->averageFingerLength += finger.lengthToCenter;
		}
		else {
			static int p = 0;
			++p;
//			std::cout<<"\nSkippin a fingar;";
		}
	}
	// Average it.
	if (hand->fingers.Size())
		hand->averageFingerLength /= hand->fingers.Size();
}



/// First texture painted to is in raw image co-ordinates, while the second one is in relative angle/distance coordinates .
void CVFingerExtractionFilter::PaintOutput(cv::Mat * pipelineTexture, Texture * relativeTexture)
{
	assert(false);
}


