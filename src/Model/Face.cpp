/// Emil Hedemalm
/// 2013-10-01

#include "Vertex.h"
#include "Edge.h"
#include "PhysicsLib.h"

Face::Face(){

}

/// Quad initializer.
Face::Face(Vertex * v0, Vertex * v1, Vertex * v2, Vertex * v3){
	vertexList.Add(v0);
	vertexList.Add(v1);
	vertexList.Add(v2);
	vertexList.Add(v3);
}

Face::~Face(){
	/// The triangles are the only dynamically allocated part that needs removal upon destruction.
	triangleList.ClearAndDelete();
}

void Face::RecalculateNormal(){
	assert(vertexList.Size() >= 3);
	if (vertexList.Size() < 3)
		return;
	Vector3f v0,v1,v2;
	v0 = vertexList[0]->position;
	v1 = vertexList[1]->position;
	v2 = vertexList[2]->position;
	normal = (v2 - v0).CrossProduct(v1 - v0);
	normal.Normalize();
}

/// Recalculates triangles, creating them if they did not previously exist.
void Face::RecalculateTriangles(){
	if (vertexList.Size() < 3){
		if (triangleList.Size())
			triangleList.ClearAndDelete();
		return;
	}
	if (triangleList.Size() != vertexList.Size() - 2){
		triangleList.ClearAndDelete();
		for (int i = 0; i < vertexList.Size() - 2; ++i){
			Triangle * t = new Triangle();
			triangleList.Add(t);
		}
	}
	/// Update the triangles.
	for (int i = 0; i < vertexList.Size() - 2; ++i){
		Triangle * t = triangleList[i];
#define VERT(p) vertexList[p]->position
		t->Set3Points(VERT(i), VERT(i+1), VERT(i+2));
	}

}

/// Computes average position of this face (average of vertices)
void Face::ComputePosition(){
	Vector3f newVec;
	for (int i = 0; i < vertexList.Size(); ++i){
		newVec += vertexList[i]->position;
	}
	newVec /= vertexList.Size();
	position = newVec;
}