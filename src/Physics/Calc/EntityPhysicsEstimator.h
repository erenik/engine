/// Emil Hedemalm
/// 2014-02-07
/// Entity physics estimator

#ifndef ENTITY_PHYSICS_ESTIMATOR_H
#define ENTITY_PHYSICS_ESTIMATOR_H

#include "PhysicsLib/Estimator.h"
class Entity;

/// Entity physics estimator
class EntityPhysicsEstimator : public Estimator {
public:
	EntityPhysicsEstimator(Entity * owner);
	/// Calculates and sets new values for the entity.
	virtual void Process();
	/// Adds a position using current time to that estimator.
	void AddPosition(Vector3f pos, long long timeStamp);
	/// Adds a rotation using current time to that estimator.
	void AddRotation(Vector3f rot, long long timeStamp);
	/// Adds a velocity using current time to the estimator.
	void AddVelocity(Vector3f vel, long long timeStamp);
	/// See EstimationMode enum
	int estimationMode;
	int estimationDelay;
	int smoothingDuration;
private:
	EstimatorVec3f positionEstimator, rotationEstimator, velocityEstimator;
	Entity * owner;

	Vector3f lastPosition;
	long long lastTime;
};

#endif
