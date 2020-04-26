/// Emil Hedemalm
/// 2015-02-20
/// Separation into files, as it has started confusing me.
/// All functions related to setting up a shadow-mapping pre-pass.

#include "Render/RenderPass.h"
#include "Lighting.h"

#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "PhysicsLib/Shapes/AABB.h"

#include "Graphics/GraphicsProperty.h"

bool RenderPass::SetupLightPOVCamera(GraphicsState& graphicsState)
{
	// Now it becomes tricky..
	static Camera * camera = NULL;
	bool anyShadows = false;
	if (camera == NULL)
		camera = CameraMan.NewCamera("LightPOVCamera");

	Lighting * lighting = &graphicsState.lighting;
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
		float maxDist = MaximumFloat(mDist, m2Dist);

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
		GraphicsThreadGraphicsState.SetCamera(camera);
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
//		glViewport(-shadowMapResolution/2, -shadowMapResolution/2, shadowMapResolution, shadowMapResolution);
//		glViewport(0, 0, shadowMapResolution, shadowMapResolution);
		glDisable(GL_SCISSOR_TEST);

		light->shadowMappingMatrix = shadowMappingMatrix;
		// Take current shadow map texture we created earlier and make sure the camera is bound to it for usage later.
		// Save matrix used to render shadows properly later on?
//			light->inverseTransposeMatrix = ;
		light->shadowMap = GraphicsThreadGraphicsState.activeViewport->shadowMapDepthBuffer->renderBuffers[0]->texture;
		assert(light->shadowMap);
		return true;
	}
	return false;
}
/// Frame buffer object for deferred shading
GLuint frameBufferObject = 0;	// Main frame buffer object to use
GLuint depthBuffer = 0;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
GLuint positionTexture = 0, diffuseTexture = 0, depthTexture = 0, normalTexture = 0, specularTexture = 0, tangentTexture = 0, normalMapTexture = 0, pickingTexture = 0;
GLuint diffuseBuffer = 0, normalBuffer = 0, positionBuffer = 0, specularBuffer = 0;
// tangentBuffer = 0,	normalMapBuffer = 0, pickingBuffer = 0;

/// Creates it as needed.
bool RenderPass::BindShadowMapFrameBuffer(GraphicsState& graphicsState)
{
	CheckGLError("Before RenderPass::BindShadowMapFrameBuffer");
	if (!viewport->shadowMapDepthBuffer)
	{
		viewport->shadowMapDepthBuffer = new FrameBuffer("ShadowMapDepthBuffer");
	}
	if (!viewport->shadowMapDepthBuffer->IsGood() || viewport->shadowMapDepthBuffer->size != (Vector2i(1,1) * shadowMapResolution))
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

#include "Window/AppWindow.h"

bool newT = true;
// Set up/fetch render buffers for this, based on the viewport.
bool RenderPass::BindDeferredGatherFrameBuffer(GraphicsState& graphicsState)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
	glEnable(GL_DEPTH_TEST); // Enable depth testing

	Vector2i requestedRenderSize = viewport->size;
	if (viewport->window == MainWindow())
		requestedRenderSize = GraphicsThreadGraphicsState.renderResolution;

	if (!viewport->deferredGatherBuffer)
	{
		viewport->deferredGatherBuffer = new FrameBuffer("DeferredGatherBuffer");
		viewport->deferredGatherBuffer->useFloatingPointStorage = true;
	}
	if (!viewport->deferredGatherBuffer->IsGood() || viewport->deferredGatherBuffer->size != requestedRenderSize)
	{
		if (!viewport->deferredGatherBuffer->CreateDeferredBuffers(requestedRenderSize))
		{
			SAFE_DELETE(viewport->deferredGatherBuffer);
			return false;
		}
	}
	int error = glGetError();
	/// Make frame buffer active
	viewport->deferredGatherBuffer->Bind();

	// Set viewport size to render accordingly.
	if (viewport == MainWindow()->MainViewport())
		glViewport(0, 0, requestedRenderSize.x, requestedRenderSize.y);
	else
		viewport->SetGLViewport();
	// Set buffers to render into (the textures ^^)
	viewport->deferredGatherBuffer->SetDrawBuffers();
	CheckGLError("RenderPass::BindDeferredGatherFrameBuffer");
	return true;
}

bool RenderPass::BindDeferredOutputFrameBuffer(GraphicsState& graphicsState)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
	glEnable(GL_DEPTH_TEST); // Enable depth testing

	Vector2i requestedRenderSize = viewport->size;
	if (viewport->window == MainWindow())
		requestedRenderSize = GraphicsThreadGraphicsState.renderResolution;
	if (!viewport->deferredOutputBuffer)
	{
		viewport->deferredOutputBuffer = new FrameBuffer("DeferredOutputBuffer");
		viewport->deferredOutputBuffer->useFloatingPointStorage = true;
	}
	if (!viewport->deferredOutputBuffer->IsGood() || viewport->deferredOutputBuffer->size != requestedRenderSize)
	{
		if (!viewport->deferredOutputBuffer->CreateDeferredOutputBuffers(requestedRenderSize))
		{
			SAFE_DELETE(viewport->deferredOutputBuffer);
			return false;
		}
	}
	int error = glGetError();
	/// Make frame buffer active
	viewport->deferredOutputBuffer->Bind();
	// Set viewport size to render accordingly.
	if (viewport == MainWindow()->MainViewport())
		glViewport(0, 0, requestedRenderSize.x, requestedRenderSize.y);
	else
		viewport->SetGLViewport();
	// Set buffers to render into (the textures ^^)
	viewport->deferredOutputBuffer->SetDrawBuffers();
	CheckGLError("RenderPass::BindDeferredOutputFrameBuffer");
	return true;
}
bool RenderPass::BindPostProcessOutputFrameBuffer(GraphicsState& graphicsState)
{
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
	glDisable(GL_DEPTH_TEST); // Disable depth testing

	Vector2i requestedRenderSize = viewport->size;
	FrameBuffer * ppob = viewport->postProcessOutputBuffer;
	if (viewport->window == MainWindow())
		requestedRenderSize = GraphicsThreadGraphicsState.renderResolution;
	if (!ppob)
	{
		ppob = viewport->postProcessOutputBuffer = new FrameBuffer("PostProcessOutputBuffer");
		ppob->useFloatingPointStorage = true;
	}
	if (!ppob->IsGood() || ppob->size != requestedRenderSize)
	{
		if (!ppob->CreatePostProcessOutputBuffers(requestedRenderSize))
		{
			SAFE_DELETE(ppob);
			return false;
		}
	}
	int error = glGetError();
	/// Make frame buffer active
	ppob->Bind();
	// Set viewport size to render accordingly.
	if (viewport == MainWindow()->MainViewport())
		glViewport(0, 0, requestedRenderSize.x, requestedRenderSize.y);
	else
		viewport->SetGLViewport();
	// Set buffers to render into (the textures ^^)
	ppob->SetDrawBuffers();
	/// Assume some uniforms are relevant for this pass.
	viewport->exposure;
	Vector3f col = viewport->averageScreenColor;
	float brightness = (col.x + col.y + col.z) / 3.f;
	float minBrightness = 0.25f;
	float maxBrightness = 0.5f;
	float newExposure = viewport->exposure;
	if (newExposure != newExposure)
		newExposure = 1.f;
	if (brightness > maxBrightness)
		newExposure *= 0.67f;
	if (brightness < minBrightness)
		newExposure *= 1.5f;
	if (viewport->exposure != newExposure)
		std::cout<<"\nSetting exposure: "<<viewport->exposure;
	viewport->exposure = viewport->exposure * 0.90f + newExposure * 0.10f;
	ClampFloat(viewport->exposure, 0.05f, 20.f);
//	float exposure = 1 / (MaximumFloat(viewport->averageScreenColor.Length(), 0.01f));
//	exposure = sqrt(exposure);
	if (shader)
		glUniform1f(shader->uniformExposure, viewport->exposure);


	CheckGLError("RenderPass::BindDeferredOutputFrameBuffer");
	return true;
}

bool RenderPass::PerformIterativePingPongRenders(GraphicsState& graphicsState)
{
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
	glDisable(GL_DEPTH_TEST); // Disable depth testing
	assert(shader);
	Vector2i requestedRenderSize = MainWindow()->ClientAreaSize();
	if (viewport->window == MainWindow())
		requestedRenderSize = graphicsState.renderResolution;
	List<FrameBuffer*> fbs;
	FrameBuffer * previous = GetInputFrameBuffer();
	Timer t;
	t.Start();
	for (int i = 0; i < iterations; ++i)
	{
		/// Check for existing FrameBuffer.
		String fbName = name+String(i);
		Vector2i reqSize = previous? previous->size * 0.5f : requestedRenderSize * 0.5f;
		while (reqSize.x * reqSize.y > 10000)
			reqSize *= 0.5f;
		if (reqSize.x * reqSize.y < 15)
			continue;
		FrameBuffer * fb = viewport->GetFrameBuffer(fbName);
		if (fb == 0)
		{
			fb = new FrameBuffer(fbName);
			fb->useFloatingPointStorage = previous->useFloatingPointStorage;
			viewport->frameBuffers.AddItem(fb);
		}
		fbs.AddItem(fb);
		if (fb->size != reqSize)
		{
			fb->size = reqSize;
			fb->skipDepthBuffers = true;
			fb->CreateBuffersLikeIn(previous);
		}
		previous = fb;
	}
	t.Stop();
	int msAllocating = t.GetMs();
	CheckGLError("RenderPass::PerformIterativePingPongRenders - fb creation/resize");
	graphicsState.antialiasing = true; // Do it.
	/// Buffers created, now 
	t.Start();
	for (int i = 0; i < fbs.Size(); ++i)
	{
		// Set output.
		FrameBuffer * fb = fbs[i];
		bool ok = fb->Bind();
		assert(ok);
		ok = fb->SetDrawBuffers();
		assert(ok);
		glViewport(0, 0, fb->size.x, fb->size.y);
		CheckGLError("RenderPass::PerformIterativePingPongRenders - rendering");
		// Do initial render.
		RenderQuad(graphicsState);
		// Unbind, bind next.
		fb->BindTexturesForSampling(shader, graphicsState);
		// Ensure linear sampling.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// save texture?
	//	fb->DumpTexturesToFile();
	}
	t.Stop();
	int msRendering = t.GetMs();

	/// Unbind framebuffer first.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Analyze the last one?
	t.Start();
	//CalculateAverage
	// Grab last texture.
	FrameBuffer * last = fbs.Last();
	Texture * tex = last->renderBuffers[0]->texture;
	tex->LoadDataFromGL();
	t.Stop();
	int msLoadingFromGL = t.GetMs();
	t.Start();
	Vector3f average = tex->CalcAverageColorAllPixels();
//	std::cout<<"\nAverage color: "<<average;
	viewport->averageScreenColor = average; // Smooth a bit from frame to frame.
	t.Stop();
	int msAveraging = t.GetMs();

	// Set buffers to render into (the textures ^^)
	return true;
}

#include "Mesh/Square.h"

void RenderPass::SetupDeferredGatherAsInput(GraphicsState& graphicsState)
{
	viewport->deferredGatherBuffer->BindTexturesForSampling(shader, graphicsState);
	return;
}

void RenderPass::SetupDeferredOutputAsInput(GraphicsState& graphicsState)
{
	// Unbind the frame buffer from usage
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	viewport->deferredOutputBuffer->BindTexturesForSampling(shader, graphicsState);
	// RenderQuad func added.
	// Unbind textures?
	/*
	glActiveTexture(GL_TEXTURE0 + x);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures
	*/
	return;
}

FrameBuffer * RenderPass::GetInputFrameBuffer()
{
	switch(input)
	{
	case RenderTarget::DEFERRED_GATHER:
		return viewport->deferredGatherBuffer;
	case RenderTarget::DEFERRED_OUTPUT:
		return viewport->deferredOutputBuffer;
	case RenderTarget::POST_PROCESS_OUTPUT:
		return viewport->postProcessOutputBuffer;
	default:
		assert(false);
	}
	return 0;
}


/// Used for e.g. shadow-mapping.
void RenderPass::RenderEntitiesOnlyVertices(GraphicsState& graphicsState)
{
	bool optimized = true;
	if (!optimized)
	{
		RenderEntities(graphicsState);
		return;
	}
	EntitySharedPtr entity;
	GraphicsProperty * gp;
	Timer timer;
	timer.Start();
	/// Instancing enabled?
	if (instancingEnabled)
	{	
		// Turn it off first, though.
		glUniform1i(shader->uniformInstancingEnabled, 0);
	}
	/// Bind default array.
	graphicsState.BindVertexArrayBuffer(0);
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		// Just load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->transformationMatrix.getPointer());	
		// Render the model
		entity->model->Render(graphicsState);
		++graphicsState.renderedObjects;		// increment rendered objects for debug info
	}
	if (instancingEnabled)
	{
		glUniform1i(shader->uniformInstancingEnabled, 1);
		for (int i = 0; i < entityGroupsToRender.Size(); ++i)
		{
			RenderInstancingGroup * group = entityGroupsToRender[i];
			group->Render(graphicsState);
		}
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();	
}
