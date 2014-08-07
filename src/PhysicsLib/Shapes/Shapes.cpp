#include "Shapes.h"
#include <cassert>

#include "Ngon.h"
#include "Line.h"
#include "Sphere.h"

Ngon::Ngon(){
	assert(false);
}

/// Sphere Initializer
Sphere::Sphere(float radius, Vector3f position /* = Vector3f()*/ )
: radius(radius), position(position), sections(DEFAULT_SECTIONS)
{
}






Line::Line()
{
	Line(Vector3f(), Vector3f());
}

Line::Line(Vector3f start, Vector3f stop)
	: start(start), stop(stop)
{
	direction = stop - start;
	weight = 1;
}


// Returns ze area using showlace formulae http://en.wikipedia.org/wiki/Shoelace_formula
int Area(Vector2i p1, Vector2i p2, Vector2i p3)
{
	return (p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p2.x * p1.y - p3.x * p2.y - p1.x * p3.y) * 0.5;
}

/// Returns true if the points are provided in clockwise order. http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool Clockwise(Vector2i p1, Vector2i p2, Vector2i p3)
{
	return (p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p2.x * p1.y - p3.x * p2.y - p1.x * p3.y) > 0;
}

/// Returns true if the points are provided in clockwise order. http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
bool CounterClockwise(Vector2i p1, Vector2i p2, Vector2i p3)
{
	return (p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p2.x * p1.y - p3.x * p2.y - p1.x * p3.y) < 0;
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
	int val = (p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p2.x * p1.y - p3.x * p2.y - p1.x * p3.y);
	if (val > 0)
		return CLOCKWISE;
	else if (val < 0)
		return COUNTER_CLOCKWISE;
	return COLINEAR;
}

/// Boolean intersection test.
int Line::Intersect(Line & withOtherLine)
{
	// http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
	int or1 = Orientation(start, stop, withOtherLine.start);
	int or2 = Orientation(start, stop, withOtherLine.stop);
	if (or1 == or2)
		return false;
	int or3 = Orientation(withOtherLine.start, withOtherLine.stop, start);
	int or4 = Orientation(withOtherLine.start, withOtherLine.stop, stop);
	if (or3 != or4)
		return false;

	// Ok, we got an intersection confirmed, now the question is what type.
	return 1;
}

float Line::Length()
{
	return (stop - start).Length();
}

// Calculates distance to point.
float Line::Distance(Vector3f point)
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
	float y1 = (start.y * weight + line.start.y * line.weight) / totalWeight;
	float y2 = (stop.y * weight + line.stop.y * line.weight) / totalWeight;
	start.y = y1;
	stop.y = y2;
	start.x = start.x < line.start.x ? start.x : line.start.x;
	stop.x = stop.x > line.stop.x ? stop.x : line.stop.x;
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
