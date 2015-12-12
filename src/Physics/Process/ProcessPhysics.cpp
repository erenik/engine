// Emil Hedemalm
// 2013-10-23
// The main iterator/integrator/collission-handling function.

#include "Message/Message.h"

#include "PhysicsState.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/CollisionResolver.h"
#include "Physics/CollisionDetector.h"
#include "Physics/Integrator.h"

#include "Physics/Messages/CollisionCallback.h"
#include "PhysicsLib/AABBSweeper.h"

#include "PhysicsLib/Estimator.h"
#include "Message/MessageManager.h"

#include "File/LogFile.h"
#include "Graphics/FrameStatistics.h"
// #include "Graphics/GraphicsManager.h"
//#include "Entity/Entity.h"

static Timer recalc, moving;


/// Processes physics for all registered objects
void PhysicsManager::ProcessPhysics()
{
	if (physicsState->simulationPaused)
		return;
	/// Returns straight away if paused.
	if (paused)
		return;
	if (physicalEntities.Size() == 0)
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

	// To be sent for Collision callback.
	List<Message*> messages;


	/// Do one process for each 10 ms we've gotten stored up
	/// Get sub-time to calculate.
	float dt = 0.010f * simulationSpeed;
	float timeDiff = dt;
	float timeInSecondsSinceLastUpdate = dt;

	static float timeRemainingFromLastIteration = 0.f;
	/// Add time from last iteration that wasn't spent (since only evaluating one physics step at a time, 10 ms default).
	float timeToIterate = totalTimeSinceLastUpdate + timeRemainingFromLastIteration;
	float stepSize = 0.010f;
	int steps = timeToIterate / stepSize + 0.5f;

	/// Use a new step size based on the amount of steps. This will hopefully vary between 5.0 and 15.0 then.
	float newStepSize = timeToIterate / steps;
	int newStepSizeMs = newStepSize * 1000;
	newStepSize = newStepSizeMs * 0.001f;
	if (newStepSize < 0.005f || newStepSize > 0.015f)
	{
		LogPhysics("Step size out of good range: "+String(newStepSize), WARNING);
		if (newStepSize < 0.f)
			return;
//		assert(False)
	}
//	assert(newStepSize > 0.005f && newStepSize < 0.015f);
	

//	if (steps < 1) // At least 1 physics simulation per frame, yo. Otherwise you get a 'stuttering' effect when some frames have movement and some don't.
//		steps = 1;
	float timeLeft = timeToIterate - steps * newStepSizeMs * 0.001f;
	/// Store time we won't simulate now.
	timeRemainingFromLastIteration = timeLeft;
//	std::cout<<"\nSteps: "<<steps;

	for(int i = 0; i < steps; ++i)
	{
		/// Set current time in physics for this frame. This time is not the same as real time.
		physicsNowMs += newStepSizeMs;
			
		/// Process estimators (if any) within all registered entities?
		int milliseconds = newStepSizeMs;
		for (int i = 0 ; i < physicalEntities.Size(); ++i)
		{
			Entity * entity = physicalEntities[i];
			List<Estimator*> & estimators = entity->physics->estimators;
			for (int j = 0; j < estimators.Size(); ++j)
			{
				Estimator * estimator = estimators[j];
				estimator->Process(milliseconds);
				if (entity->name == "ExplosionEntity")
					int lp = 5;
				// Recalculate other stuff too.
				entity->physics->UpdateProperties(entity);
				// Re-calculate transform matrix, as it was probably affected..?
				entity->RecalculateMatrix(Entity::ALL_PARTS);
				if (estimator->finished)
				{
					estimators.RemoveIndex(j, ListOption::RETAIN_ORDER);
					--j;
					delete estimator;
				}
			}
		}

		/// Awesome.
		Integrate(newStepSize);
		
		/// Apply external constraints
	//	ApplyContraints();		

		/// Apply pathfinding for all relevant entities
		ApplyPathfinding();
        
		Timer collisionTimer;
		collisionTimer.Start();

		Timer timer;
		timer.Start();
		/// Detect collisions.
		List<Collision> collisions;
		Timer sweepTimer;
		sweepTimer.Start();
		// Generate pair of possible collissions via some optimized way (AABB-sorting or Octree).
		List<EntityPair> pairs = this->aabbSweeper->Sweep();
		sweepTimer.Stop();
		int sweepDur = sweepTimer.GetMs();
//		std::cout<<"\nAABB sweep pairs: "<<pairs.Size()<<" with "<<physicalEntities.Size()<<" entities";
		Timer detectorTimer;
		detectorTimer.Start();
		if (collisionDetector)
		{
			collisionDetector->DetectCollisions(pairs, collisions);
		}
		
		// Old approach which combined collision-detection and resolution in a big mess...
		else 
			DetectCollisions();
		detectorTimer.Stop();
		int detectorMs = detectorTimer.GetMs();
		timer.Stop();
		int thisFrame = timer.GetMs();
		FrameStats.physicsCollisionDetection += thisFrame;

		timer.Start();
		/// And resolve them.
		if (collisionResolver)
			collisionResolver->ResolveCollisions(collisions);
		timer.Stop();
		FrameStats.physicsCollisionResolution += timer.GetMs();

		timer.Start();
		messages.Clear();
		for (int i = 0; i < collisions.Size(); ++i)
		{
			Collision & c = collisions[i];
			if (c.one->physics->onCollision)
				c.one->OnCollision(c);
			if (c.two->physics->onCollision)
				c.two->OnCollision(c);
			if (c.one->name == "ExplosionEntity" || c.two->name == "ExplosionEntity")
			{
				int p = 14;
			}
			if (c.one->physics->collisionCallback || c.two->physics->collisionCallback)
			{
				/// Check max callbacks.
				int & maxCallbacks1 = c.one->physics->maxCallbacks;
				int & maxCallbacks2 = c.two->physics->maxCallbacks;
				if (maxCallbacks1 == 0 || maxCallbacks2 == 0)
					continue;
				CollisionCallback * cc = new CollisionCallback(c.one, c.two);
				cc->impactNormal = c.collisionNormal;
				messages.Add(cc);
				if (maxCallbacks1 > 0)
					--maxCallbacks1;
				if (maxCallbacks2 > 0)
					--maxCallbacks2;
			}
		}
		if (messages.Size())
			MesMan.QueueMessages(messages);
		timer.Stop();
		FrameStats.physicsCollisionCallback += timer.GetMs();

		collisionTimer.Stop();
		FrameStats.physicsCollisions += collisionTimer.GetMs();
		int64 colMs = collisionTimer.GetMs();
		if (colMs > 50)
		{
			std::cout<<"\nCollision detection and resolution taking "<<colMs<<" milliseconds per frame.";
		}
	}
	// Recalc matrices for the semi-dynamic ones.
	physicsIntegrator->RecalculateMatrices(semiDynamicEntities);

	// Reset previous frame-times
//	FrameStats.physicsIntegration = integration;
	collissionProcessingFrameTime = 0;
	physicsMeshCollisionChecks = 0;
}
