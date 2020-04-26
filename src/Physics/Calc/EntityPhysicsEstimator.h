/// Emil Hedemalm
/// 2014-02-07
/// Entity physics estimator

#ifndef ENTITY_PHYSICS_ESTIMATOR_H
#define ENTITY_PHYSICS_ESTIMATOR_H

#include "PhysicsLib/EstimatorVec3f.h"
#include "Entity/Entity.h"

class Time;

/// Entity physics estimator
class EntityPhysicsEstimator : public Estimator {
public:
	EntityPhysicsEstimator(EntitySharedPtr owner);


	/** Estimates values for given time. If loop is true, the given time will be modulated to be within the interval of applicable time-values.
		If the estimator's output pointer is set, data for the given estimation will be written there accordingly.
	*/
	virtual void Estimate(const Time & forGivenTimeInMs, bool loop);
	/// New from base class.
	virtual void Process(int timeInMs);

	/// Calculates and sets new values for the entity.
	virtual void Process();
	/// Adds a position using current time to that estimator.
	void AddPosition(const Vector3f & pos, long long timeStamp);
	/// Adds a rotation using current time to that estimator.
	void AddRotation(const Vector3f & rot, long long timeStamp);
	/// Adds a velocity using current time to the estimator.
	void AddVelocity(const Vector3f & vel, long long timeStamp);
	/// Returns last calculated position. Do Note that this function will not update the position, only Process will do that.
	Vector3f Position();
	/// See EstimationMode enum
	int estimationMode;
	int estimationDelay;
	int smoothingDuration;
private:
	EstimatorVec3f positionEstimator, rotationEstimator, velocityEstimator;
	EntitySharedPtr owner;

	Vector3f lastPosition;
	long long lastTime;
};

#endif
