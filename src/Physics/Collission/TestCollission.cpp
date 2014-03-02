/// Emil Hedemalm
/// 2013-03-01

#include "../Entity/Entity.h"
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "Collissions.h"
#include "PhysicsLib/PhysicsMesh.h"
#include "Physics/Collission/CollissionShapeOctree.h"
#include <cstring>

/** Tests if a collission should occur between the two objects and
	either resolves it straight away or queues the collission to be solved at a later time
	depending on current settings.
*/
bool PhysicsManager::TestCollission(Entity * one, Entity * two, List<Collission> & collissionList){

	Collission data;

	/// Skip if for some reason they are equal to each other (implementation dependant if this check is made)
	if (one == two)
		return false;

	/// Skip if both objects are in rest next to each other ^.^
	if (one->physics->state & PhysicsState::IN_REST &&
		two->physics->state & PhysicsState::IN_REST)
		return false;

	// Save away one and two straight away!
	data.one = one;
	data.two = two;

	/// First check what shapes we have to compare.
	int shapeTypes[ShapeType::NUM_TYPES];
	int size = sizeof(shapeTypes);
	int types = ShapeType::NUM_TYPES;
	memset(shapeTypes, 0, sizeof(shapeTypes));
	++shapeTypes[one->physics->physicsShape];
	++shapeTypes[two->physics->physicsShape];

	bool shouldCollide = false;
	///=================================================================================//
	///  Sphere-Sphere Intersection
	///=================================================================================//
	if(shapeTypes[ShapeType::SPHERE] == 2){
		if (!SpheresColliding(one->positionVector, two->positionVector, one->physics->physicalRadius + two->physics->physicalRadius))
			return false;
		if(SphereSphereCollission(one, two, data)){
			shouldCollide = true;
		}
	}
	///=================================================================================//
	///  Plane-Sphere Intersection
	///=================================================================================//
	else if (shapeTypes[ShapeType::PLANE] == 1 && shapeTypes[ShapeType::SPHERE]){
		Entity * planeEntity = NULL,
			* sphereEntity = NULL;
		if (one->physics->physicsShape == ShapeType::PLANE){
			planeEntity = one;
			sphereEntity = two;
		}
		else {
			planeEntity = two;
			sphereEntity = one;
		}
		if (PlaneSphereCollission(planeEntity, sphereEntity, data))
			shouldCollide = true;
	}
	///=================================================================================//
	///  Mesh-Sphere Intersection
	///=================================================================================//
	else if (shapeTypes[ShapeType::MESH] == 1 && shapeTypes[ShapeType::SPHERE]){
		Entity * meshEntity = NULL,
			* sphereEntity = NULL;
		if (one->physics->physicsShape == ShapeType::MESH){
			meshEntity = one;
			sphereEntity = two;
		}
		else {
			meshEntity = two;
			sphereEntity = one;
		}
		// Perform a quick opting sphere-sphere out before evaluating further!
		if (!SpheresColliding(one->positionVector, two->positionVector, one->physics->physicalRadius + two->physics->physicalRadius)){
			return false;
		}

#define USE_COLLISSION_SHAPE_OCTREE true

		// If it's got an optimized octree, use it instead, yo.
		if (meshEntity->physics->usesCollissionShapeOctree && USE_COLLISSION_SHAPE_OCTREE){
		//	std::cout<<"\nUsing collission shape octree to optimized collission detection.";
			PhysicsMesh * physicsMesh = meshEntity->physics->physicsMesh;
			List<Collission> physicsMeshCollissions;
			int tests = physicsMesh->collissionShapeOctree->FindCollissions(sphereEntity, physicsMeshCollissions, meshEntity->transformationMatrix);
		//	std::cout<<"\nPhysicsMeshCollissions: "<<physicsMeshCollissions.Size();
		//	std::cout<<"\nPhysicsMeshChecks: "<<tests<<" out of "<<physicsMesh->triangles.Size()<<" triangles tested";
			Physics.physicsMeshCollissionChecks += tests;
			for (int i = 0; i < physicsMeshCollissions.Size(); ++i){
				Collission col = physicsMeshCollissions[i];
				col.one = one;
				col.two = two;
				collissionList.Add(col);
			}
		}
		else if (MeshSphereCollission(meshEntity, sphereEntity, data)){
		//	std::cout<<"\nRegular MeshSphere every-tri-vs-sphere collission detection.";
			shouldCollide = true;
		}
	}
	else if (shapeTypes[ShapeType::NULL_TYPE] > 0){

		std::cout<<"\nINFO: PhysicsShape is NULL_TYPE yet part of dynamic calculations!";
		assert(false && "Bad type in PhysicsManager::TestCollission");
	}
	else if (shapeTypes[ShapeType::TRIANGLE] && shapeTypes[ShapeType::SPHERE]){
		Entity * tri = NULL,
			* sphere= NULL;
		if (one->physics->physicsShape == ShapeType::TRIANGLE){
			tri = one;
			sphere = two;
		}
		else {
			tri = two;
			sphere = one;
		}
		if (TriangleSphereCollission(tri, sphere, data))
			shouldCollide = true;
	}
	else if (shapeTypes[ShapeType::QUAD] && shapeTypes[ShapeType::SPHERE]){
		Entity * quad = NULL,
			* sphere= NULL;
		if (one->physics->physicsShape == ShapeType::TRIANGLE){
			quad = one;
			sphere = two;
		}
		else {
			quad = two;
			sphere = one;
		}
		if (QuadSphereCollission(quad, sphere, data))
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
		if (one->physics->physicsShape == ShapeType::CUBE){
			cube = one;
			sphere = two;
		}
		else {
			cube = two;
			sphere = one;
		}
		//
		// Perform a quick opting sphere-sphere out before evaluating further!
		if (!SpheresColliding(one->positionVector, two->positionVector, one->physics->physicalRadius + two->physics->physicalRadius)){
			return false;
		}
	//	std::cout<<"\nTesting collission for: "<<cube->name;
		if (CubeSphereCollission(cube, sphere, data))
			shouldCollide = true;
	}
	else {
		std::cout<<"\nINFO: Unknown collission imminent.";
		assert(false && "Bad type in PhysicsManager::TestCollission");
	}

	// Check if collission should occur and add it to the list if so.
	if (shouldCollide)
		collissionList.Add(data);

	return false;
}

