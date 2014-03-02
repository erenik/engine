/// Emil Hedemalm
/// 2013-10-01

#ifndef FACE_H
#define FACE_H

struct Vertex;
struct Edge;
struct Triangle;

#include "MathLib.h"
#include "List/List.h"

struct Face {
	Face();
	/// Quad initializer.
	Face(Vertex *, Vertex *, Vertex *, Vertex *);
	~Face();
	void RecalculateNormal();
	/// Recalculates triangles, creating them if they did not previously exist.
	void RecalculateTriangles();
	/// Computes average position of this face (average of vertices)
	void ComputePosition();

	Vector3f normal;
	// Average position
	Vector3f position;
	List<Vertex*> vertexList;
	List<Edge*> edgeList;
	List<Triangle*> triangleList;
};

#endif