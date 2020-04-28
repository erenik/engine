/// Emil Hedemalm
/// 2014-04-06
/// Messages for adjusting in-game lighting conditions that are used by the shaders!

#include "GMLight.h"
#include "GraphicsMessages.h"
#include "Graphics/GraphicsManager.h"
#include "Light.h"
#include "GraphicsState.h"


GMClearLighting::GMClearLighting()
: GraphicsMessage(0)
{
}
void GMClearLighting::Process(GraphicsState* graphicsState)
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->DeleteAllLights();
	graphicsState->dynamicLights.ClearAndDelete();
}


GMAddLight::GMAddLight(Light * newLight)
: GraphicsMessage(GM_ADD_LIGHT), light(newLight)
{

}
GMAddLight::~GMAddLight()
{

}
void GMAddLight::Process(GraphicsState* graphicsState)
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->lights.Add(light);
}



GMSetAmbience::GMSetAmbience(ConstVec3fr value) 
: GraphicsMessage(GM_SET_AMBIENCE), value(value)
{

}

GMSetAmbience::~GMSetAmbience()
{

}
void GMSetAmbience::Process(GraphicsState* graphicsState)
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->SetAmbient(value);
}

GMSetSkyColor::GMSetSkyColor(ConstVec3fr value)
: GraphicsMessage(GM_SET_SKY_COLOR), value(value)
{
}
GMSetSkyColor::~GMSetSkyColor(){}
void GMSetSkyColor::Process(GraphicsState * graphicsState)
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->skyColor = value;
}


GMSetLight::GMSetLight(Light * light, int target, ConstVec3fr value)
: GraphicsMessage(GM_SET_LIGHT), target(target), vec3Value(value), light(light) 
{
	assert(light);
	switch(target)
	{
	case LT_ATTENUATION:
	case LT_COLOR:
	case LT_POSITION:
		break;
	default:
		assert(false);
	}
}
GMSetLight::GMSetLight(Light * light, int target, float value)
: GraphicsMessage(GM_SET_LIGHT), target(target), fValue(value), light(light)
{
	assert(light);
	switch(target)
	{
	case LT_CONSTANT_ATTENUATION:
		break;
	default:
		assert(false);
	}
}

GMSetLight::~GMSetLight()
{

}
void GMSetLight::Process(GraphicsState* graphicsState)
{
	if (!light)
		return;
	switch(target)
	{
		case LT_ATTENUATION:
			light->attenuation = vec3Value;
			break;
		case LT_COLOR:
			light->specular = light->diffuse = vec3Value;
			break;
		case LT_POSITION:
			light->position = vec3Value;
			break;
		case LT_CONSTANT_ATTENUATION:
			light->attenuation.x = fValue;
			break;
		default:
			assert(false);
	}
}
