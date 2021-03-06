// Emil Hedemalm
// 2013-07-20

#include "PhysicsMessage.h"
#include "../PhysicsProperty.h"
#include "../../Entity/Entity.h"
#include "../PhysicsManager.h"
#include "../Collision/Collision.h"
#include "Model/Model.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"
#include "Pathfinding/PathfindingProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"

// For resets and similar
PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target)
	: target(target)
{
	type = PM_SET_ENTITY;
	entities = targetEntities;
	switch(target)
	{
		case PT_RESET_ROTATION:
		case PT_INHERIT_POSITION_ONLY:
			break;
		default:
			assert(false && "Bad target in PMSetEntity");
	}
}


PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, float value)
: target(target)
{
	dataType = FLOAT;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity* entity = entities[i];
	}
	fValue = value;
	switch(target){
		case PT_MASS:
		case PT_SCALE:
		case PT_SET_SCALE:
		case PT_FRICTION:
		case PT_RESTITUTION:
		case PT_VELOCITY_RETAINED_WHILE_TURNING:
		case PT_POSITION_Y:
		case PT_POSITION_X:
		case PT_ROTATION_YAW:
		case PT_ANGULAR_DAMPING:
			break;
		default:
			assert("Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, Vector2f value, long long timeStamp)
: target(target), timeStamp(timeStamp)
{
	dataType = VECTOR2F;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity* entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	vec2fValue = value;
	switch(target)
	{
		case PT_SET_SCALE:
		case PT_VELOCITY:
		case PT_POSITION:
		case PT_SET_POSITION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}


PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, const Vector3f & value, long long timeStamp)
: target(target), timeStamp(timeStamp)
{
	dataType = VECTOR3F;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity* entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	vec3fValue = value;
	switch(target)
	{
		case PT_VELOCITY:
		case PT_ANGULAR_VELOCITY:
		case PT_CONSTANT_ROTATION_VELOCITY:
		case PT_POSITION:
		case PT_TRANSLATE:
		case PT_SCALE:
		case PT_ROTATE:
		case PT_SET_POSITION:
			break;
		case PT_SET_SCALE:
			assert(vec3fValue[0] && vec3fValue[1] && vec3fValue[2]);
			break;
		case PT_SET_ROTATION:
		case PT_RELATIVE_VELOCITY:
		case PT_ACCELERATION:  
		case PT_RELATIVE_ACCELERATION:
		case PT_ACCELERATION_MULTIPLIER:
		case PT_ANGULAR_ACCELERATION:
		case PT_RELATIVE_ANGULAR_ACCELERATION:
		case PT_DESTINATION:
		case PT_RELATIVE_ROTATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, const Quaternion & value, long long timeStamp /*= 0*/)
	: target(target), timeStamp(timeStamp)
{
	dataType = QUATERNION;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity* entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	qValue = value;
	switch(target)
	{
		case PT_ANGULAR_VELOCITY:
		case PT_CONSTANT_ROTATION_VELOCITY:
		case PT_ROTATE:
		case PT_SET_PRE_TRANSLATE_ROTATION:
		case PT_SET_ROTATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, bool value)
: target(target)
{
	dataType = BOOLEAN;
	type = PM_SET_ENTITY;
	entities = targetEntities;
	bValue = value;
	switch(target)
	{
		case PT_USE_QUATERNIONS:
		case PT_COLLISIONS_ENABLED:
		case PT_COLLISION_CALLBACK:
		case PT_NO_COLLISSION_RESOLUTION:
		case PT_LOCK_POSITION:
		case PT_SIMULATION_ENABLED:
		case PT_ESTIMATION_ENABLED:
		case PT_PAUSED:
		case PT_FACE_VELOCITY_DIRECTION:
		case PT_PLANE_COLLISIONS_ONLY:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, int value)
: target(target)
{
	dataType = INTEGER;
	type = PM_SET_ENTITY;
	dataType = INTEGER;
	entities = targetEntities;
	iValue = value;
	switch(target){
		case PT_PHYSICS_TYPE:
		case PT_PHYSICS_SHAPE:
		case PT_ESTIMATION_MODE:
		case PT_ESTIMATION_DELAY:
		case PT_ESTIMATION_SMOOTHING_DURATION:
		case PT_GRAVITY_MULTIPLIER:
		case PT_COLLISION_CATEGORY:
		case PT_COLLISION_FILTER:
		case PT_SET_SCALE:
		case PT_POSITION_X:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, Waypoint * waypoint)
: target(target), entities(targetEntities), waypoint(waypoint)
{
	switch(target)
	{
		case PT_CURRENT_WAYPOINT:
			break;
		default:
			assert(false && "Mistmatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(List< Entity* > targetEntities, int target, Entity* referenceEntity)
	: target(target), entities(targetEntities), referenceEntity(referenceEntity)
{
	switch(target)
	{
		case PT_SET_PARENT:
			break;
		default:
			assert(false && "Bad target");
	}
}


void PMSetEntity::Process()
{
#define ASSERT_ENTITY_NOT_STATIC { \
	if (entity->physics->type == PhysicsType::STATIC){ \
/*		std::cout<<"\nEntity "<<entity->name<<" physics is static, converting to dynamic."; */\
		Physics.SetPhysicsType(entity, PhysicsType::DYNAMIC); \
	} \
}
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		if (!entity)
			continue;
		// Create if not there.
		if (!entity->physics)
			entity->physics = new PhysicsProperty();
		PhysicsProperty * physics = entity->physics;
		PhysicsProperty * pp = physics;
		PathfindingProperty * pathProp = entity->pathfindingProperty;
		switch(target)
		{
			case PT_INHERIT_POSITION_ONLY:
			{
				entity->inheritPositionOnly = true;
				break;
			}
			case PT_PLANE_COLLISIONS_ONLY:
			{
				entity->physics->SetPlaneCollisionsOnly(bValue);
				break;
			}
			case PT_FACE_VELOCITY_DIRECTION:
			{
				pp->faceVelocityDirection = bValue;
				break;
			}
			case PT_USE_QUATERNIONS:
				pp->useQuaternions = bValue;
				break;
			// Waypoints and pathfinding.
			case PT_CURRENT_WAYPOINT:
				// Set the waypoint in the entity property..
				pathProp->currentWaypoint = waypoint;
				waypoint->entities.Add(entity);
				break;
			// Parenting
			case PT_SET_PARENT:
			{
				// If already has parent, decouble.
				if (entity->parent)
					entity->parent->children.RemoveItem(entity);
				entity->parent = referenceEntity; // Set new one.
				if (referenceEntity)
					referenceEntity->children.Add(entity);
				// Update transform straight away?
				entity->RecalculateMatrix();
				break;
			}
			/// Stuff.
			case PT_RELATIVE_VELOCITY:
			{
				ASSERT_ENTITY_NOT_STATIC
				pp->relativeVelocity = vec3fValue;
				break;
			}
			case PT_PAUSED:
			{
				physics->paused = bValue;
				break;
			}
			case PT_COLLISION_CATEGORY:
				physics->collisionCategory = iValue;
				break;
			case PT_COLLISION_FILTER:
				physics->collisionFilter = iValue;
				break;
			case PT_RESET_ROTATION:
			{
				physics->orientation = Quaternion();
				entity->rotation = Vector3f();
				break;	
			}
			// Float types?
			case PT_MASS:
				entity->physics->mass = fValue;
				break;
			case PT_GRAVITY_MULTIPLIER:
				if (dataType == INTEGER)
					entity->physics->gravityMultiplier = iValue;
				else if (dataType == FLOAT)
					entity->physics->gravityMultiplier = fValue;
				break;
			// Waypoint?
			case PT_DESTINATION:
			{
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
				if (pathProp->CurrentPath().Size()){
					pathProp->SetQueuedDestination(to);
					return;
				}
				assert(to);
				assert(from);
				Path path = PathMan.GetPath(from, to);
				if (path.Size()){
					entity->pathfindingProperty->QueuePath(path);
				}
				break;
			}
			/// Int-types
			case PT_PHYSICS_TYPE:
				Physics.SetPhysicsType(entity, iValue);
				break;
			case PT_PHYSICS_SHAPE:
				Physics.SetPhysicsShape(entity, iValue);
				break;
			case PT_ESTIMATION_MODE:
				if (entity->physics->estimator)
					entity->physics->estimator->estimationMode = iValue;
				break;
			case PT_ESTIMATION_DELAY:
				if (entity->physics->estimator)
					entity->physics->estimator->estimationDelay = iValue;
				break;
			case PT_ESTIMATION_SMOOTHING_DURATION:
				if (entity->physics->estimator)
					entity->physics->estimator->smoothingDuration = iValue;
			/// Vec3 types
			case PT_VELOCITY:
				ASSERT_ENTITY_NOT_STATIC;
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
							assert(!entity->physics->velocity.IsInfinite());
							entity->physics->linearMomentum = entity->physics->velocity * entity->physics->mass;
						}
						break;
				}
				break;
			case PT_ANGULAR_VELOCITY:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->angularVelocity = vec3fValue; // ang vel pretty much only used for quaternions, but could be used for non-Quats too.
				break;
			case PT_CONSTANT_ROTATION_VELOCITY:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->constantAngularVelocity = vec3fValue;
				break;
			case PT_SET_POSITION:
			case PT_POSITION:
			{
				switch(dataType)
				{
					case VECTOR2F:
						entity->localPosition[0] = vec2fValue[0];
						entity->localPosition[1] = vec2fValue[1];
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
			case PT_POSITION_Y:
				entity->localPosition[1] = fValue;
				entity->RecalculateMatrix();
				break;
			case PT_POSITION_X:
				switch(dataType)
				{
					case DataType::FLOAT:
						entity->localPosition.x = fValue;
						break;
					case DataType::INTEGER:
						entity->localPosition.x = iValue;
						break;						
				}
				entity->RecalculateMatrix();
				break;
			case PT_POSITION_Z:
				entity->localPosition[2] = fValue;
				entity->RecalculateMatrix();
				break;
			case PT_TRANSLATE:
				entity->Translate(vec3fValue);
				break;
			case PT_SCALE:
				entity->Scale(vec3fValue);
				entity->physics->UpdateProperties(entity);
			//	std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			case PT_SET_SCALE:
			{
				PhysicsMan.UnregisterEntity(entity);
				switch(dataType)
				{
					case INTEGER:
						entity->SetScale(Vector3f(iValue, iValue, iValue));
						break;
					case FLOAT:	
						entity->SetScale(Vector3f(fValue, fValue, fValue)); 
						break;
					case VECTOR2F: 
						entity->scale[0] = vec2fValue[0]; 
						entity->scale[1] = vec2fValue[1]; 
						entity->RecalculateMatrix(); 
						break;
					case VECTOR3F:	
						entity->SetScale(vec3fValue); 
						break;
				}
				entity->physics->UpdateProperties(entity);
				/// Re-register to avoid bugs in collision-detection optimized systems (such as AABBSweeper) which do not actively re-sort non-dynamic entities!
				PhysicsMan.RegisterEntity(entity);

		//		std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			}
			case PT_ROTATE:
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
			case PT_ROTATION_YAW:
			{
				//
				Vector3f newRotation = entity->rotation;
				newRotation[1] = fValue;
				entity->SetRotation(newRotation);
				break;
			}
			case PT_SET_PRE_TRANSLATE_ROTATION: 
			{
				switch(dataType)
				{
					case QUATERNION:
					{
						physics->preTranslateRotationQ = qValue;
						break;
					}
					default:
						assert(false);
				}
				break;	
			}
			case PT_SET_ROTATION: 
			{
				switch(dataType)
				{
					case VECTOR3F:
						if (entity->physics->estimationEnabled)
						{
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
			case PT_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->acceleration = vec3fValue;
				break;
			case PT_ANGULAR_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->angularAcceleration = vec3fValue;
				break;
			case PT_RELATIVE_ANGULAR_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->relativeAngularAcceleration = vec3fValue;
				break;
			case PT_RELATIVE_ACCELERATION:
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->relativeAcceleration = vec3fValue;
				break;
			case PT_RELATIVE_ROTATIONAL_VELOCITY: // earlier PT_RELATIVE_ROTATION
				ASSERT_ENTITY_NOT_STATIC;
				entity->physics->relativeRotationalVelocity = vec3fValue;
				break;
			case PT_ANGULAR_DAMPING:
				entity->physics->angularDamping = fValue;
				break;
			case PT_FRICTION:
				entity->physics->friction = fValue;
				break;
			case PT_RESTITUTION:
				entity->physics->restitution = fValue;
				break;
			case PT_COLLISIONS_ENABLED:
				/// Best to re-register the entity to make sure that everything works as intended..!
				Physics.UnregisterEntity(entity);
				entity->physics->collisionsEnabled = false;
				// 
				Physics.RegisterEntity(entity);
			/*	if (bValue == false){
					if (Physics.entityCollisionOctree->Exists(entity))
						Physics.entityCollisionOctree->RemoveEntity(entity);
				}
				*/
				break;
			case PT_COLLISION_CALLBACK:
				entity->physics->collisionCallback = bValue;
				break;
			case PT_NO_COLLISSION_RESOLUTION:
				entity->physics->noCollisionResolutions = bValue;
				break;
            case PT_LOCK_POSITION:
                if (bValue)
                    entity->physics->locks |= POSITION_LOCKED;
                else
                    entity->physics->locks &= ~POSITION_LOCKED;
                break;
			case PT_SIMULATION_ENABLED:
				entity->physics->simulationEnabled = bValue;
				break;
			case PT_ESTIMATION_ENABLED:
				entity->physics->estimationEnabled = bValue;
				if (bValue && !entity->physics->estimator)
				{
					entity->physics->estimator = new EntityPhysicsEstimator(entity);
				}
				break;
			case PT_VELOCITY_RETAINED_WHILE_TURNING:
				entity->physics->velocityRetainedWhileRotating = fValue;
				break;
			case PT_LINEAR_DAMPING:
				entity->physics->linearDamping = fValue;
				break;
			default:
				assert(false && "Implement=?");
				break;
		}
		/// Recalculate radius, etc.
		pp->UpdateProperties(entity);

		/// Update collission state for it
		Vector3f nullVec;
		if (entity->registeredForPhysics)
			UpdateCollisionState(entity, nullVec);
	}
}
