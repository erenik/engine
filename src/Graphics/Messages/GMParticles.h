/// Emil Hedemalm
/// 2014-06-26
/// Messages related to particles and particle-systems

#include "GraphicsMessage.h"
#include "PhysicsLib/Shapes/Contour.h"

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

/** Registers a particle system for global operation.
	Are these deleted later on? Probably shouldn't.
*/
class GMRegisterParticleSystem : public GraphicsMessage 
{
public:
	GMRegisterParticleSystem(ParticleSystem * pa);
	virtual void Process();
private:
	ParticleSystem * pa;
};

/// Attaches a particle emitter to a specific particle system to dictate where new particles should emerge.
class GMAttachParticleEmitter : public GraphicsMessage 
{
public:
	GMAttachParticleEmitter(ParticleEmitter * pe, ParticleSystem * pa);
	virtual void Process();
private:
	ParticleSystem * ps;
	ParticleEmitter * pe;
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


class GMSetParticleSystem : public GraphicsMessage
{
public:
	GMSetParticleSystem(ParticleSystem * ps, int target, Vector3f vec3Value);
	GMSetParticleSystem(ParticleSystem * ps, int target, float fValue);
	virtual void Process();
private:
	ParticleSystem * ps;
	int target;
	Vector3f vec3Value;
	float fValue;
};


// Got generating particles using specific data.
class GMGenerateParticles : public GraphicsMessage {
public:
	GMGenerateParticles(String particleTypeName, void * extraData);
	void Process();
private:
	String name;
	void * data;
};


