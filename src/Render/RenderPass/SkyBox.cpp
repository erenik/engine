/// Emil Hedemalm
/// 2015-02-20
/// Separation into files, as it has started confusing me.
/// Rendering the sky box.

#include "Render/RenderPass.h"
#include "Lighting.h"

void RenderPass::RenderSkyBox()
{
	// Grab an entity for comparison...
	if (ActiveShader() == 0)
		return;

	/// Render sphere from inside-out, so change clock-wise thingy for the culling, or just disable culling briefly..
	glDisable(GL_CULL_FACE);
	glCullFace(GL_NONE);
//	glFrontFace(GL_CW /* or GL_CCW */);

	// Set up camera.
	// Grab viewmatrix.
	Matrix4f viewMatrix = graphicsState->camera->ViewMatrix4f();
	Matrix4f rotMatrix = graphicsState->camera->RotationMatrix4f();
	Matrix4f invView = rotMatrix.InvertedCopy();
	
	// Was here.

	// Disable depth test and depth-write.
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	if (shader->uniformSunPosition != -1)
	{
		// Set it! o.o
		List<Light*> lights = graphicsState->lighting->GetLights();
		for (int i = 0; i < lights.Size(); ++i)
		{
			Light * light = lights[i];
			if (light->isStar)
			{
				Vector3f normedPos = light->position.NormalizedCopy();
				glUniform3fv(shader->uniformSunPosition, 1, normedPos.v); 
				glUniform4fv(shader->uniformSunColor, 1, light->diffuse.v); 
			}
		}
		// Set sky-color.
		glUniform3fv(shader->uniformSkyColor, 1, graphicsState->lighting->skyColor.v);
	}
	/// Load it into shader.
	glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, rotMatrix.getPointer());
	// Get da box.
	Model * box = ModelMan.GetModel("cube");
	if (!box)
		return;
	box->BufferizeIfNeeded();
	box->Render();
	return;		
}
