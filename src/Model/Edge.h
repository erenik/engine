/// Emil Hedemalm
/// 2013-10-01

#ifndef EDGE_H
#define EDGE_H

#include "Vertex.h"
#include "Face.h"
#include "MathLib.h"

class Edge 
{
public:
	Edge();
	Edge(Vertex * one, Vertex * two);
	Edge(const Vector3f & start, const Vector3f & stop);
	Vertex * start;
	Vertex * stop;
	/// Is not using any vertices...
	Vector3f startVec3f, stopVec3f;
	/// Planes that it belongs to?
	List<Face*> faceList;
};

#endif