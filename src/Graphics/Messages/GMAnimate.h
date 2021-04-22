/// Emil Hedemalm
/// 2014-06-22
/// For animating textures using the AnimationSet and AnimationManager features, mainly using sprite-based approaches.

#ifndef GM_ANIMATE_H
#define GM_ANIMATE_H

#include "GraphicsMessage.h"

class GMQueueAnimation : public GraphicsMessage
{
public:
	GMQueueAnimation(String animationName, Entity* forEntity);
	virtual void Process(GraphicsState* graphicsState);
private:
	String animationName;
	Entity* entity;
};

class GMPlayAnimation : public GraphicsMessage 
{
public:
	GMPlayAnimation(String animationName, Entity* forEntity);
	virtual void Process(GraphicsState* graphicsState);
private:
	String animationName;
	Entity* entity;
};

// For skeletal animations
class GMPlaySkeletalAnimation : public GraphicsMessage 
{
public:
	GMPlaySkeletalAnimation(Entity* entity);
	virtual void Process(GraphicsState* graphicsState);
private:
	Entity* entity;
};

#endif




