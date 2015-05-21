/// Emil Hedemalm
/// 2015-04-18
/// Handler for communicating with the X AppWindow system,.. due to all particularities and diffuse inclusion files and weird definition-hoggings..

#ifndef X_WINDOW_SYSTEM_H
#define X_WINDOW_SYSTEM_H

#include "Device/DeviceScreen.h"
#include "MathLib/Vector2i.h"

class AppWindow;

namespace XWindowSystem 
{
	bool InitThreadSupport();
	// Connects to the X server (WindowSystem) using XOpenDisplay.
	bool Initialize();
	/// Closes connection to the X server.
	bool Shutdown();
	/// Number of attached screens.
	List<DeviceScreen> Screens();

	void SetupDefaultWindowProperties();
	void CreateDefaultWindow();
	void SetupProtocols();

	bool CreateGLContext(AppWindow * forWindow);
	bool DestroyGLContext(AppWindow * forWindow);

	// For testing-purposes.
	void MainLoop();

	// if double buffering is enabled for created windows.
	extern bool swapBuffers;
};

#endif
