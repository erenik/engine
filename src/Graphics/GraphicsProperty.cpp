// Emil Hedemalm
// 2013-06-15

#include "GraphicsProperty.h"
#include "CompactGraphics.h"
#include "Graphics/Effects/GraphicEffect.h"
#include "Graphics/Particles/ParticleSystem.h"
#include "Light.h"
#include "Graphics/Animation/AnimationSet.h"
#include "Graphics/Animation/Animation.h"
#include "TextureManager.h"


GraphicsProperty::GraphicsProperty()
{
	effects = NULL;
	dynamicLights = NULL;
	staticLights = NULL;
	particleSystems = NULL;
	flags = 0;
	hasAnimation = false;
	animStartTime = 0;
	queuedAnimation = NULL;
	currentAnimation = NULL;
	visible = true;
	textColor = Vector4f(1,1,1,1);
	textSizeRatio = 1.0f;
}


/// Loads save data from target CompactGraphics equivalent that can be saved to file.
bool GraphicsProperty::LoadDataFrom(const CompactGraphics * cGraphics)
{
	effects = NULL;
	dynamicLights = NULL;
	staticLights = NULL;
	particleSystems = NULL;
	flags = cGraphics->flags;
	return true;
}

GraphicsProperty::~GraphicsProperty()
{
	if (effects){
	    for (int i = 0; i < effects->Size(); ++i){
            GraphicEffect * e =(*effects)[i];
            delete e;
	    }
		effects->Clear();
		delete effects;
	}
	effects = NULL;

	if (particleSystems){
	    for (int i = 0; i < particleSystems->Size(); ++i){
            ParticleSystem * ps = (*particleSystems)[i];
            delete ps;
	    }
		particleSystems->Clear();
		delete particleSystems;
	}
	particleSystems = NULL;

	if (dynamicLights){
	    for (int i = 0; i < dynamicLights->Size(); ++i)
            delete (*dynamicLights)[i];
		dynamicLights->Clear();
		delete dynamicLights;
	}
	dynamicLights = NULL;

	if (staticLights){
	    for (int i = 0; i < staticLights->Size(); ++i)
            delete (*staticLights)[i];
		staticLights->Clear();
		delete staticLights;
	}
	staticLights = NULL;
}

/// Sets current animation. Only called from the GMSetEntity message.
void GraphicsProperty::SetAnimation(String name)
{
	Animation * anim = animationSet->GetAnimation(name);
	if (anim == currentAnimation && (!anim || anim->repeatable))
		return;
	currentAnimation = anim;
	animStartTime = Timer::GetCurrentTimeMs();
	std::cout<<"\nSetAnimation "<<name<<" with start time: "<<animStartTime;
}

/// Sets queued animation. Only called from the GMSetEntity message.
void GraphicsProperty::SetQueuedAnimation(String name)
{
	Animation * anim = animationSet->GetAnimation(name);
	if (anim)
		queuedAnimation = anim;
}

/// Fetches relevant texture for current frame time. This assumes that the element has an active animation playing.
Texture * GraphicsProperty::GetTextureForCurrentFrame(int64 & frameTime)
{
	/// No current animation?If we got a queued animation..
	if (currentAnimation == NULL)
	{
		// Check for a queued animation
		// No current animation? just set it then
		if (queuedAnimation)
		{
			currentAnimation = queuedAnimation;
			queuedAnimation = NULL;	
			animStartTime = frameTime;
		}
		else 
		{
			// Judge if the past animation is finishing..?
		}
	}
	/// If still no current animation after checking queued animations? use base frame!
	if (!currentAnimation)
	{
		return TexMan.GetTextureBySource(animationSet->baseFrame);
	}

	/// Calculate animation-time.
	int64 animTime = frameTime - animStartTime;
	if (animTime > currentAnimation->totalDuration && currentAnimation->repeatable){
		int c = animTime / currentAnimation->totalDuration;
		if (c > 1000)
			animStartTime = frameTime;
		animStartTime += currentAnimation->totalDuration;
	}

	/// Get right frame
	Texture * texture = currentAnimation->GetTexture(animTime);

	// If not repeatable, halt it once it exceeds its duration.
	if (animTime > currentAnimation->totalDuration)
	{
		currentAnimation = NULL;	
	}

	return texture;
}
