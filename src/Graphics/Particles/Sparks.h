/// Emil Hedemalm
/// 2014-06-26
/// Emits what may look like hot sparks, decaying over time into transparency.

#ifndef SPARKS_H
#define SPARKS_H

#include "ParticleSystem.h"

class Entity;

class Sparks : public ParticleSystem 
{
public:
	/// Creates a global particle system
	Sparks(bool emitWithEmittersOnly);
	/// Creates a particle system which will be attached to a specific entity.
    Sparks(Entity * reference, bool emitWithEmittersOnly);
	virtual ~Sparks();

	virtual void Initialize();
	/// Update buffers to use when rendering.
	virtual void UpdateBuffers();

	void PrintData();
	
	// Relative to the entity.
	Vector4f relativePosition;
private:

	Vector3f previousPosition;
	Vector3f previousDirection;
};



#endif







