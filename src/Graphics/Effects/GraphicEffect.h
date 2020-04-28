// Emil Hedemalm
// 2013-06-15

#ifndef GRAPHIC_EFFECT_H
#define GRAPHIC_EFFECT_H

#include <Util.h>
#include <MathLib.h>

class GraphicsState;
class Model;

class Entity;
#define EntitySharedPtr std::shared_ptr<Entity>

namespace GFX {
enum graphicEffects{
	NULL_TYPE,
	STATIC_MODEL,		// A statically rendered model, for example the ship's spherical shield
	ALPHA_MODEL_EFFECT,	// Static model with interactive alpha-effects
};
};

struct GraphicEffect{
	friend class GMSetGraphicEffect;
public:
	GraphicEffect(String name, int type);
	// Renders the shiat!
	virtual void Render(GraphicsState * graphics) = 0;
protected:
	// Variables
	String name;
	int type;
	Model * model;
	/// Primary multiplication color o-o
	float maxAlpha;
	Vector4f primaryColor;
	Vector3f relativeRotation;
	Vector3f relativeScale;
	/// If needed
	EntitySharedPtr entity;
};

#endif