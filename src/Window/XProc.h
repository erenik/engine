// Emil Hedemalm
// 2013-07-04

#ifndef XPROC_INCLUDED
#define XPROC_INCLUDED

#include "OS/OS.h"
#include "Window/WindowSystem.h"

#ifdef USE_X11

/// XWindow Script Processor, return NULL for basic actions, integer numbers for exit codes.
void * XProc();

#endif // LINUX

#endif // XPROC_INCLUDED
