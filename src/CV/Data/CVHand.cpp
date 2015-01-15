/// Emil Hedemalm
/// 2014-06-27
/// Computer Vision CVHand class.

#include "CVHand.h"
#include "CV/OpticalFlow/OpticalFlow.h"

CVHand::CVHand()
{
	contour = NULL;
	bad = false;
	averagedVelocityMagnitude = 0;
}
	

CVHand::CVHand(CVContour * contour)
: contour(contour)
{
	// Set things from the contour if possible
	if (contour)
	{
		center = contour->centerOfMass;
	}
}
CVHand::~CVHand()
{

}

// .
List<OpticalFlowQuadrant*> CVHand::GetQuadrantsWithinContour(OpticalFlow * fromOpticalFlow, float byDistanceToCenter)
{
	List<OpticalFlowQuadrant*> quadrants;
	for (int i = 0; i < fromOpticalFlow->Elements(); ++i)
	{
		OpticalFlowQuadrant * quad = fromOpticalFlow->Element(i);
		bool inside = false;
		/// Point in contour check.
		Vector3f center = (quad->max + quad->min) * 0.5f;

		if ((center - this->center).LengthSquared() < byDistanceToCenter)
			inside = true;


		/// Compare with our contour-segments instead?
		/*
		if (this->contour->PointInside(center))
		{
			quadrants.Add(quad);
		}
		*/
		if (inside)
			quadrants.Add(quad);
	}
	return quadrants;
}

List<OpticalFlowQuadrant*> CVHand::GetQuadrantsWithinContour(OpticalFlow * fromOpticalFlow, bool fromSegments, Vector3f byPointInContourComparisonUsingGivenVector)
{

	List<OpticalFlowQuadrant*> quadrants, initialQuadrants;

	Vector3f dir = byPointInContourComparisonUsingGivenVector.NormalizedCopy();

	// Do initial filtering based on the given vector.
	for (int i = 0; i < fromOpticalFlow->Elements(); ++i)
	{
		OpticalFlowQuadrant * q = fromOpticalFlow->Element(i);
	//	if (q->center.DotProduct(dir) < (q->max.x - q->center.x))
			initialQuadrants.Add(q);
	}


	for (int i = 0; i < initialQuadrants.Size(); ++i)
	{
		OpticalFlowQuadrant * quad = initialQuadrants[i];
		Vector3f quadCenter = quad->center;
		bool inside = false;
		
		/// Compare with our contour-segments instead?
		if (this->contour)
		{
			if (fromSegments)
			{
				inside = this->contour->PointInsideSegments(quadCenter, byPointInContourComparisonUsingGivenVector);			
			}
			else 
			{
				inside = this->contour->PointInside(quadCenter, byPointInContourComparisonUsingGivenVector);
			}
		}
		if (inside)
			quadrants.Add(quad);
	}
	return quadrants;
}


int CVHand::IdentifyFingers(float thumbToOtherFingerThreshold)
{
	
	// Calculate extra stuff for all fingers first?
	int starts = 0, ends = 0;
	CVContourSegment * finalStop = NULL, 
		* firstStart = NULL;

	int type = FingerType::FINGER;

	// Calculate general stuff.
	for (int i = 0; i < fingers.Size(); ++i)
	{
		CVFinger * finger = &fingers[i];
		float climbToDescentDifferenceRatio = AbsoluteValue(1 - finger->climbSegments.Size() / (float)finger->descentSegments.Size());
		float plateauToDescentDifferenceRatio = AbsoluteValue(1 - finger->plateauSegments.Size() / (float)finger->descentSegments.Size());
		Vector3f centerToPoint = finger->point - center;
		finger->direction = centerToPoint.NormalizedCopy();
		// p=p
		finger->CalculateFingerWidth();
	}

	// Check general direction of all fingers, as well as other statistics, each time 4+ fingers are visible.
	static Vector3f averageFingerDirection;
	static float minimumFingerWidth = 10000.f;
	static float averageFingerArea = 0.f;
	static Vector3f thumbDirection;		// Last seen thumb's relative direction to the center of hand.
	static float thumbLength = 0.f;
	static float averageFingerLength = 0.f;

	static List<CVFinger> fingersLastFrame;

	// Fetch em only when we have a significant amount of fingers visible.
	if (fingers.Size() >= 5)
	{
		minimumFingerWidth = 1000000.f;
		averageFingerArea = 0.f;
		for (int i = 0; i < fingers.Size(); ++i)
		{
			CVFinger & finger = fingers[i];
			averageFingerArea += finger.area;
			averageFingerDirection += finger.direction;
			averageFingerLength += finger.lengthToCenter;
			if (finger.width < minimumFingerWidth)
				minimumFingerWidth = finger.width;
		}
		averageFingerArea /= fingers.Size();
		averageFingerDirection.Normalize();
		averageFingerLength /= fingers.Size();
	}
	
	/// Use width and area-detection for the thumb only when 5 fingers are clearly visible.
	if (fingers.Size() == 5)
	{
		
		/// Check dot products of the finger's directions with the average one.
		float leastDot = 5.f;
		CVFinger * leastDotFinger = NULL;
		float nextLeastDot = 5.f;
		CVFinger * nextLeastDotFinger = NULL;
		for (int i = 0; i < fingers.Size(); ++i)
		{
			CVFinger & finger = fingers[i];

			// Assign thumb-ratio based on finger width compared to minimum finger width.
			finger.thumbRatio = finger.width / minimumFingerWidth;
			// IF the finger area is extremely large, decrease thumb-ratio (and also any other specific-finger-ratio)!
			if (finger.area > averageFingerArea * 1.8f)
			{
				finger.thumbRatio -= (finger.area - averageFingerArea * 1.8f) / averageFingerArea;
			}

			finger.directionDotAverage = finger.direction.DotProduct(averageFingerDirection);
			if (finger.directionDotAverage < leastDot)
			{
				nextLeastDot = leastDot;
				nextLeastDotFinger = leastDotFinger;
				leastDot = finger.directionDotAverage;
				leastDotFinger = &finger;
			}
			else if (finger.directionDotAverage < nextLeastDot)
			{
				nextLeastDot = finger.directionDotAverage;
				nextLeastDotFinger = &finger;
			}
		}
		// Check the least and next least. Is the difference big? If so then the least could be considered the thumb.
		if (nextLeastDotFinger && nextLeastDotFinger)
		{
			if (leastDot < nextLeastDot - thumbToOtherFingerThreshold)
			{
				leastDotFinger->thumbRatio += AbsoluteValue(leastDot - nextLeastDot);
			}
		}
	}
	// Thumb-detection when non-5 fingers. 
	else {
	}
	/// Make finger with highest thumbRatio thumb.
	CVFinger * maxThumb = NULL;
	for (int i = 0; i < fingers.Size(); ++i)
	{
		CVFinger * finger = &fingers[i];
		if (!maxThumb)
			maxThumb = finger;
		else if (finger->thumbRatio > maxThumb->thumbRatio)
			maxThumb = finger;
	}
	if (maxThumb && maxThumb->thumbRatio > 0.5f)
	{
		maxThumb->type = FingerType::THUMB;
		thumbDirection = maxThumb->direction;
		thumbLength = maxThumb->lengthToCenter;
		return 1;
	}
	return 0;
}
