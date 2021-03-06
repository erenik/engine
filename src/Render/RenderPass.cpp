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
	iterations = 1;
	shader = 0;
	shaderName = "Phong";
	depthTestEnabled = true;
	type = RENDER_ENTITIES;
	camera = DEFAULT_CAMERA;
	output = RenderTarget::DEFAULT;

	lights = PRIMARY_LIGHT;
	shadowMapping = false;
	shadows = false;
	shadowMapResolution = 512;
	viewport = NULL;

	sortBy = 0;
	sortByIncreasing = true;
	clear = true;
}

RenderPass::~RenderPass()
{
}

#include "Mesh/Square.h"

void RenderPass::RenderQuad(GraphicsState & graphicsState)
{
	/// Should have been set earlier, dun need to set again.
//	glViewport(0, 0, windowWorkingArea[0], windowWorkingArea[1]);

	/// Reset projection matrix.
//	graphicsState.projectionMatrixF = ();
//	shader->SetProjectionMatrix(graphicsState.projectionMatrixF);
//	shader->SetViewMatrix(graphicsState.projectionMatrixF);
	Matrix4f proj = Matrix4f();
	proj.LoadIdentity();
	proj.InitOrthoProjectionMatrix(-1,1,-1,1, 0.10f, 10.f);
	proj.Translate(0, 0, 1.f);

	int width = viewport->size.x, height = viewport->size.y;
	Vector4f point = Vector4f((float)viewport->size.x, (float)viewport->size.y, 0, 1.0f);
//	point = proj * point;
//	point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
//	point = proj * point;
//	point = Vector4f(0, 0, 0, 1.0f);
//	point = proj * point;

	Matrix4f view = Matrix4f();
	shader->SetProjectionMatrix(proj);
	Matrix4f translate, model;
//	translate.InitTranslationMatrix(Vector3f(0,0,+50.1f));
	view = translate;
	shader->SetViewMatrix(translate);
	glDisable(GL_CULL_FACE);
	shader->SetModelMatrix(Matrix4f());
	Vector3f renderedPoint;

	/*
	/// Update projection matrix
	Matrix4f projection = Matrix4f();
	projection.LoadIdentity();
	projection.InitOrthoProjectionMatrix(0, width, 0, height, 1.0f, 100.0f);
//	projection.InitProjectionMatrix(-1000, 1000, -500, 500, -1, -100);
	projection.Translate(0, 0, 1.0f);
	/// Just testing that the matrix is set correctly..
	Vector4f point = Vector4f((float)width, (float)height, 0, 1.0f);
	point = projection * point;
	point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
	point = projection * point;
	point = Vector4f(0, 0, 0, 1.0f);
	point = projection * point;
	PrintGLError("GLError in RenderUI getting uniform locations");
*/

/*	// Load matrices into shader
    Matrix4f view = Matrix4f();
    // Upload view matrix too...
    glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, view.getPointer());
    PrintGLError("GLError in RenderUI uploading viewMatrix");
	// Upload projection matrix
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, projection.getPointer());
    PrintGLError("GLError in RenderUI uploading projectionMatrix");

	graphicsState.projectionMatrixF = graphicsState.projectionMatrixD = projection;
	graphicsState.viewMatrixF = graphicsState.viewMatrixD.LoadIdentity();
	graphicsState.modelMatrixF.LoadIdentity();
	*/

	Matrix4f mvp = model * view * proj;
	Vector3f projectedPoint = mvp.Product(Vector4f(renderedPoint, 1));
	Vector3f projP2 = (graphicsState.modelMatrix * graphicsState.viewMatrixF * graphicsState.projectionMatrixF).Product(renderedPoint);
//	std::cout<<"\nProjected point: "<<projectedPoint<<" vs "<<projP2;
	// Allocate o-o
	static Square * deferredRenderingBox = 0;
	if (deferredRenderingBox == 0)
	{
		float size = 1.0f;
		deferredRenderingBox = new Square();
		deferredRenderingBox->SetDimensions(-size, size, -size, size);
		deferredRenderingBox->Bufferize(true);
	}
	
	deferredRenderingBox->name = "DeferredLighting";
	deferredRenderingBox->Render(&graphicsState);
}

// Renders this pass. Returns false if some error occured, usually mid-way and aborting the rest of the procedure.
bool RenderPass::Render(GraphicsState & graphicsState)
{
	viewport = graphicsState.activeViewport;
	CheckGLError("Before RenderPass::Render");
	// Set shader to use in this render-pass.
	if (!shader || (shader->name != shaderName))
	{
		shader = ShadeMan.SetActiveShader(&graphicsState, shaderName);
		if (!shader)
		{
			std::cout<<"\nSkipping render pass. Failed to grab specified shader: "<<shaderName;
			return false;
		}
	}
	else {
		ShadeMan.SetActiveShader(&graphicsState, shader);
	}
	// Return if we couldn't even set the shader..
    if (shader == NULL)
	    return false;
    
	// Setup output buffers?
	if (!SetupOutput(graphicsState))
		return false;
	/// Check basic type. If specific, call other procedures.
	switch(type)
	{
		case RENDER_APP_STATE:
		{
			AppState * state = StateMan.ActiveState();
			if (!state)
				return false;
			state->Render(&graphicsState);
			return true;
		}
		default:
			break;
	}
	
	// Default no vertex buffer.
	graphicsState.BindVertexArrayBuffer(0);

	graphicsState.settings |= ENABLE_SPECIFIC_ENTITY_OPTIONS;

	bool backfaceCullingEnabled = true;
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

	glDepthFunc(GL_LESS);

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
			/// value set for farplane?
			if (graphicsState.farPlane > 0)
				graphicsState.camera->farPlane = graphicsState.farPlane;
			break;
		}
		case RenderPass::LIGHT:
		{
			if (!SetupLightPOVCamera(graphicsState))
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
		List<Light*> lights = graphicsState.lighting.GetLights();
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
		ShadeMan.SetActiveShader(&graphicsState, nullptr);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (shader)
	{
		/// Load lighting settings to shader ^^
		graphicsState.lighting.LoadIntoShader(&graphicsState, shader);
	}
	
	// Set primary color
	glUniform4f(shader->uniformPrimaryColorVec4, 1.f,1.f,1.f,1.f);

	// Reset bound textures
	graphicsState.currentTexture = NULL;

	shader = ActiveShader();
	// Set uniforms as applicable.
	if (shader)
	{
		// Update view and projection matrix in specified shader
		Vector3f camPos = camera.Position();
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
		glUniform4f(shader->uniformEyePosition, camPos.x, camPos.y, camPos.z, 1.0f);
		CheckGLError("RenderPass, eye position");
	}	
	// Check for instancing before we start.
	instancingEnabled = false;
	assert(shader);
	if (shader->uniformInstancingEnabled != -1) // should be more or less guaranteed for any instancing renderer based on models.
		instancingEnabled = true;


	bool renderQuad = false;
	switch(input)
	{
		/// Use previously rendered-to render-buffers associated with this viewport.
		case RenderTarget::DEFERRED_GATHER:
		{
			SetupDeferredGatherAsInput(graphicsState);
			renderQuad = true;
			break;
		}
		case RenderTarget::DEFERRED_OUTPUT:
		{
			SetupDeferredOutputAsInput(graphicsState);
			/// Render the quad.
			renderQuad = true;
			break;
		}
		case RenderTarget::POST_PROCESS_OUTPUT:
			viewport->postProcessOutputBuffer->BindTexturesForSampling(shader, graphicsState);
			renderQuad = true;
			break;
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
			RenderEntitiesOnlyVertices(graphicsState);
			break;
		}
		case RenderTarget::ENTITIES:
		{
			entitiesToRender = graphicsState.entities;
			RenderEntities(graphicsState);
			break;
		}
		case RenderTarget::ENTITY_GROUP:
		{
			for (int i = 0; i < graphicsState.entityGroups.Size(); ++i)
			{
				EntityGroup * eg = graphicsState.entityGroups[i];
				if (eg->name == inputGroup)
				{
					entitiesToRender = *eg;
					RenderEntities(graphicsState);
				}
				break;
			}
		}
		case RenderTarget::REMAINING_ENTITIES:
		{
			entitiesToRender = *graphicsState.defaultEntityGroup;
			RenderEntities(graphicsState);
			break;
		}
		case RenderTarget::SOLID_ENTITIES:
		{
//			instancingEnabled = false;
			/// If possible, part instanced, part individually.
			if (instancingEnabled)
			{
				entitiesToRender = graphicsState.solidEntitiesNotInstanced;
				entityGroupsToRender = graphicsState.solidEntityGroups;	
			}
			/// Default, one at a time.
			else 
			{
				entitiesToRender = graphicsState.solidEntities;
			}
			RenderEntities(graphicsState);
			// Only entities for now!
			break;
		}
		case RenderTarget::ALPHA_ENTITIES:
		{
			entitiesToRender = graphicsState.alphaEntities;
			RenderAlphaEntities(graphicsState);
			break;
		}
		case RenderTarget::PARTICLE_SYSTEMS:
		{
			// Render all registered particle systems.
			for (int i = 0; i < graphicsState.particleEffectsToBeRendered.Size(); ++i)
			{
				auto ps = graphicsState.particleEffectsToBeRendered[i];
				ps->Render(&graphicsState);
			}
			break;
		}
		case RenderTarget::SKY_BOX:
		{
			RenderSkyBox(graphicsState);
			break;
		}
		default:
			assert(false);
	}

	/// Do the render for those with quad (deferred).
	if (renderQuad)
	{
		if (iterations > 1)			
			PerformIterativePingPongRenders(graphicsState);
		else
			RenderQuad(graphicsState);
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
			/// Outputs should already be taken core of during initialization, but maybe print something?
			// Set defaults for... stuff.
			if (debug == 18)
				graphicsState.activeViewport->deferredGatherBuffer->DumpTexturesToFile();
//			assert(false);
//			graphicsState.activeViewport->frameBuffer->DumpTexturesToFile();
			break;
		}
		case RenderTarget::DEFERRED_OUTPUT:
			if (debug == 19)
				graphicsState.activeViewport->deferredOutputBuffer->DumpTexturesToFile();
			break;
	}
	CheckGLError("RenderPass::Render - extraction");
	// Unbind the framebuffer so that UI and stuff will render as usual.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Restore active camera from the graphicsState
	graphicsState.SetCamera(oldCamera);
	CheckGLError("RenderPass::Render");
	return true;
}

bool RenderPass::SetupOutput(GraphicsState& graphicsState)
{
	// Target determines glViewport, glFrameBuffer, etc.
	switch(output)
	{
		case RenderTarget::DEFAULT:
		{
			// Set default render target.. however one does that.
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			if (viewport == nullptr)
				break;
			viewport->SetGLViewport();
			graphicsState.shadowPass = false;
			// Ensure depth-testing etc. is enabled. 
			glDepthMask(true);
			break;
		}
		case RenderTarget::SHADOW_MAPS:
		{
			// Fetch shadow-map framebuffer for this viewport?
			BindShadowMapFrameBuffer(graphicsState);
			graphicsState.shadowPass = true;
			GLboolean wasEnabled = glIsEnabled(GL_SCISSOR_TEST);
			// Disable scissor for proper clear?
			glDisable(GL_SCISSOR_TEST);
			// Ensure depth-testing etc. is enabled. 
			glDepthMask(true);			
			break;
		}
		case RenderTarget::DEFERRED_GATHER:
		{
			// Set up/fetch render buffers for this, based on the viewport.
			BindDeferredGatherFrameBuffer(graphicsState);
			break;
		}
		case RenderTarget::DEFERRED_OUTPUT:
		{
			BindDeferredOutputFrameBuffer(graphicsState);
			break;
		}
		case RenderTarget::POST_PROCESS_OUTPUT:
		{
			BindPostProcessOutputFrameBuffer(graphicsState);
			break;
		}
		case RenderTarget::MINIFICATION_BUFFERS:
		{
			/// Do the minify!
			// Do rendering later.
			break;
		}
		default:
			assert(false);
	}
	// Clear the screen if set so.
	if (clear)
	{
		// Clear depth  and color for our target.
		glClearColor(0.1f, 0.1f, 0.1f, 0.0f); // Use prescribed clearColor instead of random grey later?
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	return true;
}

/// 1 X, 2 Y, 3 Z, list of entities.
void SortBy(int axis, List< Entity* > & entities, bool increasing)
{
	int currentItems = entities.Size();
	Entity* entity;
	Entity* entity2;
	Entity* tmp;
	/// Use insertion sort, as we can assume that the entities will remain nearly sorted all the time?
	/// http://en.wikipedia.org/wiki/Insertion_sort
	for (int i = 0; i < currentItems; ++i)
	{
		// Calculate distances as we go.
		entity = entities[i];
		// Get value?
		float v = entity->worldPosition.z;
		tmp = entity;
		// Compare with previous items.
		for (int j = i - 1; j >= 0; --j)
		{
			entity2 = entities[j];
			float v2 = entity2->worldPosition.z;
			// If zdepth is lower on tmp, move entity2 up one step.
			if (v < v2)
			{
				entities[j + 1] = entities[j]; 
			}
			// Once we find another item with a lesser depth, place tmp in the previous spot.
			else {
				entities[j + 1] = tmp;
				tmp = 0;
				// Break inner loop.
				break;
			}
		}
		// Special case, placing the closest one.
		if (tmp)
			entities[0] = tmp;
	}
}

void RenderPass::RenderEntities(GraphicsState & graphicsState)
{
	// Sort entities?
	if (sortBy > 0)
		SortBy(sortBy, entitiesToRender, sortByIncreasing);

	bool optimized = true;
	// Old here.
	if (!optimized)
	{
		graphicsState.modelMatrixD.LoadIdentity();
		graphicsState.modelMatrixF.LoadIdentity();
		Timer timer;
		timer.Start();
		// Render all entities listed in the graphicsState!
		for (int i = 0; i < entitiesToRender.Size(); ++i)
		{
			Entity* entity = entitiesToRender[i];
			entity->Render(graphicsState);
		}
		timer.Stop();
		FrameStats.renderEntities += timer.GetMs();
	}
	// New here!
	Timer timer;

	timer.Start();
	Texture * diffuseMap = NULL, * specularMap = NULL, * emissiveMap = NULL,
		* normalMap = NULL;

	// Set render state for all.
	glUniform1i(shader->uniformUseDiffuseMap, 1);
	glUniform1i(shader->uniformUseSpecularMap, 1);
	glUniform1i(shader->uniformUseNormalMap, 0);


	/// Instancing enabled?
	if (instancingEnabled)
	{	
		// Turn it off first, though.
		glUniform1i(shader->uniformInstancingEnabled, 0);
	}
	Entity* entity;
	GraphicsProperty * gp;
	// Render all entities listed in the graphicsState!
	bool depthWrite = true;
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		if (!entity->IsVisible())
			continue;
		gp = entity->graphics;
		if (entity->model == nullptr) {
			LogGraphics("Attempting to render entity with nullptr model.", LogLevel::WARNING);
			continue;
		}
		/// Set entity depth write as needed.
		if (depthWrite != gp->depthWrite)
		{
			depthWrite = gp->depthWrite;
			glDepthMask(depthWrite);
		}
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
			if (diffuseMap)
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
		if (shader->uniformEmissiveMap != -1 && entity->emissiveMap != emissiveMap)
		{
			emissiveMap = entity->emissiveMap;
			/// Bind emissive map.
			glActiveTexture(GL_TEXTURE0 + shader->emissiveMapIndex);		// Select server-side active texture unit
			glBindTexture(GL_TEXTURE_2D, emissiveMap? emissiveMap->glid : 0);
		}
		if (entity->emissiveMap && shader->uniformEmissiveMapFactor != -1)
		{
			// Setup emissive factor for those entities requiring varying values of it.
			glUniform1f(shader->uniformEmissiveMapFactor, gp->emissiveMapFactor);
		}

		glUniform4f(shader->uniformPrimaryColorVec4, entity->graphics->color[0], entity->graphics->color[1], entity->graphics->color[2], entity->graphics->color[3]);

		// Set multiplicative base color (1,1,1,1) default.
//		glUniform4fv(shader->uniformPrimaryColorVec4, 1, gp->visuals.color.v);

		// Load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->renderTransform->getPointer());
		glUniformMatrix4fv(shader->uniformNormalMatrix, 1, false, entity->normalMatrix.getPointer());

		// Render the model
		entity->model->Render(&graphicsState);
		++graphicsState.renderedObjects;		// increment rendered objects for debug info
	}
	if (instancingEnabled)
	{
		glUniform1i(shader->uniformInstancingEnabled, 1);
		for (int i = 0; i < entityGroupsToRender.Size(); ++i)
		{
			RenderInstancingGroup * group = entityGroupsToRender[i];
			group->Render(&graphicsState);
		}
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();
	CheckGLError("RenderPass::RenderEntities");


	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		// Do one for now...
		entity = entitiesToRender[i];
		if (entity->graphics->text.Length() == 0)
			continue;
		entity->RenderText(graphicsState);
	}
}

void RenderPass::RenderAlphaEntities(GraphicsState & graphicsState)
{
	bool optimized = true;
	Timer timer;
	timer.Start();
	entitiesToRender.SortByDistanceToCamera(graphicsState.camera);
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
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Default, additive
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard rendering, also used for sprites.
	glEnable(GL_DEPTH_TEST); // Don't just ignore depth..
	// Disable it if not already done so earlier..?
//	glDisable(GL_DEPTH_TEST);

	Entity* entity;
	GraphicsProperty * gp;
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		if (!optimized)
		{
			entity->Render(graphicsState);
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
		if (diffuseMap)
			diffuseMap->SetSamplingMode();
		// Specular
		if (shader->uniformSpecularMap != -1)
		{
			glActiveTexture(GL_TEXTURE0 + shader->specularMapIndex);		// Select server-side active texture unit
			glBindTexture(GL_TEXTURE_2D, specularMap? specularMap->glid : 0);
			if (specularMap)
			{
				specularMap->SetSamplingMode();
			}
		}
		/// Bind emissive map.
		if (shader->uniformEmissiveMap != -1)
		{
			glActiveTexture(GL_TEXTURE0 + shader->emissiveMapIndex);		// Select server-side active texture unit
			glBindTexture(GL_TEXTURE_2D, emissiveMap? emissiveMap->glid : 0);
			glUniform1f(shader->uniformEmissiveMapFactor, gp->emissiveMapFactor);
		}
		CheckGLError("Setting texture maps");
		
		// Just load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->renderTransform->getPointer());
		if (shader->uniformNormalMatrix != -1)
			glUniformMatrix4fv(shader->uniformNormalMatrix, 1, false, entity->normalMatrix.getPointer());
	
		
		CheckGLError("Matrices");

		// Set multiplicative base color (1,1,1,1) default.
		glUniform4fv(shader->uniformPrimaryColorVec4, 1, gp->color.v);

		// Render the model
		entity->model->Render(&graphicsState);
		++graphicsState.renderedObjects;		// increment rendered objects for debug info
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();
}

