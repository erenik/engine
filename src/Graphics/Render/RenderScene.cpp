// Emil Hedemalm
// 2013-07-03 Linuxifying!

#include "Graphics/GraphicsManager.h"
#include "Mesh/Square.h"
#include "OS/Sleep.h"

#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"

#include "../RenderSettings.h"

#include "Render/RenderPipeline.h"

#include "File/LogFile.h"

// Renders the scene normally using the active camera using frustum culling.
void GraphicsManager::RenderScene()
{
	


	glBlendFunc(GL_ONE, GL_ZERO);
	GLuint error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in RenderScene "<<error;
	}

	Shader * shader = ShadeMan.SetActiveShader("Phong");
    if (shader == NULL){
    //    std::cout<<"\nUnable to set Phong shader in GraphicsManager::RenderScene";
    #ifdef WINDOWS
		LogGraphics("Unable to set Phong shader.");
        assert(shader && "Unable to set \"Phong\" shader");
    #endif
        return;
    }
	graphicsState->settings |= ENABLE_SPECIFIC_ENTITY_OPTIONS;

	// Set fog properties as needed.
	glUniform1f(shader->uniformFogBeginDistance, graphicsState->fogBegin);
	glUniform1f(shader->uniformFogEndDistance, graphicsState->fogEnd);
	glUniform3f(shader->uniformFogColor, graphicsState->clearColor[0], graphicsState->clearColor[1], graphicsState->clearColor[2]);

	if (backfaceCullingEnabled){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
	//	glDisable(GL_CULL_FACE);
	}

	// Set some standard rendering options.
    glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	// Reset matrices: This may as well be done before rendering is done, since the matrices
	// will need to be reset all the time depending on the content to be rendered and where ^^
	/// Camera is already updated, so just use it's matrices straight away ^^
	Camera & camera = *graphicsState->camera;

	// Camera calculations are now done inside the camera, so the camPos/camLookingAtVector/camUpVector can now be removed!
	// If view frustum culling is enabled, set it in the settings and update the frustum with the camera's current position.
	if (true /*useOctree && frustumCullingActive*/){
		graphicsState->viewFrustum.SetCamPos(Vector3f((Vector4f)camera.Position()), Vector3f((Vector4f)camera.LookingAt()), Vector3f((Vector4f)camera.UpVector()));
		Frustum & viewFrustum = graphicsState->viewFrustum;
//		std::cout<<"\nViewfrustum: "<<viewFrustum.left<<" "<<viewFrustum.right<<" nearplane: "<<viewFrustum.nearPlaneDistance<<" farplane: "<<viewFrustum.farPlaneDistance;
	}

	if (!shader)
		return;

	// Load in the model and view matrices
//	shader->uniformViewMatrix = glGetUniformLocation(shader->shaderProgram, "viewMatrix");
	glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState->viewMatrixF.getPointer());
	error = glGetError();
//	shader->uniformModelMatrix = glGetUniformLocation(shader->shaderProgram, "modelMatrix");
	glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState->modelMatrixF.getPointer());
	error = glGetError();
	// Set later! ALSO: glProgramUniform is in a later GL version compared to glUniform!
/*	if (shader && shader->uniformEyePosition != -1)
		if (glProgramUniform4f != NULL)
		glProgramUniform4f(shader->shaderProgram, shader->uniformEyePosition, camera.Position()[0], camera.Position()[1], camera.Position()[2], 1.0f);
*/
	error = glGetError();

	rendering = true;

//	point = Vector4f(width/3, height*3/2, 0, 1);
//	point = projection * point;

	// Load projection matrix into shader
//	GLuint uniformProjectionMatrix = glGetUniformLocation(shader->shaderProgram, "projectionMatrix");
	graphicsState->projectionMatrixF = graphicsState->projectionMatrixD;
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState->projectionMatrixF.getPointer());

	Matrix4f mvp = graphicsState->projectionMatrixF * graphicsState->viewMatrixF * graphicsState->modelMatrixF;
	/// Just testing that the matrix is set correctly..
	//Vector4f point = Vector4f(1.0, 1.0, 1.0, 1.0f);
	//point = mvp * point;

	//point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
	//point = mvp * point;

	//point = Vector4f(0, 0, 0, 1.0f);
	//point = mvp * point;

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
		graphicsState->settings |= USE_LEGACY_GL;
	else
		graphicsState->settings &= ~USE_LEGACY_GL;

	/// Deferred rendering setting ^^
	bool useDeferred = false;
	/// Switch off deferred if GL version is too low. 3.0 is require for handling gl*Framebuffer functions
	/// Seems buggy even at 3.ish, maybe raise cap to 4?
	if (GL_VERSION_MAJOR < 3)
		useDeferred = false;
  //  std::cout<<"\nUsing deferred: "<<useDeferred;
	 // If support is via EXT (OpenGL version < 3), add the EXT suffix; otherwise functions are core (OpenGL version >= 3)
    // or ARB without the EXT suffix, so just get the functions on their own.
    std::string suffix = (support_framebuffer_via_ext ? "EXT" : "");
	// Bind functions
/*	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress((std::string("glGenFramebuffers") + suffix).c_str());
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress((std::string("glDeleteFramebuffers") + suffix).c_str());
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress((std::string("glBindFramebuffer") + suffix).c_str());
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress((std::string("glFramebufferTexture2D") + suffix).c_str());
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress((std::string("glCheckFramebufferStatus") + suffix).c_str());
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress((std::string("glGenerateMipmap") + suffix).c_str());
*/

	if (graphicsState->settings & USE_LEGACY_GL){
		// Set default shader program
		shader = NULL;
		ShadeMan.SetActiveShader(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	/// Deferred rendering, check GL version too! Need > 3.0 for FBOs (Frame Buffer Objects)
	else if (useDeferred){
		/// Clear errors
		error = glGetError();
		/// Use Deferred-shader to store all data correctly!
		if (ShadeMan.SetActiveShader("Deferred") == NULL){
			std::cout<<"\nUnable to set Deferred Shader. Breaking rendering.";
		//	assert(false && "Unable to set Deferred Shader. Breaking rendering.");
			return;
		}
		glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture
		glEnable(GL_DEPTH_TEST); // Enable depth testing
		PrintGLError("Error setting Deferred shader as active shading program");
		/// Generate frame buffer
		InitFrameBuffer();
		error = glGetError();
		/// Make frame buffer active
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		error = glGetError();
		int result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
			GL_COLOR_ATTACHMENT4,
			GL_COLOR_ATTACHMENT5,
			GL_COLOR_ATTACHMENT6,
			GL_COLOR_ATTACHMENT7
		}; //, GL_COLOR_ATTACHMENT3};
	//	glDrawBuffers(frameBufferColorAttachmentsSet, buffers);
		glDrawBuffers(7, buffers);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		error = glGetError();
	}
	/// Only set shader properties if we're using modern GL!
	else {
		ShadeMan.SetActiveShader("Flat");
	//	ShadeMan.SetActiveShader("Normal");
	//	ShadeMan.SetActiveShader("Depth");
		Shader * shader = ShadeMan.SetActiveShader("Phong");

		/// Load lighting settings to shader ^^
		LoadLighting(graphicsState->lighting, shader);
	}
	// Set primary color
	glUniform4f(shader->uniformPrimaryColorVec4, 1.f,1.f,1.f,1.f);

	// Reset bound textures
	graphicsState->currentTexture = NULL;
	// Update view and projection matrix in specified shader
	if (shader && shader->uniformProjectionMatrix != -1)
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState->projectionMatrixF.getPointer());
	// Update view and projection matrix in specified shader
	if (shader && shader->uniformViewMatrix != -1)
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState->viewMatrixF.getPointer());
	// Update camera in the world
	if (shader && shader->uniformEyePosition != -1)
		glUniform4f(shader->uniformEyePosition, camera.Position()[0], camera.Position()[1], camera.Position()[2], 1.0f);

	error = glGetError();

	// Render vfcOctree with regular objects
	if (vfcOctree){
		//vfcOctree->Render(); // Old
		vfcOctree->SetCullingFrustum(camera.GetFrustum());
		// Render with culling! o.o TODO: Not working?
	//	vfcOctree->RenderWithCulling(*graphicsState);

        // Render without culling
		vfcOctree->Render(*graphicsState);
	}
	else {
	}

	// If we have entities that specifically need alpha-blending
	// (windows, water, special effects, etc.)
	// Render these now straight after regular rendering,
	// using camera-wise depth-sorting and disabling writing to the Z-buffer.

	/// If we're using deferred, we'll now have written all necessary date to the framebuffer to begin
	/// computing lighting!
	if (useDeferred){

		// Unbind the frame buffer from usage
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		/// Enter light-data as uniforms or alternatively render in a for-loop....
		error = glGetError();
		/// Set shader program
		Shader * shader = ShadeMan.SetActiveShader("Lighting");

		// Set primary color and other uniforms
		if (shader->uniformPrimaryColorVec4 == -1){
			int loc = glGetUniformLocation(shader->shaderProgram, "primaryColorVec4");
			if (loc != -1)
				shader->uniformPrimaryColorVec4 = loc;
		}
		else
			glUniform4f(shader->uniformPrimaryColorVec4, 1,1,1,1);

		/// Load lighting settings to shader ^^
		LoadLighting(graphicsState->lighting, shader);

		// Update camera in the world
		if (shader && shader->uniformEyePosition != -1)
			glUniform4f(shader->uniformEyePosition, camera.Position()[0], camera.Position()[1], camera.Position()[2], 1.0f);

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
		else
			std::cout<<"\nUnable to find uniform for diffuseMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "depthMap");
		if (loc != -1)
			glUniform1i(loc, 1);
		else
			std::cout<<"\nUnable to find uniform for depthMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "normalMap");
		if (loc != -1)
			glUniform1i(loc, 2);
		else
			std::cout<<"\nUnable to find uniform for normalMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "positionMap");
		if (loc != -1)
			glUniform1i(loc, 3);
		else
			std::cout<<"\nUnable to find uniform for positionMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "specularMap");
		if (loc != -1)
			glUniform1i(loc, 4);
		else
			std::cout<<"\nUnable to find uniform for specularMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "tangentMap");
		if (loc != -1)
			glUniform1i(loc, 5);
		else
			std::cout<<"\nUnable to find uniform for tangentMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "bumpMap");
		if (loc != -1)
			glUniform1i(loc, 6);
		else
			std::cout<<"\nUnable to find uniform for bumpMap.";
		loc = glGetUniformLocation(shader->shaderProgram, "pickingMap");
		if (loc != -1)
			glUniform1i(loc, 7);
		else
			std::cout<<"\nUnable to find uniform for pickingMap.";

		glEnable(GL_TEXTURE_2D);
		// Bind sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLint maxTextures;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextures);

		// When rendering an objectwith this program.
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		error = glGetError();
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

		// Set sampler in client state
/*		if (shader->uniformBaseTexture != -1)
			glUniform1i(shader->uniformBaseTexture, 0);		// Sets sampler
		if (shader->uniformNormalMap != -1)
			glUniform1i(shader->uniformNormalMap, 1);		// Sets sampler
*/
		graphicsState->currentTexture = 0;

		// Render!
		error = glGetError();

		// Render square for the window
		deferredRenderingBox->name = "DeferredLighting";
		deferredRenderingBox->Render(*graphicsState);

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
		ShadeMan.SetActiveShader(0);	// Set default program, matrices should still be correct
	//	for (int i = 0; i <

		error = glGetError();
	}

	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in RenderScene "<<error;
	}
}
