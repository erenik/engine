/// Emil Hedemalm
/// 2014-07-11
/** Implementatoin of the finger identification filter which uses output from both the convexity defects 
	and contour segmentation relative position/angle approaches.
*/

#include "CVHandFilters.h"
#include "CV/CVPipeline.h"
#include "Mathlib/AEMath.h"

CVFingerIdentificationFilter::CVFingerIdentificationFilter()
	: CVDataFilter(CVFilterID::FINGER_IDENTIFICATION)
{
	thumbToOtherFingersThreshold = new CVFilterSetting("Thumb-Finger diff threshold", 0.2f);
	settings.Add(thumbToOtherFingersThreshold);
}
int CVFingerIdentificationFilter::Process(CVPipeline * pipe)
{
	/// Time-based directions, derivated each time 5 fingers are easily distinguished, as well as the thumb clearly distinguishable from the rest.
	static Vector3f upVector(0,1,0);
	static Vector3f handDirVector(1,0,0);

	// Identify fingers in each hand.
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		CVHand & hand = pipe->hands[i];
		// No need to hinvestigate
		if (hand.fingers.Size() == 0)
			continue;
		List<CVFinger> & fingers = hand.fingers;
		// Mark the hand as bad, hopefully ignoring its results later on.
		if (hand.fingers.Size() > 5)
		{
			hand.bad = true;
		}
		// Just remove the hand if it was bad?
		if (hand.bad)
		{
			pipe->hands.RemoveIndex(i);
			--i;
			continue;
		}

		/// See if the fingers are distinguishable.
		int result = hand.IdentifyFingers(thumbToOtherFingersThreshold->GetFloat());

		static CVHand lastHand;

		/// Finger-matching function. Could probably be moved to.. CVFinger..?
		if (!result)
		{
			MatchFingersWithOldFingers(&hand, &lastHand);
		};

		// Copy current hand for next iteration.
		lastHand = hand;

	}



	returnType = CVReturnType::HANDS;
	return returnType;
}


struct CVFingerComparison
{
	CVFinger * newFinger,  * oldFinger;
	float similarity;
};

/// Attempts to determine the type of all fingers based on the fingers that were present in the last frame.
void CVFingerIdentificationFilter::MatchFingersWithOldFingers(CVHand * inHand, CVHand * comparedToOldHand)
{
	List<CVFinger> & fingers = inHand->fingers, 
		& fingersLastFrame = comparedToOldHand->fingers;

	List<CVFingerComparison> fingerComparisons;
	for (int i = 0; i < fingers.Size(); ++i)
	{
		CVFinger * finger = &fingers[i];
		// Compare with last detected values concerning direction and relative size.
		for (int j = 0; j < fingersLastFrame.Size(); ++j)
		{
			CVFinger * lastFinger = &fingersLastFrame[j];

			CVFingerComparison fingerComparison;
			fingerComparison.newFinger = finger;
			fingerComparison.oldFinger = lastFinger;
			
			float similarity = pow(finger->direction.DotProduct(lastFinger->direction), 8.f);
			similarity -= (finger->lengthToCenter - lastFinger->lengthToCenter) / comparedToOldHand->averageFingerLength; 
			
			fingerComparison.similarity = similarity;
			fingerComparisons.Add(fingerComparison);

		}
	}
	// Alright. Buncha pairs done. Sort them by similarity.
	for (int i = 0; i < fingerComparisons.Size(); ++i)
	{
		CVFingerComparison & fc1 = fingerComparisons[i];
		for (int j = i + 1; j < fingerComparisons.Size(); ++j)
		{
			CVFingerComparison & fc2 = fingerComparisons[j];
			// Sort by highest similarity.
			if (fc2.similarity > fc1.similarity)
			{
				/// Swap em.
				CVFingerComparison tmp = fc1;
				fc1 = fc2;
				fc2 = tmp;
			}
		}
	}
	// Sorted out, now transfer the relevant data for all fingers still lacking it in the comparisons.
	for (int i = 0; i < fingerComparisons.Size(); ++i)
	{
		CVFingerComparison & fc1 = fingerComparisons[i];
		if (fc1.similarity < 0.7f)
			continue;
		switch(fc1.newFinger->type)
		{
			case FingerType::FINGER:	
				fc1.newFinger->type = fc1.oldFinger->type;
				break;
		}
	}
}

