// Emil Hedemalm
// 2020-08-01
// Thrust emitter made to produce more particles in the center of a cone and less to the sides, as well as interpolating between the positions of this and previous frames to get a smooth emission.

#include "ThrustEmitter.h"

ThrustEmitter::ThrustEmitter(Vector3f initialPosition)
	: ParticleEmitter(EmitterType::POINT_DIRECTIONAL) 
	, previousPosition(initialPosition)
{
	newType = true;
	positionEmitter = Emitter(EmitterType::POINT);
	velocityEmitter = Emitter(EmitterType::WEIGHTED_CIRCLE_ARC_XY);
	velocityEmitter.arcLength = PI * 0.5f; // One quarter circle default?
	velocityEmitter.arcOffset = 0.5f;
	velocityEmitter.weight = 2.0f;

	// Example settings from Space shooter
	SetEmissionVelocity(8.f);
	particlesPerSecond = 40;
	velocityEmitter.arcOffset = PI;
	velocityEmitter.weight = 1.1f;
	entityToTrack = nullptr;
	SetParticleLifeTime(0.5f);
	SetScale(0.15f);
	SetColor(Vector4f(0.1f, 0.5f, 1.0f, 0.5f));
	SetRatioRandomVelocity(0.5f);

}
ThrustEmitter::~ThrustEmitter() {}

/// Set default stats 
void ThrustEmitter::Nullify() {

}

/// Called each frame to update position and other according to automations (if any)
void ThrustEmitter::Update() {
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
bool ThrustEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity) {
	bool result = ParticleEmitter::GetNewParticle(position, velocity);
	position += diffVector * rand() * MathLib::OneDivRandMaxFloat();
	velocity *= 0.5f + 0.5f * rand() * MathLib::OneDivRandMaxFloat();
	return result;
}
/// Extended particle emission.
bool ThrustEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color) {
	bool result = ParticleEmitter::GetNewParticle(position, velocity, scale, lifeTime, color);
	position += diffVector * rand() * MathLib::OneDivRandMaxFloat();
	return result;
}

void ThrustEmitter::SetRatioRandomVelocity(float part) {

}
