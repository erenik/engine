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
	TextFont::defaultFontSource = "img/fonts/myFont.png";

*/
void SetApplicationDefaults();

/// Some global variables for how the application should behave.
namespace Application 
{
	/// Defines if the application quits straight away when pressing the X or hitting e.g. ALT+F4, or open a query dialogue about it.
	extern bool queryOnQuit;
	extern String name;
	extern bool quitOnHide; // If true, application will quit when all windows have been hidden from the user. Default .. false/true..?

	/// Controls the main loop of the program. Set to false from within the Deallocator-thread.
	extern bool live;

#ifdef WINDOWS

#include "Windows.h"


	extern HINSTANCE hInstance;
	extern int nCmdShow;
#endif


};


#endif
