/// Emil Hedemalm
/// 2015-04-18
/// Accessor functions for a screen connected to the active device.

#include "Screen.h"

#include "OS/OS.h"

#include "Window/WindowSystem.h"
#include "Window/XWindowSystem.h"

/// Returns size in XY of active or primary screen (no distinction)
Vector2i GetScreenSize()
{		
	return PrimaryScreen().size;
}

/// o.o
DeviceScreen PrimaryScreen()
{
	List<DeviceScreen> screens = Screens();
	for (int i = 0; i < screens.Size(); ++i)
	{
		DeviceScreen & screen = screens[i];
		return screen;
	}
	std::cout<<"\nWARNING: Bad screen returned as PrimaryScreen";
//	std::coassert(false &&  "Bad screen");
	return DeviceScreen();
}

// o.o
List<DeviceScreen> Screens()
{
	List<DeviceScreen> screens;
#ifdef WINDOWS
	DeviceScreen primary;
	primary.size.x = GetSystemMetrics(SM_CXSCREEN);
	primary.size.y = GetSystemMetrics(SM_CYSCREEN);
	screens.AddItem(primary);
#elif defined USE_X11
    screens = XWindowSystem::Screens();
#endif

	return screens;
}

