/// Emil Hedemalm
/// 2014-02-07
/// Entity physics estimator

#include "EntityPhysicsEstimator.h"
#include "Timer/Timer.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"

EntityPhysicsEstimator::EntityPhysicsEstimator(Entity * owner)
: owner(owner), positionEstimator(1000), rotationEstimator(1000), velocityEstimator(1000)
{
	/// 100 delay + some ms to compensate?
	estimationDelay = 100;
	estimationMode = EstimationMode::INTERPOLATION;
	smoothingDuration = 100;
	lastTime = 0;
	positionEstimator.EnableExtrapolation();
	rotationEstimator.EnableExtrapolation();
	velocityEstimator.EnableExtrapolation();
}
/// Calculates and sets new values for the entity.
void EntityPhysicsEstimator::Process(){
	
	long long cTime = Timer::GetCurrentTimeMs();
	/// Set same estimation mode and estimation delay.
#define SetDefaults(a) a.mode = estimationMode;\
	a.synchronizationDelay = estimationDelay;\
	a.smoothingDuration = smoothingDuration;

	SetDefaults(positionEstimator);
	SetDefaults(rotationEstimator);
	SetDefaults(velocityEstimator);

	owner->positionVector = positionEstimator.Calculate(cTime);
	owner->rotationVector = rotationEstimator.Calculate(cTime);
	owner->physics->velocity = velocityEstimator.Calculate(cTime);
	owner->RecalculateMatrix();

	
	
//	std::cout<<"\nRotationVector: "<<owner->rotationVector;

	/// Calculate an average velocity.
	float divisor = (cTime - lastTime) / 1000.0f;
	if (divisor == 0)
		return;
	Vector3f velocity = (owner->positionVector - lastPosition) / (divisor);
//	owner->physics->velocity = velocity;

	/// Save variables for calculatiang average velocity next loop.
	lastPosition = owner->positionVector;
	lastTime = cTime;
	
//	std::cout<<"\nPosition: "<<(int)owner->positionVector.x<<" "<<(int)owner->positionVector.y<<" "<<(int)owner->positionVector.z;
}
//	EstimatorVec3f positionEstimator, rotationEstimator;


/// Adds a position using current time to that estimator.
void EntityPhysicsEstimator::AddPosition(Vector3f pos, long long timeStamp){
	positionEstimator.AddState(pos, timeStamp);
}
/// Adds a rotation using current time to that estimator.
void EntityPhysicsEstimator::AddRotation(Vector3f rot, long long timeStamp){
	rotationEstimator.AddState(rot, timeStamp);
}
/// Adds a velocity using current time to the estimator.
void EntityPhysicsEstimator::AddVelocity(Vector3f vel, long long timeStamp){
	velocityEstimator.AddState(vel, timeStamp);
}