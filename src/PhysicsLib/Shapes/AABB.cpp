// Emil Hedemalm
// 2013-09-04
// A class for optimizing collission-detection in physics.

#include "AABB.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"
#include "Physics/PhysicsManager.h"

/// Fucking macros (on windows)
#undef max
#undef min

AABB::AABB()
{
}

AABB::AABB(const Vector3f & min, const Vector3f & max)
: min(min), max(max)
{
    scale = max - min;
    position = (max + min) * 0.5f;
}

bool AABB::Intersect(const AABB &aabb2) const {
    if (min[0] > aabb2.max[0] ||
        min[1] > aabb2.max[1] ||
        min[2] > aabb2.max[2] ||
        max[0] < aabb2.min[0] ||
        max[1] < aabb2.min[1] ||
        max[2] < aabb2.min[2]
        )
        return false;
    return true;
}

bool debugAABB = false;

/// Recalculate the AABBs constraints based on the given entity using it.
void AABB::Recalculate(Entity * entity)
{
	if (!entity->model->mesh->aabb)
		return;
	
	/// Check physics-type.
	PhysicsProperty * pp = entity->physics;
	/// Default: mesh?
	int physicsShape = PhysicsShape::MESH;
	if (pp)
		physicsShape = pp->shapeType;
	switch(physicsShape)
	{
		case PhysicsShape::SPHERE:
		{
			Vector3f radiusVec = Vector3f(1,1,1) * entity->radius;
			this->position = entity->position;
			this->min = position - radiusVec;
			this->max = position + radiusVec;
			this->scale = radiusVec * 2;
			return;
		}
		case PhysicsShape::MESH:
			// Default use below method.
			break;
		default:
			assert(false && "Implement");
	}


	/// Recalculate from scratch if re-scaled or rotated?    
	lastScale = entity->scale;
	lastRot = entity->rotation;
	Vector3f bounds[8];
	/// Reset min and max depending on the base AABB.
	min = entity->model->mesh->aabb->min;
	max = entity->model->mesh->aabb->max;
	Vector3f newMin, newMax;
	/// 8 vectors (extents), for-looped for funs.
	for (int i = 0; i < 8; ++i)
	{
		if (i < 4)
			bounds[i][0] = min[0];
		else
			bounds[i][0] = max[0];
		if (i%4 < 2)
			bounds[i][1] = min[1];
		else
			bounds[i][1] = max[1];
		if (i%2 == 0)
			bounds[i][2] = min[2];
		else
			bounds[i][2] = max[2];

		if (debugAABB)
			std::cout<<"\nBound "<<i<<": "<<bounds[i];

		/// Transform according to entity
		bounds[i] = entity->transformationMatrix.Product(bounds[i]);

		if (debugAABB)
			std::cout<<"\nTransformed "<<i<<": "<<bounds[i];

		if (i == 0)
			newMin = newMax = bounds[i];
		else {
			// Get min
			if (bounds[i][0] < newMin[0])
				newMin[0] = bounds[i][0];
			if (bounds[i][1] < newMin[1])
				newMin[1] = bounds[i][1];
			if (bounds[i][2] < newMin[2])
				newMin[2] = bounds[i][2];
			// Get max
			if (bounds[i][0] > newMax[0])
				newMax[0] = bounds[i][0];
			if (bounds[i][1] > newMax[1])
				newMax[1] = bounds[i][1];
			if (bounds[i][2] > newMax[2])
				newMax[2] = bounds[i][2];
		}
	}

	/// Update depending on the new stats!
	min = newMin;
	max = newMax;

	if (debugAABB)
		std::cout<<"\nMin "<<min<<" Max "<<max;

	/// Update scale and such.
	position = (max + min) * 0.5f;
	scale = max - min;
	if (debugAABB)
		std::cout<<"\nPosition "<<position<<" Scale "<<scale;
}
