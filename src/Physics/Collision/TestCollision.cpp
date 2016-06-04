/// Emil Hedemalm
/// 2013-03-01

#include "../Entity/Entity.h"
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "Collisions.h"
#include "PhysicsLib/PhysicsMesh.h"
//#include "Physics/Collision/CollisionShapeOctree.h"
#include <cstring>
#include "CollisionShapeOctree.h"

/** Tests if a collission should occur between the two objects and
	either resolves it straight away or queues the collission to be solved at a later time
	depending on current settings.
*/
bool TestCollision(Entity * one, Entity * two, List<Collision> & collissionList){

	Collision data;

	/// Skip if for some reason they are equal to each other (implementation dependant if this check is made)
	if (one == two)
		return false;

	/// Skip if both objects are in rest next to each other ^.^
	if (one->physics->state & CollisionState::IN_REST &&
		two->physics->state & CollisionState::IN_REST)
		return false;

	// Save away one and two straight away!
	data.one = one;
	data.two = two;

	/// First check what shapes we have to compare.
	int shapeTypes[ShapeType::NUM_TYPES];
	int size = sizeof(shapeTypes);
	int types = ShapeType::NUM_TYPES;
	memset(shapeTypes, 0, sizeof(shapeTypes));
	++shapeTypes[one->physics->shapeType];
	++shapeTypes[two->physics->shapeType];

	bool shouldCollide = false;
	///=================================================================================//
	///  Sphere-Sphere Intersection
	///=================================================================================//
	if(shapeTypes[ShapeType::SPHERE] == 2)
	{
		if (!SpheresColliding(one->worldPosition, two->worldPosition, one->physics->physicalRadius + two->physics->physicalRadius))
			return false;
		if(SphereSphereCollision(one, two, data)){
			shouldCollide = true;
		}
	}
	///=================================================================================//
	///  Plane-Sphere Intersection
	///=================================================================================//
	else if (shapeTypes[ShapeType::PLANE] == 1 && shapeTypes[ShapeType::SPHERE]){
		Entity * planeEntity = NULL,
			* sphereEntity = NULL;
		if (one->physics->shapeType == ShapeType::PLANE){
			planeEntity = one;
			sphereEntity = two;
		}
		else {
			planeEntity = two;
			sphereEntity = one;
		}
		if (PlaneSphereCollision(planeEntity, sphereEntity, data))
			shouldCollide = true;
	}
	else if (shapeTypes[ShapeType::AABB] && shapeTypes[ShapeType::AABB])
	{
		Entity * aabbEntity = NULL,
			* sphereEntity = NULL;
		if (one->physics->shapeType == ShapeType::AABB)
		{
			aabbEntity = one;
			sphereEntity = two;
		}
		else
		{
			aabbEntity = two;
			sphereEntity = one;
		}
		if (AABBSphereCollision(aabbEntity->aabb, sphereEntity, data))
		{
			shouldCollide = true;
			/// Flip collision normal if needed, always point from one to two.
			if (aabbEntity == one)
			{
				data.collisionNormal *= -1;
				data.distanceIntoEachOther *= -1;
			}
		}
	}
	///=================================================================================//
	///  Mesh-Sphere Intersection
	///=================================================================================//
	else if (shapeTypes[ShapeType::MESH] == 1 && shapeTypes[ShapeType::SPHERE])
	{

		Entity * meshEntity = NULL,
			* sphereEntity = NULL;
		if (one->physics->shapeType == ShapeType::MESH){
			meshEntity = one;
			sphereEntity = two;
		}
		else {
			meshEntity = two;
			sphereEntity = one;
		}
		// Perform a quick opting sphere-sphere out before evaluating further!
		if (!SpheresColliding(one->worldPosition, two->worldPosition, one->physics->physicalRadius + two->physics->physicalRadius)){
			return false;
		}

#define USE_COLLISSION_SHAPE_OCTREE true

		// If it's got an optimized octree, use it instead, yo.
		if (meshEntity->physics->usesCollisionShapeOctree && USE_COLLISSION_SHAPE_OCTREE && false)
		{
			PhysicsMesh * physicsMesh = meshEntity->physics->physicsMesh;
			List<Collision> physicsMeshCollisions;
			int tests = physicsMesh->collisionShapeOctree->FindCollisions(sphereEntity, physicsMeshCollisions, meshEntity->transformationMatrix);
			PhysicsMan.physicsMeshCollisionChecks += tests;
			for (int i = 0; i < physicsMeshCollisions.Size(); ++i){
				Collision col = physicsMeshCollisions[i];
				col.one = one;
				col.two = two;
				shouldCollide = true;
				collissionList.Add(col);
			}
		}
		// All tris vs. sphere.
		else if (MeshSphereCollision(meshEntity, sphereEntity, data)){
			shouldCollide = true;
			/// Swap them to get normal right.
			/// Flip collision normal if needed, always point from one to two.
			if (meshEntity == one)
			{
				data.collisionNormal *= -1;
				data.distanceIntoEachOther *= -1;
			}
		}
	}
	else if (shapeTypes[ShapeType::TRIANGLE] && shapeTypes[ShapeType::SPHERE]){
		Entity * tri = NULL,
			* sphere= NULL;
		if (one->physics->shapeType == ShapeType::TRIANGLE){
			tri = one;
			sphere = two;
		}
		else {
			tri = two;
			sphere = one;
		}
		if (TriangleSphereCollision(tri, sphere, data))
			shouldCollide = true;
	}
	else if (shapeTypes[ShapeType::QUAD] && shapeTypes[ShapeType::SPHERE]){
		Entity * quad = NULL,
			* sphere= NULL;
		if (one->physics->shapeType == ShapeType::TRIANGLE){
			quad = one;
			sphere = two;
		}
		else {
			quad = two;
			sphere = one;
		}
		if (QuadSphereCollision(quad, sphere, data))
			shouldCollide = true;
	}
	else if (shapeTypes[ShapeType::MESH] && shapeTypes[ShapeType::QUAD]){
		assert(false && "implement");
	}
	else if (shapeTypes[ShapeType::MESH] && shapeTypes[ShapeType::PLANE]){
		assert(false && "implement");
	}
	else if (shapeTypes[ShapeType::MESH] && shapeTypes[ShapeType::TRIANGLE]){
		assert(false && "implement");
	}
	else if (shapeTypes[ShapeType::SPHERE] && shapeTypes[ShapeType::CUBE]){
		/// Check radial distance
		Entity * cube = NULL,
			* sphere= NULL;
		if (one->physics->shapeType == ShapeType::CUBE){
			cube = one;
			sphere = two;
		}
		else {
			cube = two;
			sphere = one;
		}
		//
		// Perform a quick opting sphere-sphere out before evaluating further!
		if (!SpheresColliding(one->worldPosition, two->worldPosition, one->physics->physicalRadius + two->physics->physicalRadius)){
			return false;
		}
	//	std::cout<<"\nTesting collission for: "<<cube->name;
		if (CubeSphereCollision(cube, sphere, data))
			shouldCollide = true;
	}
	else {
		std::cout<<"\nINFO: Unknown collission imminent.";
		assert(false && "Bad type in PhysicsManager::TestCollision");
	}

	// Check if collission should occur and add it to the list if so.
	if (shouldCollide)
	{
		collissionList.Add(data);
		return true;
	}

	return false;
}

