/// Emil Hedemalm
/// 2013-10-27
/// General physics messages here, move them elsewhere if they are numerous can categorizable.

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"
#include "Physics/Springs/Spring.h"
#include "PhysicsState.h"

#include "Mesh/Mesh.h"

PhysicsMessage::PhysicsMessage()
{
	type = PM_NULL;
}

PhysicsMessage::PhysicsMessage(int messageType)
{
	type = messageType;
}

PhysicsMessage::~PhysicsMessage()
{
}

void PhysicsMessage::Process()
{
	switch(type)
	{
		case PM_PAUSE_SIMULATION:
			physicsState->simulationPaused = true;
			break;
		case PM_RESUME_SIMULATION:
			physicsState->simulationPaused = false;
			break;
		case PM_RECALCULATE_PHYSICS_PROPERTIES: 
			Physics.RecalculatePhysicsProperties();
			break;
		/// For cleaing all, and also if the user sent an Unregister_Entities without it's own custom message .. ^^;;;
		case PM_CLEAR_ALL_ENTITIES: case PM_UNREGISTER_ENTITIES:
			Physics.UnregisterAllEntities();
			break;
		case PM_IGNORE_COLLISSIONS:
			Physics.ignoreCollisions = true;
			break;
		case PM_RESET_SETTINGS:
			Physics.gravitation = Vector3f(0, -DEFAULT_GRAVITY, 0);
			Physics.simulationSpeed = 1.0f;
			Physics.ignoreCollisions = false;
			break;
	}
}

PMRecalculatePhysicsMesh::PMRecalculatePhysicsMesh(Mesh * forMesh)
: PhysicsMessage(PM_RECALC_PHYS_MESH), mesh(forMesh)
{

}
void PMRecalculatePhysicsMesh::Process()
{
	Physics.RecalculatePhysicsMesh(mesh);
}


/// Creates a default spring between the entities in the list (linearly attached).
PMCreateSpring::PMCreateSpring(List< std::shared_ptr<Entity> > targetEntities, ConstVec3fr position, float springConstant, float springLength)
: PhysicsMessage(PM_CREATE_SPRING), entities(targetEntities), position(position), springConstant(springConstant), springLength(springLength)
{
	toPosition = true;	
}

/// Creates a default spring between the entities in the list (linearly attached).
PMCreateSpring::PMCreateSpring(List< std::shared_ptr<Entity> > targetEntities, float springConstant = 1.0f)
: PhysicsMessage(PM_CREATE_SPRING), entities(targetEntities), springConstant(springConstant), springLength(-1.0f)
{
	toPosition = false;
}
/// Creates a default spring between the entities in the list (linearly attached).
PMCreateSpring::PMCreateSpring(List< std::shared_ptr<Entity> > targetEntities, float springConstant, float springLength)
: PhysicsMessage(PM_CREATE_SPRING), entities(targetEntities), springConstant(springConstant), springLength(springLength)
{
	toPosition = false;
};

#include "Physics/PhysicsProperty.h"
void PMCreateSpring::Process() 
{
	/// Entity-position springs.
	if (toPosition)
	{
		for (int i = 0; i < entities.Size(); ++i)
		{
			EntitySharedPtr entity = entities[i];
			Spring * spring = new Spring(entity, position);
			spring->springConstant = springConstant;
			spring->equilibriumLength = springLength;
			entity->physics->springs.Add(spring);
			Physics.springs.Add(spring);
		}
	}
	/// Entity-entity springs
	else 
	{
		for (int i = 0; i < entities.Size()-1; ++i)
		{
			EntitySharedPtr one = entities[i], two = entities[i+1];
			Spring * spring = new Spring(one, two);
			spring->springConstant = springConstant;
			spring->equilibriumLength = springLength;
			if (spring->equilibriumLength < 0)
				spring->equilibriumLength = (one->worldPosition - two->worldPosition).Length();
			one->physics->springs.Add(spring);
			two->physics->springs.Add(spring);
			Physics.springs.Add(spring);
		}		
	}
};

PMRaycast::PMRaycast(const Ray & ray)
: PhysicsMessage(PM_RAYCAST), ray(ray)
{
	relevantEntity = NULL;
}

#include "Message/MessageManager.h"

void PMRaycast::Process()
{
	List<Intersection> isecs = PhysicsMan.Raycast(ray);
	
	Raycast * raycast = new Raycast(ray, isecs);
	raycast->relevantEntity = relevantEntity;
	raycast->msg = msg;
	MessageQueueP.Add(raycast);
}

Raycast::Raycast(const Ray & ray, List<Intersection> isecs)
: Message(MessageType::RAYCAST), ray(ray), isecs(isecs)
{
	relevantEntity = NULL;
}



