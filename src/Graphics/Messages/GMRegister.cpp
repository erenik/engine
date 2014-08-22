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
	assert(Graphics.registeredEntities.Size() == Graphics.vfcOctree->RegisteredEntities());
	entity->Bufferize();
	Graphics.RegisterEntity(entity);
	assert(Graphics.registeredEntities.Size() == Graphics.vfcOctree->RegisteredEntities());
}


GMRegisterEntities::GMRegisterEntities(Selection i_selection) : GraphicsMessage(GM_REGISTER_ENTITIES)
{
	selection = i_selection;
	for (int i = 0; i < selection.Size(); ++i){
		if (selection[i]->registeredForRendering){
			assert(!selection[i]->registeredForRendering);
		}
	}
}

void GMRegisterEntities::Process()
{
	for (int i = 0; i < selection.Size(); ++i)
	{
		if (selection[i]->registeredForRendering){
			assert(!selection[i]->registeredForRendering);
		}
	}
	assert(Graphics.registeredEntities.Size() == Graphics.vfcOctree->RegisteredEntities());
	Graphics.RegisterEntities(selection);
	assert(Graphics.registeredEntities.Size() == Graphics.vfcOctree->RegisteredEntities());
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