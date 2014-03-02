// Emil Hedemalm
// 2013-06-28

#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

#include "Pathfinding/Path.h"

/// A specification for what to render. Could possible be sent in a single message to toggle lots of shit? :)
struct RenderSettings {
	RenderSettings(){
		flags = 0;
		fogBegin = 500.0f;
		fogEnd = 2500.0f;
	};

	/// Bitwise &-ed flags for setting only some things with the message? :)
	int flags;
	// Navmesh and pathfinding.
	List<Waypoint*> selectedWaypoints;
	Path pathToRender;
	/// Color used to clear the screen.
	Vector3f clearColor;

	// For foggy-fogsome.
	float fogBegin, fogEnd;
};

#endif
