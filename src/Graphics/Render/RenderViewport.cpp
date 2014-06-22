/// Emil Hedemalm
/// 2014-06-12
/// Separated into own file.

#include "Graphics/GraphicsManager.h"
#include "Viewport.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"
#include "Maps/2D/TileMap2D.h"
#include "Graphics/FrameStatistics.h"

void GraphicsManager::RenderViewport(Viewport * vp)
{
	vp->UpdateSize();
	/// Viewport.. so width and height are based on the viewport.
	int width = vp->size.x, height = vp->size.y;
	
	/// Absolute coordinates
	glViewport(vp->bottomLeftCorner.x, vp->bottomLeftCorner.y, vp->size.x, vp->size.y);

	glEnable(GL_SCISSOR_TEST);
	glScissor(vp->bottomLeftCorner.x, vp->bottomLeftCorner.y, vp->size.x, vp->size.y);

	// draw bg
	glClearColor(vp->backgroundColor.x, vp->backgroundColor.y, vp->backgroundColor.z,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Fetch camera to use. 
	Camera * camera = vp->camera;
	if (!camera)
		camera = this->cameraToTrack;
	if (!camera)
	{
		return;
	}
	// Movement should be processed.. in physics or earlier.
//	camera->ProcessMovement(graphicsState.frameTime);
	camera->SetRatio(width, height);
	camera->Update();
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
	if (Graphics.mapToRender && ActiveViewport->renderMap){
		Graphics.mapToRender->Render();
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
	if (ActiveViewport->renderGrid)
#endif
		Graphics.RenderGrid();
	// Render vfcOctree with regular objects
	Graphics.RenderSelection();
//#define ALWAYS_PHYSICS
#ifndef ALWAYS_PHYSICS
	// Render physics if wanted.
	if (ActiveViewport->renderPhysics)
#endif
		Graphics.RenderPhysics();
	// Render lights if wanted.
	if (ActiveViewport->renderLights)
		Graphics.RenderLights();

	if (ActiveViewport->renderNavMesh)
	{
		RenderNavMesh();
		RenderPath();
	}

    /// Render simple-shapesuuu
	Graphics.RenderShapes();

	// Render ui if we got one?
	uiTimer.Start();
	if (vp->ui){
		Graphics.RenderUI(vp->ui);
	}
	FrameStats.uiTime += uiTimer.GetMs();
}
