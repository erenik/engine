// Emil Hedemalm
// 2013-07-20

#include "PhysicsMessage.h"
#include "../PhysicsProperty.h"
#include "../../Entity/Entity.h"
#include "../PhysicsManager.h"
#include "../Collision/Collision.h"
#include "Model.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"
#include "Pathfinding/PathfindingProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"

// For resets and similar
PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities)
	: target(target)
{
	type = PM_SET_ENTITY;
	entities = targetEntities;
	switch(target)
	{
		case RESET_ROTATION:
			break;
		default:
			assert(false && "Bad target in PMSetEntity");
	}
}


PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, float value)
: target(target)
{
	dataType = FLOAT;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	}
	fValue = value;
	switch(target){
		case MASS:
		case SCALE:
		case SET_SCALE:
		case FRICTION:
		case RESTITUTION:
		case VELOCITY_RETAINED_WHILE_TURNING:
		case POSITION_Y:
		case POSITION_X:
			break;
		default:
			assert("Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, Vector2f value, long long timeStamp)
: target(target), timeStamp(timeStamp)
{
	dataType = VECTOR2F;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	vec2fValue = value;
	switch(target)
	{
		case SET_SCALE:
		case VELOCITY:
		case POSITION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}


PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, Vector3f value, long long timeStamp)
: target(target), timeStamp(timeStamp)
{
	dataType = VECTOR3F;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	vec3fValue = value;
	switch(target)
	{
		case VELOCITY:
		case ANGULAR_VELOCITY:
		case CONSTANT_ROTATION_VELOCITY:
		case POSITION:
		case TRANSLATE:
		case SCALE:
		case ROTATE:
		case SET_POSITION:
			break;
		case SET_SCALE:
			assert(vec3fValue.x && vec3fValue.y && vec3fValue.z);
			break;
		case SET_ROTATION:
		case ACCELERATION:  
		case RELATIVE_ACCELERATION:
		case ACCELERATION_MULTIPLIER:
		case ANGULAR_ACCELERATION:
		case DESTINATION:
		case RELATIVE_ROTATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, Quaternion value, long long timeStamp /*= 0*/)
	: target(target), timeStamp(timeStamp)
{
	dataType = QUATERNION;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	qValue = value;
	switch(target)
	{
		case ANGULAR_VELOCITY:
		case CONSTANT_ROTATION_VELOCITY:
		case ROTATE:
		case SET_ROTATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, bool value)
: target(target)
{
	dataType = BOOLEAN;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	bValue = value;
	switch(target){
		case COLLISIONS_ENABLED:
		case COLLISSION_CALLBACK:
		case NO_COLLISSION_RESOLUTION:
		case LOCK_POSITION:
		case SIMULATION_ENABLED:
		case ESTIMATION_ENABLED:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, int value)
: target(target)
{
	dataType = INTEGER;
	type = PM_SET_ENTITY;
	dataType = INTEGER;
	entities = targetEntities;
	iValue = value;
	switch(target){
		case PHYSICS_TYPE:
		case PHYSICS_SHAPE:
		case ESTIMATION_MODE:
		case ESTIMATION_DELAY:
		case ESTIMATION_SMOOTHING_DURATION:
		case GRAVITY_MULTIPLIER:
		case COLLISION_CATEGORY:
		case COLLISION_FILTER:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}


void PMSetEntity::Process()
{
#define ASSERT_ENTITY_NOT_STATIC { \
	if (entity->physics->type == PhysicsType::STATIC){ \
		std::cout<<"\nEntity "<<entity->name<<" physics is static, converting to dynamic."; \
		Physics.SetPhysicsType(entity, PhysicsType::DYNAMIC); \
	} \
}
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		// Create if not there.
		if (!entity->physics)
			entity->physics = new PhysicsProperty();
		PhysicsProperty * physics = entity->physics;
		switch(target)
		{
			case COLLISION_CATEGORY:
				physics->collisionCategory = iValue;
				break;
			case COLLISION_FILTER:
				physics->collisionFilter = iValue;
				break;
			case RESET_ROTATION:
			{
				physics->orientation = Quaternion();
				entity->rotation = Vector3f();
				break;	
			}
			// Float types?
			case MASS:
				entity->physics->mass = fValue;
				break;
			case GRAVITY_MULTIPLIER:
				if (dataType == INTEGER)
					entity->physics->gravityMultiplier = iValue;
				else if (dataType == FLOAT)
					entity->physics->gravityMultiplier = fValue;
				break;
			// Waypoint?
			case DESTINATION:{
				NavMesh * nm = WaypointMan.ActiveNavMesh();
				assert(nm);
				if(!nm)
					return;
				Waypoint * wp = nm->GetClosestVacantWaypoint(vec3fValue);
				assert(wp);
				if(!wp)
					return;
				PathfindingProperty * pathProp = entity->pathfindingProperty;
				assert(pathProp);
				Waypoint * to = wp;
				Waypoint * from = pathProp->currentWaypoint;
			//	if (pathProp->targetWaypoint)
			//		from = pathProp->targetWaypoint;
				// Kill the previous path.
			//	pathProp->OnPathCompleted();

				// If we're already walking, queue a destination, not a full path!
				if (pathProp->CurrentPath().Waypoints()){
					pathProp->SetQueuedDestination(to);
					return;
				}
				assert(to);
				assert(from);
				Path path = PathMan.GetPath(from, to);
				if (path.Waypoints()){
					entity->pathfindingProperty->QueuePath(path);
				}
				break;
			}
			/// Int-types
			case PHYSICS_TYPE:
				Physics.SetPhysicsType(entity, iValue);
				break;
			case PHYSICS_SHAPE:
				Physics.SetPhysicsShape(entity, iValue);
				break;
			case ESTIMATION_MODE:
				if (entity->physics->estimator)
					entity->physics->estimator->estimationMode = iValue;
				break;
			case ESTIMATION_DELAY:
				if (entity->physics->estimator)
					entity->physics->estimator->estimationDelay = iValue;
				break;
			case ESTIMATION_SMOOTHING_DURATION:
				if (entity->physics->estimator)
					entity->physics->estimator->smoothingDuration = iValue;
			/// Vec3 types
			case VELOCITY:
				switch(dataType)
				{
					case VECTOR2F:
						entity->physics->velocity = vec2fValue;
						break;
					case VECTOR3F:
						if (entity->physics->estimationEnabled)
							entity->physics->estimator->AddVelocity(vec3fValue, timeStamp);
						else {
							entity->physics->velocity = vec3fValue;
							entity->physics->linearMomentum = entity->physics->velocity * entity->physics->mass;
						}
						break;
				}
				break;
			case ANGULAR_VELOCITY:
				entity->physics->angularVelocity = vec3fValue;
				break;
			case CONSTANT_ROTATION_VELOCITY:
				entity->physics->constantAngularVelocity= vec3fValue;
				break;
			case SET_POSITION:
			case POSITION:
			{
				switch(dataType)
				{
					case VECTOR2F:
						entity->position.x = vec2fValue.x;
						entity->position.y = vec2fValue.y;
						entity->RecalculateMatrix();
						break;
					case VECTOR3F:
						if (entity->physics->estimationEnabled)
							entity->physics->estimator->AddPosition(vec3fValue, timeStamp);
						else
							entity->SetPosition(vec3fValue);
						break;
				}
				break;
			}
			case POSITION_Y:
				entity->position.y = fValue;
				entity->RecalculateMatrix();
				break;
			case POSITION_X:
				entity->position.x = fValue;
				entity->RecalculateMatrix();
				break;
			case TRANSLATE:
				entity->Translate(vec3fValue);
				break;
			case SCALE:
				entity->Scale(vec3fValue);
				entity->physics->UpdateProperties(entity);
			//	std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			case SET_SCALE:
			{
				switch(dataType)
				{
					case FLOAT:	
						entity->SetScale(Vector3f(fValue, fValue, fValue)); 
						break;
					case VECTOR2F: 
						entity->scale.x = vec2fValue.x; 
						entity->scale.y = vec2fValue.y; 
						entity->RecalculateMatrix(); 
						break;
					case VECTOR3F:	
						entity->SetScale(vec3fValue); 
						break;
				}
				entity->physics->UpdateProperties(entity);
		//		std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			}
			case ROTATE:
			{
				switch(dataType)
				{
					case VECTOR3F:
						entity->Rotate(vec3fValue);
						break;
					case QUATERNION:
						entity->RotateGlobal(qValue);
						break;
				}
				entity->physics->UpdateProperties(entity);
				break;
			}
			case SET_ROTATION: 
			{
				switch(dataType)
				{
					case VECTOR3F:
						if (entity->physics->estimationEnabled){
							entity->physics->estimator->AddRotation(vec3fValue, timeStamp);
							break;
						}
						else
							entity->SetRotation(vec3fValue);
						break;
					case QUATERNION:
						entity->SetRotation(qValue);
						break;
				}
				entity->physics->UpdateProperties(entity);
				break;
			}
			case ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->acceleration = vec3fValue;
				break;
			case ANGULAR_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->angularAcceleration = vec3fValue;
				break;
			case RELATIVE_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->relativeAcceleration = vec3fValue;
				break;
			case RELATIVE_ROTATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->relativeRotation = vec3fValue;
				break;
			case FRICTION:
				entity->physics->friction = fValue;
				break;
			case RESTITUTION:
				entity->physics->restitution = fValue;
				break;
			case COLLISIONS_ENABLED:
				/// Best to re-register the entity to make sure that everything works as intended..!
				Physics.UnregisterEntity(entity);
				entity->physics->collissionsEnabled = false;
				// 
				Physics.RegisterEntity(entity);
			/*	if (bValue == false){
					if (Physics.entityCollisionOctree->Exists(entity))
						Physics.entityCollisionOctree->RemoveEntity(entity);
				}
				*/
				break;
			case COLLISSION_CALLBACK:
				entity->physics->collissionCallback = bValue;
				break;
			case NO_COLLISSION_RESOLUTION:
				entity->physics->noCollisionResolutions = bValue;
				break;
            case LOCK_POSITION:
                if (bValue)
                    entity->physics->locks |= POSITION_LOCKED;
                else
                    entity->physics->locks &= ~POSITION_LOCKED;
                break;
			case SIMULATION_ENABLED:
				entity->physics->simulationEnabled = bValue;
				break;
			case ESTIMATION_ENABLED:
				entity->physics->estimationEnabled = bValue;
				if (bValue && !entity->physics->estimator){
					entity->physics->estimator = new EntityPhysicsEstimator(entity);
				}
				break;
			case VELOCITY_RETAINED_WHILE_TURNING:
				entity->physics->velocityRetainedWhileRotating = fValue;
				break;
			default:
				assert(false && "Implement=?");
				break;
		}

		/// Update collission state for it
		Vector3f nullVec;
		if (entity->registeredForPhysics)
			UpdateCollisionState(entity, nullVec);
	}
}
