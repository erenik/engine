/// Emil Hedemalm
/// 2014-06-26
/// Messages related to particles and particle-systems

#ifndef GM_PARTICLES_H
#define GM_PARTICLES_H

#include "GraphicsMessage.h"
#include "PhysicsLib/Shapes/Contour.h"
#include "Graphics/Particles/ParticleSystem.h"

class ParticleSystem;
class ParticleEmitter;

// Attaches target particle system to an entity.
class GMAttachParticleSystem : public GraphicsMessage 
{
public:
	GMAttachParticleSystem(EntitySharedPtr entity, ParticleSystemWeakPtr pa);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	EntitySharedPtr entity;
	ParticleSystemWeakPtr pa;
};

/** Registers a particle system for global operation.
	Are these deleted later on? Probably shouldn't.
*/
class GMRegisterParticleSystem : public GraphicsMessage 
{
public:
	GMRegisterParticleSystem(ParticleSystemWeakPtr ps, bool global = true);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	ParticleSystemWeakPtr ps;
	bool global;
};

class GMUnregisterParticleSystem : public GraphicsMessage 
{
public:
	GMUnregisterParticleSystem(std::weak_ptr<ParticleSystem> ps, bool deleteOnUnregister);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	std::weak_ptr<ParticleSystem> ps;
	bool deleteOnUnregister;
};

/// Attaches a particle emitter to a specific particle system to dictate where new particles should emerge.
class GMAttachParticleEmitter : public GraphicsMessage 
{
public:
	GMAttachParticleEmitter(std::shared_ptr<ParticleEmitter> pe, ParticleSystemWeakPtr ps);
	virtual void Process(GraphicsState* graphicsState) override; 
private:
	ParticleSystemWeakPtr ps;
	std::shared_ptr<ParticleEmitter> pe;
};

class GMDetachParticleEmitter : public GraphicsMessage 
{
public:
	GMDetachParticleEmitter(std::shared_ptr<ParticleEmitter> pe);
	GMDetachParticleEmitter(std::shared_ptr<ParticleEmitter> pe, ParticleSystemWeakPtr ps);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	ParticleSystemWeakPtr ps;
	std::shared_ptr<ParticleEmitter> pe;
};

class GMClearParticles : public GraphicsMessage 
{
public:
	GMClearParticles(std::weak_ptr<ParticleSystem> inSystem);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	std::weak_ptr<ParticleSystem> ps;
};

class GMPauseEmission : public GraphicsMessage 
{
public:
	GMPauseEmission(EntitySharedPtr entity);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	EntitySharedPtr entity;
};
class GMResumeEmission : public GraphicsMessage 
{
public:
	GMResumeEmission(EntitySharedPtr entity);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	EntitySharedPtr entity;
};

// Sets the initial emitter shape/mesh of a particle system.
class GMSetParticleEmitter : public GraphicsMessage 
{
public:
	/// Sets a (2D) contour to be the emitter shape.
	GMSetParticleEmitter(std::shared_ptr<ParticleSystem> ps, const Contour & contour);
	GMSetParticleEmitter(std::shared_ptr<ParticleSystem> ps, List<std::shared_ptr<ParticleEmitter>> newEmitters);
	GMSetParticleEmitter(std::shared_ptr<ParticleEmitter> emitter, int target, const Vector3f & vec3fValue);
	GMSetParticleEmitter(std::shared_ptr<ParticleEmitter> emitter, int target, EntitySharedPtr vec3fValue);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	std::shared_ptr<ParticleSystem> ps;
	enum {
		CONTOUR,
		NEW_EMITTER_LIST,
	};
	Vector3f vec3fValue;
	EntitySharedPtr entity;
	int target;
	int type;
	Contour contour;
	std::shared_ptr<ParticleEmitter> emitter; // For when setting settings for a single emitter
	List<std::shared_ptr<ParticleEmitter>> newEmitters;
};


class GMSetParticleSystem : public GraphicsMessage
{
public:
	GMSetParticleSystem(ParticleSystem * ps, int target, const Vector3f & vec3Value);
	GMSetParticleSystem(ParticleSystem * ps, int target, float fValue);
	GMSetParticleSystem(ParticleSystem * ps, int target, int iValue);
	GMSetParticleSystem(ParticleSystem * ps, int target, bool bValue);
	GMSetParticleSystem(ParticleSystem * ps, int target, String sValue);
	GMSetParticleSystem(ParticleSystem * ps, int target, Model * model);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	ParticleSystem * ps;
	Model * model;
	int target;
	Vector3f vec3Value;
	float fValue;
	bool bValue;
	String sValue;
	int iValue;
};


// Got generating particles using specific data.
class GMGenerateParticles : public GraphicsMessage {
public:
	GMGenerateParticles(String particleTypeName, void * extraData);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	String name;
	void * data;
};

#endif
