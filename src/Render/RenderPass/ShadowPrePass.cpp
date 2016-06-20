/// Emil Hedemalm
/// 2015-02-20
/// Separation into files, as it has started confusing me.
/// All functions related to setting up a shadow-mapping pre-pass.

#include "Render/RenderPass.h"
#include "Lighting.h"

#include "Physics/PhysicsManager.h"
#include "PhysicsLib/Shapes/AABB.h"

#include "Graphics/GraphicsProperty.h"

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
//		glViewport(-shadowMapResolution/2, -shadowMapResolution/2, shadowMapResolution, shadowMapResolution);
//		glViewport(0, 0, shadowMapResolution, shadowMapResolution);
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
/// Frame buffer object for deferred shading
GLuint frameBufferObject = 0;	// Main frame buffer object to use
GLuint depthBuffer = 0;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
GLuint positionTexture = 0, diffuseTexture = 0, depthTexture = 0, normalTexture = 0, specularTexture = 0, tangentTexture = 0, normalMapTexture = 0, pickingTexture = 0;
GLuint diffuseBuffer = 0, normalBuffer = 0, positionBuffer = 0, specularBuffer = 0;
// tangentBuffer = 0,	normalMapBuffer = 0, pickingBuffer = 0;

/// Creates it as needed.
bool RenderPass::BindShadowMapFrameBuffer()
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

bool newT = true;
// Set up/fetch render buffers for this, based on the viewport.
bool RenderPass::BindDeferredGatherFrameBuffer()
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
	glEnable(GL_DEPTH_TEST); // Enable depth testing

//	newT = false;
	if (newT)
	{
		if (!viewport->deferredGatherBuffer)
		{
			viewport->deferredGatherBuffer = new FrameBuffer("DeferredGatherBuffer");
		}
		if (!viewport->deferredGatherBuffer->IsGood() || viewport->deferredGatherBuffer->size != viewport->size)
		{
			if (!viewport->deferredGatherBuffer->CreateDeferredBuffers(viewport->size))
			{
				SAFE_DELETE(viewport->deferredGatherBuffer);
				return false;
			}
		}
		int error = glGetError();
		/// Make frame buffer active
		viewport->deferredGatherBuffer->Bind();
		// Clear depth  and color - else nothing will render.
		glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set viewport size to clear?
		viewport->SetGLViewport();
		// Set buffers to render into (the textures ^^)
		viewport->deferredGatherBuffer->SetDrawBuffers();
		CheckGLError("RenderPass::BindDeferredGatherFrameBuffer");
		return true;
	}

	CheckGLError("Before RenderPass::BindDeferredGatherFrameBuffer");

	/// Use Deferred-shader to store all data correctly!
	/*
	if (ShadeMan.SetActiveShader("Deferred") == NULL){
		std::cout<<"\nUnable to set Deferred Shader. Breaking rendering.";
	//	assert(false && "Unable to set Deferred Shader. Breaking rendering.");
		return;
	}*/
	/// Generate frame buffer
	/// OpenGL specific data
	//
	GLuint error;
	/// Setup main Frame buffer object
	if (!frameBufferObject)
		glGenFramebuffers(1, &frameBufferObject);
	error = glGetError();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	/// Establish some variables before we try tweaking properties..
	int textureWidth = viewport->size.x;
	int textureHeight = viewport->size.y;
	/// Try oversampling o-o
	bool overSampling = false;
	if (overSampling){
		glEnable( GL_MULTISAMPLE );
		textureWidth *= 2;
		textureHeight *= 2;
	}
	else
		glDisable(GL_MULTISAMPLE);
	/// Setup Render buffers
	error = glGetError();
	if (!depthBuffer){
		glGenRenderbuffers(1, &depthBuffer);
		glGenRenderbuffers(1, &diffuseBuffer);
		glGenRenderbuffers(1, &positionBuffer);
		glGenRenderbuffers(1, &normalBuffer);
		glGenRenderbuffers(1, &specularBuffer);
//		glGenRenderbuffers(1, &tangentBuffer);
//		glGenRenderbuffers(1, &normalMapBuffer);
//		glGenRenderbuffers(1, &pickingBuffer);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	
	glBindRenderbuffer(GL_RENDERBUFFER, diffuseBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuseBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	
	glBindRenderbuffer(GL_RENDERBUFFER, normalBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB16F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, normalBuffer);	// Attach the depth buffer fbo_depth to our frame buffer

	glBindRenderbuffer(GL_RENDERBUFFER, positionBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, positionBuffer);	// Attach the depth buffer fbo_depth to our frame buffer

	glBindRenderbuffer(GL_RENDERBUFFER, specularBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, specularBuffer);	// Attach the depth buffer fbo_depth to our frame buffer

/*	glBindRenderbuffer(GL_RENDERBUFFER, tangentBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_RENDERBUFFER, tangentBuffer);	// Attach the depth buffer fbo_depth to our frame buffer

	glBindRenderbuffer(GL_RENDERBUFFER, normalMapBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_RENDERBUFFER, normalMapBuffer);	// Attach the depth buffer fbo_depth to our frame buffer

	glBindRenderbuffer(GL_RENDERBUFFER, pickingBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the AppWindow
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_RENDERBUFFER, pickingBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	*/

	glBindRenderbuffer(GL_RENDERBUFFER, 0);	// Bind to 0 when done.
	/// Set up textures
	if (!diffuseTexture)
		glGenTextures(1, &diffuseTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, diffuseTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	if (!normalTexture)
		glGenTextures(1, &normalTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, normalTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!positionTexture)
		glGenTextures(1, &positionTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, positionTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	CheckGLError("Error initializing frame buffer textures");

	if (!depthTexture)
		glGenTextures(1, &depthTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, depthTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureWidth, textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	CheckGLError("Error initializing depth texture");

	/// Set up textures
	if (!specularTexture)
		glGenTextures(1, &specularTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, specularTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	/*
	if (!tangentTexture)
		glGenTextures(1, &tangentTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, tangentTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!normalMapTexture)
		glGenTextures(1, &normalMapTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, normalMapTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!pickingTexture)
		glGenTextures(1, &pickingTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, pickingTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our AppWindow
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	*/

	// Bind framebuffer first?

	// Try bidning color 0 to null.
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

	// Attach textures to be rendered to
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuseTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, positionTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, specularTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");

	/*
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tangentTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, normalMapTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, pickingTexture, 0); // Attach the texture to the color buffer in our frame buffer
	CheckGLError("glFramebufferTexture2D");
	*/

	int frameBufferColorAttachmentsSet = 4;

	/// Attach depth texture too
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depthTexture, 0); // Attach the texture to the color buffer in our frame buffer
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, depthTexture, 0); /// Attach depth texture o.O
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0); /// Attach depth texture o.O
	CheckGLError("GLError");

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	CheckGLError("glCheckFramebufferStatusEXT");

	// Check that frame buffer is okay to work on.
	int result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch(result) {
		case GL_FRAMEBUFFER_COMPLETE: // yay :3
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout<<"\nINFO: Framebuffer incomplete attachment.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout<<"\nINFO: Framebuffer incomplete, missing attachment. Attach an image!";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout<<"\nINFO: Framebuffer incomplete draw buffer.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout<<"\nINFO: Framebuffer incomplete read buffer.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout<<"\nINFO: Framebuffer incomplete multisample.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout<<"\nINFO: Framebuffer incomplete layer targets.";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout<<"\nINFO: Framebuffer unsupported.";
			break;
		default:
			std::cout<<"\nINFO: Unknown error in framebuffer ...";
			break;
	}
	if (result != GL_FRAMEBUFFER_COMPLETE){
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);
		std::cout<<"\nINFO: FrameBuffer not ready to be used.";
		Sleep(10);
	}

	/*
	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (this->GL_VERSION_MAJOR >= 4 && this->GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}*/

	// Unbind our frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/// Make frame buffer active
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result == 0){
		std::cout<<"\nFramebuffer states bad!";
	}
	// Clear depth  and color
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set buffers to render into (the textures ^^)
	GLenum buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
/*		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7*/
	}; //, GL_COLOR_ATTACHMENT3};
//	glDrawBuffers(frameBufferColorAttachmentsSet, buffers);
	glDrawBuffers(4, buffers);

//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


/*	if (!viewport->shadowMapDepthBuffer)
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
	*/
	error = glGetError();
	/// Make frame buffer active
//	viewport->shadowMapDepthBuffer->Bind();
	// Set viewport size to clear?
//	glViewport(0, 0, viewport->shadowMapDepthBuffer->size.x, viewport->shadowMapDepthBuffer->size.y);
	viewport->SetGLViewport();
//	glViewport(0,0,viewport->size.x, viewport->size.y);

	// Set buffers to render into (the textures ^^)
//	viewport->shadowMapDepthBuffer->SetDrawBuffers();
//	CheckGLError("RenderPass::BindShadowMapFrameBuffer");
	return true;
}

#include "Mesh/Square.h"

void RenderPass::SetupDeferredGatherAsInput()
{
	if (newT)
	{
		// Unbind the frame buffer from usage
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		viewport->deferredGatherBuffer->BindTexturesForSampling(shader);

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
		deferredRenderingBox->Render();

		//glDeleteBuffers(1, &box.vboBuffer);
		//box.vboBuffer = NULL;

		/// Unbind textures~
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures
		// Unbind normalMap too
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures
		glActiveTexture(GL_TEXTURE0 + 0);

		return;
	}
	// Unbind the frame buffer from usage
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	/*
uniform sampler2D diffuseMap;
uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;*/
	// Bind textures to our lighting shader
//	GLuint uniformBaseTexture = glGetUniformLocation(shader->shaderProgram, "baseImage");
	GLuint loc = glGetUniformLocation(shader->shaderProgram, "diffuseMap");
	if (loc != -1)
		glUniform1i(loc, 0);
	loc = glGetUniformLocation(shader->shaderProgram, "depthMap");
	if (loc != -1)
		glUniform1i(loc, 1);
	loc = glGetUniformLocation(shader->shaderProgram, "normalMap");
	if (loc != -1)
		glUniform1i(loc, 2);
	loc = glGetUniformLocation(shader->shaderProgram, "positionMap");
	if (loc != -1)
		glUniform1i(loc, 3);
	loc = glGetUniformLocation(shader->shaderProgram, "specularMap");
	if (loc != -1)
		glUniform1i(loc, 4);
	loc = glGetUniformLocation(shader->shaderProgram, "tangentMap");
	if (loc != -1)
		glUniform1i(loc, 5);
	loc = glGetUniformLocation(shader->shaderProgram, "bumpMap");
	if (loc != -1)
		glUniform1i(loc, 6);
	loc = glGetUniformLocation(shader->shaderProgram, "pickingMap");
	if (loc != -1)
		glUniform1i(loc, 7);

	glEnable(GL_TEXTURE_2D);
	// Bind sampler
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint maxTextures;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextures);

	// When rendering an objectwith this program.
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, positionTexture);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, specularTexture);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, tangentTexture);
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, normalMapTexture);
	glActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);

	graphicsState->currentTexture = 0;

	// Render square for the AppWindow
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
	deferredRenderingBox->Render();

	//glDeleteBuffers(1, &box.vboBuffer);
	//box.vboBuffer = NULL;

	/// Unbind textures~
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures
	// Unbind normalMap too
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures
	glActiveTexture(GL_TEXTURE0 + 0);

	// Render the light-sources o-o
//	ShadeMan.SetActiveShader(0);	// Set default program, matrices should still be correct
//	for (int i = 0; i <
}


/// Used for e.g. shadow-mapping.
void RenderPass::RenderEntitiesOnlyVertices()
{
	bool optimized = true;
	if (!optimized)
	{
		RenderEntities();
		return;
	}
	Entity * entity;
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
	graphicsState->BindVertexArrayBuffer(0);
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		// Just load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->transformationMatrix.getPointer());	
		// Render the model
		entity->model->Render();
		++graphicsState->renderedObjects;		// increment rendered objects for debug info
	}
	if (instancingEnabled)
	{
		glUniform1i(shader->uniformInstancingEnabled, 1);
		for (int i = 0; i < entityGroupsToRender.Size(); ++i)
		{
			RenderInstancingGroup * group = entityGroupsToRender[i];
			group->Render();
		}
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();	
}
