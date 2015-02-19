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

#include "Graphics/Camera/Camera.h"

#include "PhysicsLib/Shapes/Ray.h"

#include "Globals.h"
#include "Render/FrameBuffer.h"

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
	shadowMapDepthBuffer = NULL;
	printShadowMaps = false;
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
	SAFE_DELETE(shadowMapDepthBuffer);
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

/// Calls glViewport with the location and the size of this viewport.
void Viewport::SetGLViewport()
{
	glViewport(bottomLeftCorner.x, bottomLeftCorner.y, size.x, size.y);
}

/// Update size based on window it resides in.
void Viewport::UpdateSize()
{
	assert(window);
	if (relative)
	{
	//	Vector2i windowSize = window->Size();
		Vector2i windowWorkingArea = window->ClientAreaSize();
		size = relativeSize.ElementMultiplication(windowWorkingArea);
		bottomLeftCorner = relativeOffset.ElementMultiplication(windowWorkingArea);
	}
	else 
	{
		assert(size.MaxPart());
	}

	/// Absolute-values of the window... meaning?
	absMin = bottomLeftCorner;
	absMax = bottomLeftCorner + size;
}


UserInterface * Viewport::GetUI() 
{ 
	return ui; 
};


/// Sets up a frame-buffer for this viewport, resizing it as needed. Creates frame buffer if needed. Returns false if something failed along the way.
bool Viewport::BindFrameBuffer()
{
	assert(false && "Create framebuffer yourself, in the RenderPass");
	/*
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
	*/
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


bool Viewport::GetRayFromViewportCoordinates(Vector2i coords, Ray & ray)
{
	if (!camera)
		return false;
	// Use coords with the attached camera?
	assert(this->camera);

	
	const Camera & camera = *this->camera;
	
//	std::cout<<"\nCamera: "<<camera.name;


	/// Calculate near plane coordinates
	Vector3f camPosition = camera.Position();
	Vector3f camLookingAt = camera.LookingAt();
	Vector4f nearPlaneCenter = camPosition + camLookingAt * camera.nearPlane;
	Vector4f nearPlaneUpVec = camera.UpVector();
#define upVector nearPlaneUpVec
	Vector4f nearPlaneRightVec = -camera.LeftVector();
#define rightVector nearPlaneRightVec

  //  std::cout<<"\nCamPosition: "<<camPosition<<" LookingAt: "<<camLookingAt;

	/// Lower left corner
    Frustum frustum = camera.GetFrustum();
	Vector4f nearPlaneLLCorner = nearPlaneCenter - nearPlaneUpVec * frustum.top
		- nearPlaneRightVec * frustum.right;

 //   std::cout<<"\nNearPlaneCenter: "<<nearPlaneCenter<<" NearPlaneLowerLeftcorner: "<<nearPlaneLLCorner;

	// Get relative positions of where we clicketiclicked, from 0.0 to 1.0 (0,0 in lower left corner)
	float clientAreaWidth = (float)size[0];
	float clientAreaHeight = (float)size[1];
	float relativeX = coords[0] / clientAreaWidth,
		  relativeY = coords[1] / clientAreaHeight;

    float zoom = camera.zoom;

 //   relativeX -= 0.5f;
 //   relativeY -= 0.5f;
	Vector3f frustumRight = frustum.hitherBottomRight - frustum.hitherBottomLeft;
	Vector3f frustumUp = frustum.hitherTopLeft - frustum.hitherBottomLeft;
	Vector4f startPoint = frustum.hitherBottomLeft + frustumUp * relativeY
                        + frustumRight * relativeX;
	//Vector4f startPoint = nearPlaneCenter + nearPlaneUpVec * relativeY * frustum.top * 2;

//	nearPlaneLLCorner + nearPlaneUpVec * relativeY * camera.GetFrustum().top * 2
//						+ nearPlaneRightVec * relativeX * camera.GetFrustum().right * 2;

	Vector4f clickDirection;
	if (camera.projectionType == Camera::PROJECTION_3D)
		clickDirection = startPoint - camera.Position();
	/// Straight direction if orthogonal, always.
	else if (camera.projectionType == Camera::ORTHOGONAL)
		clickDirection = camera.LookingAt();
	clickDirection.Normalize3();
	Vector3f endPoint = startPoint - clickDirection * camera.farPlane;
//	std::cout<<"\nEndpoint: "<<endPoint;

	endPoint = frustum.fartherBottomLeft + (frustum.fartherTopLeft - frustum.fartherBottomLeft) * relativeY
		+ (frustum.fartherBottomRight - frustum.fartherBottomLeft) * relativeX;
//	std::cout<<" Endpoint 2: "<<endPoint;

    /// Adjust start point so it looks correct on-screen.
   // startPoint += relativeY * nearPlaneUpVec * frustum.top/ zoom * 0.5f;
   // startPoint += relativeX * nearPlaneRightVec * frustum.right / zoom * 0.5f;

	Ray result;
	result.start = startPoint;
	result.direction = clickDirection;
	ray = result;
	if (debug == 10)
	{
		std::cout<<"\nRayFromScreenCoords S: "<<ray.start<<" D: "<<ray.direction;
	}
//	std::cout<<"\nRay dir: "<<result.direction<<" start: "<<result.start;
	return true;
}
