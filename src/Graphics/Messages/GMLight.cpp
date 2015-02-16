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
void GMClearLighting::Process()
{
	GraphicsState * graphicsState = graphicsState;
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
void GMAddLight::Process()
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
void GMSetAmbience::Process()
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->SetAmbient(value);
}

GMSetSkyColor::GMSetSkyColor(ConstVec3fr value)
: GraphicsMessage(GM_SET_SKY_COLOR), value(value)
{
}
GMSetSkyColor::~GMSetSkyColor(){}
void GMSetSkyColor::Process()
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->skyColor = value;
}


GMSetLight::GMSetLight(Light * light, int target, ConstVec3fr value)
: GraphicsMessage(GM_SET_LIGHT), target(target), vec3Value(value), light(light) 
{
	switch(target)
	{
	case LightTarget::ATTENUATION:
	case LightTarget::COLOR:
	case LightTarget::POSITION:
		break;
	default:
		assert(false);
	}
}
GMSetLight::GMSetLight(Light * light, int target, float value)
: GraphicsMessage(GM_SET_LIGHT), target(target), fValue(value), light(light)
{
}
GMSetLight::~GMSetLight()
{

}
void GMSetLight::Process()
{
	if (!light)
		return;
	switch(target)
	{
		case LightTarget::ATTENUATION:
			light->attenuation = vec3Value;
			break;
		case LightTarget::COLOR:
			light->specular = light->diffuse = vec3Value;
			break;
		case LightTarget::POSITION:
			light->position = vec3Value;
			break;
		default:
			assert(false);
	}
}
