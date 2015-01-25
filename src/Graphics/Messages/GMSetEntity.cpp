// Emil Hedemalm
// 2013-07-01

#include "Shader.h"
#include "GraphicsMessages.h"
#include "GMSetEntity.h"
#include "TextureManager.h"
#include "Model/Model.h"

#include "PhysicsLib/EstimatorFloat.h"

#include "Graphics/Animation/AnimationManager.h"
#include "Graphics/Animation/AnimationSet.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/GraphicsManager.h"

GMSetEntityTexture::GMSetEntityTexture(Entity * entity, Texture * texture)
	: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entity(entity), t(texture)
{
	target = DIFFUSE_MAP | SPECULAR_MAP;
}


GMSetEntityTexture::GMSetEntityTexture(Entity * entity, int target, Texture * texture)
: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entity(entity), target(target), textureSource(String()), t(texture)
{
	assert(target >= DIFFUSE_MAP && target <= MAX_TEXTURE_TARGETS);
}
GMSetEntityTexture::GMSetEntityTexture(Entity * entity, int target, String texture)
: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entity(entity), target(target), textureSource(texture), t(NULL)
{
	assert(target >= DIFFUSE_MAP && target <= MAX_TEXTURE_TARGETS);
}

void GMSetEntityTexture::Process()
{
	if (entity == 0)
	{
		std::cout<<"\nNULL entity in GMSetEntityTexture!";
		return;
	}
	if (t == NULL){
		t = TexMan.GetTexture(textureSource);
		if (!t){
            std::cout<<"\nERROR: No such texture \""<<textureSource<<"\"";
            return;
        }
    }
	entity->SetTexture(target, t);
	if (t->glid == -1)
        TexMan.BufferizeTexture(t);
	Graphics.renderQueried = true;
};

/// For general procedures that do stuff..
GMSetEntity::GMSetEntity(Entity * entity, int target)
	: GraphicsMessage(GM_SET_ENTITY), entities(entity), target(target)
{
	switch(target)
	{
		case GT_CLEAR_CAMERA_FILTER:
			break;
		default: 
			assert(false && "Bad target");
	}
}

GMSetEntity::GMSetEntity(List<Entity*> entities, int target, Entity * otherEntity)
: GraphicsMessage(GM_SET_ENTITY), entities(entities), target(target), otherEntity(otherEntity)
{
	switch(target)
	{
		case GT_PARENT:
			break;
		default:
			assert(false);
	}
}


GMSetEntity::GMSetEntity(List<Entity*> entities, int target, Camera * camera)
	: GraphicsMessage(GM_SET_ENTITY), entities(entities), target(target), camera(camera)
{
	switch(target)
	{
		case GT_CAMERA_FILTER:
		case GT_ADD_CAMERA_FILTER:
		case GT_REMOVE_CAMERA_FILTER:
			break;
		default: assert(false && "Bad target");
	}
}

GMSetEntity::GMSetEntity(List<Entity*> entities, int target, String string)
: GraphicsMessage(GM_SET_ENTITY), entities(entities), target(target), string(string)
{
	switch(target)
	{
		case GT_ANIMATION:
		case GT_QUEUED_ANIMATION:
		case GT_ANIMATION_SET:
			break;
		default:
			assert(false && "Bad target in GMSetEntity");
	}
}

GMSetEntity::GMSetEntity(Entity * entity, int target, Model * model)
: GraphicsMessage(GM_SET_ENTITY), entities(entity), target(target), model(model)
{
	switch(target){
		case GT_MODEL:
		case GT_ANIMATION_SET:
			break;
		default:
			assert(false && "Bad target in GMSetEntity");
	}
}


void GMSetEntity::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity)
		{
			std::cout<<"\nNull entity in GMSetEntity";
			continue;
		}
		/// Should probably just attach this when adding it for rendering...?
		assert(entity->graphics);
		GraphicsProperty * graphics = entity->graphics;

		switch(target)
		{
			// Parenting
			case GT_PARENT:
			{
				entity->parent = otherEntity;
				otherEntity->children.Add(entity);
				// Recalculate its matrix.
				entity->RecalculateMatrix();
				break;
			}
			// Filter to enable per-viewport disabled rendering.
			case GT_CAMERA_FILTER:
			case GT_ADD_CAMERA_FILTER:
				if (!entity->graphics->cameraFilter.Exists(camera))
					entity->graphics->cameraFilter.Add(camera);
				break;
			case GT_REMOVE_CAMERA_FILTER:
				entity->graphics->cameraFilter.Remove(camera);
				break;
			case GT_CLEAR_CAMERA_FILTER:
				entity->graphics->cameraFilter.Clear();
				break;
			case GT_ANIMATION_SET:
			{
				AnimationSet * anim = AnimationMan.GetAnimationSet(string);
				if (!anim)
					return;
				entity->graphics->animationSet = anim;
				if (anim->animations.Size())
					entity->graphics->hasAnimation = true;
				break;
			}
			case GT_ANIMATION:
				assert(entity->graphics);
				entity->graphics->SetAnimation(string);
				break;
			case GT_QUEUED_ANIMATION:
				assert(entity->graphics);
				entity->graphics->SetQueuedAnimation(string);
				break;
			case GT_MODEL:
				entity->model = model;
				break;
			default:
				assert(false && "Bad target in GMSetEntity");
		};
	}
}

GMSetEntityb::GMSetEntityb(List<Entity*> entities, int target, bool value, bool recursive)
	: GraphicsMessage(GM_SET_ENTITY_BOOLEAN), entities(entities), target(target), bValue(value), recurse(recursive)
{
	switch(target)
	{
		case GT_VISIBILITY:
		case GT_REQUIRE_DEPTH_SORTING:
		case GT_DEPTH_TEST:
		case GT_ANIMATE_SKIN_USING_SHADERS:
		case GT_PAUSE_ANIMATIONS:
			break;
		default:
			assert(false && "Bad target in GMSetEntityb");
	}
}
void GMSetEntityb::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (entity->children.Size())
			entities.Add(entity->children);
		assert(entity->graphics);
		switch(target)
		{
			case GT_DEPTH_TEST:
				entity->graphics->depthTest = bValue;
				break;
			case GT_VISIBILITY:
				entity->graphics->visible = bValue;
				break;
			case GT_REQUIRE_DEPTH_SORTING:
				if (bValue)
					entity->graphics->flags |= RenderFlags::REQUIRES_DEPTH_SORTING;
				else 
					entity->graphics->flags &= ~RenderFlags::REQUIRES_DEPTH_SORTING;
				break;
			case GT_ANIMATE_SKIN_USING_SHADERS:
				entity->graphics->shaderBasedSkeletonAnimation = bValue;
				break;
			case GT_PAUSE_ANIMATIONS:
				entity->graphics->allAnimationsPaused = bValue;
				break;
		}
	}
}


GMSetEntitys::GMSetEntitys(Entity * entity, int target, String value)
	: GraphicsMessage(GM_SET_ENTITY_STRING), entity(entity), target(target), sValue(value)
{
	switch(target)
	{
		case GT_TEXT:
		case GT_ANIMATION:
		case GT_ANIMATION_SET:
			break;
		default:
			assert(false && "Bad target in GMSetEntitys");
	}
}
void GMSetEntitys::Process()
{
	assert(entity->graphics);
	switch(target)
	{
		case GT_TEXT:
			entity->graphics->text = sValue;
			break;
		case GT_ANIMATION:
			assert(entity->graphics);
			entity->graphics->SetAnimation(sValue);
			break;
		case GT_ANIMATION_SET:
		{
			AnimationSet * anim = AnimationMan.GetAnimationSet(sValue);
			assert(anim && "Unable to find anim!");
			if (!anim)
				return;
			entity->graphics->animationSet = anim;
			if (anim->animations.Size())
				entity->graphics->hasAnimation = true;
			break;
		}

	}
}


GMSetEntityf::GMSetEntityf(List<Entity*> entities, int target, float value)
	: GraphicsMessage(GM_SET_ENTITY_FLOAT), entities(entities), target(target), fValue(value)
{
	switch(target)
	{
		case GT_TEXT_SIZE_RATIO:
		case GT_ALPHA:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityf::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		assert(entity->graphics);
		switch(target)
		{
			case GT_ALPHA:
				entity->graphics->color.w = fValue;
				break;
			case GT_TEXT_SIZE_RATIO:
				entity->graphics->textSizeRatio = fValue;
				break;
		}
	}
}

GMSetEntityi::GMSetEntityi(List<Entity*> entities, int target, int value)
	: GraphicsMessage(GM_SET_ENTITY_INTEGER), entities(entities), target(target), iValue(value)
{
	switch(target)
	{
		case GT_BLEND_MODE_SRC:
		case GT_BLEND_MODE_DST:
			break;
		default:
			assert(false);
	}
}

void GMSetEntityi::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		/// Should probably just attach this when adding it for rendering...?
		assert(entity->graphics);
		GraphicsProperty * graphics = entity->graphics;

		switch(target)
		{
			case GT_BLEND_MODE_SRC:
				graphics->blendModeSource = iValue;
				break;
			case GT_BLEND_MODE_DST:
				graphics->blendModeDest = iValue;
				break;
		}
	}
}



GMSetEntityVec4f::GMSetEntityVec4f(List<Entity*> entities, int target, const Vector4f & value)
	: GraphicsMessage(GM_SET_ENTITY_VEC4F), entities(entities), target(target), vec4fValue(value)
{
	switch(target)
	{
		case GT_TEXT_COLOR:
		case GT_TEXT_POSITION:
		case GT_RENDER_OFFSET:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityVec4f::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		assert(entity->graphics);
		switch(target)
		{
			case GT_TEXT_COLOR:
				entity->graphics->textColor = vec4fValue;
			//	std::cout<<"lall";
				break;
			case GT_TEXT_POSITION:
				entity->graphics->textPositionOffset = vec4fValue;
				break;
			case GT_RENDER_OFFSET:
				entity->graphics->renderOffset = vec4fValue;
				break;
		}
	}
}

GMSlideEntityf::GMSlideEntityf(Entities entities, int target, EstimatorFloat * usingPrefilledEstimator)
	:  GraphicsMessage(GM_SLIDE_ENTITY), entities(entities), target(target), estimatorFloat(usingPrefilledEstimator)
{
	switch(target)
	{
		case GT_ALPHA:
			break;
		default:
			assert(false);
	}
}

GMSlideEntityf::GMSlideEntityf(Entities entities, int target, float targetValue, int timeInMs)
	:  GraphicsMessage(GM_SLIDE_ENTITY), entities(entities), target(target), targetValue(targetValue), timeInMs(timeInMs)
{
	switch(target)
	{
		case GT_ALPHA:
			break;
		default:
			assert(false && "Invalid target in GMSlideEntity");
	}
}

#include "PhysicsLib/EstimatorFloat.h"

void GMSlideEntityf::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity)
		{
			std::cout<<"\nNULL entity.";
			continue;
		}
		GraphicsProperty * graphics = entity->graphics;
		assert(graphics);
		// Create the slider (estimator)
		if (!estimatorFloat)
		{
			// Non pre-filled? 
			estimatorFloat = new EstimatorFloat();
			estimatorFloat->AddStateMs(graphics->color.w, 0);
			estimatorFloat->AddStateMs(this->targetValue, this->timeInMs);
		}
		switch(target)
		{
			case GT_ALPHA:
				estimatorFloat->variableToPutResultTo = &graphics->color.w;
				break;
			default:
				assert(false);
		}
		graphics->estimators.Add(estimatorFloat);
	}
}


GMClearEstimators::GMClearEstimators(Entities entities)
	: GraphicsMessage(0), entities(entities)
{

}
void GMClearEstimators::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity)
		{
			std::cout<<"\nWARNING: NULL entity in GMClearEstimators";
			continue;
		}
		entity->graphics->estimators.ClearAndDelete();
	}
}

