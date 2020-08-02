// Emil Hedemalm
// 2020-08-2
// Emitter which takes position update between frames into account for position at least

#include "SmoothedPositionParticleEmitter.h"

SmoothedPositionParticleEmitter::SmoothedPositionParticleEmitter(Vector3f initialPosition)
: ParticleEmitter()
, previousPosition(initialPosition)
{

}

void SmoothedPositionParticleEmitter::Update() {
	if (previousPosition.MaxPart() == 0)
		previousPosition = position;

	// Save old position
	previousPosition = position;
	// Before updating it.
	ParticleEmitter::Update();
	// Update vector to interpolate apply to particles
	diffVector = previousPosition - position;

}

/// Default new particle.
bool SmoothedPositionParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity) {
	bool result = ParticleEmitter::GetNewParticle(position, velocity);
	position += diffVector * rand() * MathLib::OneDivRandMaxFloat();
	return result;
}

/// Extended particle emission.
bool SmoothedPositionParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color) {
	assert(false);
	return false;
}
