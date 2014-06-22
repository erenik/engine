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
#include "Window/Window.h"

/// Win32 includes!
#ifdef WINDOWS
#include <windows.h>
/// Linux includes!
#elif defined USE_X11
#include <GL/glx.h>     // connect X server with OpenGL
#include <X11/Xlib.h>
extern Window                  window;
extern Display*                display; // connection to X server
#endif

/// Renders the active scene, including UI, etc.
void GraphicsManager::RenderWindow()
{

	Window * window = graphicsState.activeWindow;
	Vector2i windowSize = graphicsState.activeWindow->WorkingArea();
	Timer timer;
	timer.Start();

#define PrintTime(a) {  timer.Stop(); if (timer.GetMs() > 50) std::cout<<a<<timer.GetMs(); timer.Start();  }
	
	PrintTime("\nFrame start!");

	FrameStats.Reset();

    Timer preRenderTimer;
    preRenderTimer.Start();

    // Set some standard rendering options.
    glEnable(GL_DEPTH_TEST);

	// Backface culling
	if (backfaceCullingEnabled){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}
	else
		glDisable(GL_CULL_FACE);

	PrintTime("\nCullface: ");


    // Clears the color and depth buffers
   // glClearColor(0.1f, 0.1f, 0.15f, 0.5f);
//    glClearColor(renderSettings->clearColor.x, renderSettings->clearColor.y, renderSettings->clearColor.z, 1.0f);
    PrintTime("\nglClearColor: ");

	Vector4f color = window->backgroundColor; 
	glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/// Testing...
//	return;

	PrintTime("\nglClear: ");

	// Reset matrices (needed?)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	graphicsState.modelMatrixD.LoadIdentity();
	graphicsState.modelMatrixF.LoadIdentity();

	// Reset settings to default
	graphicsState.renderedObjects = 0;
	graphicsState.settings |= RENDER_LIGHT_POSITION;	// Enable light-rendering.
	
	// Reset scissor-variables
	graphicsState.viewportX0 = graphicsState.viewportY0 = 0;

	// OK til here.

	// Set default shader program
	Shader * shader = SetShaderProgram("Flat");
	assert(shader && "Unable to set \"Flat\" shader");

    Timer viewportsTimer, viewportTimer;
    viewportsTimer.Start();

    preRenderFrameTime = preRenderTimer.GetMs();

	PrintTime("\nPre-render stuff: ");


	
//	this->cameraToTrack->ProcessMovement(graphicsState.frameTime);

	
	/// Render all viewports..
	if (window->renderViewports)
	{

		List<Viewport*> viewports = graphicsState.activeWindow->viewports;
		// assert(viewports.Size() && "Really? No viewport to render anything?");
		for (int i = 0; i < viewports.Size(); ++i){
			viewportTimer.Start();
			Viewport * vp = viewports[i];
			if (vp == NULL){
				viewports.RemoveIndex(i);
				continue;
			}
			if (vp->camera)
			{
				graphicsState.camera = vp->camera;
				vp->camera->ProcessMovement(graphicsState.frameTime);
			}
			graphicsState.activeViewport = vp;
			RenderViewport(vp);
			if (i < 4)
				renderViewportFrameTime[i] = (float)viewportTimer.GetMs();
		}
		renderViewportsFrameTime = (float)viewportTimer.GetMs();
	}
	
	PrintTime("\nRendering viewports: ");

	// Reset scissor-variables
	graphicsState.viewportX0 = graphicsState.viewportY0 = 0;

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

	PrintTime("\nView port rendered");

	// Reset viewport and the projection matrices as needed after viewports have been drawn!
	glViewport(0, 0, windowSize.x, windowSize.y);
//	std::cout<<"\nViewport size: "<<width<<" x" <<height;
//	UpdateProjection();

	if (window->renderState && StateMan.ActiveState())
		StateMan.ActiveState()->Render(graphicsState);

	// And render FPS while we're at it...!
	if (window->renderFPS)
		RenderFPS();
	
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
		glReadPixels(0, 0, windowSize.x, windowSize.y, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
		window->getNextFrame = false;
	}
	
	// If recording?
	RenderCapture();
}
