#include "Viewport.h"
#include "Graphics/GraphicsManager.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"

Viewport::Viewport(int x, int y, int i_width, int i_height)
{
	x0 = x;
	y0 = y;
	width = i_width;
	height = i_height;
	id = idEnumerator++;
};

Viewport::~Viewport()
{

}

/// Returns the most recently updated projection matrix for this camera
const Vector4f Viewport::Metrics()
{ 
	return Vector4f((float)x0, (float)y0, (float)width, (float)height); 
};

/// Unique ID for this viewport.
int Viewport::ID(){
	return id;
}
/// Unique ID
int Viewport::idEnumerator = 0;