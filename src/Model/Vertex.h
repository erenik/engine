/// Emil Hedemalm
/// 2013-10-01

#ifndef VERTEX_H
#define VERTEX_H

#include "Edge.h"
#include "Face.h"
#include "MathLib.h"
#include "List/List.h"

struct Vertex {
	Vertex(Vector3f initPos);

	Vector3f position;
	List<Edge*> edgeList;
	List<Face*> faceList;
};

#endif