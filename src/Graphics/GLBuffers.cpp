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


/// Returns a new buffer to be used.
unsigned int GLBuffers::New()
{
	unsigned int newBufferID;
	glGenBuffers(1, &newBufferID);
	buffers.Add(newBufferID);
	return newBufferID;
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

