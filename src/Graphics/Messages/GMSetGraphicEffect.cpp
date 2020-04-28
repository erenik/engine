// Emil Hedemalm
// 2013-06-15

#include "Entity/Entity.h"
#include "Graphics/GraphicsProperty.h"
#include "GMSetGraphicEffect.h"
#include "Graphics/Effects/GraphicEffect.h"
#include "GraphicsMessages.h"

// Float
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, float value, EntitySharedPtr owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), floatValue(value), owner(owner){
}
/// Vector
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, const Vector3f & value, EntitySharedPtr owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), vec3fValue(value), owner(owner){
}
// For any pointer, yo.
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, void * data, EntitySharedPtr owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), data(data), owner(owner){

}

void GMSetGraphicEffect::Process(GraphicsState* graphicsState)
{
	GraphicEffect * gfx = NULL;
	List<GraphicEffect*> & effects = owner->graphics->effects;
	for (int i = 0; i < effects.Size(); ++i)
	{
		if (effects[i]->name == effectName)
		{
			gfx = effects[i];
			break;
		}
	}
	if (gfx == NULL)
		return;

	switch(target){
		case GT_RELATIVE_SCALE:
			gfx->relativeScale = vec3fValue;
			break;
		case GT_ALPHA:
			gfx->primaryColor[3] = floatValue;
			if (gfx->primaryColor[3] > gfx->maxAlpha)
				gfx->primaryColor[3] = gfx->maxAlpha;
			break;
		default:
			assert(false && "Undefined target in GMSetGraphicEffect!");
			break;
	};
}
	