/// Emil Hedemalm
/// 2014-07-11
/// Finger class. Used by CVHand.

#include "CVFinger.h"
#include <cstring>

#include "PhysicsLib/Shapes/Line.h"

CVFinger::CVFinger()
{
	type = FingerType::UNKNOWN;
	area = -1;
	numFingers = 1;
	width = -1;
	lengthToCenter = 0.f;
	thumbRatio = 0;
}

/// Recalculates finger-point based on its contourSegments. Returns false if it could not be done.
bool CVFinger::RecalculatePoint()
{
	CVContourSegment * lastStart = NULL, 
		* firstEnd = NULL;
	if (climbSegments.Size() == 0)
		return false;
	lastStart = &climbSegments.Last();
	point = lastStart->rawInputStop;
	return true;
}

/// Tries to assess the type of finger this mostly is. Will revert to UNKNOWN if it is uncertain. 
int CVFinger::DetermineType(float fingerRatio)
{
	/// Return type if already set before?
	if (type)
		return type;

	type = FingerType::FINGER;
	return type;
}
	

/// Based on the contour segments.
float CVFinger::ContourArea()
{
	// Calculate contour area of the finger to see if it should be included.
	std::vector<cv::Point> fingerContour;
	for (int i = 0; i < contourSegments.Size(); ++i)
	{
		CVContourSegment & seg = contourSegments[i];
		cv::Point point(seg.rawInputStart.x, seg.rawInputStart.y);
		fingerContour.push_back(point);
	}
	area = cv::contourArea(fingerContour);
	return area;
}

/** Calculates the maximum curvatur found within the contour segments. 
	A value close to 1.0 means that there exist at least 1 180-degree turn, 
	while a value of 0 would mean the contour segments form a straight line.
*/
float CVFinger::MaximumCurvature()
{
	float maxCurve = 0;
	List<Vector3f> directions;
	for (int i = 0; i < contourSegments.Size(); ++i)
	{
		CVContourSegment & seg = contourSegments[i];
		directions.Add((seg.rawInputStop - seg.rawInputStart).NormalizedCopy());
	}
	float dot;
	float curve;
	for (int i = 0; i < directions.Size(); ++i)
	{
		Vector3f & dir1 = directions[i];
		for (int j = i+1; j < directions.Size(); ++j)
		{
			Vector3f & dir2 = directions[j];
			dot = dir1.DotProduct(dir2);
			// Re-scale the dot product from the [-1,1] interval so an inverted [0,1] interval.
			curve = (-dot * 0.5f)+0.5f;
			if (curve > maxCurve)
				maxCurve = curve;
		}
	}
	return maxCurve;
	
}

/** Returns a number indicating number of approximately parallel segments. 
	It uses an algorithm similar to the one used in MaximumCurvature, but has to evaluate it on all possible segment-pairs.
*/
int CVFinger::ParallelSegments(float minimumDotProductToBeConsideredParallel)
{
	float maxCurve = 0;
	for (int i = 0; i < contourSegments.Size(); ++i)
	{
		CVContourSegment & seg = contourSegments[i];
		seg.rawInputDirection = (seg.rawInputStop - seg.rawInputStart).NormalizedCopy();
	}
	int pairs = 0;
	float dot;
	float curve;
	Vector3f averageDirection;
	List<CVContourSegment> tempContourSegmentList = contourSegments;
	for (int i = 0; i < tempContourSegmentList.Size(); ++i)
	{
		CVContourSegment & seg = tempContourSegmentList[i];
		Vector3f & dir1 = seg.rawInputDirection;
		List<CVContourSegmentPair> pairsToAdd;
		for (int j = i+1; j < tempContourSegmentList.Size(); ++j)
		{
			CVContourSegment & seg2 = tempContourSegmentList[j];
			Vector3f & dir2 = seg2.rawInputDirection;
			dot = dir1.DotProduct(dir2);
			// Re-scale the dot product from the [-1,1] interval so an inverted [0,1] interval.
			curve = (-dot * 0.5f)+0.5f;
			if (curve > 
				minimumDotProductToBeConsideredParallel
					)
			{
				/** Check dot product of this pair compared to their directions. If not perpendicular, skip it, as it will
					Not really be of interest later on when calculating finger thickness/width.
				*/
				Vector3f oneToTwo = (seg2.rawInputStart - seg.rawInputStart).NormalizedCopy();
				float betweenDotDir = oneToTwo.DotProduct(dir1);
				if (AbsoluteValue(betweenDotDir) > 0.2f)
					continue;

				// Remove them both from the list of pairs to be examined. Decrement both i and j so that it works as intended.
				++pairs;


				CVContourSegmentPair newPair;
				newPair.one = tempContourSegmentList[i];
				newPair.two = tempContourSegmentList[j];
				newPair.distance = (newPair.one.rawInputStart - newPair.two.rawInputStart).Length();
				pairsToAdd.Add(newPair);

				averageDirection += newPair.one.rawInputDirection;
				// Break the inner loop.
			//	break;
			}
		}
		// Go through list of proposed pairs. Keep/Add only the one of shortest distance.
		float leastDistance = 1000000.f;
		CVContourSegmentPair * leastDistancePair = NULL;
		for (int j = 0; j < pairsToAdd.Size(); ++j)
		{
			CVContourSegmentPair & p = pairsToAdd[j];
			if (p.distance < leastDistance)
			{
				leastDistance = p.distance;
				leastDistancePair = &p;
			}
		}
		if (leastDistancePair)
			parallelPairs.Add(*leastDistancePair);
		
	}

	averageDirection.Normalize();

	int pairsPreFilter = parallelPairs.Size();
	/// For each pair, check their compliance with the average direction. Skip those not aligned with it.
	for (int i = 0; i < parallelPairs.Size(); ++i)
	{
		CVContourSegmentPair & pair = parallelPairs[i];
		Vector3f dir = pair.one.rawInputDirection;
		float dot = dir.DotProduct(averageDirection);
		if (dot < minimumDotProductToBeConsideredParallel)
		{
			parallelPairs.RemoveIndex(i, ListOption::RETAIN_ORDER);
			--i;
		}
		/*
		// Also check that each starting-segment only occurs once.
		else 
		{
			for (int j = i+1; j < parallelPairs.Size(); ++j)
			{
				CVContourSegmentPair & pair2 = parallelPairs[j];
				if (pair2.one.rawInputStart = pair.one.rawInputStart)
				{
					parallelPairs.RemoveIndex(j, ListOption::RETAIN_ORDER);
					--j;
				}
			}
		}
		*/
	}
	int parsPostFilter = parallelPairs.Size();
	int pairsRemoved = pairsPreFilter - parsPostFilter;
	return parallelPairs.Size();
}

/// Calculates and stores the width. Requires ParallelSegments to be called first to calculate all parallel segment pairs.
float CVFinger::CalculateFingerWidth()
{
	float minDist = 1000000.f;
	float maxDist = 0;
	float averageDist = 0;

	if (parallelPairs.Size() == 0)
	{
		std::cout<<"\nParallel pairs list empty! CAlculate it first D:";
		return -1;
	}
	for (int i = 0; i < parallelPairs.Size(); ++i)
	{
		CVContourSegmentPair & pair = parallelPairs[i];
		Line line(pair.one.rawInputStart, pair.one.rawInputStop);
		float distance = line.Distance(pair.two.rawInputStart);
		pair.distance = distance;

		averageDist += distance;
		if (distance > maxDist)
			maxDist = distance;
		else if (distance < minDist)
			minDist = distance;
	}
	averageDist /= parallelPairs.Size();
	this->width = averageDist;
	return width;
}


/// Gets color suitable for rendering this finger.
Vector4f CVFinger::GetColor() const
{
	Vector4f fingerColor = Vector3f(50.f, 150.f, 55.f);
	switch(type)
	{
		case FingerType::INDEX_OR_PINKY:
			fingerColor = Vector3f(255.f, 50.0f, 100.f);
			break;
		case FingerType::THUMB:
			fingerColor = Vector3f(255.f, 155.f, 50.f);
			break;
		case FingerType::BAD:
			fingerColor = Vector3f(0,0,0);
			break;
	}
	return fingerColor;
}
