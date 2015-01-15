/// Emil Hedemalm
/// 2014-06-27
/// Computer Vision Contour class.

#ifndef CV_CONTOUR_H
#define CV_CONTOUR_H

#include "PhysicsLib/Shapes/Contour.h"
#include "CV/OpenCV.h"

#undef max

// Classifications based on contour complexity and bounding ellipse size/form/shape/angle.
namespace ContourClass { 
	enum {
		NONE,
		ENTIRE_HAND,
		FINGER_AND_HAND_CONTOUR,
		FINGER,
		THUMB,
		FINGERTIP,
	};
};

namespace BoundingType {
	enum {
		NONE,
		RECT,
		ELLIPSE,
	};
};

namespace SegmentType {
	enum {
		NONE,
		EDGE,
		CLIMB, EXTENDED_FINGER_START = CLIMB, 
		PLATEAU, EXTENDED_FINGER = PLATEAU,
		DESCENT, EXTENDED_FINGER_STOP = DESCENT,
	};
};

/// A segment of the contour.
class CVContourSegment 
{
public:
	CVContourSegment();
	// In indices of the points in the original contour.
	int startIndex, stopIndex;

	/// In raw-image co-ordinates 
	Vector3f rawInputStart, rawInputStop;
	float rawInputLength;

	/// Normalized direction in the raw image...?
	Vector3f rawInputDirection;

	/// In relative angle/distance compared to center of the contour
	Vector3f angleDistanceStart, angleDistanceStop;
	Vector3f angleDistanceNormalizedDirection;
	float angleDistanceLength;

	/** Energy, based on relative Y and also the relative Ydiff throughout the segment. 
		Higher energy should coincide with finger features while lower energy should coincide with the rest of the hand.
	*/
	float energy;

	/// For identification and rendering, as well as categorization. Use as wanted. enum provided above for suggestions.
	int type;
	
	/// Likelyhood that this segment is any of these types. Limits not determined yet, but highest is most likely.
	int climb;
	int plateau;
	int descent;


	/// If true, lies along the edge of the input image, potentially making it irrelevant for later analysis.
	bool edge;
	/// If true, was used as starting point for the hand-parser and finger extraction algorithm.. or whichever algoirthm you are debugging!
	bool start;
};

/// Extension to the base contour class, focusing on our image analysis tasks.
class CVContour : public Contour 
{
public:
	CVContour();


	/// Checks if the point is located inside this contour, using the contour segments instead of every single point. 
	bool PointInsideSegments(Vector3f point, Vector3f comparisonDir = Vector3f(0,1,0));

	/// Extracts segments based on existing data. Returns amount of generated segments.
	int ExtractSegments(float minSegmentLength = 10.f);
	/** Detects which segments are edge-segments, based on given min and max coordinates of the whole input image the contour is based on.
		Calculates the edgeLength at the same time, which is the sum of all edge segments' lengths.
	*/
	void FindEdges(Vector2i min, Vector2i max);
	void ApproximateInnerCircle();
	// Give user the choice of chosing the center, as this may be either center of mass, of the maximum inner circle, etc.
	void CalculateRelativeAngles(Vector2f usingCenter);
	/// Relative contour-point distance when compared to the largest inner circle radius and center.
	void CalculateRelativeDistances(Vector2f usingInnerCircleCenter, float andRadius);
	// Fills the angleDistancePositions vector.
	void GatherRelativeIntoJointVector();


	// Calculate segment energy based on the newly calculated relative angles. Useful to know where to start search-algorithms later on.
	void CalculateContourSegmentEnergies();

	/// Calculating segment energy should be done externally, as this will vary, most likely on the scales of all other stuff.

	/// cv data form.
	std::vector<cv::Point> cvPointList;
	cv::RotatedRect boundingEllipse;
	cv::RotatedRect boundingRect;
	// If ellipse or rect is used.
	int boundingType;
	int contourClass;

	// Total length of all edge-segments.
	float edgeLength;

	/// Approximated inner circle.
	Vector2f largestInnerCircleCenter;
	/// Yup.
	float largestInnerCircleRadius;

	/// Relative contour-point distance when compared to the largest inner circle radius and center.
	CircularList<float> relativesDistances;
	/// Angle when comparing the contour position with the center. Should vary from 0 to 1.0 (normalized from 0 to 360 degrees)
	CircularList<float> relativeAngles;
	// List of relative angle/Distance pairs, scaled to render onto texture.
	CircularList<Vector2f> angleDistancePositions;
	

	/// Colors for each contour-point. Used for painting the output. May be omitted.
	List<Vector3f> contourPointColors;

	// Segmentation of the contour based on the above distance/angle/angularPosition data.
	CircularList<CVContourSegment> segments;
	int leastEnergySegmentIndex;

};


#endif
