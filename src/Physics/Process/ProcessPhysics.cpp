// Emil Hedemalm
// 2013-10-23
// The main iterator/integrator/collission-handling function.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/CollisionResolver.h"
#include "Physics/CollisionDetector.h"

#include "PhysicsLib/AABBSweeper.h"

#include "Graphics/FrameStatistics.h"

#include "Graphics/GraphicsManager.h"
//#include "Entity/Entity.h"

static Timer recalc, moving;


/// Processes physics for all registered objects
void PhysicsManager::ProcessPhysics()
{
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

	/// Debugging time statistics
	float messageProcessingTime = 0;
	float recalculatingPropertiesDuration = 0;
	float collissionProcessingFrameTime = 0;

	// Reset previous frame-times
	recalculatingPropertiesDuration = 0;
	float integration = 0;
	collissionProcessingFrameTime = 0;
	physicsMeshCollisionChecks = 0;

	/// Do one process for each 10 ms we've gotten stored up
	while (totalTimeSinceLastUpdate > ZERO){
		/// Get sub-time to calculate.
		float dt = 0.010f * simulationSpeed;
#define timeDiff    dt
#define timeInSecondsSinceLastUpdate dt
		if (totalTimeSinceLastUpdate < timeInSecondsSinceLastUpdate)
			timeInSecondsSinceLastUpdate = totalTimeSinceLastUpdate;
		totalTimeSinceLastUpdate -= timeInSecondsSinceLastUpdate;

		recalc.Start();
		// Begin by recalculating physics position and scales
	//	this->RecalculatePhysicsProperties();
		recalc.Stop();
		recalculatingPropertiesDuration += recalc.GetMs();

		moving.Start();
		/// Awesome.
		Integrate(timeInSecondsSinceLastUpdate);
		moving.Stop();
		integration += moving.GetMs();

		/// Apply external constraints
	//	ApplyContraints();
		
		/// Apply pathfinding for all relevant entities
		ApplyPathfinding();
        
		/// Detect collisions.
		List<Collision> collisions;
		if (collisionDetector)
		{
			collisionDetector->DetectCollisions(physicalEntities, collisions);
		}
		// Old approach which combined collision-detection and resolution in a big mess...
		else 
			DetectCollisions();

		
		/// And resolve them.
		if (collisionResolver)
			collisionResolver->ResolveCollisions(collisions);

	}

	// Reset previous frame-times
	FrameStats.physicsIntegration = integration;
	collissionProcessingFrameTime = 0;
	physicsMeshCollisionChecks = 0;
}
