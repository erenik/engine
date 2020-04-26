/// Emil Hedemalm
/// 2014-06-12
/// Separated into own file.

#include "Graphics/GraphicsManager.h"
#include "Viewport.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"
#include "Maps/2D/TileMap2D.h"
#include "Graphics/FrameStatistics.h"

#include "Render/RenderPipeline.h"

float backgroundFactor = 0.f;

void GraphicsManager::RenderViewport(Viewport * vp)
{
	CheckGLError("GraphicsManager::RenderViewport - before");

	Timer timer;
	timer.Start();

	vp->UpdateSize();
	/// Viewport.. so width and height are based on the viewport.
	int width = vp->size[0], height = vp->size[1];
//	std::cout<<"\nViewport size: "<<width<<"x"<<height;
	
	/// Absolute coordinates
	graphicsState.activeViewport->SetGLViewport();
	glEnable(GL_SCISSOR_TEST);
	glScissor(vp->bottomLeftCorner[0], vp->bottomLeftCorner[1], vp->size[0], vp->size[1]);

	// draw bg
	Vector3f bgColor = vp->backgroundColor + Vector3f(1,1,1) * backgroundFactor;
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fetch camera to use. 
	Camera * camera = vp->camera;
	if (!camera)
		camera = this->cameraToTrack;
	if (!camera)
	{
		return;
	}
	// Movement should be processed.. in physics or earlier.
//	camera->ProcessMovement(GraphicsThreadGraphicsState.frameTime);
	camera->SetRatioI(width, height);
	camera->UpdateProjectionMatrix();
	// Set active camera to current one
	graphicsState.camera = camera;

	// Copy over the matrices to float
	graphicsState.viewMatrixF = graphicsState.viewMatrixD = camera->ViewMatrix4d();
	graphicsState.modelMatrix.LoadIdentity();
	graphicsState.projectionMatrixF = graphicsState.projectionMatrixD = camera->ProjectionMatrix4d();

	// Clear lists so that the render-passes are performed as requested.
	graphicsState.graphicEffectsToBeRendered.Clear();
	graphicsState.particleEffectsToBeRendered.Clear();
	// Add global particle systems if they are within range?
	graphicsState.particleEffectsToBeRendered.Add(Graphics.globalParticleSystems);


	Timer sceneTimer, alphaEntitiesTimer, effectsTimer, uiTimer;
	sceneTimer.Start();

	// Cull entities depending on the viewport and camera.
	// TODO: Actually cull it too. 
	graphicsState.entities = registeredEntities;
	
	timer.Stop();
	FrameStats.renderPrePipeline += timer.GetMs();

	/// Old pipeline configuration! Only testing with the regular entities first. 
	RenderPipeline * renderPipeline = graphicsState.renderPipe;
	/// Test with alpha-entities and other passes later on...
	if (renderPipeline)
	{
		renderPipeline->Render(graphicsState);
	}
	// Default/old fixed pipeline.
	else {
		// Render le map as wanted?
		if (Graphics.mapToRender && ActiveViewport->renderMap){
			Graphics.mapToRender->Render(graphicsState);
		}
		// Render the scene, now
		else
			Graphics.RenderScene();
	}	

	timer.Start();
	
	FrameStats.sceneTime += sceneTimer.GetMs();

	CheckGLError("Pre alpha entities GraphicsManager::RenderViewport");
	// Render alpha entities!
	alphaEntitiesTimer.Start();
	Graphics.RenderAlphaEntities();
	FrameStats.alphaTime += alphaEntitiesTimer.GetMs();

	CheckGLError("Pre effects GraphicsManager::RenderViewport");
	// Render effects!
	effectsTimer.Start();
	Graphics.RenderEffects();
	FrameStats.effectsTime += effectsTimer.GetMs();

	CheckGLError("Pre skeletons GraphicsManager::RenderViewport");
	Graphics.RenderSkeletons();

	/// Wosh.
	if (Graphics.renderLookAtVectors)
		Graphics.RenderEntityVectors();

	// Render the grid, yo, for orientation in world space.
	if (vp->renderGrid)
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

	timer.Stop();
	FrameStats.renderPostPipeline += timer.GetMs();
}
