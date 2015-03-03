/// Emil Hedemalm
/// 2015-01-22
/// Stars, as if gliding through space. Could also be just any arbitrary particles.

#include "Stars.h"
#include "Graphics/OpenGL.h"
#include "GraphicsState.h"
#include "Entity/Entity.h"
#include "Graphics/Camera/Camera.h"
#include "TextureManager.h"
#include "../GraphicsManager.h"
#include "../FrameStatistics.h"


StarEmitter::StarEmitter(ConstVec3fr point)
: ParticleEmitter(point)
{
	newType = true;
}

StarEmitter::~StarEmitter()
{

}

/// Default new particle.
bool StarEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	/// Fetch default position and velocity.
	ParticleEmitter::GetNewParticle(position, velocity);
	return true;
}

/// Extended particle emission.
bool StarEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity, float & newParticleScale, float & lifeTime, Vector4f & color)
{
	/// Fetch default values.
	ParticleEmitter::GetNewParticle(position, velocity, newParticleScale, lifeTime, color);
	/// But re-distribute the life time.
	lifeTime *= 1.f;
	return true;
}


/// Creates a global particle system
Stars::Stars(bool emitWithEmittersOnly)
: ParticleSystem("Stars", emitWithEmittersOnly)
{
}
	
/// Creates a particle system which will be attached to a specific entity.
Stars::Stars(Entity * reference, bool emitWithEmittersOnly)
: ParticleSystem("Sparks", emitWithEmittersOnly)
{
	/// Set link to entity.
	relativeTo = reference;
}

Stars::~Stars()
{
    std::cout<<"\nStars Destructor.....";
}

void Stars::Initialize()
{
	ParticleSystem::Initialize();
	SetAlphaDecay(DecayType::NONE);
}

		/// Integrates all particles.
void Stars::ProcessParticles(float & timeInSeconds)
{
#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	
#else // Not SSE_PARTICLES

	/// Move/Process all alive particles
	for (int i = 0; i < aliveParticles; ++i)
	{
		positions[i] += velocities[i] * timeInSeconds;
		lifeDurations[i] += timeInSeconds;
		// If duration has elapsed life-time..
		if (lifeDurations[i] > lifeTimes[i])
		{
			int lastIndex = aliveParticles - 1;
			// Kill it, by moving in the last used data to replace it.
			positions[i] = positions[lastIndex];
			velocities[i] = velocities[lastIndex];
			lifeDurations[i] = lifeDurations[lastIndex];
			colors[i] = colors[lastIndex];
			lifeTimes[i] = lifeTimes[lastIndex];
			scales[i] = scales[lastIndex];

			// Decrement i so we don't skip processing of the one we moved back.
			--i;
			// Decrement alive particles.
			--aliveParticles;
		}
	}
#endif
}

/// Update buffers to use when rendering.
void Stars::UpdateBuffers()
{
#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	
#else // Not SSE_PARTICLES
	ParticleSystem::UpdateBuffers();
	return;
	for (int i = 0; i < aliveParticles; ++i)
	{
		Vector3f & pos = positions[i];
		int index = i * 4;
		particlePositionSizeData[index] = pos[0];
		particlePositionSizeData[index+1] = pos[1];
		particlePositionSizeData[index+2] = pos[2];
//		particlePositionSizeData[index+3] = scales[i];

		Vector4f & color = colors[i];
		particleColorData[index] = color[0] * 255;
		particleColorData[index+1] = color[1] * 255;
		particleColorData[index+2] = color[2] * 255;
		particleColorData[index+3] = 255.f;
	}
#endif
}


void Stars::PrintData(){
}


void Stars::AttachTo(Entity * entity, ConstMat4r relativePosition)
{
	assert(false);
}
