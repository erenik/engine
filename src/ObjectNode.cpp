/*

#include "ObjectNode.h"

ObjectNode::ObjectNode() : Node() {
	material = new Material();
	vertex = NULL;

	u = NULL;
	v = NULL;
	
	texture = 0;

	/// Position for bounding sphere
	position = Vector3f();
	radius = 2.0f;

	inFrustum = Frustum::INSIDE;
}

ObjectNode::~ObjectNode(){
	if (vertex != NULL)
		delete[] vertex;
	if (material)
		delete material;
}
*/