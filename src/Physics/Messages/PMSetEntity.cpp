// Emil Hedemalm
// 2013-07-20

#include "PhysicsMessage.h"
#include "../PhysicsProperty.h"
#include "../../Entity/Entity.h"
#include "../PhysicsManager.h"
#include "../Collission/Collission.h"
#include "Model.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"
#include "Pathfinding/PathfindingProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, float value)
: target(target){
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	}
	fValue = value;
	switch(target){
		case SCALE:
		case SET_SCALE:
		case FRICTION:
		case RESTITUTION:
		case VELOCITY_RETAINED_WHILE_TURNING:
			break;
		default:
			assert("Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, Vector3f value, long long timeStamp)
: target(target), timeStamp(timeStamp){
	type = PM_SET_ENTITY;
	entities = targetEntities;
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
	//	std::cout<<"entity ang acc:"<<entity->physics->angularAcceleration;
	}
	vec3fValue = value;
	switch(target){
		case VELOCITY:
		case POSITION:
		case TRANSLATE:
		case SCALE:
		case ROTATE:
		case SET_POSITION:
		case SET_SCALE:
		case SET_ROTATION:
		case ACCELERATION:  case ACCELERATION_MULTIPLIER:
		case ANGULAR_ACCELERATION:
		case DESTINATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}

PMSetEntity::PMSetEntity(int target, List<Entity*> targetEntities, bool value)
: target(target){
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
: target(target){
	type = PM_SET_ENTITY;
	entities = targetEntities;
	iValue = value;
	switch(target){
		case PHYSICS_TYPE:
		case ESTIMATION_MODE:
		case ESTIMATION_DELAY:
		case ESTIMATION_SMOOTHING_DURATION:
			break;
		default:
			assert(false && "Mismatched target and value in PMSetEntity!");
	}
}


void PMSetEntity::Process(){
#define ASSERT_ENTITY_NOT_STATIC { \
	if (entity->physics->type == PhysicsType::STATIC){ \
		std::cout<<"\nEntity "<<entity->name<<" physics is static, converting to dynamic."; \
		Physics.SetPhysicsType(entity, PhysicsType::DYNAMIC); \
	} \
}
	/// Assertions earlier should guarantee correct target now
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
		switch(target){
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
				if (entity->physics->estimationEnabled)
					entity->physics->estimator->AddVelocity(vec3fValue, timeStamp);
				else
					entity->physics->velocity = vec3fValue;
				break;
			case SET_POSITION:
			case POSITION:
				if (entity->physics->estimationEnabled)
					entity->physics->estimator->AddPosition(vec3fValue, timeStamp);
				else
					entity->position(vec3fValue);
				break;
			case TRANSLATE:
				entity->translate(vec3fValue);
				break;
			case SCALE:
				entity->Scale(vec3fValue);
				entity->physics->UpdateProperties(entity);
				std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			case ROTATE:
				entity->rotate(vec3fValue);
				entity->physics->UpdateProperties(entity);
				break;
			case SET_SCALE:
				entity->SetScale(vec3fValue);
				entity->physics->UpdateProperties(entity);
				std::cout<<"\nEntity scale set to "<<vec3fValue;
				break;
			case SET_ROTATION: {
				if (entity->physics->estimationEnabled){
					entity->physics->estimator->AddRotation(vec3fValue, timeStamp);
					break;
				}
				else
					entity->rotationVector = vec3fValue;
				#ifdef USE_QUATERNIONS
				// http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
				/// Convert from carteesian to quaternions.
					Vector3f direction = vec3fValue.NormalizedCopy();
				//	float amount = vec3fValue.Length();

				//	float cosAng = cos(amount * 0.5f);
				//	float sinAng = sin(amount * 0.5f);
					Quaternion q = Quaternion(vec3fValue, 1);
					q.Normalize();

					entity->physics->orientation = q;
				#endif
				entity->recalculateMatrix();
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
			case FRICTION:
				entity->physics->friction = fValue;
				break;
			case RESTITUTION:
				entity->physics->restitution = fValue;
				break;
			case COLLISIONS_ENABLED:
				/// Best to re-register the entity to make sure that everything works as intended..!
				Physics.UnregisterEntity(entity);
				/// If registered for collissions, remove that now.
				entity->physics->collissionsEnabled = bValue;
				Physics.RegisterEntity(entity);
			/*	if (bValue == false){
					if (Physics.entityCollissionOctree->Exists(entity))
						Physics.entityCollissionOctree->RemoveEntity(entity);
				}
				*/
				break;
			case COLLISSION_CALLBACK:
				entity->physics->collissionCallback = bValue;
				break;
			case NO_COLLISSION_RESOLUTION:
				entity->physics->noCollissionResolutions = bValue;
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
			UpdateCollissionState(entity, nullVec);
	}
}
