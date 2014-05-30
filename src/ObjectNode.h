
#ifndef OBJECTLEAF_H
#define OBJECTLEAF_H

/*
#include "Node.h"
#include "MathLib.h"
#include "Material.h"
#include "Texture.h"
#include "Frustum.h"

/// A base class for renderable Entity nodes. Providing pointers for vertices, UV-coordinates, texture ID and a material.
class ObjectNode : public Node {

public:
	/// Anulls the vertex pointer. 
	ObjectNode();
	/// Deallocates the vertices if they are non-null. 
	~ObjectNode();
	/// Renders the vertices of the Entity. 
	void Render();

	/// Have these public for quicker usage.
	Vertex3f * vertex;
	/// UV-coordinates for each vertex.
	float * u;
	/// UV-coordinates for each vertex.
	float * v;

	/// Texture to be used
	Texture * texture;

	/// Color-material properties
	Material * material;

	/// Position for the bounding sphere
	Vector3f position;
	/// Bounding radius of the Entity.
	float	radius;
	/// Specifies if it is currently in the frustum or not.
	bool inFrustum;
private:
	
};


*/

#endif