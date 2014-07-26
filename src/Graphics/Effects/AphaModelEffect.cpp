// Emil Hedemalm
// 2013-06-15

#include "AlphaModelEffect.h"
#include "ModelManager.h"
#include "GraphicsState.h"
#include "Shader.h"
#include "Entity/Entity.h"

#ifndef max
#define max(a,b) ((a > b)? (a) : (b))
#endif

AlphaModelEffect::AlphaModelEffect(String name, String modelName, Entity * reference)
: GraphicEffect(name, GFX::ALPHA_MODEL_EFFECT) {
	model = ModelMan.GetModel(modelName);
	assert(model);
	entity = reference;
	linearDecay = 0.10f;
	relativeDecay = 0.1f;
	maxAlpha = 0.5f;
	primaryColor = Vector4f(1,1,1,maxAlpha);
}

/// Renders, but also updates it's various values using the given timestep since last frame.
void AlphaModelEffect::Render(GraphicsState * graphicsState)
{
	assert(model);
	/// Override alpha-changes from outside if they decreased it!
	if (primaryColor.w < lastAlpha)
		primaryColor.w = lastAlpha,

	assert(entity);
	static Matrix4f modelMatrix;
	if (entity){
		modelMatrix.LoadIdentity();
		modelMatrix.Translate(entity->position);
		modelMatrix.Scale(entity->radius * (entity->scale.ElementMultiplication(relativeScale)).MaxPart());
		modelMatrix.Multiply(Matrix4f::GetRotationMatrixX(entity->rotation.x));
		modelMatrix.Multiply(Matrix4f::GetRotationMatrixY(relativeRotation.y + entity->rotation.y));
		graphicsState->modelMatrixF = modelMatrix;
	};


	/// Set model matrix and other stuff.
	if (graphicsState->activeShader){
		// Model matrix
		glUniformMatrix4fv(graphicsState->activeShader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
		// Alpha
		glUniform4f(graphicsState->activeShader->uniformPrimaryColorVec4, primaryColor.x, primaryColor.y, primaryColor.z, primaryColor.w);
	}
	primaryColor.w -= linearDecay * graphicsState->frameTime;
	primaryColor.w *= pow(1.0f - relativeDecay, graphicsState->frameTime); 
	primaryColor.w = max(primaryColor.w, 0.0f);
	lastAlpha = primaryColor.w;
	relativeRotation.y += 0.2f * graphicsState->frameTime;

//	std::cout<<"\nAlpha: "<<primaryColor.w;

	// Render stuffs.
	model->mesh->Render();

}
