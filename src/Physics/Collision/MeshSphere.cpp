// Emil Hedemalm
// 2013-03-19

#include "../Entity/Entity.h"
#include "Collisions.h"
#include "../PhysicsProperty.h"
#include "Graphics/GraphicsManager.h"

#include "PhysicsLib/PhysicsMesh.h"
#include "PhysicsLib/Shapes/Quad.h"

bool MeshSphereCollision(Entity* meshEntity, Entity* sphereEntity, Collision &data)
{
	assert(meshEntity->physics->shapeType == ShapeType::MESH &&
		sphereEntity->physics->shapeType == ShapeType::SPHERE);

//	std::cout<<"\nCollision check with "<<meshEntity->name;
	/// If Mesh-Mesh comparison, begin with a spherical or AABB check before we use the costly checks.
	Vector3f distanceVector = meshEntity->worldPosition - sphereEntity->worldPosition;
	/// Real distance between them, please....
	float distance = distanceVector.Length();
	distance -= meshEntity->physics->physicalRadius + sphereEntity->physics->physicalRadius;
	distance = AbsoluteValue(distance);
	/// Fix for the above which seems to glitch at times.
	if (distance < ZERO)
		distance = ZERO;
	/// No collission if they're not into each other.
	if (distance < 0)
		return false;

	/// Determine which mesh we are to compare with
	PhysicsMesh * physicsMesh = meshEntity->physics->physicsMesh;
	assert(physicsMesh && "Entity doesn't have a valid physics mesh!");
	if (!physicsMesh)
		return false;
		
	return SphereFaceCollision(sphereEntity, physicsMesh->triangles, physicsMesh->quads, data, meshEntity->transformationMatrix.IsIdentity()? 0 : &meshEntity->transformationMatrix, physicsMesh->planeCollisionsOnly);
}
