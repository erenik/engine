/// Emil Hedemalm
/// 2014-08-06
/// Simple shapes

#ifndef LINE_H
#define LINE_H

/// Used for line-segments.
class Line 
{
public:
	Line();
	Line(Vector3f start, Vector3f stop);
	float Length();
	// Calculates distance to point.
	float Distance(Vector3f point);
	/// Merges with other lines, taking weights into consideration for Y, but expanding X as necessary.
	void MergeYExpandX(Line & line);
	/// Merges with other line, taking weights into consideration.
	void Merge(Line & line);
	// Calculates the minimum distance to the other line, assuming they are
//	float Distance(Line & otherLine);

	// General parameters
	Vector3f start;
	Vector3f stop;
	Vector3f direction;

	// Merge operations require more thought, and abstraction into 2D or 3D.
//	Vector3f startMin, startMax, stopMin, stopMax;

	// Half-width or "radius" of the line.
//	float radius;

	// Weight used in merging operations.
	int weight;
};

#endif
