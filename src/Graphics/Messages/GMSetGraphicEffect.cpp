// Emil Hedemalm
// 2013-06-15

#include "Entity/Entity.h"
#include "Graphics/GraphicsProperty.h"
#include "GMSetGraphicEffect.h"
#include "Graphics/Effects/GraphicEffect.h"
#include "GraphicsMessages.h"

// Float
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, float value, Entity * owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), floatValue(value), owner(owner){
}
/// Vector
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, Vector3f value, Entity * owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), vec3fValue(value), owner(owner){
}
// For any pointer, yo.
GMSetGraphicEffect::GMSetGraphicEffect(int target, String effectName, void * data, Entity * owner)
: GraphicsMessage(GM_SET_GRAPHIC_EFFECT), target(target), effectName(effectName), data(data), owner(owner){

}

void GMSetGraphicEffect::Process(){
	GraphicEffect * gfx = NULL;
	List<GraphicEffect*> * effects;
	if (owner){
		effects = owner->graphics->effects;
	}
	assert(effects && "WARNING: No valid list was provided in GMSetGraphicEffect!");
	for (int i = 0; i < effects->Size(); ++i){
		if ((*effects)[i]->name == effectName){
			gfx = (*effects)[i];
			break;
		}
	}
	assert(gfx && "WARNING: Unable to find effect in GMSetGraphicEffect, returning!");
	if (gfx == NULL)
		return;

	switch(target){
		case GT_RELATIVE_SCALE:
			gfx->relativeScale = vec3fValue;
			break;
		case GT_ALPHA:
			gfx->primaryColor.w = floatValue;
			if (gfx->primaryColor.w > gfx->maxAlpha)
				gfx->primaryColor.w = gfx->maxAlpha;
			break;
		default:
			assert(false && "Undefined target in GMSetGraphicEffect!");
			break;
	};
}
	