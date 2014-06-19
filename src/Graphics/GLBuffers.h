/// Emil Hedemalm
/// 2014-06-17
/// Manager for all gl buffers (generated via glGenBuffers), in order
/// to properly deallocate them on exit (via glDeleteBuffers)

#ifndef GL_BUFFERS_H
#define GL_BUFFERS_H

#include "List/List.h"

class GLBuffers
{
public:
	/// Returns a new buffer to be used.
	static unsigned int New();
	// Frees all buffers.
	static void FreeAll();
private:
	static List<int> buffers;
};

class GLVertexArrays 
{
public:
	static unsigned int New();
	static void FreeAll();
private:
	static List<int> vertexArrays;
};


#endif