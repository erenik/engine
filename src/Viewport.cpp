/// Emil Hedemalm
/// 2014-06-12
/// Merge of the previously divided Viewport and RenderViewport classes.

#include "Viewport.h"

#include "GraphicsState.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/GLBuffers.h"

#include "Entity/Entity.h"
#include "UI/UserInterface.h"
#include "Window/Window.h"

#include "Render/RenderBuffer.h"

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
	relative = false;
}

// Set initial default/NULL-values.
void Viewport::Initialize()
{
//	camera = new Camera()
	camera = NULL;
	// Default to relative size of window.
	relative = true;
	relativeSize = Vector2f(1,1);
	ui = NULL;
	window = NULL;
	id = idEnumerator++;
	
	// Render Booleans
	renderGrid = true;
	renderPhysics = false;
	renderCollisionTriangles = false;
	renderSeparatingAxes = false;
	renderFPS = true;
	renderAI = false;
	renderNavMesh = false;
	renderUI = true;
	renderLights = false;
	renderMap = true;
	renderLookAtVectors = false;

	// Render stuff
	frameBuffer = 0;
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
	//	Vector2i windowSize = window->Size();
		Vector2i windowWorkingArea = window->WorkingArea();
		size = relativeSize.ElementMultiplication(windowWorkingArea);
		bottomLeftCorner = relativeOffset.ElementMultiplication(windowWorkingArea);
	}
	else 
	{
		assert(size.MaxPart());
	}
}


UserInterface * Viewport::GetUI() 
{ 
	return ui; 
};


/// Sets up a frame-buffer for this viewport, resizing it as needed. Creates frame buffer if needed. Returns false if something failed along the way.
bool Viewport::BindFrameBuffer()
{
	if (!frameBuffer)
	{
		frameBuffer = new FrameBuffer(this, size);
		frameBuffer->CreateRenderBuffers();
	}
	if (!frameBuffer->IsGood())
	{
		// Try and rebuild it..?
		frameBuffer->CreateRenderBuffers();
		return false;
	}
	int error = glGetError();
	/// Make frame buffer active
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->frameBufferObject);
	AssertGLError("Viewport::BindFrameBuffer");
	// Clear depth  and color
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set buffers to render into (the textures ^^)
	GLenum buffers[10];
	int numBuffers = frameBuffer->GetDrawBuffers(buffers);
	glDrawBuffers(numBuffers, buffers);
	AssertGLError("Viewport::BindFrameBuffer");
	return true;
};

void Viewport::CreateFrameBuffer()
{
		assert(false);
}


// For toggling all debug renders.
void Viewport::EnableAllDebugRenders(bool enabled/* = true*/)
{
	renderAI = renderFPS = renderGrid =
		renderPhysics = renderNavMesh = renderLights = enabled;
}

