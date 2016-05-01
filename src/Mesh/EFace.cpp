/// Emil Hedemalm
/// 2014-07-28
/// An editable face. Related to the EMesh and other E* classes within the Mesh directory.

#include "EFace.h"
#include "EVertex.h"

EFace::EFace(){}
EFace::EFace(EVertex * one, EVertex * two, EVertex * three)
{
	AddVertex(one);
	AddVertex(two);
	AddVertex(three);
}


/// Adds target vertex to this face, establishing the binding both within the face and within the vertex.
void EFace::AddVertex(EVertex* vertex)
{
	// Don't re-add it... or?
	if (vertices.Exists(vertex))
		return;
	vertices.Add(vertex);
	if (!vertex->faces.Exists(this))
		vertex->faces.Add(this);
}

// If < 3 vertices, is ignored. Uses 3 first vertices to calculate normal
void EFace::CalculateNormal()
{
	if (vertices.Size() < 3)
		return;
	
}

