/// Emil Hedemalm
/// 2015-04-18
/// Accessor functions for a screen connected to the active device.

#ifndef SCREEN_H
#define SCREEN_H

#include "MathLib/Vector2i.h"
#include "List/List.h"

class DeviceScreen;

/// Returns size in XY of active or primary screen (no distinction)
Vector2i GetScreenSize();
/// o.o
DeviceScreen PrimaryScreen();
// o.o
List<DeviceScreen> Screens();

class DeviceScreen 
{
public:
	/// If numbered, 0 should be primary one.
	int number;
	Vector2i size;
};



#endif
