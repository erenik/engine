// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

GMRegisterEntity::GMRegisterEntity(Entity * i_entity) : GraphicsMessage(GM_REGISTER_ENTITY){
	entity = i_entity;
}

void GMRegisterEntity::Process()
{
	GraphicsManager & manager = GraphicsMan;
	assert(manager.vfcOctree);
	///	entity->model->mesh.Bufferize();
	entity->Bufferize();
	Graphics.RegisterEntity(entity);
}


GMRegisterEntities::GMRegisterEntities(Entities i_selection) : GraphicsMessage(GM_REGISTER_ENTITIES)
{
	selection = i_selection;
}

void GMRegisterEntities::Process()
{
	for (int i = 0; i < selection.Size(); ++i)
	{
		if (selection[i]->registeredForRendering){
			assert(!selection[i]->registeredForRendering);
		}
	}
	Graphics.RegisterEntities(selection);
}


GMRegister::GMRegister(List<ParticleSystem*> particleSystems)
: GraphicsMessage(GM_REGISTER), 
particleSystems(particleSystems),
target(GT_PARTICLE_SYSTEMS)
{
}

void GMRegister::Process()
{	
	switch(target){
		case GT_PARTICLE_SYSTEMS:
			Graphics.particleSystems += particleSystems;
			break;
		default:
			assert(false);
	}
}

GMClear::GMClear(int target)
: GraphicsMessage(GM_CLEAR), target(target){
}

void GMClear::Process()
{
	switch(target){
		case GT_PARTICLE_SYSTEMS:
			Graphics.particleSystems.Clear();
			break;
		default:
			assert(false);
	}
}