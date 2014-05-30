/// Emil Hedemalm
/// 2014-04-06
/// Messages for adjusting in-game lighting conditions that are used by the shaders!

#include "GMLight.h"
#include "GraphicsMessages.h"
#include "Graphics/GraphicsManager.h"
#include "Light.h"


GMClearLighting::GMClearLighting()
: GraphicsMessage(0)
{
}
void GMClearLighting::Process()
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->DeleteAllLights();
}


GMAddLight::GMAddLight(Light newLight)
: GraphicsMessage(GM_ADD_LIGHT), light(newLight)
{

}
GMAddLight::~GMAddLight()
{

}
void GMAddLight::Process()
{
	Lighting * lighting = Graphics.ActiveLighting();
	lighting->Add(&light);
}



GMSetAmbience::GMSetAmbience(Vector3f value) 
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


GMSetLight::GMSetLight(Light * light, int target, Vector3f value)
: GraphicsMessage(GM_SET_LIGHT), target(target), vec3Value(value), light(light) 
{
	switch(target)
	{
	case ATTENUATION:
	case COLOR:
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
	case ATTENUATION:
		light->attenuation = vec3Value;
		break;
	case COLOR:
		light->diffuse = vec3Value;
		break;
	}
}
