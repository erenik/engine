/// Emil Hedemalm
/// 2014-06-26
/// Emits what may look like hot sparks, decaying over time into transparency.

#ifndef SPARKS_H
#define SPARKS_H

#include "ParticleSystem.h"

class Entity;

class Sparks : public ParticleSystem {
public:
    Sparks(Entity * reference);
    virtual ~Sparks();
    void Process(float timeInSeconds);
    void Render();
    void PrintData();
	void AttachTo(Entity * entity, Matrix4f relativePosition);

	// Relative to the entity.
	Vector4f relativePosition;
private:
    float primaryVelocity;
    float sideVelocityRange;

	Vector3f previousPosition;
	Vector3f previousDirection;

};



#endif







