/// Emil Hedemalm
/// 2015-01-22
/// Stars, as if gliding through space. Could also be just any arbitrary particles.

#ifndef STARS_H
#define STARS_H

#include "ParticleSystem.h"

class Entity;

class StarEmitter : public ParticleEmitter 
{
public:
	StarEmitter(const Vector3f & point);
	virtual ~StarEmitter();
	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity);
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color);
private:
	Random colorRandom;
};

class Stars : public ParticleSystem 
{
public:
	/// Creates a global particle system
	Stars(bool emitWithEmittersOnly);
	/// Creates a particle system which will be attached to a specific entity.
    Stars(Entity * reference, bool emitWithEmittersOnly);
	virtual ~Stars();
	
	void InitStars(); 
		/// Integrates all particles.
	virtual void ProcessParticles(float & timeInSeconds);
	/// Update buffers to use when rendering.
	virtual void UpdateBuffers();

    void PrintData();
	void AttachTo(Entity * entity, Matrix4f relativePosition);

	// Relative to the entity.
	Vector4f relativePosition;
private:

	Vector3f previousPosition;
	Vector3f previousDirection;
};



#endif

