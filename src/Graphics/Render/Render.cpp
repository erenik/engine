/// Emil Hedemalm
/// 2013-03-01

/// Contains the general rendering function pertaining to the GraphicsManager
#include "Graphics/GraphicsManager.h"
#include "../../Pathfinding/WaypointManager.h"
#include "../../Pathfinding/PathManager.h"
#include "RenderViewport.h"
#include "StateManager.h"
#include "OS/Sleep.h"
#include "OS/WindowSystem.h"
#include "GraphicsState.h"
#include "../FrameStatistics.h"
#include "../RenderSettings.h"
#include "Graphics/Camera/Camera.h"

/// Win32 includes!
#ifdef WINDOWS
#include <windows.h>
extern HDC	hdc;			// Device context
/// Linux includes!
#elif defined USE_X11
#include <GL/glx.h>     // connect X server with OpenGL
#include <X11/Xlib.h>
extern Window                  window;
extern Display*                display; // connection to X server
#endif

/// Renders the active scene, including UI, etc.
void GraphicsManager::Render(){

	Timer timer;
	timer.Start();

#define PrintTime(a) { /* timer.Stop(); if (timer.GetMs() > 50) std::cout<<a<<timer.GetMs(); timer.Start(); */ }
	
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
    glClearColor(renderSettings->clearColor.x, renderSettings->clearColor.y, renderSettings->clearColor.z, 1.0f);
    PrintTime("\nglClearColor: ");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	// Set default shader program
	Shader * shader = SetShaderProgram("Flat");
	assert(shader && "Unable to set \"Flat\" shader");

    Timer viewportsTimer, viewportTimer;
    viewportsTimer.Start();

    preRenderFrameTime = preRenderTimer.GetMs();

	PrintTime("\nPre-render stuff: ");

	// Process global camera
	this->cameraToTrack->ProcessMovement(graphicsState->frameTime);

	/// Render all viewports..
	if (true){
		for (int i = 0; i < renderViewports.Size(); ++i){
		    viewportTimer.Start();
			RenderViewport * vp = renderViewports[i];
			if (vp == NULL){
				renderViewports.RemoveIndex(i);
				continue;
			}
			assert(vp->camera);
			vp->Render(*graphicsState);
			if (i < 4)
                renderViewportFrameTime[i] = (float)viewportTimer.GetMs();
		}
	}
	renderViewportsFrameTime = (float)viewportsTimer.GetMs();

	PrintTime("\nRendering viewports: ");

	// Reset scissor-variables
	graphicsState->viewportX0 = graphicsState->viewportY0 = 0;

	// Deferred, bit more complex
	/*
	else {
		/// Set up FBO shit

		for (int i = 0; i < renderViewports.Size(); ++i){
			RenderViewport * vp = renderViewports[i];
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
			RenderViewport * vp = renderViewports[i];
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
	/// Default rendering if no assigned viewport
	if (renderViewports.Size() == 0){
		if (defaultViewPort == NULL){
		//	std::cout<<"\nDefault viewport is NULL! Creating it for you.";
			defaultViewPort = new RenderViewport(0,0,width,height);
			defaultViewPort->camera = cameraToTrack;
		}
		else {
			// Update default view port's camera every frame if we change it.
			defaultViewPort->width = Graphics.width;
			defaultViewPort->height = Graphics.height;
			defaultViewPort->camera = cameraToTrack;
			defaultViewPort->Render(*graphicsState);
		}
	}

    Timer postViewportTimer;
    postViewportTimer.Start();

	PrintTime("\nView port rendered");

	// Reset viewport and the projection matrices as needed after viewports have been drawn!
	glViewport(0,0,width,height);
//	std::cout<<"\nViewport size: "<<width<<" x" <<height;
	UpdateProjection();

	if (StateMan.ActiveState())
		StateMan.ActiveState()->Render(*graphicsState);

	// And render FPS while we're at it...!
	if (renderFPS)
		RenderFPS();

	// Render UI if applicable
	if (this->renderUI && ui)
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

    Timer swapBufferTimer;
    swapBufferTimer.Start();
	// Swap buffers to screen once we're finished.
#ifdef WINDOWS
    SwapBuffers(hdc);
#elif defined USE_X11
    glXSwapBuffers(display, window);
#endif
    swapBufferFrameTime = swapBufferTimer.GetMs();
    PrintTime("\nSwapping buffers: ");
//    std::cout<<"\nSwapBufferTime: "<<swapBufferFrameTime;
}
