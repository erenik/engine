/// Emil Hedemalm
/// 2014-07-09
/// A buffer representing one layer of data for a specific viewport. 

#include "RenderBuffer.h"
#include "Graphics/GLBuffers.h"
#include "Graphics/OpenGL.h"
#include "TextureManager.h"

// See above.  NOTE: The RenderBuffer constructor is special in that it will initialize all GL-related objects within the constructor.
RenderBuffer::RenderBuffer(String name, int type, int storageType, Vector2i size)
	: name(name), type(type), storageType(storageType), size(size)
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
		case BufferStorageType::DEPTH_32F:
			glStorageType = GL_DEPTH_COMPONENT32;
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
		case BufferStorageType::DEPTH_32F:
			glPixelDataFormat = GL_DEPTH_COMPONENT;
			glPixelDataType = GL_FLOAT;
			texture->bpp = 4;
			texture->format = Texture::SINGLE_32F;
			break;
	}
	// Create a standard texture with the width and height of our window
	assert(size.x > 0 && size.y > 0);
	glTexImage2D(GL_TEXTURE_2D, 0, glStorageType, size.x, size.y, 0, glPixelDataFormat, glPixelDataType, NULL); 
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

