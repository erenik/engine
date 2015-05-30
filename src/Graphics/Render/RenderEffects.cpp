// Emil Hedemalm
// 2013-06-15
#include "Graphics/GraphicsManager.h"
#include "Graphics/Effects/GraphicEffect.h"
#include "Graphics/Particles/ParticleSystem.h"
#include "GraphicsState.h"

/// Renders nicelish graphical effectslies! :#
void GraphicsManager::RenderEffects()
{
	CheckGLError("Before GraphicsManager::RenderEffects");
	
	/// Set blend modes
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// Disable depth write
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
//	glDisable(GL_CULL_FACE);
//	glCullFace(GL_FRONT);

	Shader * effect = ShadeMan.SetActiveShader("Effect");
    /// No rendering if the shader ain't compiled, ne?
    if (effect == NULL)
        return;
    glDisable(GL_TEXTURE_2D);
	// Set projection and view matrices
	glUniformMatrix4fv(effect->uniformProjectionMatrix, 1, false, graphicsState->projectionMatrixF.getPointer());
	glUniformMatrix4fv(effect->uniformViewMatrix, 1, false, graphicsState->viewMatrixF.getPointer());
	assert(effect);
	for (int i = 0; i < graphicsState->graphicEffectsToBeRendered.Size(); ++i)
	{
		graphicsState->graphicEffectsToBeRendered[i]->Render(*graphicsState);
	}
	CheckGLError("Post entity-effects GraphicsManager::RenderEffects");

    for (int i = 0; i < graphicsState->particleEffectsToBeRendered.Size(); ++i)
	{
		ParticleSystem * ps = graphicsState->particleEffectsToBeRendered[i];
		ps->Render(*graphicsState);
    }

	// Reset blend equation after particle effects are done.
	glBlendEquation(GL_FUNC_ADD);


	glDisable(GL_CULL_FACE);
	// Enable depth write
	glDepthMask(GL_TRUE);
	// Reset blend modes
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return;

	

};
