// Emil Hedemalm
// 2013-07-14

#ifndef ENGINE_EXHAUST_H
#define ENGINE_EXHAUST_H

#include "ParticleSystem.h"

class Entity;

class Exhaust : public ParticleSystem {
public:
    Exhaust(Entity* reference);
    virtual ~Exhaust();
    void Process(float timeInSeconds);
    void Render(GraphicsState * graphicsState);
    void PrintData();

	// Relative to the entity.
	Vector4f relativePosition;
private:
    float primaryVelocity;
    float sideVelocityRange;

	Vector3f previousPosition;
	Vector3f previousDirection;

};



#endif

