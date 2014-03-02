// Emil Hedemalm
// 2013-07-04

#ifndef XPROC_INCLUDED
#define XPROC_INCLUDED

#include "OS/OS.h"
#include "OS/WindowSystem.h"

#ifdef USE_X11

#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <GL/glx.h>     // connect X server with OpenGL


/// XWindow Event Processor, return NULL for basic actions, integer numbers for exit codes.
void * XProc(XEvent & event);

#endif // LINUX

#endif // XPROC_INCLUDED
