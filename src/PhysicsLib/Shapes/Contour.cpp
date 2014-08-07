/// Emil Hedemalm
/// 2014-06-26
/// An arbitrary contour. 

#include "Contour.h"
#include "Line.h"

bool Contour::PointInside(Vector3f point, Vector3f comparisonDir)
{
	// Span up a line from the point and far away in some direction.
	Line pointLine(point, point + comparisonDir * 10000.f);
	int intersections = 0;
	for (int i = 0; i < points.Size(); ++i)
	{
		// Span up a line between this and the next point
		Line line(points[i], points[(i + 1) % points.Size()]);
		bool intersect = pointLine.Intersect(line);
		intersections += intersect;
	}
	if (intersections % 2 == 1)
		return true;
	return false;
}
