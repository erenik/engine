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
	/// Number of attached screens.
	List<DeviceScreen> Screens();

	void SetupDefaultWindowProperties();
	void CreateDefaultWindow();
	void SetupProtocols();

	void Resize(AppWindow * window, Vector2i newSize);
	void ToggleFullScreen(AppWindow * window);

	// For testing-purposes.
	void MainLoop();
};

#endif
