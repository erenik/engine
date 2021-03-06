/// Emil Hedemalm
/// 2014-07-16
/// Physics collision-detection class. Subclass for custom behaviour.

#include "CollisionDetector.h"
#include "Collision/Collisions.h"

#include "Physics/Collision/CollisionShapeOctree.h"
#include "Physics/PhysicsManager.h"

#include "PhysicsLib/PhysicsMesh.h"

/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
int CollisionDetector::DetectCollisions(Entity* one, Entity* two, List<Collision> & collisions)
{
	Collision data;

	if (! (one->physics->collisionFilter & two->physics->collisionCategory &&
		two->physics->collisionFilter & one->physics->collisionCategory))
		return false;

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
		Entity* planeEntity = NULL;
		Entity* sphereEntity = NULL;
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
	///=================================================================================//
	///  Mesh-Sphere Intersection
	///=================================================================================//
	else if (shapeTypes[ShapeType::MESH] == 1 && shapeTypes[ShapeType::SPHERE])
	{
		Entity* meshEntity = NULL;
		Entity* sphereEntity = NULL;
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
		if (meshEntity->physics->usesCollisionShapeOctree && USE_COLLISSION_SHAPE_OCTREE)
		{
		//	std::cout<<"\nUsing collission shape octree to optimized collission detection.";
			PhysicsMesh * physicsMesh = meshEntity->physics->physicsMesh;
			List<Collision> physicsMeshCollisions;
			int tests = physicsMesh->collisionShapeOctree->FindCollisions(sphereEntity, physicsMeshCollisions, meshEntity->transformationMatrix);
		//	std::cout<<"\nPhysicsMeshCollisions: "<<physicsMeshCollisions.Size();
		//	std::cout<<"\nPhysicsMeshChecks: "<<tests<<" out of "<<physicsMesh->triangles.Size()<<" triangles tested";
			Physics.physicsMeshCollisionChecks += tests;
			for (int i = 0; i < physicsMeshCollisions.Size(); ++i){
				Collision col = physicsMeshCollisions[i];
				col.one = one;
				col.two = two;
				collisions.Add(col);
			}
		}
		else if (MeshSphereCollision(meshEntity, sphereEntity, data)){
		//	std::cout<<"\nRegular MeshSphere every-tri-vs-sphere collission detection.";
			shouldCollide = true;
		}
	}
	else if (shapeTypes[ShapeType::TRIANGLE] && shapeTypes[ShapeType::SPHERE]){
		Entity* tri = NULL;
		Entity* sphere= NULL;
		if (one->physics->shapeType == ShapeType::TRIANGLE){
			tri = one;
			sphere = two;
		}
		else {
			tri = two;
			sphere = one;
		}
		if (TriangleSphereCollision(tri, sphere, data, true))
			shouldCollide = true;
	}
	else if (shapeTypes[ShapeType::QUAD] && shapeTypes[ShapeType::SPHERE]){
		Entity* quad = NULL;
		Entity* sphere= NULL;
		if (one->physics->shapeType == ShapeType::TRIANGLE){
			quad = one;
			sphere = two;
		}
		else {
			quad = two;
			sphere = one;
		}
		if (QuadSphereCollision(quad, sphere, data, true))
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
		Entity* cube = NULL;
		Entity* sphere= NULL;
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
	else if (shapeTypes[ShapeType::MESH] >= 2)
	{
		// Mesh-mesh collision...
		if (!SpheresColliding(one->worldPosition, two->worldPosition, one->physics->physicalRadius + two->physics->physicalRadius))
			return false;
		data.collisionNormal = (one->worldPosition - two->worldPosition).NormalizedCopy();
		/// CBA serious collision, just do it as if they are spheres for now?
		shouldCollide = true;
	}
	else {
		std::cout<<"\nINFO: Unknown collission imminent.";
		assert(false && "Bad type in PhysicsManager::TestCollision");
	}

	// Check if collission should occur and add it to the list if so.
	if (shouldCollide)
		collisions.Add(data);

	return false;

}