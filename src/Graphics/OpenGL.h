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

#endif