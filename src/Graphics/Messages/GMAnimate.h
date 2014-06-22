/// Emil Hedemalm
/// 2014-06-22
/// For animating textures using the AnimationSet and AnimationManager features, mainly using sprite-based approaches.

#ifndef GM_ANIMATE_H
#define GM_ANIMATE_H

#include "GraphicsMessage.h"

class GMQueueAnimation : public GraphicsMessage
{
public:
	GMQueueAnimation(String animationName, Entity * forEntity);
	virtual void Process();
private:
	String animationName;
	Entity * entity;
};

#endif




