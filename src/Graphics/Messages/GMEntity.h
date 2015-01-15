/// Emil Hedemalm
/// 2014-05-17
/// Specific messages target to entities and controlling some behaviour around them, such as lights.

#ifndef GM_ENTITY_H
#define GM_ENTITY_H

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include "Graphics/GraphicsProperty.h"
#include "Entity/Entity.h"

/*

class GMAttachLight : public GraphicsMessage 
{
public:
	GMAttachLight(Entity * toEntity, Light * light)
	:  GraphicsMessage(GM_ADD_LIGHT), toEntity(toEntity), light(light)
	{
	};
	virtual void Process()
	{
		if (!toEntity->graphics)
			toEntity->graphics = new GraphicsProperty();
		GraphicsProperty * gp = toEntity->graphics;
		if (!gp->dynamicLights)
			gp->dynamicLights = new List<Light*>();
		gp->dynamicLights->Add(light);
		Graphics.dynamicLights.Add(light);	
	}
private:
	Entity * toEntity;
	Light * light;
};
*/

#endif
