// Emil Hedemalm
// 2013-07-01

#include "Graphics/Shader.h"
#include "GraphicsMessages.h"
#include "GMSetEntity.h"
#include "TextureManager.h"
#include "Model/Model.h"

#include "PhysicsLib/EstimatorFloat.h"

#include "Graphics/Animation/AnimationManager.h"
#include "Graphics/Animation/AnimationSet.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/GraphicsManager.h"
#include "File/LogFile.h"

GMSetEntityTexture::GMSetEntityTexture(List< Entity* > entities, Texture * texture)
	: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entities(entities), t(texture)
{
	target = DIFFUSE_MAP | SPECULAR_MAP;
}


GMSetEntityTexture::GMSetEntityTexture(List< Entity* > entities, int target, Texture * texture)
: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entities(entities), target(target), textureSource(String()), t(texture)
{
//	std::cout<<"Max text: "<<MAX_TEXTURE_TARGETS;
	assert(target >= DIFFUSE_MAP && target <= MAX_TEXTURE_TARGETS);
}
GMSetEntityTexture::GMSetEntityTexture(List< Entity* > entities, int target, String texture)
: GraphicsMessage(GM_SET_ENTITY_TEXTURE), entities(entities), target(target), textureSource(texture), t(NULL)
{
	assert(target >= DIFFUSE_MAP && target <= MAX_TEXTURE_TARGETS);
}

void GMSetEntityTexture::Process(GraphicsState* graphicsState)
{
	if (t == NULL && textureSource.Length())
	{
		t = TexMan.GetTexture(textureSource);
		if (!t){
			std::cout<<"\nERROR: No such texture \""<<textureSource<<"\"";
			return;
		}
	}
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		if (entity == 0)
		{
			std::cout<<"\nNULL entity in GMSetEntityTexture!";
			continue;
		}
		entity->SetTexture(target, t);
		if (t && t->glid == -1)
			TexMan.BufferizeTexture(t);
		Graphics.renderQueried = true;
	}
};

/// For general procedures that do stuff..
GMSetEntity::GMSetEntity(Entity* entity, int target)
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

GMSetEntity::GMSetEntity(List< Entity* > entities, int target, Entity* otherEntity)
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


GMSetEntity::GMSetEntity(List< Entity* > entities, int target, Camera * camera)
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

GMSetEntity::GMSetEntity(List< Entity* > entities, int target, String string)
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

GMSetEntity::GMSetEntity(Entity* entity, int target, Model * model)
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


void GMSetEntity::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		if (!entity)
		{
			std::cout<<"\nNull entity in GMSetEntity";
			continue;
		}
		/// Should probably just attach this when adding it for rendering...?
		if (!entity->graphics)
			entity->graphics = new GraphicsProperty(entity);
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

GMSetEntityb::GMSetEntityb(List< Entity* > entities, int target, bool value, bool recursive)
	: GraphicsMessage(GM_SET_ENTITY_BOOLEAN), entities(entities), target(target), bValue(value), recurse(recursive)
{
	switch(target)
	{
		case GT_VISIBILITY:
		case GT_REQUIRE_DEPTH_SORTING:
		case GT_DEPTH_TEST:
		case GT_DEPTH_WRITE:
		case GT_ANIMATE_SKIN_USING_SHADERS:
		case GT_PAUSE_ANIMATIONS:
		case GT_CAST_SHADOWS:
		case GT_IS_ALPHA_ENTITY:
			break;
		default:
			assert(false && "Bad target in GMSetEntityb");
	}
}
void GMSetEntityb::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		if (entity->children.Size())
			entities.Add(entity->children);
		if (entity->graphics == 0)
			entity->graphics = new GraphicsProperty(entity);
		GraphicsProperty * gp = entity->graphics;
		assert(entity->graphics);
		switch(target)
		{
			case GT_CAST_SHADOWS:
				gp->castsShadow = bValue;
				break;
			case GT_DEPTH_WRITE:
				gp->depthWrite = bValue;
				break;
			case GT_DEPTH_TEST:
				gp->depthTest = bValue;
				break;
			case GT_VISIBILITY:
				gp->visible = bValue;
				break;
			case GT_IS_ALPHA_ENTITY:
				if (bValue)
					gp->flags |= RenderFlag::ALPHA_ENTITY;
				else
					gp->flags &= ~RenderFlag::ALPHA_ENTITY;
				break;
			case GT_REQUIRE_DEPTH_SORTING:
				assert(false && "REQUIRES_DEPTH_SORTING removed, replaced with ALPHA_ENTITY");
	/*
				if (bValue)
					entity->graphics->flags |= RenderFlag::REQUIRES_DEPTH_SORTING;
				else 
					entity->graphics->flags &= ~RenderFlag::REQUIRES_DEPTH_SORTING;
					*/
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


GMSetEntitys::GMSetEntitys(Entity* entity, int target, String value)
	: GraphicsMessage(GM_SET_ENTITY_STRING), entity(entity), target(target), sValue(value)
{
	switch(target)
	{
		case GT_TEXT:
		case GT_ANIMATION:
		case GT_ANIMATION_SET:
		case GT_ENTITY_GROUP:
			break;
		default:
			assert(false && "Bad target in GMSetEntitys");
	}
}
void GMSetEntitys::Process(GraphicsState* graphicsState)
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
		case GT_ENTITY_GROUP:
		{
			graphicsState->RemoveEntity(entity);
			entity->graphics->group = sValue;
			graphicsState->AddEntity(entity);
			break;
		}
	}
}


GMSetEntityf::GMSetEntityf(List< Entity* > entities, int target, float value)
	: GraphicsMessage(GM_SET_ENTITY_FLOAT), entities(entities), target(target), fValue(value)
{
	switch(target)
	{
		case GT_TEXT_SIZE_RATIO:
		case GT_ALPHA:
		case GT_SCALE:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityf::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		assert(entity->graphics);
		switch(target)
		{
			case GT_ALPHA:
				entity->graphics->color[3] = fValue;
				break;
			case GT_TEXT_SIZE_RATIO:
				entity->graphics->textSizeRatio = fValue;
				break;
			case GT_SCALE:
				entity->graphics->scale = Vector3f(1,1,1) * fValue;
				break;
		}
	}
}

GMSetEntityi::GMSetEntityi(List< Entity* > entities, int target, int value)
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

void GMSetEntityi::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
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



GMSetEntityVec4f::GMSetEntityVec4f(List< Entity* > entities, int target, const Vector4f & value)
	: GraphicsMessage(GM_SET_ENTITY_VEC4F), entities(entities), target(target), vec4fValue(value)
{
	switch(target)
	{
		case GT_TEXT_COLOR:
		case GT_TEXT_POSITION:
		case GT_RENDER_OFFSET:
		case GT_COLOR:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityVec4f::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
//		assert(entity->graphics);
		if (entity->graphics == nullptr) {
			LogGraphics("Entered GMSetEntityVec4f with an entity with a null GraphicsProperty!", WARNING);
			return;
		}
		switch(target)
		{
			case GT_TEXT_COLOR:
				entity->graphics->textColor = Color(vec4fValue);
			//	std::cout<<"lall";
				break;
			case GT_TEXT_POSITION:
				entity->graphics->textPositionOffset = vec4fValue;
				break;
			case GT_RENDER_OFFSET:
				entity->graphics->renderOffset = vec4fValue;
				break;
			case GT_COLOR:
				entity->graphics->color = vec4fValue;
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
		case GT_EMISSIVE_MAP_FACTOR:
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

void GMSlideEntityf::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
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
			estimatorFloat->AddStateMs(graphics->color[3], 0);
			estimatorFloat->AddStateMs(this->targetValue, this->timeInMs);
		}
		switch(target)
		{
			case GT_ALPHA:
				estimatorFloat->variablesToPutResultTo.Add(&graphics->color[3]);
				break;
			case GT_EMISSIVE_MAP_FACTOR:
				estimatorFloat->variablesToPutResultTo.Add(&graphics->emissiveMapFactor);
				break;
			default:
				assert(false);
		}
		// Add it to the first entity only, even if shared.
		if (i == 0)
		{
			/// Look for pre-existing estimators with similar variablesToPutResultTo, and remove them if so!
			for (int j = 0; j < graphics->estimators.Size(); ++j)
			{
				Estimator * estimator = graphics->estimators[j];
				if (estimator->type == EstimatorType::FLOAT)
				{
					EstimatorFloat * floatEstim = (EstimatorFloat*) estimator;
					if (floatEstim->variablesToPutResultTo.ExistsAny(estimatorFloat->variablesToPutResultTo))
					{
						graphics->estimators.RemoveItemUnsorted(floatEstim);
						delete floatEstim;
						--j;
					}
				}
			}
			graphics->estimators.Add(estimatorFloat);
		}
	}
}


GMClearEstimators::GMClearEstimators(Entities entities)
	: GraphicsMessage(0), entities(entities)
{

}
void GMClearEstimators::Process(GraphicsState* graphicsState)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* entity = entities[i];
		if (!entity)
		{
			std::cout<<"\nWARNING: NULL entity in GMClearEstimators";
			continue;
		}
		entity->graphics->estimators.ClearAndDelete();
	}
}

