// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

GMUnregisterEntity::GMUnregisterEntity(EntitySharedPtr i_entity) : GraphicsMessage(GM_UNREGISTER_ENTITY)
{
	entity = i_entity;
}

void GMUnregisterEntity::Process(GraphicsState* graphicsState)
{
	Graphics.UnregisterEntity(entity);
}


GMUnregisterEntities::GMUnregisterEntities(List< std::shared_ptr<Entity> > entities) 
	: GraphicsMessage(GM_UNREGISTER_ENTITIES), entities(entities)
{
	/*
	for (int i = 0; i < selection.Size(); ++i){
		assert(selection[i]->registeredForRendering);
	}*/
}

void GMUnregisterEntities::Process(GraphicsState* graphicsState)
{
	Graphics.UnregisterEntities(entities);
}
