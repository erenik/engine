/// Emil Hedemalm
/// 2014-06-22
/// For animating textures using the AnimationSet and AnimationManager features, mainly using sprite-based approaches.

#include "GMAnimate.h"
#include "GraphicsMessages.h"
#include "Graphics/GraphicsProperty.h"

GMQueueAnimation::GMQueueAnimation(String animationName, Entity* forEntity)
: GraphicsMessage(GM_QUEUE_ANIMATION), entity(forEntity), animationName(animationName)
{
}

void GMQueueAnimation::Process(GraphicsState* graphicsState)
{
	if (!entity)
		return;
	if (!entity->graphics)
		return;
	entity->graphics->SetQueuedAnimation(animationName);
}


GMPlayAnimation::GMPlayAnimation(String animationName, Entity* forEntity)
: GraphicsMessage(GM_PLAY_ANIMATION), entity(forEntity), animationName(animationName)
{
}

void GMPlayAnimation::Process(GraphicsState* graphicsState)
{
	if (!entity)
		return;
	if (!entity->graphics)
		return;
	entity->graphics->SetAnimation(animationName);
}

GMPlaySkeletalAnimation::GMPlaySkeletalAnimation(Entity* entity)
	: GraphicsMessage(GM_PLAY_ANIMATION), entity(entity)
{
}
void GMPlaySkeletalAnimation::Process(GraphicsState* graphicsState)
{
	entity->graphics->skeletalAnimationEnabled = true;
}



