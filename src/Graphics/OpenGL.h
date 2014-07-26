/// Emil Hedemalm
/// 2013-09-18
/// Header to automatically include GL as needed. 
/// Windows requires that Windows.h is loaded first for some reason :<

#ifndef AE_OPEN_GL_H
#define AE_OPEN_GL_H

#include "OS/OS.h"

#include <GL/glew.h>

#ifdef OSX
    #include <OpenGL/gl.h>
#elif defined WINDOWS
	#include <Windows.h>
    #include <GL/gl.h>
#elif defined LINUX
    #include <GL/gl.h>
#endif // OSs


/// Various statistics and caps related to GL. These are/should be set ONCE upon starting the rendering thread.

/// Keeps track of GL version for the client
extern int GL_VERSION_MAJOR;
extern int GL_VERSION_MINOR;

#include "String/AEString.h"

/// Uses switch-case to print out relevant informatoin to the console. Returns the error code if any.
int CheckGLError(String inFunction);
/// Similar to CheckGLError but will assert if there is an error. Returns the error code if any.
int AssertGLError(String inFunction);

#endif