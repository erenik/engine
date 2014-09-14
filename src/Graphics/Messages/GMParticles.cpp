/// Emil Hedemalm
/// 2014-06-26
/// Messages related to particles and particle-systems

#include "GMParticles.h"

#include "Graphics/GraphicsManager.h"

#include "GraphicsMessages.h"
#include "../GraphicsProperty.h"
#include "Graphics/Particles/ParticleSystem.h"

GMAttachParticleSystem::GMAttachParticleSystem(Entity * entity, ParticleSystem * pa)
	: GraphicsMessage(GM_ATTACH_PARTICLE_SYSTEM), entity(entity), pa(pa)
{
}
void GMAttachParticleSystem::Process()
{
	ADD_GRAPHICS_PROPERTY_IF_NEEDED(entity);
	// Add it.
	entity->graphics->particleSystems.Add(pa);
}

GMRegisterParticleSystem::GMRegisterParticleSystem(ParticleSystem * pa)
: GraphicsMessage(GM_REGISTER_PARTICLE_SYSTEM), pa(pa)
{
}
void GMRegisterParticleSystem::Process()
{
	GraphicsMan.globalParticleSystems.Add(pa);
}

GMAttachParticleEmitter::GMAttachParticleEmitter(ParticleEmitter * pe, ParticleSystem * ps)
: GraphicsMessage(GM_ATTACH_PARTICLE_EMITTER), pe(pe), ps(ps)
{

}
void GMAttachParticleEmitter::Process()
{
	ps->emitters.Add(pe);
}


GMPauseEmission::GMPauseEmission(Entity * entity) 
: GraphicsMessage(GM_PAUSE_EMISSION), entity(entity)
{
}
void GMPauseEmission::Process()
{
	if (!entity->graphics)
		return;
	for (int i = 0; i < entity->graphics->particleSystems.Size(); ++i)
	{
		ParticleSystem * ps = entity->graphics->particleSystems[i];
		ps->PauseEmission();
	}
}

GMResumeEmission::GMResumeEmission(Entity * entity)
: GraphicsMessage(GM_RESUME_EMISSION), entity(entity)
{
}

void GMResumeEmission::Process()
{
	if (!entity->graphics)
		return;
	for (int i = 0; i < entity->graphics->particleSystems.Size(); ++i)
	{
		ParticleSystem * ps = entity->graphics->particleSystems[i];
		ps->ResumeEmission();
	}
}

/// Sets a (2D) contour to be the emitter shape.
GMSetParticleEmitter::GMSetParticleEmitter(ParticleSystem * ps, Contour contour)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), ps(ps), contour(contour), type(CONTOUR)
{

}
GMSetParticleEmitter::GMSetParticleEmitter(ParticleSystem * ps, List<ParticleEmitter*> newEmitters)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), ps(ps), newEmitters(newEmitters), type(NEW_EMITTER_LIST)
{
}
	
void GMSetParticleEmitter::Process()
{
	switch(type)
	{
	case CONTOUR:
		ps->SetEmitter(contour);
		break;
	case NEW_EMITTER_LIST:
		ps->SetEmitter(newEmitters);
		break;
	}
}


GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, Vector3f vec3Value)
: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), vec3Value(vec3Value)
{
	switch(target)
	{
	case GT_PARTICLE_INITIAL_COLOR:
		break;
	default:
		assert(false);
	}
}
GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, float fValue)
	: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), fValue(fValue)
{
	switch(target)
	{
	case GT_PARTICLE_EMISSION_VEOCITY:
		break;
	default:
		assert(false);
	}
}

void GMSetParticleSystem::Process()
{
	switch(target)
	{
		case GT_PARTICLE_INITIAL_COLOR:
			ps->color = vec3Value;
			break;
		case GT_PARTICLE_EMISSION_VEOCITY:
			ps->emissionVelocity = fValue;
			break;
	}
}




GMGenerateParticles::GMGenerateParticles(String particleTypeName, void * extraData)
: GraphicsMessage(GM_GENERATE_PARTICLES), name(particleTypeName), data(extraData){
}

void GMGenerateParticles::Process(){
	name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (name == "CollisionSparks"){
	//	assert(false && "Do stuff!");
	}
	else {
		assert(false && "Undefined particle type name in GMGenerateParticles!");
	}
}


