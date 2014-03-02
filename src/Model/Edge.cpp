/// Emil Hedemalm
/// 2013-10-01

#include "Edge.h"

Edge::Edge()
{
}

Edge::Edge(Vertex * one, Vertex * two){
	start = one;
	stop = two;
}

Edge::Edge(Vector3f start, Vector3f stop)
: startVec3f(start), stopVec3f(stop)
{
}
