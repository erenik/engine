// Emil Hedemalm
// 2013-07-01

#include "Shader.h"
#include "GraphicsMessages.h"
#include "GMSetEntity.h"
#include "TextureManager.h"
#include "Model.h"
#include "Graphics/Animation/AnimationManager.h"
#include "Graphics/Animation/AnimationSet.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/GraphicsManager.h"

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
		case CLEAR_CAMERA_FILTER:
			break;
		default: assert(false && "Bad target");
	}
}


GMSetEntity::GMSetEntity(List<Entity*> entities, int target, Camera * camera)
	: GraphicsMessage(GM_SET_ENTITY), entities(entities), target(target), camera(camera)
{
	switch(target)
	{
		case CAMERA_FILTER:
		case ADD_CAMERA_FILTER:
		case REMOVE_CAMERA_FILTER:
			break;
		default: assert(false && "Bad target");
	}
}

GMSetEntity::GMSetEntity(List<Entity*> entities, int target, String string)
: GraphicsMessage(GM_SET_ENTITY), entities(entities), target(target), string(string)
{
	switch(target)
	{
		case ANIMATION:
		case QUEUED_ANIMATION:
		case ANIMATION_SET:
			break;
		default:
			assert(false && "Bad target in GMSetEntity");
	}
}

GMSetEntity::GMSetEntity(Entity * entity, int target, Model * model)
: GraphicsMessage(GM_SET_ENTITY), entities(entity), target(target), model(model)
{
	switch(target){
		case MODEL:
		case ANIMATION_SET:
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
		switch(target)
		{
			// Filter to enable per-viewport disabled rendering.
			case CAMERA_FILTER:
			case ADD_CAMERA_FILTER:
				if (!entity->graphics)
					entity->graphics = new GraphicsProperty();
				if (!entity->graphics->cameraFilter.Exists(camera))
					entity->graphics->cameraFilter.Add(camera);
				break;
			case REMOVE_CAMERA_FILTER:
				if (!entity->graphics)
					return;
				entity->graphics->cameraFilter.Remove(camera);
				break;
			case CLEAR_CAMERA_FILTER:
				if (entity->graphics)
					entity->graphics->cameraFilter.Clear();
				break;
			case ANIMATION_SET:
				if (!entity->graphics)
					entity->graphics = new GraphicsProperty();
				entity->graphics->animationSet = AnimationMan.GetAnimationSet(string);
				if (entity->graphics->animationSet->animations.Size())
					entity->graphics->hasAnimation = true;
				break;
			case ANIMATION:
				assert(entity->graphics);
				entity->graphics->SetAnimation(string);
				break;
			case QUEUED_ANIMATION:
				assert(entity->graphics);
				entity->graphics->SetQueuedAnimation(string);
				break;
			case MODEL:
				entity->model = model;
				break;
			default:
				assert(false && "Bad target in GMSetEntity");
		};
	}
}

#define ENSURE_GRAPHICS_PROPERTY(e) {if(!e->graphics) e->graphics = new GraphicsProperty();}

GMSetEntityb::GMSetEntityb(Entity * entity, int target, bool value)
	: GraphicsMessage(GM_SET_ENTITY_BOOLEAN), entity(entity), target(target), bValue(value)
{

}
void GMSetEntityb::Process()
{
	ENSURE_GRAPHICS_PROPERTY(entity);
	entity->graphics->visible = bValue;
}


GMSetEntitys::GMSetEntitys(Entity * entity, int target, String value)
	: GraphicsMessage(GM_SET_ENTITY_STRING), entity(entity), target(target), sValue(value)
{
	switch(target)
	{
		case TEXT:
			break;
		default:
			assert(false && "Bad target in GMSetEntitys");
	}
}
void GMSetEntitys::Process()
{
	ENSURE_GRAPHICS_PROPERTY(entity);
	switch(target)
	{
		case TEXT:
			entity->graphics->text = sValue;
			break;
	}
}


GMSetEntityf::GMSetEntityf(Entity * entity, int target, float value)
	: GraphicsMessage(GM_SET_ENTITY_FLOAT), entity(entity), target(target), fValue(value)
{
	switch(target)
	{
		case TEXT_SIZE_RATIO:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityf::Process()
{
	ENSURE_GRAPHICS_PROPERTY(entity);
	switch(target)
	{
		case TEXT_SIZE_RATIO:
			entity->graphics->textSizeRatio = fValue;
			break;
	}
}


GMSetEntityVec4f::GMSetEntityVec4f(Entity * entity, int target, Vector4f value)
	: GraphicsMessage(GM_SET_ENTITY_VEC4F), entity(entity), target(target), vec4fValue(value)
{
	switch(target)
	{
		case TEXT_COLOR:
		case TEXT_POSITION:
			break;
		default:
			assert(false && "Bad value");
	}
}
void GMSetEntityVec4f::Process()
{
	ENSURE_GRAPHICS_PROPERTY(entity);
	switch(target)
	{
		case TEXT_COLOR:
			entity->graphics->textColor = vec4fValue;
			std::cout<<"lall";
			break;
		case TEXT_POSITION:
			entity->graphics->textPositionOffset = vec4fValue;
			break;
	}
}

