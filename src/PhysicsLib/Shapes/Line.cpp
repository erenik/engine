/// Emil Hedemalm
/// 2014-08-11
/// Simple shapes: Line.

#include "Line.h"

Line::Line()
{
	Line(Vector3f(), Vector3f());
}

Line::Line(const Vector3f & start, const Vector3f & stop)
	: start(start), stop(stop)
{
	direction = stop - start;
	weight = 1;
}


// Returns ze area using showlace formulae http://en.wikipedia.org/wiki/Shoelace_formula
int Area(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3)
{
	return (p1[0] * p2[1] + p2[0] * p3[1] + p3[0] * p1[1] - p2[0] * p1[1] - p3[0] * p2[1] - p1[0] * p3[1]) * 0.5;
}

/// Returns true if the points are provided in clockwise order. http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool Clockwise(Vector2i p1, Vector2i p2, Vector2i p3)
{
	return (p1[0] * p2[1] + p2[0] * p3[1] + p3[0] * p1[1] - p2[0] * p1[1] - p3[0] * p2[1] - p1[0] * p3[1]) > 0;
}

/// Returns true if the points are provided in clockwise order. http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool CounterClockwise(Vector2i p1, Vector2i p2, Vector2i p3)
{
	return (p1[0] * p2[1] + p2[0] * p3[1] + p3[0] * p1[1] - p2[0] * p1[1] - p3[0] * p2[1] - p1[0] * p3[1]) < 0;
}

/// Yields an orientation.
enum orientations
{
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	COLINEAR,
};

int Orientation(Vector2i p1, Vector2i p2, Vector2i p3)
{
	int val = (p1[0] * p2[1] + p2[0] * p3[1] + p3[0] * p1[1] - p2[0] * p1[1] - p3[0] * p2[1] - p1[0] * p3[1]);
	if (val > 0)
		return CLOCKWISE;
	else if (val < 0)
		return COUNTER_CLOCKWISE;
	return COLINEAR;
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool PointOnSegment(const Vector3f & lineStart, const Vector3f & lineStop, const Vector3f & point)
{
	if (point[0] <= MaximumFloat(lineStart[0], lineStop[0]) && point[0] >= MaximumFloat(lineStart[0], lineStop[0]) &&
        point[1] <= MaximumFloat(lineStart[1], lineStop[1]) && point[1] >= MaximumFloat(lineStart[1], lineStop[1]))
       return true;
 
    return false;
}

/// Boolean intersection test.
int Line::Intersect(Line & withOtherLine)
{
	// http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/

	int or1 = Orientation(start, stop, withOtherLine.start);
	int or2 = Orientation(start, stop, withOtherLine.stop);
	int or3 = Orientation(withOtherLine.start, withOtherLine.stop, start);
	int or4 = Orientation(withOtherLine.start, withOtherLine.stop, stop);

	// Ok, we got an intersection confirmed, now the question is what type.
	if (or1 != or2 && or3 != or4)
		return IntersectionType::GENERAL;
	 
	// Special Cases for colinearity, may be either be end-points occuring on the other line or complete colinearity.
	bool colinear1 = false, colinear2 = false;
	if (or1 == COLINEAR && PointOnSegment(start, stop, withOtherLine.start) ||
		or2 == COLINEAR && PointOnSegment(start, stop, withOtherLine.stop)) 
	{	
		colinear1 = true;
	}
	if (or3 == COLINEAR && PointOnSegment(withOtherLine.start, withOtherLine.stop, start) ||
		or4 == COLINEAR && PointOnSegment(withOtherLine.start, withOtherLine.stop, stop))
	{
		colinear2 = true;
	}
	
	if (colinear1 && colinear2)
	{
		return IntersectionType::COLINEAR;
	}
	else if (colinear1 || colinear2)
		return IntersectionType::POINT_IN_SEGMENT;

	return IntersectionType::NO_INTERSECTION;
}

float Line::Length()
{
	return (stop - start).Length();
}

// Calculates distance to point.
float Line::Distance(const Vector3f & point)
{
	// Ref: http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	// Return minimum distance between line segment vw and point p
	// Length of this segment
	float lengthSquared = (stop - start).LengthSquared();
	// Start == stop case
	if (lengthSquared == 0.0) 
		return (point - start).Length();

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	const float t = (point - start).DotProduct(stop - start) / lengthSquared;
	// Beyond the 'start' end of the segment
	if (t < 0.0) 
		return (point - start).Length(); 
	// Beyond the 'stop' end of the segment
	else if (t > 1.0) 
		return (point - stop).Length();  

	// Projection falls on the segment
	const Vector3f projection = start + t * (stop - start);
	return (point - projection).Length();
}

/// Merges with other lines, taking weights into consideration for Y, but expanding X as necessary.
void Line::MergeYExpandX(Line & line)
{
	float totalWeight = weight + line.weight;
	float y1 = (start[1] * weight + line.start[1] * line.weight) / totalWeight;
	float y2 = (stop[1] * weight + line.stop[1] * line.weight) / totalWeight;
	start[1] = y1;
	stop[1] = y2;
	start[0] = start[0] < line.start[0] ? start[0] : line.start[0];
	stop[0] = stop[0] > line.stop[0] ? stop[0] : line.stop[0];
	weight = totalWeight;
}

/// Merges with other line, taking weights into consideration.
void Line::Merge(Line & line)
{
	float totalWeight = weight + line.weight;
	Vector3f newStart = (start * weight + line.start * line.weight) / totalWeight;
	Vector3f newStop = (stop * weight + line.stop * line.weight) / totalWeight;
	start = newStart;
	stop = newStop;
	weight = totalWeight;
}
