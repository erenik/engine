/// Emil Hedemalm
/// 2014-07-28
/// An editable face. Related to the EMesh and other E* classes within the Mesh directory.

#ifndef EDITABLE_FACE_H
#define EDITABLE_FACE_H

#include "List/List.h"

class EVertex;

class EFace 
{
public:
	EFace();
	EFace(EVertex * one, EVertex * two, EVertex * three);
	/// Adds target vertex to this face, establishing the binding both within the face and within the vertex.
	void AddVertex(EVertex* vertex);
	// If < 3 vertices, is ignored. Uses 3 first vertices to calculate normal
	void CalculateNormal();

	/// Vertices used by this face.
	List<EVertex*> vertices;
	// Irrelevant?
//	List<EFace*> neighbouringFaces;
	
};

#endif
