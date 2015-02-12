/// Emil Hedemalm
/// 2015-02-11
/// o.o

#include "FrameBuffer.h"
#include "Viewport.h"
#include "Window/Window.h"
#include "Graphics/GLBuffers.h"
#include "Graphics/OpenGL.h"
#include "TextureManager.h"
#include "File/LogFile.h"

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

void FrameBuffer::Nullify()
{
	viewport = NULL;
	frameBufferObject = -1;
	good = false;
}

void FrameBuffer::DumpTexturesToFile()
{
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		RenderBuffer * rb = renderBuffers[i];
		Texture * tex = rb->texture;
		tex->LoadDataFromGL();
		if (viewport)
			tex->Save(viewport->window->name+" "+viewport->name+" "+rb->name+".png", true);
		else 
			tex->Save(name+"_"+rb->name+".png", true);
//		Sleep(50);
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
		* diffBuf = new RenderBuffer("Diffuse", BufferType::COLOR_BUFFER_1, BufferStorageType::RGBA_8, textureSize)//,
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
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);
		std::cout<<"\nINFO: FrameBuffer not ready to be used.";
		Sleep(10);
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
bool FrameBuffer::CreateDepthBuffer(Vector2i size)
{
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
	RenderBuffer * depthBuf = new RenderBuffer("Depth", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_16F, textureSize);
//	RenderBuffer * depthBuf = new RenderBuffer("Depth", BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_32F, textureSize);
	renderBuffers.Add(depthBuf);

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
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);
		Sleep(10);
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

/// Sets the buffers to draw in.
bool FrameBuffer::SetDrawBuffers()
{
	GLenum buffers[10];
	int numBuffers = GetDrawBuffers(buffers);
	glDrawBuffers(numBuffers, buffers);
	AssertGLError("Viewport::BindFrameBuffer");
	return true;
}
