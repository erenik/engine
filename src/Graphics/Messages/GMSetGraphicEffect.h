// Emil Hedemalm
// 2013-06-15

#ifndef GM_SET_GRAPHIC_EFFECT_H
#define GM_SET_GRAPHIC_EFFECT_H

#include "GraphicsMessage.h"

class Entity;

/** General utility Setter "function" message for the rendering pipeline, 
	like overlay images, default values and debug renders.
*/
class GMSetGraphicEffect : public GraphicsMessage {
public:
	GMSetGraphicEffect(int target, String effectName, float value, Entity * owner = NULL);
	GMSetGraphicEffect(int target, String effectName, const Vector3f & value, Entity * owner = NULL);
	GMSetGraphicEffect(int target, String effectName, void * data, Entity * owner = NULL); // For any pointer, yo.
	void Process();
private:
	int target;
	float floatValue;
	Vector3f vec3fValue;
	String effectName;
	void * data;
	Entity * owner;
};

#endif