/// Emil Hedemalm
/// 2014-06-14
/// Contains global variables set to specific instances of the application as a whole (disregarding windows, etc.)

#include "Application.h"

namespace Application {

	String name;
	
	// OS-specifics below
#ifdef WINDOWS

	HINSTANCE hInstance;
	int nCmdShow;
#endif

};