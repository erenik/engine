/// Emil Hedemalm
/// 2014-06-17
/// Manager for all gl buffers (generated via glGenBuffers), in order
/// to properly deallocate them on exit (via glDeleteBuffers)

#include "GLBuffers.h"
#include "Graphics/OpenGL.h"

List<int> GLBuffers::buffers;


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

List<int> GLVertexArrays::vertexArrays;

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
