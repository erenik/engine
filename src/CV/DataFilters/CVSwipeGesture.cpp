/// Emil Hedemalm
/// 2014-11-10
/// Swipe gesturrrrrres!~

#include "CVSwipeGesture.h"
#include "CV/CVPipeline.h"

CVSwipeGesture::CVSwipeGesture()
	: CVDataFilter(CVFilterID::SWIPE_GESTURE)
{
	swipeState = SwipeState::IDLE;
	minPointsForSwipe = new CVFilterSetting("Min points for swipe", 10);
	minPointsBeforeLeavingSwipe = new CVFilterSetting("Min points before swipe ends", 5);
	smoothing = new CVFilterSetting("Smoothing", 0.8f);
	maxFramesToAnalyze = new CVFilterSetting("Max frames to consider", 10);
	maxSwipeDuration = new CVFilterSetting("Max swipe duration", 2000);
	minFramesWithFlow = new CVFilterSetting("Min frames with flow", 5);
	about = "Max duration in milliseconds";
	settings.Add(6, 
		minPointsForSwipe,  minPointsBeforeLeavingSwipe, smoothing,
		maxFramesToAnalyze, maxSwipeDuration, minFramesWithFlow);
}

CVSwipeGesture::~CVSwipeGesture()
{

}

int CVSwipeGesture::Process(CVPipeline * pipe)
{
	// Check optical flow.
	int points = pipe->opticalFlowPoints.Size();
	Vector2f direction;
	for (int i = 0; i < points; ++i)
	{
		OpticalFlowPoint & point = pipe->opticalFlowPoints[i];
		direction += point.offset.NormalizedCopy();
	}
	if (points)
	{
		framesWithFlow++;
//		std::cout<<"\nDirection: "<<direction;
	}
//	direction.y *= 0.5f;
	direction.Normalize();
	float s0 = smoothing->GetFloat();
	float s1 = 1 - s0;
	pointsSmoothed = pointsSmoothed * s0 + points * s1;
//	std::cout<<"\nOptical flow points "<<points;
	directionSmoothed = directionSmoothed * s0 + direction * s1;

	// Copy swipe-state to pipeline, before changes herein occur.
	pipe->swipeState = swipeState;

	// State-machine style now.
	switch(swipeState)
	{
		case SwipeState::IDLE:
		{
			if (pointsSmoothed > minPointsForSwipe->GetInt())
			{
				std::cout<<"\nSwipe starting.";
				swipeStart = Time::Now();
				swipeState = SwipeState::IN_SWIPE;
				swipeDirections.Clear();
				framesWithFlow = 0;			
			} 
			break;
		}
		case SwipeState::IN_SWIPE:
		{
			if (maxFramesToAnalyze->GetInt() <= 0 || swipeDirections.Size() < maxFramesToAnalyze->GetInt())
			{
	//			std::cout<<"\nAdding smoothed direction: "<<directionSmoothed;
				swipeDirections.Add(directionSmoothed);
			}
			if (pointsSmoothed < minPointsBeforeLeavingSwipe->GetInt())
			{
				std::cout<<"\nSwipe ending.";
				swipeState = SwipeState::SWIPE_ENDED;
				/// 
			}
			break;
		}
		case SwipeState::SWIPE_ENDED:
		{
			Time now = Time::Now();
			int duration = (now - swipeStart).Milliseconds();
			std::cout<<"\nDuration: "<<duration;
			if (duration > maxSwipeDuration->GetInt() && maxSwipeDuration->GetInt() > 0)
			{
				std::cout<<"\nSwipe exceeded maximum duration. Ignored.";
				swipeState = SwipeState::IDLE;
				break;
			}
			if (framesWithFlow < minFramesWithFlow->GetInt())
			{
				std::cout<<"\nFrames not enough, skipping this swipe! o.o";
			}

			// Calculate average direction throughout the entire swipe-gesture.
			Vector2f averageDirection;
			for (int i = 0; i < swipeDirections.Size(); ++i)
			{
				averageDirection += swipeDirections[i];
			}
			averageDirection /= swipeDirections.Size();
			Vector2f averageDirectionNormalized = averageDirection.NormalizedCopy();
			// TODO: Check for a local extreme instead of an average?

			pipe->swipeGestureDirection = averageDirectionNormalized;
			/// Check direction.
			std::cout<<"\nDetected swipe in direction: "<<pipe->swipeGestureDirection;
	
			swipeState = SwipeState::IDLE;
			break;
		}
	}
	return CVReturnType::SWIPES_GESTURES;
}


