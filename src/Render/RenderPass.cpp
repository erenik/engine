/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.

#include "RenderPass.h"
#include "GraphicsState.h"
#include "Entity/Entity.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"

#include "Physics/PhysicsManager.h"
#include "PhysicsLib/Shapes/AABB.h"

#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "Viewport.h"

#include "Graphics/Particles/ParticleSystem.h"
#include "Graphics/FrameStatistics.h"

#include "AppStates/AppState.h"
#include "StateManager.h"

#include "File/LogFile.h"
#include "String/StringUtil.h"

RenderPass::RenderPass()
{
	shader = 0;
	shaderName = "Phong";
	depthTestEnabled = true;
	type = RENDER_ENTITIES;
	camera = DEFAULT_CAMERA;

	lights = PRIMARY_LIGHT;
	shadowMapping = false;
	shadows = false;
	shadowMapResolution = 512;
	viewport = NULL;
}

RenderPass::~RenderPass()
{
}

// Renders this pass. Returns false if some error occured, usually mid-way and aborting the rest of the procedure.
bool RenderPass::Render(GraphicsState & graphicsState)
{
	viewport = graphicsState.activeViewport;
	CheckGLError("Before RenderPass::Render");
	// Setup output buffers?
	if (!SetupOutput())
		return false;
	/// Check basic type. If specific, call other procedures.
	switch(type)
	{
		case RENDER_APP_STATE:
		{
			AppState * state = StateMan.ActiveState();
			state->Render(&graphicsState);
			return true;
		}
		default:
			break;
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


	// Store active camera from the graphicsState
	Camera * oldCamera = graphicsState.camera;
	switch(camera)
	{
		case DEFAULT_CAMERA:
		{
			graphicsState.camera = oldCamera;
			break;
		}
		case RenderPass::LIGHT:
		{
			if (!SetupLightPOVCamera())
			{
				// Unbind the framebuffer so that UI and stuff will render as usual.
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				// Restore active camera from the graphicsState
				graphicsState.SetCamera(oldCamera);
		//		std::cout<<"\nNo shadows to cast. Skipping pass.";
				return true;
			}
			break;
		}
	}
	// If shadows are to be rendered, look for em in the light.
	if (shadows)
	{
		List<Light*> lights = graphicsState.lighting->GetLights();
		for (int i = 0; i < lights.Size(); ++i)
		{
			Light * light = lights[i];
			if (light->shadowMap)
			{
				// Add this light's shadow map to the list of shadow maps?
				// When rendering an object with this program.
				glActiveTexture(GL_TEXTURE0 + shader->shadowMapIndex);		// Select server-side active texture unit o.o;
				// Just one shadow map for now.
				light->shadowMapIndex = 0;
				// Set matrix.
				glUniformMatrix4fv(shader->uniformShadowMapMatrix, 1, false, light->shadowMappingMatrix.getPointer());
				// Bind texture
				glBindTexture(GL_TEXTURE_2D, light->shadowMap->glid);
				// o.o pew.
				glActiveTexture(GL_TEXTURE0);
			}
		}
	}
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
	// Reset model matrix..
	graphicsState.modelMatrixF.LoadIdentity();

	// Load in the model and view matrices from selected camera.
	if (shader->uniformViewProjectionMatrix != -1)
	{
		Matrix4f viewProj = camera.ViewProjectionF();
		Vector4f stuff = viewProj * Vector4f(0,0,0,1);
		stuff /= stuff.w;
		Vector4f stuff2 = viewProj * Vector4f(0,0,-20.f,1);
		stuff2 /= stuff2.w;

		glUniformMatrix4fv(shader->uniformViewProjectionMatrix, 1, false, viewProj.getPointer());
		CheckGLError("RenderPass, setting viewProj");
	}
	else {
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, camera.ViewMatrix4f().getPointer());
		CheckGLError("RenderPass, setting view matrix");
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
		CheckGLError("RenderPass, setting model matrix");
	}
	// Load projection matrix into shader
	graphicsState.projectionMatrixF = graphicsState.projectionMatrixD;
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, camera.ProjectionMatrix4f().getPointer());

	Matrix4f mvp = graphicsState.projectionMatrixF * graphicsState.viewMatrixF * graphicsState.modelMatrixF;

	// Set parameters for regular rendering her!
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Set other matrix
	//...
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set fog uniforms as needed.
	shader->SetupFog(graphicsState);

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
		glUniform4f(shader->uniformEyePosition, camera.Position()[0], camera.Position()[1], camera.Position()[2], 1.0f);
		CheckGLError("RenderPass, eye position");
	}	
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
	CheckGLError("RenderPass::Render - rendering");
	/// Extract the texture data from the buffers to see what it looks like?
	switch(output)
	{
		case RenderTarget::SHADOW_MAPS:
		{
			/// Save current shadow maps into where it should be saved. E.g. the light that casted it, and/or into the graphicsState.
			if (graphicsState.activeViewport->printShadowMaps)
			{
				graphicsState.activeViewport->printShadowMaps = false;
				graphicsState.activeViewport->shadowMapDepthBuffer->DumpTexturesToFile();
			}
			// Reset viewport to default.
			graphicsState.activeViewport->SetGLViewport();
			break;	
		}
		case RenderTarget::DEFERRED_GATHER:
		{
			assert(false);
//			graphicsState.activeViewport->frameBuffer->DumpTexturesToFile();
			break;
		}
	}
	CheckGLError("RenderPass::Render - extraction");
	// Unbind the framebuffer so that UI and stuff will render as usual.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Restore active camera from the graphicsState
	graphicsState.SetCamera(oldCamera);
	CheckGLError("RenderPass::Render");
	return true;
}

bool RenderPass::SetupOutput()
{
	// Target determines glViewport, glFrameBuffer, etc.
	switch(output)
	{
		case RenderTarget::DEFAULT:
		{
			// Set default render target.. however one does that.
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			viewport->SetGLViewport();
			graphicsState->shadowPass = false;
			break;
		}
		case RenderTarget::SHADOW_MAPS:
		{
			// Fetch shadow-map framebuffer for this viewport?
			BindShadowMapFrameBuffer();
			graphicsState->shadowPass = true;
			break;
		}
		case RenderTarget::DEFERRED_GATHER:
		{
			// Set up/fetch render buffers for this, based on the viewport.
			if (!graphicsState->activeViewport->BindFrameBuffer())
				return false;
			break;
		}
	}
	bool wasEnabled = glIsEnabled(GL_SCISSOR_TEST);
	// Disable scissor for proper clear?
	glDisable(GL_SCISSOR_TEST);
	// Clear depth  and color for our target.
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RenderPass::SetupLightPOVCamera()
{
	// Now it becomes tricky..
	static Camera * camera = NULL;
	bool anyShadows = false;
	if (camera == NULL)
		camera = CameraMan.NewCamera("LightPOVCamera");

	Lighting * lighting = graphicsState->lighting;
	List<Light*> lights = lighting->GetLights();
	for (int i = 0; i < lights.Size(); ++i)
	{
		Light * light = lights[i];
		if (!light->castsShadow)
			continue;
		if (light->type != LightType::DIRECTIONAL)
		{
			std::cout<<"\nLight types beside directional not supported for shadow mappinag at the moment.";
			continue;
		}
		anyShadows = true;
		
		Vector3f lightPosition = light->position;
		Vector3f lpn = lightPosition.NormalizedCopy();
		// Grab AABB of all relevant entities? Check the AABB-sweeper or other relevant handler?
		AABB allEntitiesAABB = PhysicsMan.GetAllEntitiesAABB();
		float mDist = allEntitiesAABB.max.Length();
		float m2Dist = allEntitiesAABB.min.Length();
		float maxDist = max(mDist, m2Dist);

		// Distance + some buffer.
		float farPlane = light->shadowMapFarplane;
		// Zoom will vary with how wide/long the area is which has to be visible/shadowed.
		float zoom = light->shadowMapZoom;
		// Adjust light position based on the gathered data above.
//		lightPosition = lpn * maxDist;
		
//		LogGraphics("Setting sun: farplane "+String(farPlane)+" zoom: "+String(zoom)+" Position: "+VectorString(lightPosition), INFO);
		
		camera->projectionType = Camera::ORTHOGONAL;
		camera->position = lightPosition;
		
		// Set rotation based on position?
		Vector2f xz(lpn.x, lpn.z);
		float xzLen = xz.Length();
		xz.Normalize();
		Angle yaw = Angle(xz);
		yaw -= Angle(PI/2);
//		std::cout<<"Yaw: "<<yaw.Degrees();
		Angle pitch = Angle(xzLen, lpn.y);
		camera->rotation.y = yaw.Radians();
		camera->rotation.x = pitch.Radians();
		camera->zoom = zoom;
		camera->SetRatioF(1,1);
		camera->farPlane = farPlane;
		camera->Update();
		Vector3f forward = camera->LookingAt();
		Vector3f up = camera->UpVector();
		Matrix4f viewProjection = camera->ViewProjectionF();
		Matrix4f viewProj2 = camera->ViewMatrix4f() * camera->ProjectionMatrix4f();
		Vector3f pos3 = viewProj2 * Vector4f(0,0,0,1);
		Vector3f position = viewProjection * Vector4f(0,0,0,1),
			position2 = viewProjection * Vector4f(10,0,0,1),
			position3 = viewProjection * Vector4f(0,10,0,1),
			position4 = viewProjection * Vector4f(0,0,10,1);
		graphicsState->SetCamera(camera);
		light->shadowMapIndex = 0;
		float elements [16] = {	0.5, 0, 0, 0,
								0, 0.5, 0, 0,
								0, 0, 0.5, 0,
								0.5, 0.5, 0.5, 1};
		Matrix4f biasMatrix(elements);
		Vector3f vec = biasMatrix * Vector3f(0,0,0);
		Vector3f vec2 = biasMatrix * Vector3f(0.5f, 0.5f, 0.5f);
		Matrix4f shadowMappingMatrix = biasMatrix * camera->ViewProjectionF();
		Vector3f shadowSpace = shadowMappingMatrix * Vector3f(0,0,0);
		Vector3f s2 = shadowMappingMatrix * Vector3f(10,0,0),
			s3 = shadowMappingMatrix * Vector3f(0,10,0),
			s4 = shadowMappingMatrix * Vector3f(0,0,10);

		/// Set up a viewport similar to the shadow-map texture we are going to use!
		glViewport(0, 0, shadowMapResolution, shadowMapResolution);
		glDisable(GL_SCISSOR_TEST);

		light->shadowMappingMatrix = shadowMappingMatrix;
		// Take current shadow map texture we created earlier and make sure the camera is bound to it for usage later.
		// Save matrix used to render shadows properly later on?
//			light->inverseTransposeMatrix = ;
		light->shadowMap = graphicsState->activeViewport->shadowMapDepthBuffer->renderBuffers[0]->texture;
		assert(light->shadowMap);
		return true;
	}
	return false;
}

/// Creates it as needed.
bool RenderPass::BindShadowMapFrameBuffer()
{
	CheckGLError("Before RenderPass::BindShadowMapFrameBuffer");
	if (!viewport->shadowMapDepthBuffer)
	{
		viewport->shadowMapDepthBuffer = new FrameBuffer("ShadowMapDepthBuffer");
	}
	if (!viewport->shadowMapDepthBuffer->IsGood())
	{
		// Try and rebuild it..?
		if (!viewport->shadowMapDepthBuffer->CreateDepthBuffer(Vector2i(1, 1) * shadowMapResolution))
		{
			SAFE_DELETE(viewport->shadowMapDepthBuffer);
			return false;
		}
	}
	int error = glGetError();
	/// Make frame buffer active
	viewport->shadowMapDepthBuffer->Bind();
	// Set viewport size to clear?
	glViewport(0, 0, viewport->shadowMapDepthBuffer->size.x, viewport->shadowMapDepthBuffer->size.y);
	// Set buffers to render into (the textures ^^)
	viewport->shadowMapDepthBuffer->SetDrawBuffers();
	CheckGLError("RenderPass::BindShadowMapFrameBuffer");
	return true;
}



