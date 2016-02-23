/// Emil Hedemalm
/// 2014-09-19
/// Emitter dedicated to creating "spark"-like particles, as if something is on fire or creating heat-related sparks

#include "SparksEmitter.h"

Random SparksEmitter::velocityRandom;
Random SparksEmitter::lifeTimeRandom;

SparksEmitter::SparksEmitter()
: ParticleEmitter()
{
}

SparksEmitter::SparksEmitter(List<Triangle> triangles)
	: ParticleEmitter(triangles)
{
	Nullify();
	velocityEmitter.triangles = positionEmitter.triangles = &this->tris;
}

SparksEmitter::SparksEmitter(const Vector3f & point)
: ParticleEmitter(point)
{
	Nullify();
}

SparksEmitter::~SparksEmitter()
{

}

/// Set default stats for Sparks
void SparksEmitter::Nullify()
{
	velRandPart = 0.8f;
	velConstPart = 0.2f;
}

/// Default new particle.
bool SparksEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	/// Fetch default position and velocity.
	ParticleEmitter::GetNewParticle(position, velocity);
	/// Randomly distribute velocity along 20-100% of the max value.
	velocity *= velocityRandom.Randf(velRandPart) + velConstPart;
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


void SparksEmitter::SetRatioRandomVelocity(float part)
{
	velRandPart = part;
	velConstPart =  1 - part;
}
