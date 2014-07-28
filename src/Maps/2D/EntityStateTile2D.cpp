/// Emil Hedemalm
/// 2014-01-20
/// EntityPropertyState used for TileMap2D interaction.

#include "Entity/EntityPropertyState.h"
#include "EntityStateTile2D.h"

#include "Graphics/GraphicsProperty.h"
#include "GraphicsState.h"
#include "Texture.h"
#include "Shader.h"
#include "TextureManager.h"
#include "Model.h"

EntityStateTile2D::EntityStateTile2D(Entity * owner)
: EntityProperty("EntityStateTile2D", EntityPropertyID::UTILITY_PROPERTIES, owner)
{
}

EntityStateTile2D::~EntityStateTile2D()
{

}
/// Render
void EntityStateTile2D::Render()
{
	std::cout<<"erer:";
}

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int EntityStateTile2D::ID()
{
	return EntityPropertyID::UTILITY_PROPERTIES;
}


void EntityStateTile2D::OnEnter(){

}

void EntityStateTile2D::Process(int timeInMs){

}

void EntityStateTile2D::OnExit(){

}

void EntityStateTile2D::ProcessMessage(Message * message)
{

}
