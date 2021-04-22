// Emil Hedemalm
// 2013-06-15

#include "AlphaModelEffect.h"
#include "Model/ModelManager.h"
#include "GraphicsState.h"
#include "Graphics/Shader.h"

#include "Entity/Entity.h"

#include "Model/Model.h"

#ifndef max
#define max(a,b) ((a > b)? (a) : (b))
#endif

AlphaModelEffect::AlphaModelEffect(String name, String modelName, Entity* reference)
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
	Shader * shader = ActiveShader();
	assert(model);
	/// Override alpha-changes from outside if they decreased it!
	if (primaryColor[3] < lastAlpha)
		primaryColor[3] = lastAlpha,

	assert(entity);
	static Matrix4f modelMatrix;
	if (entity)
	{
		assert(false);
		/* // Commented out due to expected obsolescence.
		modelMatrix = Matrix4f::InitTranslationMatrix(entity->position);
		modelMatrix.Scale(entity->Radius() * (entity->scale.ElementMultiplication(relativeScale)).MaxPart());
		modelMatrix.Multiply(Matrix4f::InitRotationMatrixX(entity->rotation[0]));
		modelMatrix.Multiply(Matrix4f::InitRotationMatrixY(relativeRotation[1] + entity->rotation[1]));
		graphicsState.modelMatrixF = modelMatrix;
		*/
	};


	float& frameTime = graphicsState->frameTime;

	/// Set model matrix and other stuff.
	if (shader){
		// Model matrix
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
		// Alpha
		glUniform4f(shader->uniformPrimaryColorVec4, primaryColor[0], primaryColor[1], primaryColor[2], primaryColor[3]);
	}
	primaryColor[3] -= linearDecay * frameTime;
	primaryColor[3] *= pow(1.0f - relativeDecay, frameTime);
	primaryColor[3] = max(primaryColor[3], 0.0f);
	lastAlpha = primaryColor[3];
	relativeRotation[1] += 0.2f * frameTime;

//	std::cout<<"\nAlpha: "<<primaryColor[3];

	// Render stuffs.
	model->Render(graphicsState);

}
