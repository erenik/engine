/// Emil Hedemalm
/// 2014-04-06
/// Messages for adjusting in-game lighting conditions that are used by the shaders!

#ifndef GM_LIGHT_H
#define GM_LIGHT_H

#include "GraphicsMessage.h"


class GMClearLighting : public GraphicsMessage 
{
public:
	GMClearLighting();
	virtual void Process();
};

// Add a light to the lighting setup.
class GMAddLight : public GraphicsMessage 
{
public:
	GMAddLight(Light newLight);
	virtual ~GMAddLight();
	virtual void Process();
private:
	Light light;
};


class GMSetAmbience : public GraphicsMessage
{
public:
	GMSetAmbience(Vector3f value);
	virtual ~GMSetAmbience();
	virtual void Process();
private:
	Vector3f value;
};

enum lightTargets {
	NULL_TARGET,
	ATTENUATION, // For calculating effective distance in the shaders.
	COLOR, // Both diffuse and specular.
};

class GMSetLight : public GraphicsMessage 
{
public:
	GMSetLight(Light * light, int target, Vector3f value);
	GMSetLight(Light * light, int target, float value);
	~GMSetLight();
	virtual void Process();
private:
	Light * light;
	int target;
	float fValue;
	Vector3f vec3Value;
};




#endif