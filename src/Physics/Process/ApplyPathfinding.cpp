/// Emil Hedemalm
/// 2013-12-21

#include "Physics/PhysicsManager.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Pathfinding/PathfindingProperty.h"
#include "Pathfinding/Waypoint.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/Path.h"

/// Applies pathfinding for all relevant entities
void PhysicsManager::ApplyPathfinding()
{
	std::cout<<"removeme";
	/*
	/// Woo!
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		
		// Fetch initial data
		EntitySharedPtr entity = dynamicEntities[i];
		PhysicsProperty * physics = entity->physics;
		PathfindingProperty * pathProp = entity->pathfindingProperty;
		if (!pathProp)
			continue;
		Path * path = NULL;
		bool hasPathsToTread = pathProp->HasPathsToTread();	
		if (pathProp){
			path = &pathProp->currentPath;
			hasPathsToTread = path->Size();
		}
	//	std::cout<<"\nBoudn to navmesh: "<<physics->boundToNavMesh<<" path: "<<path;
		/// Main variable used for now to show that an entity is involved in any kind of pathfinding..
	    if (!physics->boundToNavMesh && !hasPathsToTread)
	    	continue;

	//	assert(false);
	  //  std::cout<<"\nApplyPathfinding for entity: "<<entity->position;
	
		/// Good to know!
		Waypoint *& currentWaypoint = pathProp->currentWaypoint;
		Waypoint *& targetWaypoint = pathProp->targetWaypoint;
		Vector3f & desiredVelocity = pathProp->desiredVelocity;
		Vector3f & position = entity->worldPosition;

		
		/// If we got a target waypoint, move to it.	
		if (targetWaypoint){
			Vector3f velocity = physics->velocity;
		//	std::cout<<"\nMoving to target waypoint with velocity: "<<velocity;
			Vector3f entityToTargetWaypoint = targetWaypoint->position - entity->worldPosition;
			float velocityDotVectorToWaypoint = velocity.DotProduct(entityToTargetWaypoint);
			// Check if we've passed the waypoint (considering current velocity)
			if (velocityDotVectorToWaypoint < 0){
				std::cout<<"\nTarget waypoint behind us! Halting,.. hopefully.";
				// Not aligned, mening that we have passed it somehow, so place us at it!
				entity->localPosition = targetWaypoint->position;
				currentWaypoint->entities.Remove(entity);
				currentWaypoint = targetWaypoint;
				targetWaypoint = NULL;
				currentWaypoint->entities.Add(entity);
				/// Check if we'ge got a queued destination we should change to.
				if (pathProp->queuedDestination){
					pathProp->OnPathCompleted();
					pathProp->BeginPathIfNotAlreadyOneOne();
				}
			}	
	    }
	    // If not target waypoint no more, get next one
	    if (!targetWaypoint){
	    	assert(pathProp);
	    	Waypoint * newTargetWaypoint = NULL;
	    	Waypoint * currentWaypoint = pathProp->currentWaypoint;
	    	if (hasPathsToTread){
	    		pathProp->BeginPathIfNotAlreadyOneOne();
	    	}
	    	// Check for an active path for the entity, if it has one, follow it.
		    if (path && path->Waypoints()){
		    	std::cout<<"\ngot path to follow, checking it...";
		    	/// Check if we've reached the destination.
		    	if (path->GetIndex(currentWaypoint) == path->Waypoints() - 1){
		    		// WOHOHOHO.
		    		std::cout<<"\nArrived at destination! :)";
					std::cout<<"\nPath length: "<<path->Waypoints();
					path->Print();
					pathProp->OnPathCompleted();
		    	}
		    	/// Check if we've begun the path
		    	else if (path->GetIndex(currentWaypoint) >= 0){
		    		// On the path, get next index!
		    		Waypoint * wp = path->GetNext(currentWaypoint);
		    		newTargetWaypoint = wp;
		    	}
		    	// Not on path, teleport to the first one or just walk to it?
		    	else {
			    	Waypoint * first = path->GetWaypoint(0);
				    /// Teleport to the first waypoint?
				    entity->localPosition = first->position;
				    pathProp->currentWaypoint = first;
				    newTargetWaypoint = path->GetNext(first);
		    	}
		    }
		    // Do we have a general sense of desired velocity?
		    else if (desiredVelocity.MaxPart()){
//		    	std::cout<<"\nTrying to find a new waypuintlur.";
				if (!currentWaypoint){
//					std::cout<<"\nCurrent waypoint NULL, trying to fetch one from the active navMesh at position: "<<position;				
					/// Find our closest current waypoint in the active navMesh (there should hopefully be one).
					currentWaypoint = WaypointMan.GetClosestValidWaypoint(position);
					if (!currentWaypoint){
						std::cout<<"\nUnable to fetch valid waypoint!";
						desiredVelocity = Vector3f();
						return;
					}
					assert(currentWaypoint);
//					std::cout<<"\nFetched waypoint at: "<<currentWaypoint->position;
				}
				/// Skip any negative dot-products, but also any not coinciding at least a bit (45-degree max deviation).
				float bestDotProduct = 0.5f;
				Waypoint * bestNextNeighbour = NULL;
			//	std::cout<<"\nCurrentWaypoint Neighbours: "<<currentWaypoint->Neighbours();
				for (int i = 0; i < currentWaypoint->Neighbours(); ++i)
				{
					Waypoint * neighbour = currentWaypoint->Neighbour(i);
					if (!neighbour->passable)
						continue;
					/// Regard WP's with entities as occupied too!
					if (neighbour->entities.Size())
						continue;
					Vector3f toNeighbour = neighbour->position - currentWaypoint->position;
				//	std::cout<<"\n toNeighbour "<<i<<": "<<toNeighbour;
					toNeighbour.Normalize();
					float dotProduct = toNeighbour.DotProduct(desiredVelocity);
					if (dotProduct > bestDotProduct){
						bestNextNeighbour = neighbour;
						bestDotProduct = dotProduct;
						newTargetWaypoint = bestNextNeighbour;
					}
				}
				
		    }
		    /// If we found a new valid target waypoint, make our way there!
		    if (newTargetWaypoint)
			{
		    //	std::cout<<"\nNew target waypoint found";
				targetWaypoint = newTargetWaypoint;
				targetWaypoint->entities.Add(entity);
				currentWaypoint->entities.Remove(entity);
				Vector3f toNextNormalized = newTargetWaypoint->position - currentWaypoint->position;
				toNextNormalized.Normalize();
				/// Update velocity depending on stuff. This row update allows smooth diagnoal movement.. I think.
				/// TODO: Change the 5 to a value settable somewhere, for example the pathfindingProperty..
				physics->velocity = 5 * toNextNormalized;
			//	std::cout<<"\nNew target waypoint found: "<<newTargetWaypoint->position;
			}
	    }
	    /// If 0 desired velocity and NULL target, stop us.
		if (targetWaypoint == NULL){
			// Make sure stuffs are adjusted?
			physics->velocity = Vector3f();
		}
    }
	*/
}


