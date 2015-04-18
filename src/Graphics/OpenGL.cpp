/// Emil Hedemalm
/// 2014-07-10
/// Header to automatically include GL as needed. 
/// Windows requires that Windows.h is loaded first for some reason :<

#include "OpenGL.h"

/// Keeps track of GL version for the client
int GL_VERSION_MAJOR = 0;
int GL_VERSION_MINOR = 0;



/// Uses switch-case to print out relevant informatoin to the console. Returns the error code if any.
int CheckGLError(String inFunction)
{
	int error = glGetError();
	switch(error)
	{
		case GL_NO_ERROR: return 0;
		case GL_INVALID_ENUM:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_INVALID_FRAMEBUFFER_OPERATION: Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete, as defined here. : Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete, as defined here. ";
			break;
		case GL_TABLE_TOO_LARGE:
			std::cout<<"\n"<<inFunction<<": "<<error<<" GL_TABLE_TOO_LARGE.";
			break;
		default:
			std::cout<<"\nUnknown GLerror "<<error;
			assert(false);
			break;
	}
	return error;
}
/// Similar to CheckGLError but will assert if there is an error. Returns the error code if any.
int AssertGLError(String inFunction)
{
	int error = CheckGLError(inFunction);
	assert(error == 0);
	return error;
}

#include "Window/WindowSystem.h"

/// Linux includes and globals
#ifdef USE_X11
#include <GL/glx.h>     // connect X server with OpenGL
extern GLXContext       context; // OpenGL context
extern Display*         display; // connection to X server
extern XVisualInfo*     visual_info;
extern Window           window;
extern void testRender();
#endif

