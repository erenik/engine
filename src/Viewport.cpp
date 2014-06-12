/// Emil Hedemalm
/// 2014-06-12
/// Merge of the previously divided Viewport and RenderViewport classes.

#include "Viewport.h"
#include "Graphics/GraphicsManager.h"
#include "Entity/Entity.h"
#include "GraphicsState.h"
#include "UI/UserInterface.h"
#include "Window/Window.h"


Viewport::Viewport()
{
	Initialize();
}

Viewport::Viewport(String uiSource)
: uiSource(uiSource)
{
	Initialize();
};

Viewport::Viewport(Vector2i bottomLeftCorner, Vector2i size)
{
	Initialize();
	this->bottomLeftCorner = bottomLeftCorner;
	this->size = size;
}

// Set initial default/NULL-values.
void Viewport::Initialize(){
//	camera = new Camera()
	camera = NULL;
	relative = false;
	ui = NULL;
	window = NULL;
	id = idEnumerator++;
}

Viewport::~Viewport()
{
	camera = NULL;
	// Delete UI! o/o
	delete ui;
	ui = NULL;
}

/// Unique ID for this viewport.
int Viewport::ID(){
	return id;
}
/// Unique ID
int Viewport::idEnumerator = 0;


/// Sets the viewport to use relative coordinates.
void Viewport::SetRelative(Vector2f bottomLeftCorner, Vector2f size)
{
	relative = true;
	relativeOffset = bottomLeftCorner;
	relativeSize = size;
}

void Viewport::SetCameraToTrack(Camera * icamera){
	camera = icamera;
}


/// Update size based on window it resides in.
void Viewport::UpdateSize()
{
	assert(window);
	if (relative)
	{
		size = relativeSize.ElementMultiplication(window->Size());
		bottomLeftCorner = relativeOffset.ElementMultiplication(window->Size());
	}
}


UserInterface * Viewport::GetUI() 
{ 
	return ui; 
};

