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

GMSetEntity::GMSetEntity(Entity * entity, int target, String string)
: GraphicsMessage(GM_SET_ENTITY), entity(entity), target(target), string(string)
{
}

GMSetEntity::GMSetEntity(Entity * entity, int target, Model * model)
: GraphicsMessage(GM_SET_ENTITY), entity(entity), target(target), model(model)
{
	switch(target){
		case MODEL:
		case ANIMATION_SET:
			break;
		default:
			assert(false && "Bad target in GMSetEntity");
	}
}
void GMSetEntity::Process(){
	switch(target){
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

GMSetEntityb::GMSetEntityb(Entity * entity, int target, bool value)
	: GraphicsMessage(GM_SET_ENTITY_BOOLEAN), entity(entity), target(target), bValue(value)
{

}
void GMSetEntityb::Process()
{
	if (!entity->graphics)
		entity->graphics = new GraphicsProperty();
	entity->graphics->visible = bValue;
}


