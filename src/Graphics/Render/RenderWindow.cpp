/// Emil Hedemalm
/// 2013-03-01

/// Contains the general rendering function pertaining to the GraphicsManager
#include "Graphics/GraphicsManager.h"
#include "../../Pathfinding/WaypointManager.h"
#include "../../Pathfinding/PathManager.h"
#include "Viewport.h"
#include "StateManager.h"
#include "OS/Sleep.h"
#include "Window/WindowSystem.h"
#include "GraphicsState.h"
#include "../FrameStatistics.h"
#include "../RenderSettings.h"
#include "Graphics/Camera/Camera.h"
#include "Texture.h"
#include "Window/AppWindow.h"

/// Renders the active scene, including UI, etc.
void GraphicsManager::RenderWindow()
{

	AppWindow * window = graphicsState->activeWindow;
	Vector2i windowSize = graphicsState->activeWindow->WorkingArea();
	Timer timer;
	timer.Start();

#define PrintTime(a) {  timer.Stop(); if (timer.GetMs() > 50) std::cout<<a<<timer.GetMs(); timer.Start();  }
	
	PrintTime("\nFrame start!");

	Vector4f color = window->backgroundColor; 
	color = Vector4f(0.2f, 0.2f, 0.2f, 1.f);

	extern float backgroundFactor;
	Vector4f bgColor = color + Vector4f(1,1,1,1) * backgroundFactor;
	glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	/*
	/// Testing...
//	return;

	PrintTime("\nglClear: ");

	// Reset matrices (needed?)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	graphicsState->modelMatrixD.LoadIdentity();
	graphicsState->modelMatrixF.LoadIdentity();

	// Reset settings to default
	graphicsState->renderedObjects = 0;
	graphicsState->settings |= RENDER_LIGHT_POSITION;	// Enable light-rendering.
	
	// Reset scissor-variables
	graphicsState->viewportX0 = graphicsState->viewportY0 = 0;

	// OK til here.

	// Set default shader program
	Shader * shader = ShadeMan.SetActiveShader("Flat");
//	assert(shader && "Unable to set \"Flat\" shader");

    
    preRenderFrameTime = preRenderTimer.GetMs();

	PrintTime("\nPre-render stuff: ");


	
//	this->cameraToTrack->ProcessMovement(graphicsState->frameTime);
*/
	
	/// Render all viewports..
	Timer viewportsTimer, viewportTimer;
    viewportsTimer.Start();
	if (window->renderViewports)
	{

		List<Viewport*> viewports = graphicsState->activeWindow->viewports;
		// assert(viewports.Size() && "Really? No viewport to render anything?");
		for (int i = 0; i < viewports.Size(); ++i)
		{
			viewportTimer.Start();
			Viewport * vp = viewports[i];
			if (vp == NULL){
				viewports.RemoveIndex(i);
				continue;
			}
			if (vp->camera)
			{
				graphicsState->camera = vp->camera;
			}
			graphicsState->activeViewport = vp;
			RenderViewport(vp);
			if (i < 4)
				renderViewportFrameTime[i] = (float)viewportTimer.GetMs();
		}
		renderViewportsFrameTime = (float)viewportTimer.GetMs();
	}
	
	PrintTime("\nRendering viewports: ");
	/*
	// Reset scissor-variables
	graphicsState->viewportX0 = graphicsState->viewportY0 = 0;

	// Deferred, bit more complex
	/*
	else {
		/// Set up FBO shit

		for (int i = 0; i < renderViewports.Size(); ++i){
			Viewport * vp = renderViewports[i];
			if (vp == NULL){
				renderViewports.Remove(i);
				continue;
			}
			assert(vp->camera);
			vp->RenderScene(graphicsState);
		}
		/// Apply deferred lighting pass.
		/// Do the remaining scene-stuff now.
		for (int i = 0; i < renderViewports.Size(); ++i){
			Viewport * vp = renderViewports[i];
			if (vp == NULL){
				renderViewports.Remove(i);
				continue;
			}
			assert(vp->camera);
			vp->RenderExtras(graphicsState);
		}
	}
	*/



//	std::cout<<"\nRenderViewports: "<<renderViewports.Size();
	
    Timer postViewportTimer;
    postViewportTimer.Start();
	/*

	PrintTime("\nView port rendered");

	// Reset viewport and the projection matrices as needed after viewports have been drawn!
	glViewport(0, 0, windowSize[0], windowSize[1]);
//	std::cout<<"\nViewport size: "<<width<<" x" <<height;
//	UpdateProjection();

	if (window->renderState && StateMan.ActiveState() && false)
		StateMan.ActiveState()->Render(graphicsState);

	// And render FPS while we're at it...!
	if (window->renderFPS)
		RenderFPS();
	
		*/

	UserInterface * ui = window->ui, 
		* globalUI = window->globalUI;
	
	// Render UI if applicable
	if (window->renderUI && ui)
		RenderUI(ui);

	// Always render globalUI.
	if (globalUI)
	{
		RenderUI(globalUI);
	}

	// Render overlay if wanted~
	RenderOverlay();

	PrintTime("\nRendering FPS, UI and Overlay: ");

    glFlush();
    PrintTime("\nglFlush: ");
    postViewportFrameTime = postViewportTimer.GetMs();

	if (window->getNextFrame)
	{
		Texture * tex = window->frameTexture;
		tex->bpp = 4; // 4 bytes per pixel, RGBA
		tex->Resize(windowSize);
		glReadPixels(0, 0, windowSize[0], windowSize[1], GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
		window->getNextFrame = false;
	}
	
	// If recording?
	RenderCapture();
}
