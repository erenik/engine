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

#include "Mesh/Mesh.h"
#include "Graphics/FrameStatistics.h"

#include <ctime>

PhysicsManager * PhysicsManager::physicsManager = NULL;

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
    checkType = OCTREE;

	physicsMessageQueueMutex.Create("physicsMessageQueueMutex");
	aabbSweeper = new AABBSweeper();

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
	int ms = timer.GetMs();
	FrameStats.physicsMessages = ms;
	timer.Start();
	// Process physics from here in order to avoid graphical issues
	ProcessPhysics();
	timer.Stop();
	int ms2 = timer.GetMs();
	FrameStats.physicsProcessing = ms2;
	if (mesManMessages.Size())
	{
		MesMan.QueueMessages(mesManMessages);
		mesManMessages.Clear();
	}
	totalPhysics.Stop();
	FrameStats.totalPhysics = totalPhysics.GetMs();
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
	// Pause physics while at it ? <- lollll
//	Pause();
	List<Intersection> intersections;
	// Grab entities.
	List<Entity*> entities = physicalEntities;

	// Do some initial filtering, as most of the entities will either: not be relevant for the raycast or may be culled quickly using a sphere-ray check.
	
	// Do brute-force ray-casting. Improve later.
	float distance;
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
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

	// Resume if paused.
//	Resume();
	return intersections;
}

void PhysicsManager::RecalculateAABBs()
{
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		AABB * aabb = dynamicEntity->GetAABB();
		aabb->position = dynamicEntity->position;
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
}

void PhysicsManager::RecalculateOBBs()
{
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
	  	dynamicEntity->physics->obb->Recalculate(dynamicEntity);
    }
}

// Grab AABB of all relevant entities? Check the AABB-sweeper or other relevant handler?
AABB PhysicsManager::GetAllEntitiesAABB()
{
	AABB aabb;
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
		Entity * entity = physicalEntities[i];
#ifdef USE_SSE
		aabb.max.data = _mm_max_ps(aabb.max.data, entity->aabb->max.data);
		aabb.min.data = _mm_min_ps(aabb.min.data, entity->aabb->min.data);
#else
		aabb.max = Vector3f::Maximum(aabb->max, entity->aabb->max);
		aabb.min = Vector3f::Minimum(aabb->min, entity->aabb->min);
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
void PhysicsManager::AttachPhysicsTo(Entity * entity){
	if (entity->physics)	// Return if it already has a type
		return;
	entity->physics = new PhysicsProperty();
	entity->physics->type = PhysicsType::STATIC;
	entity->physics->shapeType = ShapeType::SPHERE;
	assert(entity->physics->shape == NULL);
	entity->physics->shape = new Sphere();
	((Sphere*)entity->physics->shape)->radius = entity->radius;
}


/// Sets physics type of target entity.
void PhysicsManager::SetPhysicsType(Entity * entity, int type){
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
void PhysicsManager::SetPhysicsType(List<Entity*> & targetEntities, int type){
	if (type >= PhysicsType::STATIC || type >= PhysicsType::NUM_TYPES){
		std::cout<<"\nERROR: Invalid physics type provided!";
		return;
	}
	Entity * entity;
	for (int i = 0; i < targetEntities.Size(); ++i){
		entity = targetEntities[i];
		SetPhysicsType(entity, type);
	}
}


/// Sets physics shape (Plane, Sphere, Mesh, etc.)
void PhysicsManager::SetPhysicsShape(List<Entity*> targetEntities, int type)
{
	if (type < ShapeType::SPHERE || type >= ShapeType::NUM_TYPES){
		std::cout<<"\nERROR: Invalid physics type provided!";
		return;
	}
	Entity * entity;
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
			((Sphere*)entity->physics->shape)->radius = entity->radius;
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
		default:
			assert(false && "FUCK YOU!");
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
List<Entity*> PhysicsManager::GetDynamicEntities(){
	return dynamicEntities;
}

/// Loads physics mesh if not already loaded.
void PhysicsManager::EnsurePhysicsMesh(Entity * targetEntity)
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
void PhysicsManager::EnsurePhysicsMeshIfNeeded(Entity * targetEntity){
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
	mesh->meshCounterpart = byMeshSource;
	for (int i = 0; i < byMeshSource->numFaces; ++i)
	{
		// Just copy shit from it
		MeshFace * faces = &byMeshSource->faces[i];
		assert((faces->numVertices <= 4 || faces->numVertices >= 3) && "Bad vertices count in faces");

		int vi0 = faces->vertices[0],
			vi1 = faces->vertices[1],
			vi2 = faces->vertices[2];

		assert(vi0 < byMeshSource->vertices.Size() && 
			vi0 >= 0);

		Vector3f p1 = byMeshSource->vertices[vi0],
			p2 = byMeshSource->vertices[vi1],
			p3 = byMeshSource->vertices[vi2];

		if (faces->numVertices == 4){
			Vector3f p4 = byMeshSource->vertices[faces->vertices[3]];
			Quad * quad = new Quad();
			quad->Set4Points(p1, p2, p3, p4);
			mesh->quads.Add(quad);
		}
		else if (faces->numVertices == 3){
			Triangle * tri = new Triangle();
			tri->Set3Points(p1, p2, p3);
			mesh->triangles.Add(tri);
		}
	}
	if (mesh->quads.Size() == 0 && mesh->triangles.Size() == 0)
		assert(false && "Unable to load physicsmesh from mesh source!");
	std::cout<<"\nCreated physics mesh for \""<<byMeshSource->source<<"\": ";
	if (mesh->triangles.Size())
		std::cout<<"\n- "<<mesh->triangles.Size()<<" triangles";
	if (mesh->quads.Size())
		std::cout<<"\n- "<<mesh->quads.Size()<<" quads";
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
PhysicsMesh * PhysicsManager::GetPhysicsMesh(const Mesh * byMeshSource){
	for (int i = 0; i < physicsMeshes.Size(); ++i){
		if (physicsMeshes[i]->meshCounterpart == byMeshSource)
			return physicsMeshes[i];
	}
	return NULL;
}


/*
private:
	/// If calculations should pause.
	bool paused;
	/// Time in milliseconds that last physics update was performed.
	long lastUpdate;

	/// Array with pointers to all registered objects.
	Entity * Entity[MAX_REGISTERED_ENTITIES];
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
		Entity * entity = physicalEntities[i];
		PhysicsProperty * physics = entity->physics;
		/// The physical position is defined as the centre of the object, physically.
	//	physics->physicalPosition = entity->position;
	//	physics->physicalPosition += entity->model->centerOfModel.ElementMultiplication(entity->scale);
		// Recalculate radius
		physics->physicalRadius = entity->model->radius * entity->scale.MaxPart();
		entity->aabb->Recalculate(entity);
	}
}

/// Processes queued messages.
void PhysicsManager::ProcessMessages()
{
	while(!physicsMessageQueueMutex.Claim(-1));
	List<PhysicsMessage*> messages;
    // Process queued messages
	messages = messageQueue;
	messageQueue.Clear();
	physicsMessageQueueMutex.Release();
	// Release mutex

	/// Then start actually processing the messages.
	for (int i = 0; i < messages.Size(); ++i)
	{
		PhysicsMessage * msg = messages[i];
		msg->Process();
	}
	messages.ClearAndDelete();
}
