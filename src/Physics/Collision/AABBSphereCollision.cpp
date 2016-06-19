// Emil Hedemalm
// 2015-02-18

#include "../Entity/Entity.h"
#include "Collisions.h"
#include "../PhysicsProperty.h"
#include "Graphics/GraphicsManager.h"

#include "PhysicsLib/PhysicsMesh.h"
#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/Quad.h"

bool AABBSphereCollision(AABB * aabb, Entity * sphereEntity, Collision &data, bool planesOnly)
{
	assert(sphereEntity->physics->shapeType == ShapeType::SPHERE);

	// Assume broad-phase has been done earlier.

	/// Test for all, only evaluate the deepest collission! o.o'
	Collision deepestCollision;
	float deepestCollisionDistance = 100000.0f;

	/// First opt-out?
// #ifndef USE_SSE
//	if (!aabb->SphereInside(sphereEntity->position, sphereEntity->radius))
//		return false;
//#endif

	/// Build the quads of the aabb.
	List<Quad> quads = aabb->AsQuads();

	bool colliding = false;
	/// Proceed with checking every quad
	for (int i = 0; i < quads.Size(); ++i)
	{
		Quad & quad = quads[i];
		/// If collide, return true
		if (QuadSphereCollision(&quad, sphereEntity, data, planesOnly))
		{
			// Add additional data if need be
			if (abs(data.distanceIntoEachOther) < deepestCollisionDistance)
			{
				deepestCollisionDistance = abs(data.distanceIntoEachOther);
				deepestCollision = data;
				colliding = true;
			}
		}
	}
	if (colliding){
		data = deepestCollision;
		return true;
	}
	return false;
}

