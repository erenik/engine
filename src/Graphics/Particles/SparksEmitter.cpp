/// Emil Hedemalm
/// 2014-09-19
/// Emitter dedicated to creating "spark"-like particles, as if something is on fire or creating heat-related sparks

#include "SparksEmitter.h"

Random SparksEmitter::velocityRandom;
Random SparksEmitter::lifeTimeRandom;

SparksEmitter::SparksEmitter(Vector3f point)
: ParticleEmitter(point)
{

}

SparksEmitter::~SparksEmitter()
{

}


/// Default new particle.
bool SparksEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	/// Fetch default position and velocity.
	ParticleEmitter::GetNewParticle(position, velocity);
	/// Randomly distribute velocity along 20-100% of the max value.
	velocity *= velocityRandom.Randf(0.8f) + 0.2f;
	return true;
}

/// Extended particle emission.
bool SparksEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity, float & newParticleScale, float & lifeTime, Vector4f & color)
{
	/// Fetch default values.
	ParticleEmitter::GetNewParticle(position, velocity, newParticleScale, lifeTime, color);
	/// But re-distribute the life time.
	lifeTime *= lifeTimeRandom.Randf(0.7f) + 0.3f;
	return true;
}

