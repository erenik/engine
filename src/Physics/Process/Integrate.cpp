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

#include "Graphics/FrameStatistics.h"

void PhysicsManager::Integrate(float timeInSecondsSinceLastUpdate)
{
    assert(timeInSecondsSinceLastUpdate > 0);
	Timer timer;

	/// Not to be confused with physicsTimeMs;
	Time now = Time::Now();
	/// If it uses estimation, process that now.
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
		Entity * entity = physicalEntities[i];
		if (entity->physics->estimationEnabled){	
			entity->physics->estimator->Estimate(now, false);
		}
	}

	if (physicsIntegrator)
	{
		physicsIntegrator->IsGood();
		physicsIntegrator->IntegrateDynamicEntities(dynamicEntities, timeInSecondsSinceLastUpdate);
		physicsIntegrator->IntegrateKinematicEntities(kinematicEntities, timeInSecondsSinceLastUpdate);
		physicsIntegrator->RecalculateMatrices(fullyDynamicEntities);
		FrameStats.physicsIntegration += physicsIntegrator->integrationTimeMs;
		FrameStats.physicsIntegrationRecalcMatrices += physicsIntegrator->entityMatrixRecalcMs;
	}
	// Old integrators built into the physics-manager.
	else 
	{
		timer.Start();
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
		timer.Stop();
		int64 ms = timer.GetMs();
		FrameStats.physicsIntegration += ms;
	}
//	std::cout<<"\nIntegration: "<<ms;

	timer.Start();
	//             if (checkType == OCTREE)	entityCollisionOctree->RepositionEntity(dynamicEntity);
	
	/// Reposition the entities as appropriate within the optimization structures.
	RecalculateAABBs();
	timer.Stop();
	int ms = timer.GetMs();
	FrameStats.physicsRecalcAABBs += ms;
//	std::cout<<"\nRe-calculating AABBs: "<<ms;

	bool recalculateOBBs = false;
	if (recalculateOBBs)
	{
		timer.Start();
		RecalculateOBBs();
		timer.Stop();
		ms = timer.GetMs();
		FrameStats.physicsRecalcOBBs += ms;
//		std::cout<<"\nRe-calculating OBBs: "<<ms;
	}

	timer.Start();
	/// Reposition the entities as appropriate within the optimization structures.
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		PhysicsProperty * pp = dynamicEntity->physics;
        Vector3f vel = dynamicEntity->physics->velocity;
		/// Re-calculate physical radius.
		if (pp->recalculatePhysicalRadius)
			pp->physicalRadius = dynamicEntity->radius * dynamicEntity->scale.MaxPart();
    //    std::cout<<"\nPost-positioning Velocity: "<<dynamicEntity->physics->velocity;
        /// Ensure that the movement didn't adjust the velocity...
        assert(vel[0] == dynamicEntity->physics->velocity[0]);
	}
	timer.Stop();
	ms = timer.GetMs();
	FrameStats.physicsRecalcProps += ms;
//	std::cout<<"\nRepositioning and re-calculating properties: "<<ms;

}