/// Emil Hedemalm
/// 2015-02-11
/// o.o

#include "FrameBuffer.h"
#include "Viewport.h"
#include "Window/AppWindow.h"
#include "Graphics/GLBuffers.h"
#include "Graphics/OpenGL.h"
#include "GraphicsState.h"
#include "TextureManager.h"
#include "File/LogFile.h"
#include "OS/Sleep.h"

bool CheckFrameBufferStatus(int status, String name);

FrameBuffer::FrameBuffer(String name)
: name(name)
{
	Nullify();
}

FrameBuffer::FrameBuffer(Viewport * vp, Vector2i initialSize)
{
	Nullify();
	this->size = initialSize;
	this->viewport = vp;
	frameBufferObject = -1;
	good = false;
}

FrameBuffer::~FrameBuffer()
{
	renderBuffers.ClearAndDelete();
}

void FrameBuffer::Free()
{
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		renderBuffers[i]->Free();
	}
}

void FrameBuffer::Nullify()
{
	viewport = NULL;
	frameBufferObject = -1;
	good = false;
	useFloatingPointStorage = false;
	skipDepthBuffers = false;
}

void FrameBuffer::DumpTexturesToFile()
{
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		Texture * tex = rb->texture;
//		switch(tex->ty
		tex->LoadDataFromGL();
		if (viewport)
			tex->Save(viewport->window->name+" "+viewport->name+" "+rb->name+"", true);
		else 
			tex->Save(name+"_"+rb->name+"", true);
//		SleepThread(50);
	}
}

/// Buffers will be written to. Number of written items will be returned.
int FrameBuffer::GetDrawBuffers(GLenum * buffers)
{
	int colorBuffers = 0;
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		if (rb->glType == GL_DEPTH_ATTACHMENT)
			continue;
		buffers[colorBuffers] = rb->glType;
		++colorBuffers;
	}
	return colorBuffers;
}
	

void FrameBuffer::CreateRenderBuffers()
{
	/// OpenGL specific data
	/// Frame buffer object for deferred shading
	//GLuint frameBufferObject;	// Main frame buffer object to use
	//GLuint depthBuffer;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
	//GLuint positionTexture;		// World coordinate position texture
	//GLuint diffuseTexture;		// Diffuse texture
	//GLuint depthTexture;		// Depth texture
	//GLuint normalTexture;		// Normal texture
	//
	GLuint error;
	/// Setup main Frame buffer object
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	error = glGetError();

	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	/// Establish some variables before we try tweaking properties..
	Vector2i textureSize = size;
	/*
	/// Try oversampling o-o
	bool overSampling = false;
	if (overSampling){
		glEnable( GL_MULTISAMPLE );
		textureSize *= 2;
	}
	else
		glDisable(GL_MULTISAMPLE);
		*/
	/// Setup Render buffers
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
	// Delete the old buffers as needed.
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		std::cout<<"\nDeallocating renderBuffer..";
		delete rb;
	}
	renderBuffers.Clear();
	
	
	// Create the buffers.
	RenderBuffer 
		* depthBuf = new RenderBuffer("Depthbuf", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_16F, textureSize),
		* diffBuf = new RenderBuffer("Diffuse", BufferType::COLOR_BUFFER_1, ColorStorageType(), textureSize)//,
	//	* norBuf = new RenderBuffer(BufferType::COLOR_BUFFER_2, BufferStorageType::RGB_16F, textureSize),
	//	* posBuf = new RenderBuffer(BufferType::COLOR_BUFFER_3, BufferStorageType::RGB_32F, textureSize)
		;
	// Try adding the rest later on..?
	renderBuffers.Add(depthBuf);
	renderBuffers.Add(diffBuf);
//	renderBuffers.Add(norBuf);
//	renderBuffers.Add(posBuf);

	// Bind to 0 when done.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	// Create textures for each buffer.
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		// Actually create it.
		rb->CreateBuffer();
		rb->CreateTexture();
		// Bind them straight away too.
		rb->BindTextureToFrameBuffer();
	}

	int frameBufferColorAttachmentsSet = 7;



	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
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
	if (result != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		std::cout<<"\nINFO: FrameBuffer not ready to be used.";
		SleepThread(10);
		return;
	}

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (GL_VERSION_MAJOR >= 4 && GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}

	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
	// Ok!
	good = true;
}



/// Creates just 1 dedicated buffer to receive the depth-data. Preferably in 32-bit floating point to ensure better quality?
bool FrameBuffer::CreateDepthBuffer(Vector2i depthBufferSize)
{
	Free();
	// Store the size here..?
	this->size = depthBufferSize;
	/// OpenGL specific data
	/// Frame buffer object for deferred shading
	/// GLuint frameBufferObject;	// Main frame buffer object to use
	/// GLuint depthBuffer;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
	GLuint error;
	/// Setup main Frame buffer object
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	error = glGetError();

	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	/// Establish some variables before we try tweaking properties..
	Vector2i textureSize = size;
	
	// Delete the old buffers as needed.
	renderBuffers.ClearAndDelete();	
	
	// Create the buffers.
	RenderBuffer * depthBuf = new RenderBuffer("Depth", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_32F, textureSize);
	renderBuffers.Add(depthBuf);

	// Bind to 0 when done.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	CreateTexturesAndBind();
	int frameBufferColorAttachmentsSet = 7;

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
	// Check that frame buffer is okay to work on.
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (!CheckFrameBufferStatus(status, name))
		return false;

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (GL_VERSION_MAJOR >= 4 && GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}
	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateRenderBuffers");	
	// Ok!
	good = true;
	return true;
}

int FrameBuffer::ColorStorageType()
{
	return useFloatingPointStorage? BufferStorageType::RGBA_16F : BufferStorageType::RGBA;
}

/// Creates a default set of render-buffers for deferred rendering. Included: diffuse, normal, position, specular, emissive (all triple-vectors) + a depth buffer.   
bool FrameBuffer::CreateDeferredBuffers(Vector2i inSize)
{
	Free();
	this->size = inSize;
	/// OpenGL specific data
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	int error = glGetError();
	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	/// Establish some variables before we try tweaking properties..
	Vector2i textureSize = size;	
	// Delete the old buffers as needed.
	renderBuffers.ClearAndDelete();	
	// Add the depth-buffer.
	renderBuffers.AddItem(new RenderBuffer("Depth", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_24F, textureSize));
	// Create the buffers.
	RenderBuffer * diffBuf = new RenderBuffer("Diffuse", BufferType::COLOR_BUFFER_1, ColorStorageType(), textureSize);
	renderBuffers.AddItem(diffBuf);
	renderBuffers.Add(new RenderBuffer("Normal", BufferType::COLOR_BUFFER_2, BufferStorageType::RGB_16F, textureSize),
		new RenderBuffer("Position", BufferType::COLOR_BUFFER_3, BufferStorageType::RGB_32F, textureSize),
		new RenderBuffer("Specular", BufferType::COLOR_BUFFER_4, BufferStorageType::RGB_32F, textureSize),
		new RenderBuffer("Emissive", BufferType::COLOR_BUFFER_5, BufferStorageType::RGB_32F, textureSize));

	// Bind to 0 when done.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	CreateTexturesAndBind();
	int frameBufferColorAttachmentsSet = renderBuffers.Size();

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
	// Check that frame buffer is okay to work on.
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (!CheckFrameBufferStatus(status, name))
		return false;

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (GL_VERSION_MAJOR >= 4 && GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}
	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateRenderBuffers");	
	// Ok!
	good = true;
	return true;
}

/// Creates a default set of render-buffers for the output of rendering (deferred or otherwise), including color (diffuse) and depth.
bool FrameBuffer::CreateDeferredOutputBuffers(Vector2i inSize)
{
	Free();
	this->size = inSize;
	/// OpenGL specific data
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	int error = glGetError();
	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreateDeferredOutputBuffers");
	Vector2i textureSize = size;
	renderBuffers.ClearAndDelete();	
	renderBuffers.AddItem(new RenderBuffer("Depth", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_24F, textureSize));
	RenderBuffer * diffBuf = new RenderBuffer("Diffuse", BufferType::COLOR_BUFFER_1, ColorStorageType(), textureSize);
	renderBuffers.AddItem(diffBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	AssertGLError("FrameBuffer::CreateDeferredOutputBuffers");
	CreateTexturesAndBind();
	int frameBufferColorAttachmentsSet = renderBuffers.Size();

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreateRenderBuffers");
	
	// Check that frame buffer is okay to work on.
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (!CheckFrameBufferStatus(status, name))
		return false;

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (GL_VERSION_MAJOR >= 4 && GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}
	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateRenderBuffers");	
	// Ok!
	good = true;
	return true;
}

/// Creates 1 or more post-processing output buffers. More may be needed for ping-pong downscaling for e.g. Bloom
bool FrameBuffer::CreatePostProcessOutputBuffers(Vector2i inSize)
{
	Free();
	this->size = inSize;
	/// OpenGL specific data
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	int error = glGetError();
	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreatePostProcessOutputBuffers");
	Vector2i textureSize = size;
	renderBuffers.ClearAndDelete();	
	RenderBuffer * diffBuf = new RenderBuffer("Diffuse", BufferType::COLOR_BUFFER_1, ColorStorageType(), textureSize);
	renderBuffers.AddItem(diffBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	AssertGLError("FrameBuffer::CreatePostProcessOutputBuffers");
	CreateTexturesAndBind();
	int frameBufferColorAttachmentsSet = renderBuffers.Size();

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreatePostProcessOutputBuffers");
	
	// Check that frame buffer is okay to work on.
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (!CheckFrameBufferStatus(status, name))
		return false;

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (GL_VERSION_MAJOR >= 4 && GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}
	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateRenderBuffers");	
	// Ok!
	good = true;
	return true;
}

/// Yas. Only inherits main stats of the other buffer, keeps size.
bool FrameBuffer::CreateBuffersLikeIn(FrameBuffer * otherBuffer)
{
	Free();
	/// OpenGL specific data
	if (frameBufferObject == -1)
		frameBufferObject = GLFrameBuffers::New();
	int error = glGetError();
	/// Bind it for manipulatoin.
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("FrameBuffer::CreateBuffersLikeIn");
	Vector2i textureSize = size;
	renderBuffers.ClearAndDelete();	
	/// Create render buffers.
	for (int i = 0; i < otherBuffer->renderBuffers.Size(); ++i)
	{
		RenderBuffer * ref = otherBuffer->renderBuffers[i];
		if (ref->type == BufferType::DEPTH_BUFFER && skipDepthBuffers)
			continue;
		RenderBuffer * newOne = new RenderBuffer(ref->name, ref->type, ref->storageType, textureSize);
		renderBuffers.AddItem(newOne);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);	
	}

	AssertGLError("FrameBuffer::CreateBuffersLikeIn");
	CreateTexturesAndBind();
	int frameBufferColorAttachmentsSet = renderBuffers.Size();

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	AssertGLError("FrameBuffer::CreateBuffersLikeIn");
	
	// Check that frame buffer is okay to work on.
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (!CheckFrameBufferStatus(status, name))
		return false;

	// Unbind our frame buffer object, along with its attached render buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	AssertGLError("FrameBuffer::CreateBuffersLikeIn");	
	// Ok!
	good = true;
	return true;
}


void FrameBuffer::CreateTexturesAndBind()
{
	// Create textures for each buffer.
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		// Actually create it.
		rb->CreateBuffer();
		rb->CreateTexture();
		// Bind them straight away too.
		rb->BindTextureToFrameBuffer();
	}
}

bool CheckFrameBufferStatus(int status, String frameBufferName)
{
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE: // yay :3
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			LogGraphics("Framebuffer "+frameBufferName+": GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", ERROR);
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
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		SleepThread(10);
		return false;
	}
	return true;
}


/// Binds it for use.
bool FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	AssertGLError("Viewport::BindFrameBuffer");
	return true;
}

#include "Graphics/Shader.h"
#include "GraphicsState.h"

/// Binds textures for sampling, e.g. using Deferred previously rendered-to buffer to do the Deferred-rendering part.
void FrameBuffer::BindTexturesForSampling(Shader * shader, GraphicsState & graphicsState)
{
	int bound = 0;
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		rb->texture->glid;
		// Check uniform associated.
		int uni = shader->GetUniformTextureByBufferType(rb->type);
		if (uni == -1)
			continue;
		glUniform1i(uni, bound);
		glActiveTexture(GL_TEXTURE0 + bound);
		glBindTexture(GL_TEXTURE_2D, rb->texture->glid);
		if (graphicsState.antialiasing == false)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else 
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		++bound;
	}
}

/// Sets the buffers to draw in.
bool FrameBuffer::SetDrawBuffers()
{
	GLenum buffers[10];
	int numBuffers = GetDrawBuffers(buffers);
	glDrawBuffers(numBuffers, buffers);
	AssertGLError("Viewport::BindFrameBuffer");
	return true;
}
