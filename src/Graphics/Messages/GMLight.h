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
	virtual void Process(GraphicsState* graphicsState) override;
};

// Add a light to the lighting setup.
class GMAddLight : public GraphicsMessage 
{
public:
	GMAddLight(Light * newLight);
	virtual ~GMAddLight();
	virtual void Process(GraphicsState* graphicsState) override;
private:
	Light * light;
};


class GMSetAmbience : public GraphicsMessage
{
public:
	GMSetAmbience(ConstVec3fr value);
	virtual ~GMSetAmbience();
	virtual void Process(GraphicsState* graphicsState) override;
private:
	Vector3f value;
};

class GMSetSkyColor : public GraphicsMessage 
{
public: 
	GMSetSkyColor(ConstVec3fr value);
	virtual ~GMSetSkyColor();
	virtual void Process(GraphicsState* graphicsState) override;
private:
	Vector3f value;
};

enum lightTargets {
	LT_NULL_TARGET,
	LT_ATTENUATION, // For calculating effective distance in the shaders.
	LT_CONSTANT_ATTENUATION, // if 0, disables. 1 default. Float
	LT_COLOR, // Both diffuse and specular.
	LT_POSITION,
//	ENABLED,
};

class GMSetLight : public GraphicsMessage 
{
public:
	GMSetLight(Light * light, int target, ConstVec3fr value);
	GMSetLight(Light * light, int target, float value);
	~GMSetLight();
	virtual void Process(GraphicsState* graphicsState) override;
private:
	Light * light;
	int target;
	float fValue;
	Vector3f vec3Value;
};




#endif