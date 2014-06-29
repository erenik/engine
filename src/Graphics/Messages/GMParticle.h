/// Emil Hedemalm
/// 2014-06-26
/// Messages related to particles and particle-systems

#include "GraphicsMessage.h"
#include "Contour.h"
class ParticleSystem;
class ParticleEmitter;

// Attaches target particle system to an entity.
class GMAttachParticleSystem : public GraphicsMessage 
{
public:
	GMAttachParticleSystem(Entity * entity, ParticleSystem * pa);
	virtual void Process();
private:
	Entity * entity;
	ParticleSystem * pa;
};

class GMPauseEmission : public GraphicsMessage 
{
public:
	GMPauseEmission(Entity * entity);
	virtual void Process();
private:
	Entity * entity;
};
class GMResumeEmission : public GraphicsMessage 
{
public:
	GMResumeEmission(Entity * entity);
	virtual void Process();
private:
	Entity * entity;
};

// Sets the initial emitter shape/mesh of a particle system.
class GMSetParticleEmitter : public GraphicsMessage 
{
public:
	/// Sets a (2D) contour to be the emitter shape.
	GMSetParticleEmitter(ParticleSystem * ps, Contour contour);
	GMSetParticleEmitter(ParticleSystem * ps, List<ParticleEmitter*> newEmitters);
	virtual void Process();
private:
	ParticleSystem * ps;
	enum {
		CONTOUR,
		NEW_EMITTER_LIST,
	};
	int type;
	Contour contour;
	List<ParticleEmitter*> newEmitters;
};

