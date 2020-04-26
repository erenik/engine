/// Emil Hedemalm
/// 2014-07-16
/// Separating the old shit into an own file for clarity's sake.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/AABBSweeper.h"


/// Old stuff
void PhysicsManager::DetectCollisions()
{
	static Timer collissionTimer;


    /// Check if we should process collissions at all.
    if (integratorType == IntegratorType::SIMPLE_PHYSICS){
        return;
    }

    /// Process collissions below..

	/// Check state flag
	if (!ignoreCollisions){
		collissionTimer.Start();
		/// AABB sweep check
        if (checkType == AABB_SWEEP){
	//		std::cout<<"\nAABB_SWEEP";
			
            /// First: Reset collission flags for all entities.
            for (int i = 0; i < physicalEntities.Size(); ++i){
                EntitySharedPtr e = physicalEntities[i];
                e->physics->collissionState = AABB_IDLE;
            }

            List<EntityPair> pairs;
            pairs.Clear();
            pairs = aabbSweeper->Sweep();
//               aabbSweeper->PrintSortedList();
    //           std::cout<<"\nBroad phase AABB sweep pairs: "<<pairs.Size();
            for (int i = 0; i < pairs.Size(); ++i)
			{
                /// For now just mark both as intersecting.
                EntityPair ep = pairs[i];
				EntitySharedPtr one = ep.one, two = ep.two;
                ep.one->physics->collissionState = AABB_INTERSECTING;
                ep.two->physics->collissionState = AABB_INTERSECTING;

				// If both are in rest/static, skip them.
				if ((one->physics->state & CollisionState::IN_REST || one->physics->type == PhysicsType::STATIC) && 
					(two->physics->state & CollisionState::IN_REST || two->physics->type == PhysicsType::STATIC))
					continue;

				/// TODO: Do Narrow-phase!
                Collision collissionData;
                bool collissionImminent = OBBOBBCollision(ep.one->physics->obb, ep.two->physics->obb, collissionData);
                if (collissionImminent){
                    /// Set them to render as collidididiing!
                    ep.one->physics->collissionState = COLLIDING;
                    ep.two->physics->collissionState = COLLIDING;
                    collissionData.one = ep.one;
                    collissionData.two = ep.two;
						
			//		std::cout<<"\n\nCollision imminent between "<<one->name<<" "<<one->position<<" and "<<two->name<<" "<<two->position<<".";

					/*
					if (ep.one->position.DotProduct(collissionData.collisionNormal) > 
						ep.two->position.DotProduct(collissionData.collisionNormal))
					{
						collissionData.collisionNormal *= -1.0f;
					}
					*/
					// Pause if prompted.
					if (pauseOnCollision){
						assert(collissionData.one && collissionData.two);
						Pause();
					}
/*
                    std::cout<<"\n=?-----------------------------------------------------------------";
                    std::cout<<"\nCollision imminent between "<<one<<" at "<<one->position<<" and "<<two<<" at "<<two->position;
                    std::cout<<"\nCollisionData: one: "<<collissionData.one<<" two: "<<collissionData.two;
                    std::cout<<"\nPreliminary collission normal: "<<collissionData.preliminaryCollisionNormal;
*/
					lastCollision = collissionData;

					bool resolveCollisions = true;
					if (resolveCollisions)
					{
						bool resolveResult;
						resolveResult = ResolveCollision(collissionData);

						/// If response was applied, make sure they're not still colliding with each other.
						if (resolveResult){
						//	std::cout<<"\nCollision response applied.";
							collissionData.results |= RESOLVED;
								
							Collision tempData;
							bool stillColliding = OBBOBBCollision(ep.one->physics->obb, ep.two->physics->obb, tempData);
							if (stillColliding && AbsoluteValue(tempData.distanceIntoEachOther) > 0.001f){
								/*
								std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
								std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
								std::cout<<"\n UGUUUUUUUUUUUUUUUUUUUUUU";
								*/
								std::cout<<"\nWARNING: Entities at "<<ep.one->worldPosition<<" and "<<ep.two->worldPosition<<" are still colliding after collission resolution! Distance: "<<tempData.distanceIntoEachOther;
								// Move them apart as needed.
							/*	EntitySharedPtr staticEntity = NULL;
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
						// TODO: Evaluate, should work out though?
						/// If prompted, send it for rendering.
			//			if (ActiveViewport->renderCollisions){
							lastCollision = collissionData;
			//			}
					}
                }
            }
        }
        /// Octree check
        else if (checkType == OCTREE){
            /// Check collissions in the entityCollisionOctree for each dynamic entity AFTER they've moved.
            /// This can be moved to the prior for-loop in order to more easily intercept fast entity-entity collissions if need be.
            // Process dynamic entities
            for (int i = 0; i < dynamicEntities.Size(); ++i){
                EntitySharedPtr entity = dynamicEntities[i];
                Vector3f & vel = entity->physics->velocity;

				/// If simulation disabled, skip it.
//					if (!entity->physics->simulationEnabled)
//						continue;

            //       if (vel.MaxPart())
            //      	std::cout<<"\nVelocity pre: "<<vel;
                
                /// Skip entities not interested in collissions.
                if (!entity->physics->collisionsEnabled)
                    continue;

                if (!entity || !entity->physics || !entity->physics->octreeNode){
                    assert(entity && entity->physics && entity->physics->octreeNode);
                }
                /// Repeat collission detection until an object is in rest again (outside collission range)
                /// This in order to avoid graphical bugs from frame to frame !
                bool colliding = true;
                int collissionsTested = 0;
                List<Collision> collissions;
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
                    int collissionChecksDone = entityCollisionOctree->FindCollisions(entity, collissions);
					/// Resolve collission if it was detected earlier!
                    /// Resolve all collissions?
                    for (int c = 0; c < collissions.Size(); ++c){
                //	if (collissions.Size() > 0){

                        /// Should probably sort the collissions by distance and then process them, yaow?

                        // But get the one with the deepest collission distance for prioritization's sake!
                        int deepest = c;
                        Collision activeCollision = collissions[c];
                        float deepestDistance = activeCollision.distanceIntoEachOther;
                        /*
                        for (int c = 1; c < collissions.Size(); ++c){
                            activeCollision = collissions[c];
                            float dist = activeCollision.distanceIntoEachOther;
                        //	float dist = (collissions[c].one->position - collissions[c].two->position).Length();
                            if (collissions[c].distanceIntoEachOther < deepestDistance)
                            {
                                deepest = c;
                                deepestDistance = dist;
                            }
                        }*/
                        Collision * deepestCollision = &collissions[deepest];
                //		assert(collissions[deepest].collisionNormal.MaxPart() > ZERO && "CollisionNormal ZERO, is this really the intent?");
                        bool resolveResult;
						resolveResult = ResolveCollision(*deepestCollision);


						// TODO: Evaluate if this works. Always adding the triangles might be bad. Add as a boolean for the physics manager?
						/*
						assert(false);
						//   if (Graphics.renderCollisionTriangles)
                            activeTriangles += collissions[deepest].activeTriangles;
                        /// It was a false positive earlier (like normal in same direction as velocity)
                        if (resolveResult == false)
                            colliding = false;
							*/
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
//		collissionProcessingFrameTime += collissionTimer.GetMs();
	}
}
