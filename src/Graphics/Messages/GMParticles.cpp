/// Emil Hedemalm
/// 2014-06-26
/// Messages related to particles and particle-systems

#include "GMParticles.h"

#include "Graphics/GraphicsManager.h"

#include "GraphicsMessages.h"
#include "../GraphicsProperty.h"
#include "Graphics/Particles/ParticleSystem.h"
#include "File/LogFile.h"
#include "Model/Model.h"

#include "TextureManager.h"

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

GMRegisterParticleSystem::GMRegisterParticleSystem(ParticleSystem * ps, bool global)
: GraphicsMessage(GM_REGISTER_PARTICLE_SYSTEM), ps(ps), global(global)
{
}
void GMRegisterParticleSystem::Process()
{
	GraphicsMan.RegisterParticleSystem(ps, global);
}

GMUnregisterParticleSystem::GMUnregisterParticleSystem(ParticleSystem * ps, bool deleteOnUnregister)
	: GraphicsMessage(GM_UNREGISTER_PARTICLE_SYSETM), ps(ps), deleteOnUnregister(deleteOnUnregister)
{
}
void GMUnregisterParticleSystem::Process()
{
	GraphicsMan.UnregisterParticleSystem(ps);
	if (ps && deleteOnUnregister)
		delete ps;
}

GMAttachParticleEmitter::GMAttachParticleEmitter(ParticleEmitter * pe, ParticleSystem * ps)
: GraphicsMessage(GM_ATTACH_PARTICLE_EMITTER), pe(pe), ps(ps)
{

}
void GMAttachParticleEmitter::Process()
{
	// Return if already attached?
	if (ps->emitters.Exists(pe))
		return;
	ps->emitters.Add(pe);
	pe->AttachTo(ps);
}

GMDetachParticleEmitter::GMDetachParticleEmitter(ParticleEmitter * pe)
	: GraphicsMessage(GM_ATTACH_PARTICLE_EMITTER), pe(pe), ps(NULL)
{
}
GMDetachParticleEmitter::GMDetachParticleEmitter(ParticleEmitter * pe, ParticleSystem * ps)
	: GraphicsMessage(GM_ATTACH_PARTICLE_EMITTER), pe(pe), ps(ps)
{
}
void GMDetachParticleEmitter::Process()
{
	if (ps)
	{
		ps->emitters.Remove(pe);
		pe->particleSystems.Remove(ps);
	}
	else 
	{
		for (int i = 0; i < pe->particleSystems.Size(); ++i)
		{
			ParticleSystem * ps = pe->particleSystems[i];
			ps->emitters.Remove(pe);
		}
		pe->particleSystems.Clear();
	}
}


GMClearParticles::GMClearParticles(ParticleSystem * inSystem)
	: GraphicsMessage(GM_CLEAR_PARTICLES), ps(inSystem)
{
}
void GMClearParticles::Process()
{
	ps->ClearParticles();
}



GMPauseEmission::GMPauseEmission(Entity * entity) 
: GraphicsMessage(GM_PAUSE_EMISSION), entity(entity)
{
	assert(entity);
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
GMSetParticleEmitter::GMSetParticleEmitter(ParticleSystem * ps, const Contour & contour)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), ps(ps), contour(contour), type(CONTOUR)
{

}
GMSetParticleEmitter::GMSetParticleEmitter(ParticleSystem * ps, List<ParticleEmitter*> newEmitters)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), ps(ps), newEmitters(newEmitters), type(NEW_EMITTER_LIST)
{
	target = GT_SET_PARTICLE_EMITTER_OF_PARTICLE_SYSTEM;
}

GMSetParticleEmitter::GMSetParticleEmitter(ParticleEmitter * emitter, int target, ConstVec3fr vec3fValue)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), target(target), emitter(emitter), vec3fValue(vec3fValue)
{
	switch(target)
	{
		case GT_EMITTER_POSITION:
		case GT_EMITTER_DIRECTION:
		case GT_EMITTER_POSITION_OFFSET:
			break;
		default:
			assert(false);
	}
}

GMSetParticleEmitter::GMSetParticleEmitter(ParticleEmitter * emitter, int target, Entity * entity)
: GraphicsMessage(GM_SET_PARTICLE_EMITTER), target(target), emitter(emitter), entity(entity)
{
	switch(target)
	{
		case GT_EMITTER_ENTITY_TO_TRACK:
			break;
		default:
			assert(false);
	}
}
	
void GMSetParticleEmitter::Process()
{
	switch(target)
	{
		case GT_EMITTER_POSITION:
			emitter->position = vec3fValue;
			break;
		case GT_EMITTER_DIRECTION:
			emitter->direction = vec3fValue;
			break;
		// track
		case GT_EMITTER_ENTITY_TO_TRACK:
			emitter->entityToTrack = entity;
			break;
		case GT_EMITTER_POSITION_OFFSET:
			emitter->positionOffset = vec3fValue;
			break;
		case GT_SET_PARTICLE_EMITTER_OF_PARTICLE_SYSTEM:
		{
			switch(type)
			{
				case CONTOUR:
					ps->SetEmitter(contour);
					break;
				case NEW_EMITTER_LIST:
					ps->SetEmitter(newEmitters);
					break;
				default:
					assert(false);
			}
			break;
		}
		default:
			assert(false);
	}
}


GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, ConstVec3fr vec3Value)
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
	case GT_PARTICLE_SCALE:
	case GT_PARTICLE_LIFE_TIME:
		break;
	default:
		assert(false);
	}
}

GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, int iValue)
	: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), iValue(iValue)
{
	switch(target)
	{
		case GT_EMISSIONS_PER_SECOND:
		case GT_BLEND_EQUATION:
			break;
		default:
			assert(false);
	}
}

GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, bool bValue)
	: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), bValue(bValue)
{
	switch(target)
	{
		case GT_USE_INSTANCED_RENDERING:
			break;
		default:
			assert(false);
	}
}


GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, String sValue)
: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), sValue(sValue)
{
	switch(target)
	{
		case GT_PARTICLE_TEXTURE:
			break;
		default:
			assert(false);
	}
}

GMSetParticleSystem::GMSetParticleSystem(ParticleSystem * ps, int target, Model * model)
: GraphicsMessage(GM_SET_PARTICLE_SYSTEM), ps(ps), target(target), model(model)
{
	switch(target)
	{
		case GT_PARTICLE_MODEL:
			break;
		default:
			assert(false);
	}
}



void GMSetParticleSystem::Process()
{
	switch(target)
	{
		case GT_PARTICLE_MODEL:
		{
			if (!model)
			{
				LogGraphics("Bad Model!", ERROR);
				return;
			}
			model->BufferizeIfNeeded();
			ps->model = model;
			break;	
		}
		case GT_EMISSIONS_PER_SECOND:
		{
			ps->emissionsPerSecond = iValue;
			break;
		}
		case GT_BLEND_EQUATION:
		{
			ps->blendEquation = iValue;
			break;
		}
		case GT_PARTICLE_LIFE_TIME:
		{
			ps->particleLifeTime = fValue;
			break;
		}
		case GT_PARTICLE_TEXTURE:
		{
			Texture * tex = TexMan.GetTexture(sValue);
			if (tex)
			{
				ps->diffuse = tex;
			}
			break;
		}
		case GT_PARTICLE_INITIAL_COLOR:
			ps->color = vec3Value;
			break;
		case GT_PARTICLE_EMISSION_VEOCITY:
			ps->SetEmissionVelocity(fValue);
			break;
		case GT_PARTICLE_SCALE:
			ps->particleSize = fValue;
			break;
		case GT_USE_INSTANCED_RENDERING:
			ps->useInstancedRendering = bValue;
			break;
		default:
			assert(false);
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


