// Emil Hedemalm
// 2013-03-19

#include "../Entity/Entity.h"
#include "Collissions.h"
#include "../PhysicsProperty.h"
#include "Graphics/GraphicsManager.h"

bool MeshSphereCollission(Entity * meshEntity, Entity * sphereEntity, Collission &data)
{
	assert(meshEntity->physics->physicsShape == ShapeType::MESH &&
		sphereEntity->physics->physicsShape == ShapeType::SPHERE);

//	std::cout<<"\nCollission check with "<<meshEntity->name;
	/// If Mesh-Mesh comparison, begin with a spherical or AABB check before we use the costly checks.
	Vector3f distanceVector = meshEntity->position - sphereEntity->position;
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
		
	/// Test for all, only evaluate the deepest collission! o.o'
	Collission deepestCollission;
	float deepestCollissionDistance = 0.0f;

	/// Proceed with checking every face of the mesh with the sphere o-o;
	Triangle tri;
	for (int i = 0; i < physicsMesh->triangles.Size(); ++i){
		tri = *physicsMesh->triangles[i];
		tri.Transform(meshEntity->transformationMatrix);
		/// If collide, return true
		if (TriangleSphereCollission(&tri, sphereEntity, data)){
			// Add additional data if need be
			if (abs(data.distanceIntoEachOther) > deepestCollissionDistance){
				deepestCollissionDistance = abs(data.distanceIntoEachOther);
				deepestCollission = data;
				if (Graphics.renderCollissionTriangles)
					deepestCollission.activeTriangles.Add(tri);
			}
		}
	}

	/// Proceed with checking every quad
	Quad quad;
	for (int i = 0; i < physicsMesh->quads.Size(); ++i){
		quad = *physicsMesh->quads[i];
		quad.Transform(meshEntity->transformationMatrix);
		/// If collide, return true
		if (QuadSphereCollission(&quad, sphereEntity, data)){
			// Add additional data if need be
			if (abs(data.distanceIntoEachOther) > deepestCollissionDistance){
				deepestCollissionDistance = abs(data.distanceIntoEachOther);
				deepestCollission = data;
			}
		}
	}
	if (deepestCollissionDistance > ZERO){
		data = deepestCollission;
		return true;
	}

	return false;
}
