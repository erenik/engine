/// Emil Hedemalm
/// 2014-08-18
/// Menu interaction filter! Works via the InputManager to generate UI events there. :)

#include "CVRenderFilters.h"

#include "CV/CVPipeline.h"

#include "Direction.h"

#include "Input/InputManager.h"

CVMenuInteraction::CVMenuInteraction()
	: CVRenderFilter(CVFilterID::MENU_INTERACTION)
{
	minSwipeLength = new CVFilterSetting("Min swipe length", 0.5f);
	minSwipePoints = new CVFilterSetting("Min swipe points", 5);
	minVelocity = new CVFilterSetting("Min velocity", 50.f);
	maxSwipeDurationMillis = new CVFilterSetting("Max swipe duration millis", 200);

	settings.Add(4, minSwipeLength, minSwipePoints, minVelocity, 
		maxSwipeDurationMillis);
}


int CVMenuInteraction::Process(CVPipeline * pipe)
{
	Time now = Time::Now();
	// Delete previous positions as their time elapses.
	for (int i = 0; i < previousPositions.Size(); ++i)
	{
		TimePosition & tp = previousPositions[i];
		int millis = (now - tp.time).Milliseconds();
		if (millis > maxSwipeDurationMillis->GetInt() * 2)
		{
			previousPositions.RemoveIndex(i);
			--i;
		}
	}
	
	/// Assume a single point cloud for this interaction step.
	if (pipe->pointClouds.Size() != 1)
		return 0;

	CVPointCloud & pointCloud = pipe->pointClouds[0];
	if (pointCloud.points.Size() == 0)
		return 0;

	Vector2f position = pointCloud.pcaCenter;

	/// Previous positions of the interacting point cloud.
	TimePosition newTp;
	newTp.position = position;
	newTp.time = now;
	previousPositions.Add(newTp);

	int probabilityUp = 0, probabilityLeft = 0, probabilityRight = 0, probabilityDown = 0;
	
	/// Calculate average velocity over the past positions. Grab the minimum and maximum positions traveled through too?
	Vector3f averageVelocity;
	int velocities = 0;
	Time timeSpent;
	TimePosition previous;
	for (int i = 0; i < previousPositions.Size(); ++i)
	{
		TimePosition & tp = previousPositions[i];
		if (i > 1)
		{
			Vector3f distTraveled = tp.position - previous.position;
			averageVelocity += distTraveled;
			Time t = tp.time - previous.time;
			timeSpent += t;
			++velocities;
		}
		previous = tp;
	}
	if (velocities > 0)
		averageVelocity /= velocities;

	/// Require at least 4 frames to evaluate any movement.
//	if (velocities < 4)
//		return 0;

	

	int micros = timeSpent.Microseconds();
	// If swipe less than 100 ms?
	if (micros > maxSwipeDurationMillis->GetInt() * 1000)
		return 0;
	averageVelocity /= micros * 0.000001f;

	/// Search for a swipe in our array of previous point cloud positions?
	float velocity = averageVelocity.Length();
	if (velocity < minVelocity->GetFloat())
		return 0;

	Vector3f norm = averageVelocity.NormalizedCopy();

	float probabilityDueToVelocity = velocity;

	float probabilities[Direction::DIRECTIONS];

	List<Vector3f> directionVectors;
	float maxProbability = 0;
	int maxProbabilityIndex = 0;
	for (int i = 0; i < Direction::DIRECTIONS; ++i)
	{
		directionVectors += Direction::GetVector(i);

		// Do probability straight away?
		float probabilityDueToDirection = norm.DotProduct(directionVectors[i]);
		float & probability = probabilities[i];
		probability = probabilityDueToDirection * probabilityDueToVelocity;
		if (probability > maxProbability)
		{
			maxProbabilityIndex = i;
			maxProbability = probability;
		}
	}
	// Require a minimum probability.
	if (maxProbability < 25)
		return 0;

	// Move according to the highest probability.
	switch(maxProbabilityIndex)
	{
		// Y-swapped due to image space coordinates.
		case Direction::UP:
			std::cout<<"\nDown o-o!";
			Input.UIDown();
			break;
		case Direction::DOWN:
			std::cout<<"\nUp o-o";
			Input.UIUp();
			break;
		case Direction::LEFT:
			std::cout<<"\nLeft? o.o";
			Input.UILeft();
			break;
		case Direction::RIGHT:
			std::cout<<"\nRight! o.o";
			Input.UIRight();
			break;
		default:
			assert(false);

	}

	ClearPreviousPositions();

	returnType = CVReturnType::POINT_CLOUDS;
	return returnType;
}

List<Entity*> CVMenuInteraction::GetEntities()
{
	return List<Entity*>();
}


/// o-o
void CVMenuInteraction::ClearPreviousPositions()
{
	previousPositions.Clear();
}
