/// Emil Hedemalm
/// 2014-06-17
/// Manager for all gl buffers (generated via glGenBuffers), in order
/// to properly deallocate them on exit (via glDeleteBuffers)

#include "GLBuffers.h"
#include "Graphics/OpenGL.h"

List<int> GLBuffers::buffers;
List<int> GLVertexArrays::vertexArrays;
List<int> GLFrameBuffers::frameBuffers;
List<int> GLRenderBuffers::renderBuffers;
List<int> GLTextures::textures;

/// Returns a new buffer to be used.
unsigned int GLBuffers::New()
{
	unsigned int newBufferID;
	glGenBuffers(1, &newBufferID);
	buffers.Add(newBufferID);
	return newBufferID;
}

/// Frees given buffer.
void GLBuffers::Free(unsigned int bufferWithId) {
	// If we have no allocated buffers (only during shutdown), no need to do anything more here.
	if (buffers.Size() == 0) 
		return;
	glDeleteBuffers(1, &bufferWithId);
	buffers.Remove(bufferWithId);
}


// Frees all buffers.
void GLBuffers::FreeAll()
{
	std::cout<<"\nFreeing buffers..";
	for (int i = 0; i < buffers.Size(); ++i)
	{
		unsigned int buffer = buffers[i];
		glDeleteBuffers(1, &buffer);
	}
	std::cout<<"\n"<<buffers.Size()<<" buffers freed.";
	buffers.Clear();
	return;
}


unsigned int GLVertexArrays::New()
{
	unsigned int newVertexArray;
	glGenVertexArrays(1, &newVertexArray);
	vertexArrays.Add(newVertexArray);
	return newVertexArray;
}

void GLVertexArrays::Free(unsigned int vertexArrayId) {
	// If we have no allocated buffers (only during shutdown), no need to do anything more here.
	if (vertexArrays.Size() == 0)
		return;
	glDeleteVertexArrays(1, &vertexArrayId);
	vertexArrays.Remove(vertexArrayId);
}


// Frees all buffers.
void GLVertexArrays::FreeAll()
{
	std::cout<<"\nFreeing vertex arrays..";
	for (int i = 0; i < vertexArrays.Size(); ++i)
	{
		unsigned int vertexArray = vertexArrays[i];
		glDeleteVertexArrays(1, &vertexArray);
	}
	std::cout<<"\n"<<vertexArrays.Size()<<" vertex arrays freed.";
	vertexArrays.Clear();
	return;
}


unsigned int GLFrameBuffers::New()
{
	unsigned int newFrameBuffer;
	glGenFramebuffers(1, &newFrameBuffer);
	frameBuffers.Add(newFrameBuffer);
	return newFrameBuffer;
}
void GLFrameBuffers::FreeAll()
{
	std::cout<<"\nFreeing frame-buffers..";
	for (int i = 0; i < frameBuffers.Size(); ++i)
	{
		unsigned int frameBuffer = frameBuffers[i];
		glDeleteFramebuffers(1, &frameBuffer);
	}
	std::cout<<"\n"<<frameBuffers.Size()<<" vertex arrays freed.";
	frameBuffers.Clear();
	return;
}

unsigned int GLRenderBuffers::New()
{
	unsigned int newRenderBuffer;
	glGenRenderbuffers(1, &newRenderBuffer);
	renderBuffers.Add(newRenderBuffer);
	return newRenderBuffer;
}
void GLRenderBuffers::Free(int glRenderBufferId)
{
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		if (renderBuffers[i] == glRenderBufferId)
		{
			unsigned int renderBuffer = renderBuffers[i];
			glDeleteRenderbuffers(1, &renderBuffer);
			renderBuffers.RemoveIndex(i);
			return;
		}
	}
}
void GLRenderBuffers::FreeAll()
{
	std::cout<<"\nFreeing render-buffers..";
	for (int i = 0; i < renderBuffers.Size(); ++i)
	{
		unsigned int renderBuffer = renderBuffers[i];
		glDeleteRenderbuffers(1, &renderBuffer);
	}
	std::cout<<"\n"<<renderBuffers.Size()<<" vertex arrays freed.";
	renderBuffers.Clear();
	return;
}


unsigned int GLTextures::New()
{
	unsigned int newTexture;
	glGenTextures(1, &newTexture);
	textures.Add(newTexture);
	return newTexture;
}
void GLTextures::FreeAll()
{
	std::cout<<"\nFreeing textures..";
	for (int i = 0; i < textures.Size(); ++i)
	{
		unsigned int texture = textures[i];
		glDeleteTextures(1, &texture);
	}
	std::cout<<"\n"<<textures.Size()<<" vertex arrays freed.";
	textures.Clear();
}


