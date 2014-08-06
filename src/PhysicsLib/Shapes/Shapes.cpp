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
