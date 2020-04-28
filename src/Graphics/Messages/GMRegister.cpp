// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

#include "Entity/EntityManager.h"

GMRegisterEntity::GMRegisterEntity(EntitySharedPtr i_entity) : GraphicsMessage(GM_REGISTER_ENTITY){
	entity = i_entity;
}

void GMRegisterEntity::Process(GraphicsState* graphicsState)
{
	if (!EntityMan.IsGood(entity))
		return;

	GraphicsManager & manager = GraphicsMan;
#ifdef VFC_OCTREE
	assert(manager.vfcOctree);
#endif
	///	entity->model->mesh.Bufferize();
	Graphics.RegisterEntity(entity);
}


GMRegisterEntities::GMRegisterEntities(Entities i_selection) : GraphicsMessage(GM_REGISTER_ENTITIES)
{
	selection = i_selection;
}

void GMRegisterEntities::Process(GraphicsState* graphicsState)
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

void GMRegister::Process(GraphicsState* graphicsState)
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

void GMClear::Process(GraphicsState* graphicsState)
{
	switch(target){
		case GT_PARTICLE_SYSTEMS:
			Graphics.particleSystems.Clear();
			break;
		default:
			assert(false);
	}
}