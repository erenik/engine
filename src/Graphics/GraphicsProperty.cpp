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
#include "PhysicsLib/Estimator.h"

#include "Entity/Entity.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"

#include "Model/SkeletalAnimationNode.h"

GraphicsProperty::GraphicsProperty(Entity* owner)
: owner(owner)
{
	temporalAliasingEnabled = false;
	emissiveMapFactor = 1.f;

	owner->graphics = this;
	castsShadow = true;
	flags = 0;
	visible = true;
	textColor = Color::defaultTextColor;
	textSizeRatio = 1.0f;

	hasAnimation = false;
	animStartTime = 0;
	animationSet = NULL;
	queuedAnimation = NULL;
	currentAnimation = NULL;

	blendModeSource = GL_SRC_ALPHA;
	blendModeDest = GL_ONE_MINUS_SRC_ALPHA;
	depthTest = true;
	depthWrite = true;

	color = Vector4f(1,1,1,1);

	skeletalAnimationEnabled = false;
	shaderBasedSkeletonAnimation = false;

	/// If true, the graphics manager and rendering pipeline should try and gather all similar entities (same model and texture) and render them in a single batch.
	renderInstanced = false;
	instancedOptions = false;

	allAnimationsPaused = false;
}


/// Loads save data from target CompactGraphics equivalent that can be saved to file.
bool GraphicsProperty::LoadDataFrom(const CompactGraphics * cGraphics)
{
	flags = cGraphics->flags;
	return true;
}

GraphicsProperty::~GraphicsProperty()
{
	estimators.ClearAndDelete();
	effects.ClearAndDelete();
	particleSystems.Clear();
	dynamicLights.ClearAndDelete();
	staticLights.ClearAndDelete();
}

/// Called when registered to Graphics Manager for rendering. Extracts initial position, etc.
void GraphicsProperty::OnRegister()
{
	smoothedPosition = owner->worldPosition;
	// Re-point the owner's render matrix usage as needed.
	if (temporalAliasingEnabled)
	{
		owner->renderTransform = &transform;	
		owner->renderPosition = &smoothedPosition;
	}
}


#include "GraphicsState.h"

/// Processes estimators related to this entity.
void GraphicsProperty::Process(int timeInMs, GraphicsState & graphicsState)
{
	/// Update smoothed position and matrices.
	if (temporalAliasingEnabled)
	{
		smoothedPosition = smoothedPosition * graphicsState.perFrameSmoothness + owner->worldPosition * (1 - graphicsState.perFrameSmoothness);
		owner->RecalculateMatrix(transform, &smoothedPosition);
		// TODO: Add a flag for temporal Anti-Alisasin so that not ALL static entities too have their shit recalculated each frame... ?
	}

	if (allAnimationsPaused)
		return;
		
	// Sprite-animation
	if (animationSet)
	{
		owner->diffuseMap = GetTextureForCurrentFrame(graphicsState.frametimeStartMs);
		assert(owner->diffuseMap);
		if (owner->diffuseMap->glid == -1)
		{
			/// Bufferize it
			TexMan.BufferizeTexture(owner->diffuseMap);
		}
	}

	/// Tweaking of various graphical-based values, e.g. Alpha.
	for (int i = 0; i < estimators.Size(); ++i)
	{
		Estimator * estimator = estimators[i];
		estimator->Process(timeInMs);
		if (estimator->finished)
		{
			estimators.RemoveIndex(i, ListOption::RETAIN_ORDER);
			--i;
			delete estimator;
		}
	}
	// Re-calculate joint transforms as needed?
	if (skeletalAnimationEnabled)
	{
		

		Model * model = owner->model;
		Mesh * mesh = model->mesh;
		
		Time now = Time::Now();
		if (!mesh->skeleton)
			return;
		mesh->skeleton->AnimateForTime(now.Milliseconds(), true);
		Matrix4f identity;
		mesh->skeleton->UpdateMatrices(identity);
			

		if (shaderBasedSkeletonAnimation)
		{
			model->UpdateSkinningMatrixMap();
		}
		else 
		{
			mesh->SkinToCurrentSkeletalAnimation();
		}
	}
}


/// Sets current animation. Only called from the GMSetEntity message.
void GraphicsProperty::SetAnimation(String name)
{
	if (!animationSet)
	{
		std::cout<<"\nTrying to set animation when lacking animation set.";
		return;
	}
	Animation * anim = animationSet->GetAnimation(name);
	if (anim == currentAnimation && (!anim || anim->repeatable))
		return;
	currentAnimation = anim;
	animStartTime = Timer::GetCurrentTimeMs();
//	std::cout<<"\nSetAnimation "<<name<<" with start time: "<<animStartTime;
}

/// Sets queued animation. Only called from the GMSetEntity message.
void GraphicsProperty::SetQueuedAnimation(String name)
{
	if (!animationSet)
	{
		std::cout<<"\nEntity "<<this->owner<<" lacking animation set!";
		return;
	}
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
	if (animTime > currentAnimation->totalDurationMs && currentAnimation->repeatable)
	{
		animTime = animTime % currentAnimation->totalDurationMs;
	}

	/// Get right frame
	Texture * texture = currentAnimation->GetTexture(animTime);

	// If not repeatable, halt it once it exceeds its duration.
	if (animTime > currentAnimation->totalDurationMs && !currentAnimation->repeatable)
	{
		currentAnimation = NULL;	
	}
	assert(texture);
	return texture;
}
