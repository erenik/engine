// Emil Hedemalm
// 2020-08-2
// Emitter which takes position update between frames into account for position at least

#pragma once

#include "ParticleEmitter.h"

class SmoothedPositionParticleEmitter : public ParticleEmitter {
public:
	SmoothedPositionParticleEmitter(Vector3f initialPosition);

	virtual void Update() override;
	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity) override;
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color) override;

private:

	Vector3f previousPosition, diffVector;
};
