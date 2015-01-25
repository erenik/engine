/// Emil Hedemalm
/// 2014-07-09
/// A buffer representing one layer of data for a specific viewport. 

#include "RenderBuffer.h"
#include "Graphics/GLBuffers.h"
#include "Graphics/OpenGL.h"
#include "TextureManager.h"

// See above.  NOTE: The RenderBuffer constructor is special in that it will initialize all GL-related objects within the constructor.
RenderBuffer::RenderBuffer(int type, int storageType, Vector2i size)
	: type(type), storageType(storageType), size(size)
{
	renderBuffer = -1;
	glType = -1;
	glStorageType = -1;
	texture = NULL;
}

void RenderBuffer::CreateBuffer()
{
	// Create it.
	if (renderBuffer == -1)
		renderBuffer = GLRenderBuffers::New();
	// Bind it.
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	AssertGLError("RenderBuffer::CreateBuffer");
	glType = -1;
	glStorageType = -1;
	switch(type)
	{
		case BufferType::DEPTH_BUFFER:		glType = GL_DEPTH_ATTACHMENT;	break;
		case BufferType::COLOR_BUFFER_1:	glType = GL_COLOR_ATTACHMENT0;	break;
		case BufferType::COLOR_BUFFER_2:	glType = GL_COLOR_ATTACHMENT1;	break;
		case BufferType::COLOR_BUFFER_3:	glType = GL_COLOR_ATTACHMENT2;	break;
		case BufferType::COLOR_BUFFER_4:	glType = GL_COLOR_ATTACHMENT3;	break;
		case BufferType::COLOR_BUFFER_5:	glType = GL_COLOR_ATTACHMENT4;	break;
		default:
			assert(false);
	}
	switch(storageType)
	{
		case BufferStorageType::DEPTH_16F:
			glStorageType = GL_DEPTH_COMPONENT16;
			break;
		case BufferStorageType::RGBA_8:
			glStorageType = GL_RGBA8;
			break;
		case BufferStorageType::RGB_16F:
			glStorageType = GL_RGB16F;
			break;
		case BufferStorageType::RGB_32F:
			glStorageType = GL_RGB32F;
			break;
		default:
			assert(false);
	}
	// Set storage.
	glRenderbufferStorage(GL_RENDERBUFFER, glStorageType, size[0], size[1]); // Set the render buffer storage to be a depth component, with a width and height of the window
	AssertGLError("RenderBuffer::CreateBuffer");
	// And bind it to the current frame-buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, glType, GL_RENDERBUFFER, renderBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	AssertGLError("RenderBuffer::CreateBuffer");
}


/// Allocates a texture object to be used for storing/receiving and later on fetching values from this buffer.
void RenderBuffer::CreateTexture()
{
	if (!texture)
		texture = TexMan.New();
	texture->size = size;
	GLuint & textureID = texture->glid;
	if (textureID == -1)
		glGenTextures(1, &textureID); // Generate one texture

	// Bind the texture for editing
	glBindTexture(GL_TEXTURE_2D, textureID); 
	AssertGLError("RenderBuffer::CreateTexture");

	int glPixelDataFormat = -1;
	int glPixelDataType = -1;
	switch(storageType)
	{
		case BufferStorageType::RGBA_8:
			glPixelDataFormat = GL_RGBA;
			glPixelDataType = GL_UNSIGNED_BYTE;
			texture->bpp = 4;
			texture->format = Texture::RGBA;
			break;
		case BufferStorageType::RGB_16F:
			glPixelDataFormat = GL_RGB;
			glPixelDataType = GL_FLOAT;
			texture->bpp = 2;
			texture->format = Texture::RGB_16F;
			break;
		case BufferStorageType::RGB_32F:
			glPixelDataFormat = GL_RGB;
			glPixelDataType = GL_FLOAT;
			texture->bpp = 4;
			texture->format = Texture::RGB_32F;
			break;
		case BufferStorageType::DEPTH_16F:
			glPixelDataFormat = GL_DEPTH_COMPONENT;
			glPixelDataType = GL_FLOAT;
			texture->bpp = 2;
			texture->format = Texture::SINGLE_16F;
			break;
	}
	// Create a standard texture with the width and height of our window
	glTexImage2D(GL_TEXTURE_2D, 0, glStorageType, size[0], size[1], 0, glPixelDataFormat, glPixelDataType, NULL); 
	AssertGLError("RenderBuffer::CreateTexture");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	AssertGLError("RenderBuffer::CreateTexture");
}

/// Binds this render buffer to the frame buffer which we are going to render to..!
void RenderBuffer::BindTextureToFrameBuffer()
{
	int error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cout<<"GL error"<<error;
		assert(false);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, glType, GL_TEXTURE_2D, texture->glid, 0); // Attach the texture to the color buffer in our frame buffer
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cout<<"GL error"<<error;
		assert(false);
	}
}


#include "Viewport.h"
#include "Window/Window.h"

FrameBuffer::FrameBuffer(Viewport * vp, Vector2i size)
	: size(size), viewport(vp)
{
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
		tex->Save(viewport->window->name+" "+viewport->name+" "+rb->name+".png", true);
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
		* depthBuf = new RenderBuffer(BufferType::DEPTH_BUFFER, BufferStorageType::DEPTH_16F, textureSize),
		* diffBuf = new RenderBuffer(BufferType::COLOR_BUFFER_1, BufferStorageType::RGBA_8, textureSize)//,
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

