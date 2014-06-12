/// Emil Hedemalm
/// 2014-06-14
/// Contains global variables set to specific instances of the application as a whole (disregarding windows, etc.)

#ifndef APPLICATION_H
#define APPLICATION_H

#include "OS/OS.h"
#include "String/AEString.h"
#include <String/AEString.h>
#include "Graphics/Fonts/Font.h"

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults();

namespace Application 
{

	extern String name;

#ifdef WINDOWS

#include "Windows.h"


	extern HINSTANCE hInstance;
	extern int nCmdShow;
#endif


};


#endif
