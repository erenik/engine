/// Emil Hedemalm
/// 2013-03-01
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "../Entity/Entity.h"
#include "Model/Model.h"
#include "Entity/Entities.h"
#include "../Collision/Collisions.h"
#include "Graphics/GraphicsManager.h"
#include "PhysicsLib/AABBSweeper.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Physics/Contact/Contact.h"
#include "Physics/Springs/Spring.h"

#include "Physics/Integrator.h"
#include "Physics/CollisionResolver.h"
#include "Physics/CollisionDetector.h"

#include "PhysicsLib/PhysicsMesh.h"
#include "Sphere.h"
#include "PhysicsLib/Shapes/Quad.h"
#include "PhysicsLib/Shapes/Cube.h"
#include "PhysicsLib/Shapes/Ray.h"
#include "PhysicsLib/Shapes/OBB.h"

#include "File/LogFile.h"
#include "Mesh/Mesh.h"
#include "Graphics/FrameStatistics.h"

#include <ctime>

PhysicsManager * PhysicsManager::physicsManager = NULL;

List<PhysicsMessage*> requeuedMessages;

// static const int MAX_REGISTERED_ENTITIES = 2048;
/// Allocate
void PhysicsManager::Allocate(){
	assert(physicsManager == NULL);
	physicsManager = new PhysicsManager();
}
PhysicsManager * PhysicsManager::Instance(){
	assert(physicsManager);
	return physicsManager;
}
void PhysicsManager::Deallocate(){
	assert(physicsManager);
	delete(physicsManager);
	physicsManager = NULL;
}

int64 physicsNowMs = 0;

PhysicsManager::PhysicsManager()
{
	assert(physicalEntities.Size() == 0);
	assert(dynamicEntities.Size() == 0);

	simulationSpeed = 1.0f;
	airDensity = 1.225f;
	lastUpdate = 0;
	paused = false;
	ignoreCollisions = false;
	gravitation[1] = -DEFAULT_GRAVITY; // Sets default gravitation (corresponds to 9.82 m/s^2 in real life, if 1 unit is 1 meter in-game.

	physicsMessageQueueMutex.Create("physicsMessageQueueMutex");
	aabbSweeper = new AABBSweeper();
	checkType = AABB_SWEEP;

	pauseOnCollision = false;
	linearDamping = 0.5f;
	angularDamping = 0.5f;

	collisionResolverType = CollisionResolverType::LAB_PHYSICS_IMPULSES;
	integratorType = IntegratorType::LAB_PHYSICS;
	/// kg per m^3
	defaultDensity = 100.0f;

	physicsIntegrator = 0;
	collisionResolver = 0;
	collisionDetector = 0;
}

PhysicsManager::~PhysicsManager()
{
	// Delete remaining messages.
	messageQueue.ClearAndDelete();
	requeuedMessages.ClearAndDelete(); // Also the queued ones.

	SAFE_DELETE(aabbSweeper);
	SAFE_DELETE(entityCollisionOctree);
	CLEAR_AND_DELETE(physicsMeshes);
	physicsMeshes.ClearAndDelete();
	physicsMessageQueueMutex.Destroy();
	contacts.ClearAndDelete();
	springs.ClearAndDelete();

	SAFE_DELETE(physicsIntegrator);
	SAFE_DELETE(collisionResolver);
	SAFE_DELETE(collisionDetector);
}

int PhysicsManager::RegisteredEntities()
{
	return this->physicalEntities.Size();
}

/// Called once per frame from controlling thread. Handles message-processing, integration, simulation.
void PhysicsManager::Process()
{
	Timer totalPhysics, timer;
	FrameStats.ResetPhysics();
	totalPhysics.Start();
	timer.Start();
	/// Process any available physics messages first
	ProcessMessages();
	timer.Stop();
	int64 ms = timer.GetMs();
	FrameStats.physicsMessages = ms;
	timer.Start();
	// Process physics from here in order to avoid graphical issues
	ProcessPhysics();
	timer.Stop();
	int64 ms2 = timer.GetMs();
	FrameStats.physicsProcessing = (float) ms2;
	if (mesManMessages.Size())
	{
		MesMan.QueueMessages(mesManMessages);
		mesManMessages.Clear();
	}
	totalPhysics.Stop();
	FrameStats.totalPhysics = (float) totalPhysics.GetMs();
}


/// Performs various tests in order to optimize performance during runtime later.
void PhysicsManager::Initialize(){
	entityCollisionOctree = new PhysicsOctree();
	InitOctree(50000); /// Cubical 10000
}
/// Initializes the entityCollisionOctree to the specified size (cube).
void PhysicsManager::InitOctree(float size){
	entityCollisionOctree->SetBoundaries(-size, size, size, -size, size, -size);
}
/// Initializes the entityCollisionOctree to the specified bounds.
void PhysicsManager::InitOctree(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound){
	entityCollisionOctree->SetBoundaries(leftBound, rightBound, topBound, bottomBound, nearBound, farBound);
}

/// Casts a ray.
List<Intersection> PhysicsManager::Raycast(Ray & ray)
{
	Timer timer;
	timer.Start();
	Timer timer2;
	timer2.Start();
//	Pause();
	static List<Intersection> intersections;
	intersections.Clear();
	static List< Entity* > entities;
	entities.Clear();

	Sphere sphere;

	Vector3f rayOriginToEntityCenter, pointOnRay, pointOnRayToEntity;
	float rayOriginToEntityCenterProjectedOntoRayDir, distAlongRay, distAlongRaySquared, distSquared, scaleSquared;
	// Do some initial filtering, as most of the entities will either: not be relevant for the raycast or may be culled quickly using a sphere-ray check.	
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
		Entity* entity = physicalEntities[i];
		PhysicsProperty * pp = entity->physics;
		/// Check filters if relevant.
		if (ray.collisionFilter)
		{
			if ((ray.collisionFilter & pp->collisionCategory) == 0)
				continue;
		}

		/// Do a simple dot-product check first for early out, no matter what physics-shape.
#ifdef USE_SSE
		SSEVec sse;
		rayOriginToEntityCenter.data = _mm_sub_ps(entity->aabb->position.data, ray.start.data);
		sse.data = _mm_mul_ps(ray.direction.data, rayOriginToEntityCenter.data);
		rayOriginToEntityCenterProjectedOntoRayDir = sse.x + sse.y + sse.z;// ray.direction.DotProduct(rayOriginToEntityCenter);		
		sse.data = _mm_sub_ps(entity->aabb->max.data, entity->aabb->position.data);
		sse.data = _mm_mul_ps(sse.data, sse.data);
		scaleSquared = sse.x + sse.y + sse.z;
		//assert(scaleSquared > 0 && "Scale of entity should probably not be 0?");
#else
		rayOriginToEntityCenter = entity->aabb->position - ray.start;
		rayOriginToEntityCenterProjectedOntoRayDir = ray.direction.DotProduct(rayOriginToEntityCenter);
#endif
		distAlongRay = rayOriginToEntityCenterProjectedOntoRayDir;
		distAlongRaySquared = distAlongRay * distAlongRay;
		if (distAlongRay < 0)
		{
			/// Comparison will be on negative ray axis, since we checked that beforre.
			if (distAlongRaySquared  > scaleSquared)
			{
				continue;
			}
		}
		// Do simple sphere-line projection check too.
#ifdef USE_SSE
		sse.data = _mm_load1_ps(&rayOriginToEntityCenterProjectedOntoRayDir);
		pointOnRay.data = _mm_add_ps(ray.start.data, _mm_mul_ps(ray.direction.data, sse.data));
		pointOnRayToEntity.data = _mm_sub_ps(entity->aabb->position.data, pointOnRay.data);
		sse.data = _mm_mul_ps(pointOnRayToEntity.data, pointOnRayToEntity.data);
		distSquared = sse.x + sse.y + sse.z;
#else
		pointOnRay = ray.start + ray.direction * rayOriginToEntityCenterProjectedOntoRayDir;
		pointOnRayToEntity = entity->aabb->position - pointOnRay;
		distSquared = pointOnRayToEntity.LengthSquared();
		scaleSquared = (entity->aabb->max - entity->aabb->position).LengthSquared();
#endif
		if (distSquared > scaleSquared)
		{
			continue;
		}
		entities.Add(entity);
	}
	timer2.Stop();
	int64 initialFilter = timer2.GetMs();

	timer2.Start();
	// Do brute-force ray-casting. Improve later.
	float distance;
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		PhysicsProperty * pp = entity->physics;
		switch(pp->shapeType)
		{
			case PhysicsShape::SPHERE:
			{
				// Grab the entity's sphere, do check.
				Sphere sphere;
				sphere.radius = pp->physicalRadius;
				sphere.position = entity->worldPosition;				
				if (ray.Intersect(sphere, &distance))
				{
					Intersection iSec;
					iSec.distance = distance;
					iSec.entity = entity;
					intersections.Add(iSec);
				}
				break;
			}
			case PhysicsShape::MESH:
			{
				// Do an initial Ray-AABB check.
				if (!ray.Intersect(*entity->aabb, &distance))
					continue;
				List<Intersection> iSecs = pp->physicsMesh->Raycast(ray, entity->transformationMatrix);
				// Mark which entity it was..
				for (int j = 0; j < iSecs.Size(); ++j)
				{
					iSecs[j].entity = entity;
				}
				intersections += iSecs;
				break;
			}
			case PhysicsShape::AABB:
			{
				float distance;
				// Mark which entity it was..
				if (ray.Intersect(*entity->aabb, &distance))
				{
					Intersection iSec;
					iSec.distance = distance;
					iSec.entity = entity;
					intersections.Add(iSec);
				}
				break;
			}
			default:
				assert(false);
		}
	}
	timer2.Stop();
	int64 checks = timer2.GetMs();
	timer2.Start();
	// Sort by distance.. No D:?
	for (int i = 0; i < intersections.Size(); ++i)
	{
		Intersection & one = intersections[i];
		for (int j = 1; j < intersections.Size(); ++j)
		{
			Intersection & two = intersections[j];
			if (one.distance > two.distance)
			{
				Intersection tmp = one;
				one = two;
				two = tmp;
			}
		}
	}
	timer2.Stop();
	int64 sortByDistance = timer2.GetMs();

	timer.Stop();
	int64 ms = timer.GetMs();
	// Resume if paused.
//	Resume();
	return intersections;
}

void PhysicsManager::RecalculateAABBs()
{
	/// Moved to be done in Entity::RecalculateMatrix
	return;
	/*
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity* dynamicEntity = dynamicEntities[i];
		AABB * aabb = dynamicEntity->GetAABB();
		aabb->position = dynamicEntity->worldPosition;
		aabb->min = aabb->position - aabb->scale * 0.5f;
		aabb->max = aabb->position + aabb->scale * 0.5f;
//		Vector3f minr = aabb->min, maxr = aabb->max;
//		aabb->Recalculate(dynamicEntity);
//		if (minr != aabb->min)
//		{
//			assert(false);
	//		std::cout<<"nererer";
//		}
    }
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity* kinematicEntities = dynamicEntities[i];
		AABB * aabb = kinematicEntities->GetAABB();
		aabb->position = kinematicEntities->position;
		aabb->min = aabb->position - aabb->scale * 0.5f;
		aabb->max = aabb->position + aabb->scale * 0.5f;
//		Vector3f minr = aabb->min, maxr = aabb->max;
//		aabb->Recalculate(dynamicEntity);
//		if (minr != aabb->min)
//		{
//			assert(false);
	//		std::cout<<"nererer";
//		}
    }
	*/
}

void PhysicsManager::RecalculateOBBs()
{
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity* dynamicEntity = dynamicEntities[i];
	  	dynamicEntity->physics->obb->Recalculate(dynamicEntity);
    }
}

// Grab AABB of all relevant entities? Check the AABB-sweeper or other relevant handler?
AABB PhysicsManager::GetAllEntitiesAABB()
{
	AABB aabb;
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
		Entity* entity = physicalEntities[i];
#ifdef USE_SSE
		aabb.max.data = _mm_max_ps(aabb.max.data, entity->aabb->max.data);
		aabb.min.data = _mm_min_ps(aabb.min.data, entity->aabb->min.data);
#else
		aabb.max = Vector3f::Maximum(aabb.max, entity->aabb->max);
		aabb.min = Vector3f::Minimum(aabb.min, entity->aabb->min);
#endif
	}
	return aabb;
}

/// Queues a message to the physics-queue, waiting for the mutex to be released before accessing it.
void PhysicsManager::QueueMessage(PhysicsMessage * msg) {
//    std::cout<<"\nMessage queued: "<<msg<<" of type "<<msg->Type();
	physicsMessageQueueMutex.Claim(-1);
	messageQueue.Add(msg);
	physicsMessageQueueMutex.Release();
}

// Enters a message into the message queue
void PhysicsManager::QueueMessages(List<PhysicsMessage*> msgs)
{
	physicsMessageQueueMutex.Claim(-1);
	messageQueue.Add(msgs);
	physicsMessageQueueMutex.Release();	
}


/// Attaches a physics property to target entity if it didn't already have one.
void PhysicsManager::AttachPhysicsTo(Entity* entity){
	if (entity->physics)	// Return if it already has a type
		return;
	entity->physics = new PhysicsProperty();
	entity->physics->type = PhysicsType::STATIC;
	entity->physics->shapeType = ShapeType::SPHERE;
	assert(entity->physics->shape == NULL);
	entity->physics->shape = new Sphere();
	((Sphere*)entity->physics->shape)->radius = entity->Radius();
}


/// Sets physics type of target entity.
void PhysicsManager::SetPhysicsType(Entity* entity, int type){
	if (type < PhysicsType::STATIC || type >= PhysicsType::NUM_TYPES){
		std::cout<<"\nERROR: Invalid physics type provided!";
		return;
	}
	if (entity->physics == NULL){
		std::cout<<"\nERROR: Entity lacks physics property!";
		return;
	}
	UnregisterEntity(entity);
	if (!entity->physics)
	{
		entity->physics = new PhysicsProperty();
	}
	entity->physics->type = type;
	RegisterEntity(entity);
};

/// Sets physics type of target entities.
void PhysicsManager::SetPhysicsType(List< Entity* > & targetEntities, int type){
	if (type >= PhysicsType::STATIC || type >= PhysicsType::NUM_TYPES){
		std::cout<<"\nERROR: Invalid physics type provided!";
		return;
	}
	Entity* entity;
	for (int i = 0; i < targetEntities.Size(); ++i){
		entity = targetEntities[i];
		SetPhysicsType(entity, type);
	}
}


/// Sets physics shape (Plane, Sphere, Mesh, etc.)
void PhysicsManager::SetPhysicsShape(List< Entity* > targetEntities, int type)
{
	if (type < ShapeType::SPHERE || type >= ShapeType::NUM_TYPES){
		std::cout<<"\nERROR: Invalid physics type provided!";
		return;
	}
	Entity* entity;
	for (int i = 0; i < targetEntities.Size(); ++i){
		entity = targetEntities[i];
		if (entity->physics == NULL){
			std::cout<<"\nERROR: Entity lacks physics property!";
			continue;
		}
		if (entity->physics->shape){
			delete entity->physics->shape;
			entity->physics->shape = NULL;
		}
		entity->physics->shapeType = type;
		switch(type){
		case ShapeType::SPHERE:
			entity->physics->shape = new Sphere();
			((Sphere*)entity->physics->shape)->radius = entity->Radius();
			break;
		case ShapeType::PLANE:
			entity->physics->shape = new Plane();
			break;
		case ShapeType::MESH: 
		{
			entity->physics->shape = NULL;
			EnsurePhysicsMeshIfNeeded(entity);
			PhysicsMesh * mesh = entity->physics->physicsMesh;
			if (!mesh)
				break;
			// Use an octree when the faces number exceeds a predefined limit.
			if (mesh->triangles.Size() > 12 || mesh->quads.Size() > 6) // 12 for cube o-o
				entity->physics->usesCollisionShapeOctree = true;
			break;
		}
		case ShapeType::TRIANGLE:
			entity->physics->shape = new Triangle();
			break;
		case ShapeType::QUAD:
			entity->physics->shape = new Quad();
			break;
		case ShapeType::CUBE:
			entity->physics->shape = new Cube();
			break;
		case ShapeType::AABB:
			entity->physics->shape = new AABB();
			break;
		default:
			assert(false && "Shape type not supported yet.");
			break;
		}
	}
}




/// Resumes physics calculations, moving entities in the world using gravitation, given velocities, etc.
void PhysicsManager::Resume(){
	lastUpdate = Timer::GetCurrentTimeMs();	// Fetch new time or time will fly by when we resume now :P
	paused = false;
}
/// Pauses physics calculations, sleeping the thread (if any) until resume is called once again.
void PhysicsManager::Pause(){
	paused = true;
	MesMan.QueueMessage(new Message("OnPhysicsPaused"));
}

/// Returns a list of all registered entities.
Entities PhysicsManager::GetEntities(){
	if (physicalEntities.Size() == 0)
		return Entities();
	return Entities(physicalEntities);
}
List< Entity* > PhysicsManager::GetDynamicEntities(){
	return dynamicEntities;
}

/// Loads physics mesh if not already loaded.
void PhysicsManager::EnsurePhysicsMesh(Entity* targetEntity)
{
	///
	if (!targetEntity->model)
		return;
    /// Check if it already has a shape
    if (targetEntity->physics->physicsMesh){

    }
    /// Otherwise, load it
    else {
        PhysicsMesh * mesh = Physics.GetPhysicsMesh(targetEntity->model->GetTriangulizedMesh());
        if (!mesh){
            mesh = Physics.LoadPhysicsMesh(targetEntity->model->GetTriangulizedMesh());
            mesh->GenerateCollisionShapeOctree();
        }
        targetEntity->physics->physicsMesh = mesh;
    }
}

/// Checks if the entity requires a physics mesh and loads it if so.
void PhysicsManager::EnsurePhysicsMeshIfNeeded(Entity* targetEntity){
	// Check if we need a physics-mesh.
	if (targetEntity->physics->shapeType == ShapeType::MESH){
		EnsurePhysicsMesh(targetEntity);
	}
}

/// Loads PhysicsMesh from mesh counterpart
PhysicsMesh * PhysicsManager::LoadPhysicsMesh(Mesh * byMeshSource)
{
	assert(byMeshSource);
	PhysicsMesh * mesh = new PhysicsMesh();
	mesh->LoadFrom(byMeshSource);
	physicsMeshes.Add(mesh);
	return mesh;
}
/// Getter funcsschtlions
PhysicsMesh * PhysicsManager::GetPhysicsMesh(String bySourceFile){
	for (int i = 0; i < physicsMeshes.Size(); ++i){
		if (physicsMeshes[i]->source == bySourceFile)
			return physicsMeshes[i];
	}
	return NULL;
}

PhysicsMesh * PhysicsManager::GetPhysicsMesh(const Mesh * byMeshSource)
{
	for (int i = 0; i < physicsMeshes.Size(); ++i)
	{
		if (physicsMeshes[i]->source == byMeshSource->source)
			return physicsMeshes[i];
	}
	return NULL;
}

// Called server-side.
void PhysicsManager::RecalculatePhysicsMesh(Mesh * mesh)
{
	int recalcs = 0;
	for (int i = 0; i < physicsMeshes.Size(); ++i)
	{
		PhysicsMesh * pm = physicsMeshes[i];
		if (pm->source == mesh->source)
		{
			// Recalc
			pm->LoadFrom(mesh);
			++recalcs;
			assert(recalcs <= 1 && "if more than one");
		}
	}
}

/*
private:
	/// If calculations should pause.
	bool paused;
	/// Time in milliseconds that last physics update was performed.
	long lastUpdate;

	/// Array with pointers to all registered objects.
	Entity* Entity[MAX_REGISTERED_ENTITIES];
	/// Amount of currently registered objects.
	int registeredEntities;

	/// Gravitation vector, may be altered with functions later if wished.
	Vector3f gravitation;
	/// Density of air.
	float airDensity;
	*/

/** Recalculates physical properties for the selected entities. */
void PhysicsManager::RecalculatePhysicsProperties(){
    /// This should be handled elsewhere as it's probably a bit of a time-sink?
    assert(false);
	for (int i = 0; i < physicalEntities.Size(); ++i){
		Entity* entity = physicalEntities[i];
		PhysicsProperty * physics = entity->physics;
		// Recalculate radius
		entity->RecalculateRadius();
		entity->aabb->Recalculate(entity);
	}
}

/// Processes queued messages.
void PhysicsManager::ProcessMessages()
{
	while(!physicsMessageQueueMutex.Claim(-1));

	static List<PhysicsMessage*> messages, processedMessages;
	messages.Clear();
	messages = requeuedMessages;
	processedMessages.Clear();
    // Process queued messages
	messages += messageQueue;
	messageQueue.Clear();
	physicsMessageQueueMutex.Release();
	// Release mutex

	/// Then start actually processing the messages.
	Time startTime = Time::Now(),
		currTime;
	bool skipHeavy = false;
	for (int i = 0; i < messages.Size(); ++i)
	{
		PhysicsMessage * msg = messages[i];
		if (skipHeavy)
		{
			switch(msg->type)
			{
				case PM_RAYCAST:
				{
					delete msg;
					continue;
				}
			}
		}
		msg->Process();
		processedMessages.AddItem(msg);
		
		//
		if (i % 100 == 0)
		{
			// check time taken.
			currTime = Time::Now();
			if ((currTime - startTime).Milliseconds() > 100)
			{
				// Break and postpone messages.
				LogPhysics("Physics messages consuming over 100 ms, starting to skip time-consuming requests.", DEBUG);
				skipHeavy = true;
			//	requeuedMessages = messages.Part(i + 1);
//				requeuedMessages.ClearAndDelete();
			}
		}
		
	}
	/// Delete what we had time to process so far.
	processedMessages.ClearAndDelete();
}
