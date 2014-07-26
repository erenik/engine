/// Emil Hedemalm
/// 2014-06-14
/// Contains global variables set to specific instances of the application as a whole (disregarding windows, etc.)

#ifndef APPLICATION_H
#define APPLICATION_H

#include "OS/OS.h"
#include "String/AEString.h"
#include <String/AEString.h>
#include "Graphics/Fonts/TextFont.h"

/// Call to set application name, root directories for various features, etc.
/** Describe how an application would typicall set up stuff in it?

	...
	Application::name = "My application";

*/
void SetApplicationDefaults();

/// Some global variables for how the application should behave.
namespace Application 
{
	/// Defines if the application quits straight away when pressing the X or hitting e.g. ALT+F4, or open a query dialogue about it.
	extern bool queryOnQuit;
	extern String name;

#ifdef WINDOWS

#include "Windows.h"


	extern HINSTANCE hInstance;
	extern int nCmdShow;
#endif


};


#endif
