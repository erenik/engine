#include "Viewport.h"
#include "Graphics/GraphicsManager.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"

Viewport::Viewport(float x, float y, float i_width, float i_height)
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

/// Unique ID for this viewport.
int Viewport::ID(){
	return id;
}
/// Unique ID
int Viewport::idEnumerator = 0;