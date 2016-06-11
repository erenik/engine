/// Emil Hedemalm
/// 2014-07-28
/// An editable face. Related to the EMesh and other E* classes within the Mesh directory.

#include "EFace.h"
#include "EVertex.h"
#include "ENormal.h"

EFace::EFace()
{
	Nullify();
}
EFace::EFace(EVertex * one, EVertex * two, EVertex * three)
{
	Nullify();
	AddVertex(one);
	AddVertex(two);
	AddVertex(three);
	CalculateNormal();
}
void EFace::Nullify()
{
	normal = 0;
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

/// Calculates normal (CCW/counter-clockwise-up normal).
void EFace::CalculateNormal()
{
	// Make vectors.
	Vector3f vec1 = *vertices[1] - *vertices[0],
		vec2 = *vertices[2] - *vertices[0];
	vec1.Normalize();
	vec2.Normalize();
	// Cross product.
	normal = new ENormal();
	*normal = vec1.CrossProduct(vec2).NormalizedCopy();
}

