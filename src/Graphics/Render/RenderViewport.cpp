// Emil Hedemalm
// 2013-06-09

#include "RenderViewport.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Camera/Camera.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"
#include "Maps/2D/TileMap2D.h"
#include "GraphicsState.h"
#include "../FrameStatistics.h"
#include "UI/UserInterface.h"

RenderViewport::RenderViewport(String uiSource)
: Viewport(0,0,0,0), uiSource(uiSource)
{
	Initialize();
};
RenderViewport::RenderViewport(float x, float y, float width, float height)
: Viewport(x,y,width,height)
{
	Initialize();
}

// Set initial default/NULL-values.
void RenderViewport::Initialize(){
//	camera = new Camera()
	camera = NULL;
	relative = false;
	viewPortUI = NULL;
}

RenderViewport::~RenderViewport()
{
	camera = NULL;
	// Delete UI! o/o
	delete viewPortUI;
	viewPortUI = NULL;
}

void RenderViewport::SetRelative(float ix, float iy, float iwidth, float iheight){
	relative = true;
	x0 = ix;
	y0 = iy;
	width = iwidth;
	height = iheight;
}
void RenderViewport::SetCameraToTrack(Camera * icamera){
	camera = icamera;
}

void RenderViewport::AdjustToWindow(int w_width, int w_height){
	// Update absolute-values.
  //  assert(false);
	assert(relative);

	// Assume this function is only called from the render-thread.
	// Adjust UI as needed.
	if (viewPortUI){
		if (viewPortUI->AdjustToWindow(width * w_width, height * w_height)){
			if (viewPortUI->IsBuffered())
				viewPortUI->Unbufferize();
			viewPortUI->Bufferize();
		}
	}
}

void RenderViewport::Render(GraphicsState &graphicsState){
	if (width == 0 || height == 0){
		std::cout<<"\nRenderViewport::Render: NULL viewport width or height! Is this the intent?";
		return;
	}



	/// Relative coordinates
	if (relative){
		// Reset scissor-variables
		graphicsState.viewportX0 = x0 * Graphics.width;
		graphicsState.viewportY0 = y0 * Graphics.height;
		glViewport(x0 * Graphics.width, y0 * Graphics.height, width * Graphics.width, height * Graphics.height);
	}
	/// Absolute coordinates
	else
		glViewport(x0, y0, width, height);

//    std::cout << "Relative=" << relative << " Width="<< width << " Height= " << height << std::endl;

	// Updates the projection matrices using the manager's width/height
	Graphics.UpdateProjection(width, height);
	// Update camera
	camera->ProcessMovement(graphicsState.frameTime);
	// Set active camera to current one
	graphicsState.camera = camera;

	// Copy over the matrices to float
	graphicsState.viewMatrixF = graphicsState.viewMatrixD = camera->ViewMatrix4d();
	graphicsState.modelMatrixF = graphicsState.modelMatrixD.LoadIdentity();
	graphicsState.projectionMatrixF = graphicsState.projectionMatrixD = camera->ProjectionMatrix4d();

	graphicsState.graphicEffectsToBeRendered.Clear();
	graphicsState.particleEffectsToBeRendered.Clear();

	Timer sceneTimer, alphaEntitiesTimer, effectsTimer, uiTimer;
	sceneTimer.Start();
	// Render le map as wanted?
	if (Graphics.mapToRender && Graphics.renderMap){
		Graphics.mapToRender->Render(graphicsState);
	}
	// Render the scene, now
	else
		Graphics.RenderScene();
	FrameStats.sceneTime += sceneTimer.GetMs();

	// Render alpha entities!
	alphaEntitiesTimer.Start();
	Graphics.RenderAlphaEntities();
	FrameStats.alphaTime += alphaEntitiesTimer.GetMs();

	// Render effects!
	effectsTimer.Start();
	Graphics.RenderEffects();
	FrameStats.effectsTime += effectsTimer.GetMs();

	/// Wosh.
	if (Graphics.renderLookAtVectors)
		Graphics.RenderEntityVectors();

	// Render the grid, yo, for orientation in world space.
//#define ALWAYS_GRID
#ifndef ALWAYS_GRID
	if (Graphics.renderGrid)
#endif
		Graphics.RenderGrid();
	// Render vfcOctree with regular objects
	Graphics.RenderSelection();
//#define ALWAYS_PHYSICS
#ifndef ALWAYS_PHYSICS
	// Render physics if wanted.
	if (Graphics.renderPhysics)
#endif
		Graphics.RenderPhysics();
	// Render lights if wanted.
	if (Graphics.renderLights)
		Graphics.RenderLights();

	// Renders names of all AI-characters, settings set in the AIManager or GraphicsManager...?!
	Graphics.RenderAI();	
	if (Graphics.renderNavMesh){
		/// Get mutex for the activeNavMesh
		if (WaypointMan.GetActiveNavMeshMutex(10)){
			Graphics.RenderNavMesh();
			if (PathMan.GetLatsPathMutex(10)){
				Graphics.RenderPath();
				PathMan.ReleaseLastPathMutex();
			}
			WaypointMan.ReleaseActiveNavMeshMutex();
		}
	}

    /// Render simple-shapesuuu
	Graphics.RenderShapes();

	// Render ui if we got one?
	uiTimer.Start();
	if (viewPortUI){
		Graphics.RenderUI(viewPortUI);
	}
	FrameStats.uiTime += uiTimer.GetMs();
}
