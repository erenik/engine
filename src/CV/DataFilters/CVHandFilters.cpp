/// Emil Hedemalm
/// 2014-06-27
/// All filters concerning hands and finger detection.

#include "CVHandFilters.h"
#include "CV/CVPipeline.h"
#include "TextureManager.h"
#include "Window/AppWindowManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "UI/UserInterface.h"
#include "UI/UIImage.h"

#include "List/CircularList.h"

using namespace std;


/// Emulates one or more hands to be displayed on the screen.
CVHandEmulator::CVHandEmulator()
: CVDataFilter(CVFilterID::HAND_EMULATOR)
{
	this->size = new CVFilterSetting("Relative size", 0.5f);
	this->numHands = new CVFilterSetting("Num hands", 1);
	settings.Add(size);
	settings.Add(numHands);
	about = "Emulates one or more hands.";
}
int CVHandEmulator::Process(CVPipeline * pipe)
{
	pipe->hands.Clear();
	int handsToCreate = numHands->GetInt();
	for (int i = 0; i < handsToCreate; ++i)
	{
		CVHand newHand = CVHand();
		newHand.center = Vector2f(pipe->initialInput.cols, pipe->initialInput.rows);
		newHand.center *= 0.5f;
		for (int j = 0; j < 5; ++j)
		{
			CVFinger newFinger;
			newFinger.point = Vector2f(10 * j, 60);
			newHand.fingers.Add(newFinger);
		}
		pipe->hands.Add(newHand);
	}
	// Set what to render later on by specifying return-type..
	returnType = CVReturnType::HANDS;
	return returnType;
}


// Own custom hand-detector, identifying fingers by first identifying relevant convexity defects (convex hull - contour)
CVHandDetector::CVHandDetector()
	: CVDataFilter(CVFilterID::HAND_DETECTOR)
{
	minimumArea = new CVFilterSetting("Minimum contour area", 1000);
	minimumFingerDistance = new CVFilterSetting("Minimum finger distance", 200);
	settings.Add(minimumArea);
	settings.Add(minimumFingerDistance);
	about = "CVHand-detector utilizing contour-convex-hull defects.";
}

int CVHandDetector::Process(CVPipeline * pipe)
{
	// Clear hands if any.
	pipe->hands.Clear();
	for (int i = 0; i < pipe->contours.Size() && i < 1; ++i)
	{
		CVContour * contour = &pipe->contours[i];
		std::vector<cv::Point> cvContour = pipe->cvContours[i];
		
		float contourArea = cv::contourArea(cvContour);
		// Skip areas below minimum size threshold
		if (contourArea < minimumArea->GetInt())
			continue;
		CVHand newHand(contour);
		
		
		// Calculate rect too.

		// Creat fingerrrrs ovo
		if (pipe->convexityDefects.Size())
		{
			// Create a "finger" at the start and end of each defect.
			for (int j = 0; j < pipe->convexityDefects.Size(); ++j)
			{
				cv::Vec4i defect = pipe->convexityDefects[j];
				if (defect[0] >= cvContour.size() || defect[1] >= cvContour.size())
					continue;
				cv::Point point = cvContour[defect[0]];
				CVFinger finger;
				finger.point = Vector2f(point.x, point.y);
				newHand.fingers.Add(finger);
				point = cvContour[defect[1]];
				finger.point = Vector2f(point.x, point.y);
				newHand.fingers.Add(finger);
			}
			
			// Merge duplicate fingers
			for (int j = 0; j < newHand.fingers.Size(); ++j)
			{
				CVFinger finger = newHand.fingers[j];
				Vector2f finger1 = finger.point;
				for (int k = j + 1; k < newHand.fingers.Size(); ++k)
				{
					CVFinger fing = newHand.fingers[k];
					Vector2f finger2 = fing.point;
					float distance = (finger2 - finger1).Length();
					if (distance < minimumFingerDistance->GetInt())
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
	returnType = CVReturnType::HANDS;
	return returnType;
}




CVHandPersistance::CVHandPersistance()
	: CVDataFilter(CVFilterID::HAND_PERSISTENCE)
{
	/// Max frames to look back at.
	maxFrames = new CVFilterSetting("Max frames", 3);
	// For smoothing velocity over X frames.
	velocitySmoothingFactor = new CVFilterSetting("Velocity smoothing factor", 0.2f);
	// Threshold for identifying this hand as one detected in the last frame. Default 1.0
	areaThresh = new CVFilterSetting("Area size threshold", 0.5f);
	// Threshold for identifying this hand as one detected in the last frame. Compared to overall size of the contour. Default 1.0.
	distanceThresh = new CVFilterSetting("Distance ratio", 0.01f);

	settings.Add(4, maxFrames, velocitySmoothingFactor, areaThresh,
		distanceThresh);
	about = "Most settings control how the matching process \nis conducted to detect contours in past frames\nwhich are related to the ones in the current frame.";
}

struct HandComparison 
{
	void ExtractRatios()
	{
		contourSizeRatio = hand1->contourAreaSize < hand2->contourAreaSize? hand1->contourAreaSize / hand2->contourAreaSize : hand2->contourAreaSize / hand1->contourAreaSize;
		positionDistance = (hand1->center - hand2->center).Length();
	};
	CVHand * hand1, * hand2;
	// Total similarity, calculate as wanted.
	float similarity;
	// Raw comparison data.
	float contourSizeRatio;
	float positionDistance;
};

int CVHandPersistance::Process(CVPipeline * pipe)
{
	// Store the current hands in the list
	List<CVHand> hands = pipe->hands;

	List<HandComparison> totalComparisons;
	// Compute likeylihood that the detected hands are the same ones as detected in the previous frame, or earlier frames if they were not visible in the last frame?
	for (int i = 0; i < hands.Size(); ++i)
	{
		CVHand & hand = hands[i];
		List<HandComparison> comparisons;
		for (int j = 0; j < pastFramesHands.Size(); ++j)
		{
			List<CVHand> & frameHands = pastFramesHands[j];
			for (int k = 0; k < frameHands.Size(); ++k)
			{
				CVHand & frameHand = frameHands[k];
				HandComparison comp;
				comp.hand1 = &hand;
				comp.hand2 = &frameHand;
				comp.ExtractRatios();
				// Calculate ratio according to settings.
				comp.similarity = 1.f;
				// Size of contour
				if (comp.contourSizeRatio < areaThresh->GetFloat())
					comp.similarity *= 0;
				else
					comp.similarity *= comp.contourSizeRatio;
				// Decrease similarty for each pixel offset.
				comp.similarity *= 1.f - (comp.positionDistance * distanceThresh->GetFloat());
				// Multiply similarity by a factor according to how many frames ago it was saved.
				comp.similarity *= 1.f / (1 + k);

				// Ignore negative ones.
				if (comp.similarity <= 0)
					continue;
				// Add to list of comparisons
				comparisons.Add(comp);
			}
		}
		if (comparisons.Size() == 0)
			continue;
		// Use hand with largest comparison value (if above minimum).
		HandComparison hc = comparisons[0];
		for (int j = 1; j < comparisons.Size(); ++j)
		{
			HandComparison & hc2 = comparisons[j];
			if (hc2.similarity > hc.similarity)
				hc = hc2;
		}
		// Perform persistence smoothing between the best comparison and the hand.
		float newFactor = (1 - velocitySmoothingFactor->GetFloat());
		float oldFactor = velocitySmoothingFactor->GetFloat();
		hand.approximateVelocityFromOpticalFlow = hand.approximateVelocityFromOpticalFlow * newFactor +
			hc.hand2->approximateVelocityFromOpticalFlow * oldFactor;

		hand.averagedVelocityMagnitude = hand.averagedVelocityMagnitude * newFactor +
			hc.hand2->averagedVelocityMagnitude * oldFactor;

		// Add to total comparisons.
		totalComparisons += comparisons;
	}

	// Copy the averaged/persisted hands into the pipeline
	pipe->hands = hands;
	
	pastFramesHands.Insert(hands, 0);
	if (pastFramesHands.Size() > maxFrames->GetInt())
	{
		pastFramesHands.RemoveIndex(pastFramesHands.Size() - 1, ListOption::RETAIN_ORDER);
	}
	
	// Return all the hands o-o
	returnType = CVReturnType::HANDS;
	return returnType;
}

CVFingerActionFilter::CVFingerActionFilter()
	: CVDataFilter(CVFilterID::FINGER_ACTION)
{
	minimumDuration = new CVFilterSetting("Minimum state duration", 500);
	settings.Add(minimumDuration);
	maxStatesStored = new CVFilterSetting("Max states stored", 20);
	settings.Add(maxStatesStored);

	// Check over 250 frames?
	framesToTrack = 250;
	filesSaved = 0;

	postFilter = TexMan.New();
	preFilter = TexMan.New();
	postFilter->width = preFilter->width = framesToTrack + 50;
	postFilter->height = preFilter->height = 25; // 3 pixels per .. finger?
	preFilter->CreateDataBuffer();
	postFilter->CreateDataBuffer();		
	preFilter->SetColor(Vector4f(0,0,0,0));
	postFilter->SetColor(Vector4f(0,0,0,0));

	framesPassed = 0;

#define PROFILE_FINGER_ACTION_FILTER
}
int CVFingerActionFilter::Process(CVPipeline * pipe)
{
	CVHand * hand = NULL;
	if (pipe->hands.Size())
		hand = &pipe->hands[0];
	// Return if no hand?
	if (!hand)
		return 0;

	// Check finger now, but update it to the last finger after all processing is done.
	int fingersNow = hand->fingers.Size();
#ifdef PROFILE_FINGER_ACTION_FILTER
	// Store le value for statistical analysis
	preFilter->SetPixel(framesPassed, fingersNow * 3, Vector3f(1, 0, 0));
#endif

	int64 now = Timer::GetCurrentTimeMs();

	FingerState * lastFingerState = NULL;
	if (fingerStates.Size())
		lastFingerState = &fingerStates.Last();
	// If the finger state has not changed, update the last added state's duration, if any.
	if (lastFingerState && fingersNow == lastFingerState->fingers) 
	{
		lastFingerState->duration = now - lastFingerState->start;
		lastFingerState->stop = now;
	}	
	// Check if we should add a finger due to the time it has been visible (long enough).
	else if (fingersNow == fingersLastFrame)
	{
		int64 duration = now - lastFingerStartTime;
		/// Add it!
		if (duration > minimumDuration->GetInt())
		{
			// Create new state for the new finger amount.
			FingerState newState;
			newState.fingers = fingersNow;
			newState.start = lastFingerStartTime;
			newState.duration = duration;
			for (int i = 0; i < hand->fingers.Size(); ++i)
			{
				newState.positions.Add(hand->fingers[i].point);
			}
			fingerStates.Add(newState);
			// If we pass 20 or something, start deleting old states.
			if (fingerStates.Size() >= maxStatesStored->GetInt())
				fingerStates.RemoveIndex(0, ListOption::RETAIN_ORDER);	
		}
	}
	// When finger count changes. Defined implicitly due to the previous if-clause.
	else /* if (fingersNow != fingersLastFrame) */
	{
		// Set finger start time!
		lastFingerStartTime = now;
	}

	
	// Synchronize the pipeline list of finger states with our own.
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
			}
		}
	}

	// Clear their list.
	pipe->fingerStates.Clear();
	// And copy over our list
	for (int i = 0; i < fingerStates.Size(); ++i)
	{
		FingerState & state = fingerStates[i];
		pipe->fingerStates.Add(state);
	}

#ifdef PROFILE_FINGER_ACTION_FILTER
	// Store le value for statistical analysis
	if (pipe->fingerStates.Size())
	{
		FingerState & lastState = pipe->fingerStates.Last();
	//	std::cout<<"\nLastState duration: "<<lastState.duration;
		postFilter->SetPixel(framesPassed, lastState.fingers * 3, Vector3f(1 - (lastState.duration - minimumDuration->GetInt())/ (float)minimumDuration->GetInt() * 0.25, (lastState.duration - minimumDuration->GetInt()) / (float)minimumDuration->GetInt(),0));
		++framesPassed;
		if (framesPassed > framesToTrack)
		{
			std::cout<<"\nSaving FingerStateFilter statistics to output/";
			
			preFilter->Save("output/"+String::ToString(filesSaved)+"FingerStatesPreFilter", true);
			postFilter->Save("output/"+String::ToString(filesSaved)+"FingerStatesPostFilter", true);

			// Clear the lists
			preFilter->SetColor(Vector4f(0,0,0,0));
			postFilter->SetColor(Vector4f(0,0,0,0));
			++filesSaved;
			framesPassed = 0;
		}
	}
#endif

	fingersLastFrame = fingersNow;
		
	// Should replace with "fingers" type and render them somewhere...
	returnType = CVReturnType::HANDS;
	return returnType;
}


/// Filter which tries to associate optical flow quadrants with the existing hands/hand contours,
/// then approximate the velocity of the hand from those detected quadrants.
CVApproximateHandVelocityByOpticalFlow::CVApproximateHandVelocityByOpticalFlow()
	: CVDataFilter(CVFilterID::APPROXIMATE_HAND_VEOCITY_BY_OPTICAL_FLOW)
{
	checkType = new CVFilterSetting("Check type", 0);
	distance = new CVFilterSetting("MaxDistance", 100.f);
	settings.Add(2, checkType, distance);
	about = "Check type: 0 - radial distance to center\n1 - quadrant center point in contour\n2 - same as 1 but with segments";
}

int CVApproximateHandVelocityByOpticalFlow::Process(CVPipeline * pipe)
{
	// Fetch flow from pipeline.
	String statusString = "Hands: "+String::ToString(pipe->hands.Size());

	OpticalFlow & flow = pipe->opticalFlow;
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		CVHand & hand = pipe->hands[i];
		statusString += "\nHand " + String(i);

		List<OpticalFlowQuadrant*> quadrantsWithin;
		switch(checkType->GetInt())
		{
			case 0:
				quadrantsWithin = hand.GetQuadrantsWithinContour(&flow, distance->GetFloat());
				break;
			case 1:
				quadrantsWithin = hand.GetQuadrantsWithinContour(&flow, false, Vector3f(0,1.0));
				break;
			case 2:
				quadrantsWithin = hand.GetQuadrantsWithinContour(&flow, true, Vector3f(0,1.0));
				break;
		}
		hand.approximateVelocityFromOpticalFlow = Vector3f();
		if (!quadrantsWithin.Size())
		{
			statusString += " lacking related quadrants";
			continue;
		}
		for (int i = 0; i < quadrantsWithin.Size(); ++i)
		{
			OpticalFlowQuadrant * q = quadrantsWithin[i];
			hand.approximateVelocityFromOpticalFlow += q->averageFlow;
		}
		
		hand.approximateVelocityFromOpticalFlow /= quadrantsWithin.Size();
		hand.averagedVelocityMagnitude = hand.approximateVelocityFromOpticalFlow.Length();

		statusString += ": aVm "+String(hand.averagedVelocityMagnitude, 3); 
	
	}
	
	// Set string to display in the info. o-o
	status = statusString;
	return 0;
}

/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVApproximateHandVelocityByOpticalFlow::Paint(CVPipeline * pipe)
{
	pipe->initialInput.copyTo(pipe->output);
	// Paint hands, then paint the velocity from optical flow as a nice arrow, if possible.
	RenderContourSegments(pipe);
	// Renders velocity within.
	RenderHands(pipe);
	
}


CVFilterHandsByMovement::CVFilterHandsByMovement()
	: CVDataFilter(CVFilterID::FILTER_HANDS_BY_MOVEMENT)
{
	type = new CVFilterSetting("Type", 0);
	thresh = new CVFilterSetting("Threshold value", 1.f);
	settings.Add(2, type, thresh);
	
	about = "Type 0 - Average velocity, 1 average velocity magnitude";
}
int CVFilterHandsByMovement::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		CVHand & hand = pipe->hands[i];
		bool remove = false;
		switch(type->GetInt())
		{
			case 0: 
			{			
				float velocity = hand.approximateVelocityFromOpticalFlow.Length();
				if (velocity < thresh->GetFloat())
					remove = true;
				break;
			}
			case 1:
				if (hand.averagedVelocityMagnitude < thresh->GetFloat())
					remove = true;
				break;
		}
		// remove it?
		if (remove)
		{
			pipe->hands.RemoveIndex(i); 
			--i;
		}
	}
	returnType = CVReturnType::HANDS;
	return returnType;
}
