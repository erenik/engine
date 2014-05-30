/// Emil Hedemalm
/// 2014-01-20
/// EntityState used for TileMap2D interaction.

#include "EntityStates/EntityStates.h"
#include "EntityStateTile2D.h"

#include "Graphics/GraphicsProperty.h"
#include "GraphicsState.h"
#include "Texture.h"
#include "Shader.h"
#include "TextureManager.h"
#include "Model.h"

EntityStateTile2D::EntityStateTile2D(Entity * owner)
: EntityState(EntityStateType::TILE_2D,owner)
{
	this->entity = owner;
}
/// Render
void EntityStateTile2D::Render(){
	/// Assign textures
	int texturesToApply = 0;
	int error;
	GraphicsProperty * graphics = entity->graphics;
	Texture * diffuseMap = NULL;
//	std::cout<<"\nRendering: "<<entity->name;
	// Find diffuseMap!
	if (graphics && graphics->hasAnimation){
		diffuseMap = graphics->GetTextureForCurrentFrame(graphicsState.currentFrameTime);
		if (diffuseMap->glid == -1){
			/// Bufferize it
			TexMan.BufferizeTexture(diffuseMap);
		}
	}
	if (diffuseMap == NULL){
		diffuseMap = TexMan.GetTextureBySource("img/RuneRPG/Units/200.png");
	}
	// Bind texture if it isn't already bound.
	if (diffuseMap){
		texturesToApply |= DIFFUSE_MAP;
		// When rendering an object with this program.
		glActiveTexture(GL_TEXTURE0 + 0);		// Select server-side active texture unit
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap->glid);
		error = glGetError();
		// Set sampler in client graphicsState
		// Stupid optimization check?
	//	if (graphicsState.activeShader->uniformBaseTexture != -1)
			glUniform1i(graphicsState.activeShader->uniformBaseTexture, 0);		// Sets sampler
		error = glGetError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		graphicsState.currentTexture = diffuseMap;
	//	std::cout<<" with texture:"<<diffuseMap->name;
	}
	else if (diffuseMap == NULL){
		glBindTexture(GL_TEXTURE_2D, (GLuint)NULL);
		graphicsState.currentTexture = NULL;
	}
	/// Push textures to gl
	if (texturesToApply & DIFFUSE_MAP){
		glUniform1i(graphicsState.activeShader->uniformUseDiffuseMap, 1);
	}

	/// Load model matrix
	// Save old matrix to the stack
	Matrix4d tmp = graphicsState.modelMatrixD;
	// Apply transformation
	Matrix4f modelMatrix = Matrix4f::Translation(Vector3f(entity->position.x, entity->position.y, 0));
//	std::cout<<" at position: "<<entity->position;
	// Set uniform matrix in shader to point to the GameState modelView matrix.
	glUniformMatrix4fv(graphicsState.activeShader->uniformModelMatrix, 1, false, modelMatrix.getPointer());
	error = glGetError();

	bool render = true;
	// Only render if previous states say so.
	if (render){
		/// Use just one model, you know which one..!
		// Render the model
		Model * model = entity->model;
//		std::cout<<"\nRendering with model: "<<model;

		model->triangulizedMesh->Render();
		++graphicsState.renderedObjects;		// increment rendered objects for debug info
	}
	// Revert the model matrix to the old one in the stack
	graphicsState.modelMatrixD = tmp;
	/// Stock up graphical effects to render if any
	if (graphics != NULL){
		if (graphics->effects != NULL)
			graphicsState.graphicEffectsToBeRendered += *graphics->effects;
        if (graphics->particleSystems != NULL)
            graphicsState.particleEffectsToBeRendered += *graphics->particleSystems;
	}
}

void EntityStateTile2D::OnEnter(){

}

void EntityStateTile2D::Process(float time){

}

void EntityStateTile2D::OnExit(){

}

void EntityStateTile2D::ProcessMessage(Message * message){

}
