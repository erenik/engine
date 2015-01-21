/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.

#include "RenderPass.h"
#include "GraphicsState.h"
#include "Entity/Entity.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Camera/Camera.h"

#include "RenderBuffer.h"
#include "Viewport.h"

#include "Graphics/Particles/ParticleSystem.h"
#include "Graphics/FrameStatistics.h"

RenderPass::RenderPass()
{
	shader = 0;
	shaderName = "Phong";
	depthTestEnabled = true;
}


// Renders this pass. Returns false if some error occured, usually mid-way and aborting the rest of the procedure.
bool RenderPass::Render(GraphicsState & graphicsState)
{
	switch(output)
	{
		case RenderTarget::DEFAULT:
		{
			// Set default render target.. however one does that.
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			break;
		}
		case RenderTarget::DEFERRED_GATHER:
		{
			// Set up/fetch render buffers for this, based on the viewport.
			if (!graphicsState.activeViewport->BindFrameBuffer())
				return false;
			break;
		}
	}

	// Set shader to use in this render-pass.
	if (!shader)
	{
		shader = ShadeMan.SetActiveShader(shaderName);
		if (!shader)
		{
			std::cout<<"\nSkipping render pass. Failed to grab specified shader: "<<shaderName;
			return false;
		}
	}
	else {
		ShadeMan.SetActiveShader(shader);
	}
	// Return if we couldn't even set the shader..
    if (shader == NULL)
	{
        return false;
    }
	graphicsState.settings |= ENABLE_SPECIFIC_ENTITY_OPTIONS;

	// Set fog properties as needed.
	glUniform1f(shader->uniformFogBeginDistance, graphicsState.fogBegin);
	glUniform1f(shader->uniformFogEndDistance, graphicsState.fogEnd);
	glUniform3f(shader->uniformFogColor, graphicsState.clearColor.x, graphicsState.clearColor.y, graphicsState.clearColor.z);

	bool backfaceCullingEnabled = false;
	if (backfaceCullingEnabled){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
	//	glDisable(GL_CULL_FACE);
	}

	// Set some standard rendering options.
	if (depthTestEnabled)
	    glEnable(GL_DEPTH_TEST);
	else 
		glDisable(GL_DEPTH_TEST);

	glDepthMask(GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	// Reset matrices: This may as well be done before rendering is done, since the matrices
	// will need to be reset all the time depending on the content to be rendered and where ^^
	/// Camera is already updated, so just use it's matrices straight away ^^
	Camera & camera = *graphicsState.camera;

	// Camera calculations are now done inside the camera, so the camPos/camLookingAtVector/camUpVector can now be removed!
	// If view frustum culling is enabled, set it in the settings and update the frustum with the camera's current position.
	if (true /*useOctree && frustumCullingActive*/){
		graphicsState.viewFrustum.SetCamPos(Vector3f((Vector4f)camera.Position()), Vector3f((Vector4f)camera.LookingAt()), Vector3f((Vector4f)camera.UpVector()));
		Frustum & viewFrustum = graphicsState.viewFrustum;
	}

	if (!shader)
		return false;

	// Load in the model and view matrices
	glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
	CheckGLError("RenderPass, setting view matrix");
	glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	CheckGLError("RenderPass, setting model matrix");

	// Load projection matrix into shader
	graphicsState.projectionMatrixF = graphicsState.projectionMatrixD;
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());

	Matrix4f mvp = graphicsState.projectionMatrixF * graphicsState.viewMatrixF * graphicsState.modelMatrixF;

	// Set parameters for regular rendering her!
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Set other matrix
	//...
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	/// Set legacy rendering setting
	bool useLegacy = false;
	if (useLegacy)
		graphicsState.settings |= USE_LEGACY_GL;
	else
		graphicsState.settings &= ~USE_LEGACY_GL;

	/// Deferred rendering setting ^^
	bool useDeferred = false;
	/// Switch off deferred if GL version is too low. 3.0 is require for handling gl*Framebuffer functions
	/// Seems buggy even at 3.ish, maybe raise cap to 4?
	if (GL_VERSION_MAJOR < 3)
		useDeferred = false;
  //  std::cout<<"\nUsing deferred: "<<useDeferred;
/*	 // If support is via EXT (OpenGL version < 3), add the EXT suffix; otherwise functions are core (OpenGL version >= 3)
    // or ARB without the EXT suffix, so just get the functions on their own.
    std::string suffix = (support_framebuffer_via_ext ? "EXT" : "");
	// Bind functions
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress((std::string("glGenFramebuffers") + suffix).c_str());
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress((std::string("glDeleteFramebuffers") + suffix).c_str());
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress((std::string("glBindFramebuffer") + suffix).c_str());
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress((std::string("glFramebufferTexture2D") + suffix).c_str());
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress((std::string("glCheckFramebufferStatus") + suffix).c_str());
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress((std::string("glGenerateMipmap") + suffix).c_str());
*/

	if (graphicsState.settings & USE_LEGACY_GL){
		// Set default shader program
		shader = NULL;
		ShadeMan.SetActiveShader(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (shader)
	{
		/// Load lighting settings to shader ^^
		LoadLighting(graphicsState.lighting, shader);
	}
	
	// Set primary color
	glUniform4f(shader->uniformPrimaryColorVec4, 1.f,1.f,1.f,1.f);

	// Reset bound textures
	graphicsState.currentTexture = NULL;

	Shader * shader = ActiveShader();
	// Set uniforms as applicable.
	if (shader)
	{
		// Update view and projection matrix in specified shader
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
		glUniform4f(shader->uniformEyePosition, camera.Position().x, camera.Position().y, camera.Position().z, 1.0f);
	}
	
	CheckGLError("RenderPass, eye position");


	switch(input)
	{
		/// Use previously rendered-to render-buffers associated with this viewport.
		case RenderTarget::DEFERRED_GATHER:
		{
		
			break;
		}
		case RenderTarget::ENTITIES:
		{
			Entities entitiesToRender = graphicsState.entities;
			Timer timer;
			timer.Start();
			entitiesToRender.SortByDistanceToCamera(graphicsState.camera);
			timer.Stop();
			FrameStats.renderSortEntities += timer.GetMs();
			timer.Start();
			// Render all entities listed in the graphicsState!
			for (int i = 0; i < entitiesToRender.Size(); ++i)
			{
				Entity * entity = entitiesToRender[i];
				entity->Render(graphicsState);
			}
			timer.Stop();
			FrameStats.renderEntities += timer.GetMs();
			// Only entities for now!
			break;
		}
		case RenderTarget::PARTICLE_SYSTEMS:
		{
			// Render all registered particle systems.
			for (int i = 0; i < graphicsState.particleEffectsToBeRendered.Size(); ++i)
			{
				ParticleSystem * ps = graphicsState.particleEffectsToBeRendered[i];
				ps->Render(graphicsState);
			}
			break;
		}
	}
	/// Extract the texture data from the buffers to see what it looks like?
	switch(output)
	{
		case RenderTarget::DEFERRED_GATHER:
		{
			graphicsState.activeViewport->frameBuffer->DumpTexturesToFile();
			break;
		}
	}
	// Unbind the framebuffer so that UI and stuff will render as usual.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}


