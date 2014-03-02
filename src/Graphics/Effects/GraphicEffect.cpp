// Emil Hedemalm
// 2013-06-15
#include "GraphicEffect.h"
#include "Entity/Entity.h"

GraphicEffect::GraphicEffect(String name, int type)
: name(name), type(type), model(NULL), entity(NULL)
{ 
	primaryColor = Vector4f(1.f,1.f,1.f,1.f);
	relativeScale = Vector3f(1.f,1.f,1.f);
	maxAlpha = 1.0f;
};