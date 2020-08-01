// Emil Hedemalm
// 2020-08-01
// Thrust emitter made to produce more particles in the center of a cone and less to the sides, as well as interpolating between the positions of this and previous frames to get a smooth emission.

#pragma once

#include "ParticleEmitter.h"

class ThrustEmitter : public ParticleEmitter {

public:
	// Initial pos to make the position smoothing work.
	ThrustEmitter(Vector3f initialPosition);
	virtual ~ThrustEmitter();
	/// Set default stats for Sparks
	void Nullify();

	/// Called each frame to update position and other according to automations (if any)
	virtual void Update() override;

	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity) override;
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color) override;

	void SetRatioRandomVelocity(float part);

private:
	Vector3f previousPosition, diffVector;

	float velRandPart, velConstPart;

	static Random velocityRandom;
	static Random lifeTimeRandom;
};
