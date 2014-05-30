#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "MathLib.h"
#include "PhysicsLib.h"

/** A struct to properly divide window screen-space.
	Can include settings/options that act on a per-viewport basis.
*/
class Viewport {
public:
	/// Default constructor, sets some variables
	Viewport(int x, int y, int width, int height);
	virtual ~Viewport();

	const Vector4f Metrics();
	/// Unique ID for this viewport.
	int ID();

protected:
	/// Unique ID
	static int idEnumerator;
	int id;
	// Size details
	bool relative; // If using relative or absolute values!
	int x0, y0, width, height;
	float relativeXOffset, relativeYOffset;
	float relativeWidth, relativeHeight;
};

/// Screen-to-world space functions, defined by input variables.
/** Returns a ray in 3D space using the given mouse and camera data.
	Mouse coordinates are assumed to be in screen-pixel space (i.e. 0.0 to 800.0 or similar)
*/
Ray GetRayFromScreenCoordinates(int mouseX, int mouseY, Viewport& viewport);


#endif
