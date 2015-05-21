/// Emil Hedemalm
/// 2015-05-20
/// Lazy includer for all X related.

#ifndef X_INCLUDES_H
#define X_INCLUDES_H

#undef Time // Time declared in Xlib, use AETime to refer to our time.
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <GL/glx.h>     // connect X server with OpenGL

#endif