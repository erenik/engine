/// Emil Hedemalm
/// 2014-05-17
/// Specific messages target to entities and controlling some behaviour around them, such as lights.

#ifndef GM_ENTITY_H
#define GM_ENTITY_H

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include "Graphics/GraphicsProperty.h"
#include "Entity/Entity.h"


class GMAttachLight : public GraphicsMessage 
{
public:
	GMAttachLight(Light * light, Entity * toEntity)
	:  GraphicsMessage(GM_ADD_LIGHT), toEntity(toEntity), light(light)
	{
	};
	virtual void Process()
	{
		if (!toEntity->graphics)
			toEntity->graphics = new GraphicsProperty(toEntity);
		GraphicsProperty * gp = toEntity->graphics;
		/// Un register?
		if (toEntity->registeredForRendering)
			GraphicsMan.UnregisterEntity(toEntity);
		gp->dynamicLights.Add(light);
		GraphicsMan.RegisterEntity(toEntity);
	}
private:
	Entity * toEntity;
	Light * light;
};

#endif
