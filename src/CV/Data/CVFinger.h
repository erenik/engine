/// Emil Hedemalm
/// 2014-07-11
/// Finger class. Used by CVHand.

#ifndef CV_FINGER_H
#define CV_FINGER_H

#include "CVContour.h"

struct CVContourSegmentPair
{
	CVContourSegment one, two;
	float distance;
};

namespace FingerType  {
	enum fingersTypes 
	{
		UNKNOWN,
		BAD, // Set if for example there exist no start or end segments.
		
		/// Thumb to pinky in a row here!
		THUMB,
		INDEX_FINGER,
		MIDDLE_FINGER,
		RING_FINGER,
		LITTLE_FINGER, // A.k.a. Pinky, small finger, etc.

		MIDDLE_OR_RING_FINGER,
		INDEX_OR_PINKY, // Since the hand may be rotated...
		FINGER, // May assume any finger. Hopefully most often the two middle fingers, although that might be determined later.
	};
};

class CVFinger 
{
public:
	CVFinger();
	/// Recalculates finger-point based on its contourSegments. Returns false if it could not be done.
	bool RecalculatePoint();
	/** Tries to assess the type of finger this mostly is. Will revert to UNKNOWN if it is uncertain.
		fingerRatio denotes the ratio between the length before and after a finger point in order to determine which fingers are the index/pinky.
	*/
	int DetermineType(float fingerRatio);

	/// Based on the contour segments.
	float ContourArea();
	/** Calculates the maximum curvatur found within the contour segments. 
		A value close to 1.0 means that there exist at least 1 180-degree turn, 
		while a value of 0 would mean the contour segments form a straight line.
	*/
	float MaximumCurvature();

	/** Returns a number indicating number of approximately parallel segments. 
		It uses an algorithm similar to the one used in MaximumCurvature, but has to evaluate it on all possible segment-pairs.
	*/
	int ParallelSegments(float minimumDotProductToBeConsideredParallel);

	/// Calculates and stores the width. Requires ParallelSegments to be called first to calculate all parallel segment pairs.
	float CalculateFingerWidth();

	float width;
	// Length from center of hand to tip.
	float lengthToCenter;
	List<CVContourSegmentPair> parallelPairs;
	

	
	/// Gets color suitable for rendering this finger.
	Vector4f GetColor() const;

	// Set after calling ContourArea once. Otherwise -1.
	float area;

	// Since the histogram based approach returns starts and stops, save them as well!
	Vector3f start;
	Vector3f stop;
	// End-point.
	Vector3f point;


	/// Normalized direction from the center to the hand to the finger's point.
	Vector3f direction;
	/// How aligned this direction(above) is with the general direction of all fingers on the hand.
	float directionDotAverage;
	/// How likely it is that this finger is a thumb.
	float thumbRatio;
	/// How likely it is that this finger is.... 


	/// If this in reality is a detection of two or more joint fingers, this will be non-1
	int numFingers;

	// See FingerTypes above.
	int type;

	// All contour-segments associated with this finger.
	List<CVContourSegment> climbSegments, plateauSegments, descentSegments;
	List<CVContourSegment> contourSegments;
};

#endif
