/// Emil Hedemalm
/// 2014-07-04
/// Computer Vision Contour class.

#include "CVContour.h"
#include "PhysicsLib/Shapes/Line.h"

CVContourSegment::CVContourSegment()
{
	climb = plateau = descent = 0;
	edge = false;
	energy = 0;
	type = NULL;
	startIndex = stopIndex = 0;
	start = false;
}

CVContour::CVContour()
{
	contourClass = ContourClass::NONE;
	boundingType = BoundingType::NONE;
	edgeLength = 0;
}

/// Checks if the point is located inside this contour, using the contour segments instead of every single point. 
bool CVContour::PointInsideSegments(Vector3f point, Vector3f comparisonDir /*= Vector3f(0,1,0)*/)
{
	// Span up a line from the point and far away in some direction.
	Line pointLine(point, point + comparisonDir * 10000.f);
	int intersections = 0;
	Vector3f toSegment;
	// Segmeeeeents!
	for (int i = 0; i < segments.Size(); ++i)
	{
		CVContourSegment & segment = segments[i];

		// Assume the segment is small enough that we can cull it if it lies in the opposite direction of the comparison dir.
		toSegment = segment.rawInputStart - point;
		// Should be no intersection then.
		if (toSegment.DotProduct(comparisonDir) < 0)
			continue;

		// Span up a line between this and the next point
		Line line(segment.rawInputStart, segment.rawInputStop);
		bool intersect = pointLine.Intersect(line);
		intersections += intersect;
	}
	if (intersections % 2 == 1)
		return true;
	return false;
}


/// Extracts segments based on existing data.
int CVContour::ExtractSegments(float minSegmentLength)
{
	segments.Clear();
	if (points.Size() == 0)
		return 0;
	float segmentLength = 0.0f;
	Vector3f segmentStart = points[0];
	int startIndex = 0;
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector3f point = points[i];
		Vector3f next = points[i+1 % points.Size()];
		Vector3f toNext = next - point;
		float distance = toNext.Length();
		toNext.Normalize();
			
		segmentLength += distance;

		// Assume that we are working in 2D for all points.
		assert(toNext.z == 0);

		if (segmentLength > minSegmentLength)
		{
			// Calculate segment length now..
			segmentLength = (next - segmentStart).Length();

			// Add as a segment.
			CVContourSegment seg;
			seg.rawInputStart = segmentStart;
			seg.rawInputStop = next;
			seg.rawInputLength = segmentLength;
			seg.energy = 0;


			seg.startIndex = startIndex;
			seg.stopIndex = i;

			// Reset for next segment
			segmentStart = next;
			segmentLength = 0;
			startIndex = i;
			segments.Add(seg);
		}


		// Wrap around the last one to the first segment.. merge'em?
		else if (i == points.Size() - 1)
		{
			CVContourSegment & last = segments[0];
			last.rawInputStart = segmentStart;
			last.rawInputLength += segmentLength;
		}
	}
	return segments.Size();
}

// Min and max
void CVContour::FindEdges(Vector2i min, Vector2i max)
{
	edgeLength = 0;
	// Mark segments along the edges appropriately.
	for (int j = 0; j < segments.Size(); ++j)
	{
		CVContourSegment & seg = segments[j];
		Vector3f start = seg.rawInputStart;
		if (start.x <= min.x || start.x >= max.x ||
			start.y  <= min.y || start.y >= max.y)
		{
			seg.edge = true;
			edgeLength += seg.rawInputLength;
		}
	}
}	

void CVContour::ApproximateInnerCircle()
{
	/// Approximate hand center
	for (int i = 0; i < points.Size(); ++i)
	{
		averageContourVertexPosition += points[i];
	}
	if (!points.Size())
	{
		assert(false);
		return;
	}
		
	averageContourVertexPosition *= 1.f / points.Size();
	// Total gotten.
	largestInnerCircleCenter = centerOfMass + centerOfMass - averageContourVertexPosition;
	// Done!
	float leastLengthSquared = (points[0] - largestInnerCircleCenter).LengthSquared();
	float l ;
	for (int i = 1; i < points.Size(); ++i)
	{
		l = (points[i] - largestInnerCircleCenter).LengthSquared();
		if (l < leastLengthSquared)
			leastLengthSquared = l;
	}
	largestInnerCircleRadius = sqrt(leastLengthSquared);
	
}



		// Fetch maximal inscribed circle of the contour?
		/*
		// Fetch cross product of centerToPoint and an up(in)-vector to get the vector we assume the contour should be following (assuming a circle)
		Vector3f centerToPoint = point - center;
		centerToPoint.Normalize();
		
		Vector3f outVec(0,0,1);
		Vector3f shouldDir = centerToPoint.CrossProduct(outVec).NormalizedCopy();
		
		float relX = shouldDir.DotProduct(toNext);
		float relY = centerToPoint.DotProduct(toNext);
		// Now we have x and y, calculate angle.
		float radians;
		if (relY != 0)
			radians = atan2(relY, relX);
		else 
		{
			if (relX)
				radians = 0;
			else
				radians = PI / 2.f;
		}
			
		float degrees = radians * 57.2957795f;
		*/
//		relativeAngles.Add(degrees);


// Give user the choice of chosing the center, as this may be either center of mass, of the maximum inner circle, etc.
void CVContour::CalculateRelativeAngles(Vector2f usingCenter)
{	
	relativeAngles.Clear();
	// Calculate the relative angles.
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector3f centerToPoint = points[i] - usingCenter;
		float angularPosition = atan2(centerToPoint.y, centerToPoint.x);
		angularPosition = angularPosition / (2 * PI) + 0.5f;
		relativeAngles.Add(angularPosition);
	}
}


/// Relative contour-point distance when compared to the largest inner circle radius and center.
void CVContour::CalculateRelativeDistances(Vector2f usingInnerCircleCenter, float andRadius)
{	
	relativesDistances.Clear();
	// Calculate relative distances.
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector3f centerToPoint = points[i] - usingInnerCircleCenter;
		float relativeDistance = centerToPoint.Length() / andRadius;
		relativesDistances.Add(relativeDistance);
	}
}
	
// Fills the angleDistancePositions vector.
void CVContour::GatherRelativeIntoJointVector()
{
	angleDistancePositions.Clear();
	assert(relativeAngles.Size() == relativesDistances.Size());
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector2f vec(relativeAngles[i], relativesDistances[i]);
		angleDistancePositions.Add(vec);
	}
	assert(points.Size() == relativeAngles.Size() && 
		relativesDistances.Size() == angleDistancePositions.Size()
		);
}
	
// Calculate segment energy based on the newly calculated relative angles. Useful to know where to start search-algorithms later on.
void CVContour::CalculateContourSegmentEnergies()
{
	assert(angleDistancePositions.Size() == points.Size());
		// Find the segments of 
	int leastEnergyIndex = -1;
	float leastEnergy = 1000000000.f;
	for (int i = 0; i < segments.Size(); ++i)
	{
		/// Assign  each segment its according angle distance coordinates.
		CVContourSegment & seg = segments[i];
		seg.angleDistanceStart = angleDistancePositions[seg.startIndex];
		seg.angleDistanceStop = angleDistancePositions[seg.stopIndex];

		// Average height.
		seg.energy = (seg.angleDistanceStart.y + seg.angleDistanceStop.y) * 0.5f;
		// And the difference.
		seg.energy += AbsoluteValue(seg.angleDistanceStart.y - seg.angleDistanceStop.y);

		if (seg.edge == true)
			seg.energy = 0;

		if (seg.energy > leastEnergy)
		{
			leastEnergy = seg.energy;
			leastEnergyIndex = i;
		}
	}
	leastEnergySegmentIndex = leastEnergyIndex;
}


