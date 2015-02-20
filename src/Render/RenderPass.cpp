/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.

#include "RenderPass.h"
#include "Entity/Entity.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "Graphics/GraphicsProperty.h"

#include "Graphics/Particles/ParticleSystem.h"

#include "AppStates/AppState.h"
#include "StateManager.h"

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
	// Default no vertex buffer.
	graphicsState.BindVertexArrayBuffer(0);

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
		default:
			assert(false);
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
		glUniformMatrix4fv(shader->uniformViewProjectionMatrix, 1, false, viewProj.getPointer());
	}
	else {
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, camera.ViewMatrix4f().getPointer());
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());
	}
	CheckGLError("RenderPass, setting camera matrices");
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
	// Check for instancing before we start.
	instancingEnabled = false;
	if (shader->uniformInstancingEnabled != -1) // should be more or less guaranteed for any instancing renderer based on models.
		instancingEnabled = true;


	switch(input)
	{
		/// Use previously rendered-to render-buffers associated with this viewport.
		case RenderTarget::DEFERRED_GATHER:
		{
		
			break;
		}
		case RenderTarget::SHADOW_CASTING_ENTITIES:
		{
			/// If possible, part instanced, part individually.
			if (instancingEnabled)
			{
				entitiesToRender = graphicsState.shadowCastingEntitiesNotInstanced;
				entityGroupsToRender = graphicsState.shadowCastingEntityGroups;	
			}
			/// Default, one at a time.
			else 
			{
				entitiesToRender = graphicsState.shadowCastingEntities;
			}
			RenderEntitiesOnlyVertices();
			break;
		}
		case RenderTarget::ENTITIES:
		{
			entitiesToRender = graphicsState.entities;
			RenderEntities();
			break;
		}
		case RenderTarget::SOLID_ENTITIES:
		{
			entitiesToRender = graphicsState.solidEntities;
			RenderEntities();
			// Only entities for now!
			break;
		}
		case RenderTarget::ALPHA_ENTITIES:
		{
			entitiesToRender = graphicsState.alphaEntities;
			RenderAlphaEntities();
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
		case RenderTarget::SKY_BOX:
		{
			RenderSkyBox();
			break;
		}
		default:
			assert(false);
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
			GLboolean wasEnabled = glIsEnabled(GL_SCISSOR_TEST);
			// Disable scissor for proper clear?
			glDisable(GL_SCISSOR_TEST);
			// Clear depth  and color for our target.
			glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
			// Ensure depth-testing etc. is enabled. 
			glDepthMask(true);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			break;
		}
		case RenderTarget::DEFERRED_GATHER:
		{
			// Set up/fetch render buffers for this, based on the viewport.
			if (!graphicsState->activeViewport->BindFrameBuffer())
				return false;
			break;
		}
		default:
			assert(false);
	}
	return true;
}



void RenderPass::RenderEntities()
{
	bool optimized = true;
	// Old here.
	if (!optimized)
	{
		graphicsState->modelMatrixD.LoadIdentity();
		graphicsState->modelMatrixF.LoadIdentity();
		Timer timer;
		timer.Start();
		// Render all entities listed in the graphicsState!
		for (int i = 0; i < entitiesToRender.Size(); ++i)
		{
			Entity * entity = entitiesToRender[i];
			entity->Render(*graphicsState);
		}
		timer.Stop();
		FrameStats.renderEntities += timer.GetMs();
	}
	// New here!
	Timer timer;
//	timer.Start();
//	entitiesToRender.SortByDistanceToCamera(graphicsState->camera);
//	timer.Stop();
//	FrameStats.renderSortEntities += timer.GetMs();

	timer.Start();
	Texture * diffuseMap = NULL, * specularMap = NULL, * emissiveMap = NULL,
		* normalMap = NULL;

	// Set render state for all.
	glUniform1i(shader->uniformUseDiffuseMap, 1);
	glUniform1i(shader->uniformUseSpecularMap, 1);
	glUniform1i(shader->uniformUseNormalMap, 0);

	Entity * entity;
	GraphicsProperty * gp;
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		gp = entity->graphics;
		if (entity->diffuseMap != diffuseMap)
		{
			diffuseMap = entity->diffuseMap;
			// Optimized per-entity render.
			int error = 0;
			// To send to the shadar
			int texturesToApply = 0;
			// Bind texture if it isn't already bound.
			glActiveTexture(GL_TEXTURE0 + shader->diffuseMapIndex);		// Select server-side active texture unit
			// Bind texture
			glBindTexture(GL_TEXTURE_2D, diffuseMap? diffuseMap->glid : 0);
			/// Sets glTExParameter for Min/Mag filtering <- this needed every time?
			diffuseMap->SetSamplingMode();
		}
		if (entity->specularMap != specularMap)
		{
			specularMap = entity->specularMap;
			// Specular
			glActiveTexture(GL_TEXTURE0 + shader->specularMapIndex);		// Select server-side active texture unit
			glBindTexture(GL_TEXTURE_2D, specularMap? specularMap->glid : 0);
			if (specularMap)
			{
				specularMap->SetSamplingMode();
			}
		}
		if (entity->normalMap != normalMap)
		{
			normalMap = entity->normalMap;
			// Normal map
			glActiveTexture(GL_TEXTURE0 + shader->normalMapIndex);		
			glBindTexture(GL_TEXTURE_2D, normalMap? normalMap->glid : 0);
		}
		if (entity->emissiveMap != emissiveMap)
		{
			emissiveMap = entity->emissiveMap;
			/// Bind emissive map.
			glActiveTexture(GL_TEXTURE0 + shader->emissiveMapIndex);		// Select server-side active texture unit
			glBindTexture(GL_TEXTURE_2D, emissiveMap? emissiveMap->glid : 0);
		}

		// Set multiplicative base color (1,1,1,1) default.
//		glUniform4fv(shader->uniformPrimaryColorVec4, 1, gp->color.v);

		// Load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->transformationMatrix.getPointer());
		glUniformMatrix4fv(shader->uniformNormalMatrix, 1, false, entity->normalMatrix.getPointer());

		// Render the model
		entity->model->Render(*graphicsState);
		++graphicsState->renderedObjects;		// increment rendered objects for debug info
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();
	CheckGLError("RenderPass::RenderEntities");
}

void RenderPass::RenderAlphaEntities()
{
	bool optimized = true;
	Timer timer;
	timer.Start();
	entitiesToRender.SortByDistanceToCamera(graphicsState->camera);
	timer.Stop();
	FrameStats.renderSortEntities += timer.GetMs();

	timer.Start();
	Texture * diffuseMap, * specularMap, * emissiveMap;

	// Set render state for all.
	glUniform1i(shader->uniformUseDiffuseMap, 1);
	glUniform1i(shader->uniformUseSpecularMap, 1);
	glUniform1i(shader->uniformUseNormalMap, 0);

	/// Set blend modes
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// Disable depth write
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// Disable it if not already done so earlier..?
//	glDisable(GL_DEPTH_TEST);

	Entity * entity;
	GraphicsProperty * gp;
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		if (!optimized)
		{
			entity->Render(*graphicsState);
			continue;
		}
		gp = entity->graphics;
		diffuseMap = entity->diffuseMap;
		specularMap = entity->specularMap;
		emissiveMap = entity->emissiveMap;
		// Optimized per-entity render.
		int error = 0;
		// To send to the shadar
		int texturesToApply = 0;
		// Bind texture if it isn't already bound.
		glActiveTexture(GL_TEXTURE0 + shader->diffuseMapIndex);		// Select server-side active texture unit
		// Bind texture
		glBindTexture(GL_TEXTURE_2D, diffuseMap? diffuseMap->glid : 0);
		/// Sets glTExParameter for Min/Mag filtering <- this needed every time?
		diffuseMap->SetSamplingMode();
		// Specular
		glActiveTexture(GL_TEXTURE0 + shader->specularMapIndex);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, specularMap? specularMap->glid : 0);
		if (specularMap)
		{
			specularMap->SetSamplingMode();
		}
		/// Bind emissive map.
		glActiveTexture(GL_TEXTURE0 + shader->emissiveMapIndex);		// Select server-side active texture unit
		glBindTexture(GL_TEXTURE_2D, emissiveMap? emissiveMap->glid : 0);

		CheckGLError("Setting texture maps");
		
		// Just load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->transformationMatrix.getPointer());
		glUniformMatrix4fv(shader->uniformNormalMatrix, 1, false, entity->normalMatrix.getPointer());
	
		
		CheckGLError("Matrices");

		// Set multiplicative base color (1,1,1,1) default.
		glUniform4fv(shader->uniformPrimaryColorVec4, 1, gp->color.v);

		// Render the model
		entity->model->Render(*graphicsState);
		++graphicsState->renderedObjects;		// increment rendered objects for debug info
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();
}

