/// Emil Hedemalm
/// 2014-09-19
/// Emitter dedicated to creating "spark"-like particles, as if something is on fire or creating heat-related sparks
#pragma once

#include "ParticleEmitter.h"

class SparksEmitter : public ParticleEmitter
{
public:
	SparksEmitter();
	SparksEmitter(List<Triangle> triangles);
	SparksEmitter(const Vector3f & point);
	virtual ~SparksEmitter();
	/// Set default stats for Sparks
	void Nullify();

	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity);
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color);

	// Sets part of velocity to be random, e.g. 0.2, will make the constant part become 0.8
	void SetRatioRandomVelocity(float part);

private:
	float velRandPart, velConstPart;

	static Random velocityRandom;
	static Random lifeTimeRandom;
};




