// Emil Hedemalm
// 2013-10-23
// The main iterator/integrator/collission-handling function.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/AABBSweeper.h"
#include "Graphics/GraphicsManager.h"
//#include "Entity/Entity.h"

/// Processes physics for all registered objects
void PhysicsManager::ProcessPhysics(){
	/// Returns straight away if paused.
	if (paused)
		return;

	activeTriangles.Clear();

	time_t currentTime = Timer::GetCurrentTimeMs();
	time_t millisecondsSinceLastUpdate = currentTime - lastUpdate;
	lastUpdate = currentTime;
  //  std::cout<<"\nCurrent time: "<<currentTime<<" last update: "<<lastUpdate;

	/// Throw away time if we've got more than 1 second, since this assumes we're debugging
	if (millisecondsSinceLastUpdate > 100){
		if (millisecondsSinceLastUpdate > 1000)
			std::cout<<"\nPhysicsManager::Throwing away "<<millisecondsSinceLastUpdate / 1000<<" debugging seconds";
		millisecondsSinceLastUpdate = 100;
	}

	float totalTimeSinceLastUpdate = millisecondsSinceLastUpdate * 0.001f;
	/// Multiply the total time since last update with the simulation speed multiplier before actual calculations are begun.
	totalTimeSinceLastUpdate = totalTimeSinceLastUpdate * simulationSpeed;

	/// Just return if simulation speed decreases beyond 0.1%!
	if (simulationSpeed <= 0.0001f){
		return;
	}

	// Reset previous frame-times
	recalculatingPropertiesDuration = 0;
	movingDuration = 0;
	collissionProcessingFrameTime = 0;
	physicsMeshCollissionChecks = 0;
	static Timer recalc, moving, collissionTimer;

	/// Do one process for each 10 ms we've gotten stored up
	while (totalTimeSinceLastUpdate > ZERO){
		/// Get sub-time to calculate.
		float dt = 0.010f * simulationSpeed;
#define timeDiff    dt
#define timeSinceLastUpdate dt
		if (totalTimeSinceLastUpdate < timeSinceLastUpdate)
			timeSinceLastUpdate = totalTimeSinceLastUpdate;
		totalTimeSinceLastUpdate -= timeSinceLastUpdate;

		recalc.Start();
		// Begin by recalculating physics position and scales
	//	this->RecalculatePhysicsProperties();
		recalc.Stop();
		recalculatingPropertiesDuration += recalc.GetMs();

		moving.Start();
		/// Awesome.
		Integrate(timeSinceLastUpdate);
		moving.Stop();
		movingDuration += moving.GetMs();

		/// Apply external constraints
	//	ApplyContraints();
		
		/// Apply pathfinding for all relevant entities
		ApplyPathfinding();
        
        /// Check if we should process collissions at all.
        if (integrator == Integrator::SIMPLE_PHYSICS){
        	continue;
        }

        /// Process collissions below..

		/// Check state flag
		if (!ignoreCollissions){
			collissionTimer.Start();
			/// AABB sweep check
            if (checkType == AABB_SWEEP){
		//		std::cout<<"\nAABB_SWEEP";
			
                /// First: Reset collission flags for all entities.
                for (int i = 0; i < physicalEntities.Size(); ++i){
                    Entity * e = physicalEntities[i];
                    e->physics->collissionState = AABB_IDLE;
                }

                List<EntityPair> pairs;
                pairs.Clear();
                pairs = aabbSweeper->Sweep();
 //               aabbSweeper->PrintSortedList();
     //           std::cout<<"\nBroad phase AABB sweep pairs: "<<pairs.Size();
                for (int i = 0; i < pairs.Size(); ++i){
                    /// For now just mark both as intersecting.
                    EntityPair ep = pairs[i];
					Entity * one = ep.one, * two = ep.two;
                    ep.one->physics->collissionState = AABB_INTERSECTING;
                    ep.two->physics->collissionState = AABB_INTERSECTING;

					// If both are in rest/static, skip them.
					if ((one->physics->state & PhysicsState::IN_REST || one->physics->type == PhysicsType::STATIC) && 
						(two->physics->state & PhysicsState::IN_REST || two->physics->type == PhysicsType::STATIC))
						continue;

					/// TODO: Do Narrow-phase!
                    Collission collissionData;
                    bool collissionImminent = OBBOBBCollission(&ep.one->physics->obb, &ep.two->physics->obb, collissionData);
                    if (collissionImminent){
                        /// Set them to render as collidididiing!
                        ep.one->physics->collissionState = COLLIDING;
                        ep.two->physics->collissionState = COLLIDING;
                        collissionData.one = ep.one;
                        collissionData.two = ep.two;
						
				//		std::cout<<"\n\nCollission imminent between "<<one->name<<" "<<one->position<<" and "<<two->name<<" "<<two->position<<".";

						/*
						if (ep.one->position.DotProduct(collissionData.collissionNormal) > 
							ep.two->position.DotProduct(collissionData.collissionNormal))
						{
							collissionData.collissionNormal *= -1.0f;
						}
						*/
						// Pause if prompted.
						if (pauseOnCollission){
							assert(collissionData.one && collissionData.two);
							Pause();
						}
/*
                        std::cout<<"\n=?-----------------------------------------------------------------";
                        std::cout<<"\nCollission imminent between "<<one<<" at "<<one->position<<" and "<<two<<" at "<<two->position;
                        std::cout<<"\nCollissionData: one: "<<collissionData.one<<" two: "<<collissionData.two;
                        std::cout<<"\nPreliminary collission normal: "<<collissionData.preliminaryCollissionNormal;
*/
						lastCollission = collissionData;

						bool resolveCollissions = true;
						if (resolveCollissions){
							bool resolveResult = ResolveCollission(collissionData);

							/// If response was applied, make sure they're not still colliding with each other.
							if (resolveResult){
							//	std::cout<<"\nCollission response applied.";
								collissionData.results |= RESOLVED;
								
								Collission tempData;
								bool stillColliding = OBBOBBCollission(&ep.one->physics->obb, &ep.two->physics->obb, tempData);
								if (stillColliding && AbsoluteValue(tempData.distanceIntoEachOther) > 0.001f){
									/*
									std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
									std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
									std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
									*/
									std::cout<<"\nWARNING: Entities at "<<ep.one->position<<" and "<<ep.two->position<<" are still colliding after collission resolution! Distance: "<<tempData.distanceIntoEachOther;
									// Move them apart as needed.
								/*	Entity * staticEntity = NULL;
									if (one->physics->type == PhysicsType::STATIC)
										staticEntity = one;
									if (two->physics->physicsType == PhysicsType::STATIC)
										staticEntity = two;
									if (staticEntity){
										/// Move away the dynamic entity.
										dynamicEntity->
									}
									*/
									// Pause if prompted.
									bool pauseOnStillColliding = false;
									if (pauseOnStillColliding){
										assert(collissionData.one && collissionData.two);
										Pause();
									}
								}
							}
							//	assert(!stillColliding);
							/// If prompted, send it for rendering.
							if (Graphics.renderCollissions){
								lastCollission = collissionData;
							}
						}
                    }
                }
            }
            /// Octree check
            else if (checkType == OCTREE){
            	/// Check collissions in the entityCollissionOctree for each dynamic entity AFTER they've moved.
                /// This can be moved to the prior for-loop in order to more easily intercept fast entity-entity collissions if need be.
                // Process dynamic entities
                for (int i = 0; i < dynamicEntities.Size(); ++i){
                    Entity * entity = dynamicEntities[i];
                    Vector3f & vel = entity->physics->velocity;

					/// If simulation disabled, skip it.
//					if (!entity->physics->simulationEnabled)
//						continue;

             //       if (vel.MaxPart())
              //      	std::cout<<"\nVelocity pre: "<<vel;
                
                    /// Skip entities not interested in collissions.
                    if (!entity->physics->collissionsEnabled)
                        continue;

                    if (!entity || !entity->physics || !entity->physics->octreeNode){
                        assert(entity && entity->physics && entity->physics->octreeNode);
                    }
                    /// Repeat collission detection until an object is in rest again (outside collission range)
                    /// This in order to avoid graphical bugs from frame to frame !
                    bool colliding = true;
                    int collissionsTested = 0;
                    List<Collission> collissions;
    #define MAX_COLLISSIONS_PER_PHYSICS_FRAME	2
                    while (colliding){
                        assert(entity);
                        assert(entity->physics);
                        assert(entity->physics->octreeNode);
                        // If for some reason we're outside the root node now, stop processing us.
                        if (entity->physics->octreeNode == NULL){
                            colliding = false;
                            std::cout<<"\nEntity octreeNode invalid, stopping calculations for it.";
                            UnregisterEntity(entity);
                            continue;
                        }
                        int collissionChecksDone = entityCollissionOctree->FindCollissions(entity, collissions);
						/// Resolve collission if it was detected earlier!
                        /// Resolve all collissions?
                        for (int c = 0; c < collissions.Size(); ++c){
                    //	if (collissions.Size() > 0){

                            /// Should probably sort the collissions by distance and then process them, yaow?

                            // But get the one with the deepest collission distance for prioritization's sake!
                            int deepest = c;
                            Collission activeCollission = collissions[c];
                            float deepestDistance = activeCollission.distanceIntoEachOther;
                            /*
                            for (int c = 1; c < collissions.Size(); ++c){
                                activeCollission = collissions[c];
                                float dist = activeCollission.distanceIntoEachOther;
                            //	float dist = (collissions[c].one->position - collissions[c].two->position).Length();
                                if (collissions[c].distanceIntoEachOther < deepestDistance)
                                {
                                    deepest = c;
                                    deepestDistance = dist;
                                }
                            }*/
                            Collission * deepestCollission = &collissions[deepest];
                    //		assert(collissions[deepest].collissionNormal.MaxPart() > ZERO && "CollissionNormal ZERO, is this really the intent?");
                            bool resolveResult = ResolveCollission(collissions[deepest]);
                            if (Graphics.renderCollissionTriangles)
                                activeTriangles += collissions[deepest].activeTriangles;
                            /// It was a false positive earlier (like normal in same direction as velocity)
                            if (resolveResult == false)
                                colliding = false;
                            // collissions.Clear();
                            colliding = false;
                        }
                        colliding = false;
                //		else
                //			colliding = false;
                        ++collissionsTested;
                        if (collissionsTested >= MAX_COLLISSIONS_PER_PHYSICS_FRAME){
                            break;
                        }
                    }

//                    if (vel.MaxPart())
  //                  std::cout<<"\nVelocity post: "<<vel;
                }

			}
			collissionTimer.Stop();
			collissionProcessingFrameTime += collissionTimer.GetMs();
		}
	}
}
