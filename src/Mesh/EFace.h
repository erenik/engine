/// Emil Hedemalm
/// 2014-07-28
/// An editable face. Related to the EMesh and other E* classes within the Mesh directory.

#ifndef EDITABLE_FACE_H
#define EDITABLE_FACE_H

#include "List/List.h"

class EVertex;
class ENormal;
class EUV;

class EFace 
{
public:
	EFace();
	/// Sets vertices and calculates normal automatically (CCW/counter-clockwise-up normal).
	EFace(EVertex * one, EVertex * two, EVertex * three);
	EFace(EVertex * one, EVertex * two, EVertex * three, EVertex * four);
	void Nullify();
	/// Adds target vertex to this face, establishing the binding both within the face and within the vertex.
	void AddVertex(EVertex* vertex);
	/// Calculates normal (CCW/counter-clockwise-up normal).
	void CalculateNormal();
	/// u0 v0 corresponds to one, and u1 v1 to vertex four?
	void SetUVCoords(float u0, float v0, float u1, float v1);

	/// Vertices used by this face.
	List<EVertex*> vertices;
	List<EUV*> uvs;
	// Irrelevant?
//	List<EFace*> neighbouringFaces;
	/// If this face is associated with 1 shared normal for all vertices (no smoothing).
	ENormal * normal; 
};

#endif
