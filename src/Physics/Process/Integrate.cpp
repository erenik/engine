/// Emil Hedemalm
/// 2013-10-23
/// The main integrator-handling function that increases accelerations, momentums, 
/// re-calculates velocities, moves entities and re-places them in the collission detection structures.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"
#include "Physics/Integrator.h"

#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/OBB.h"


void PhysicsManager::Integrate(float timeInSecondsSinceLastUpdate)
{
    assert(timeInSecondsSinceLastUpdate > 0);

	if (physicsIntegrator)
	{
		physicsIntegrator->IntegrateDynamicEntities(dynamicEntities, timeInSecondsSinceLastUpdate);
		physicsIntegrator->IntegrateKinematicEntities(kinematicEntities, timeInSecondsSinceLastUpdate);
	}
	// Old integrators built into the physics-manager.
	else 
	{
		float timeSinceLastUpdate = timeInSecondsSinceLastUpdate;
		// Process dynamic entities
		for (int i = 0; i < dynamicEntities.Size(); ++i)
		{
			Entity * dynamicEntity = dynamicEntities[i];
			PhysicsProperty * physics = dynamicEntity->physics;

			/// If it uses estimation, process that now.
			if (dynamicEntity->physics->estimationEnabled){	
				dynamicEntity->physics->estimator->Process();
			}


			/// If simulation disabled, skip it.
			if (!dynamicEntity->physics->simulationEnabled)
				continue;

			bool own = false;
			bool coursePhysics = !own;
			bool customPhysics = own;
			/// Physics as calculated using simplified physics of only velocities and no acceleration forces whatsoever.
			if (integratorType == IntegratorType::SIMPLIFIED_PHYSICS)
			{
			  //  std::cout<<"\nSIMPLIFIED_PHYSICS";
				/// Grab pointers
				PhysicsProperty * physics = dynamicEntity->physics;
				if (physics->velocity.MaxPart()){
				//    std::cout<<"\nVelocity: "<<physics->velocity;
				}
				dynamicEntity->position += physics->velocity * timeSinceLastUpdate;
				dynamicEntity->RecalculateMatrix();
			}
			/// Physics as calculated with strict Rigid body physics
			else if (integratorType == IntegratorType::LAB_PHYSICS)
			{
				LabPhysicsIntegrate(dynamicEntity, timeSinceLastUpdate);
			}
			//////////////////////////////////////////////////////////////////////////////////////////////////
			/// Physics as I designed it earlier.
			//////////////////////////////////////////////////////////////////////////////////////////////////
			else if (integratorType == IntegratorType::APPROXIMATE)
			{
				ApproximateIntegrate(dynamicEntity, timeSinceLastUpdate);
			} /// End of custom physics step calculation

			// Recalculate the matrix!
			dynamicEntity->RecalculateMatrix();


		}
	}
	/// Reposition the entities as appropriate within the optimization structures.
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		PhysicsProperty * pp = dynamicEntity->physics;
        Vector3f vel = dynamicEntity->physics->velocity;
        /// Update octree or AABB position as needed.
        if (dynamicEntity->physics->collissionsEnabled){
            if (checkType == OCTREE)
                this->entityCollisionOctree->RepositionEntity(dynamicEntity);
            else if (checkType == AABB_SWEEP){
                /// Recalculate AABB
                dynamicEntity->physics->aabb->Recalculate(dynamicEntity);
                dynamicEntity->physics->obb->Recalculate(dynamicEntity);
            }
        }

		/// Re-calculate physical radius.
		pp->physicalRadius = dynamicEntity->radius * dynamicEntity->scale.MaxPart();

    //    std::cout<<"\nPost-positioning Velocity: "<<dynamicEntity->physics->velocity;
        /// Ensure that the movement didn't adjust the velocity...
        assert(vel.x == dynamicEntity->physics->velocity.x);
	}
}