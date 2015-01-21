// Emil Hedemalm
// 2013-06-06

#include "GraphicsState.h"
#include "GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "TextureManager.h"
#include "Multimedia/MultimediaManager.h"
#include "FrameStatistics.h"
#include "GLBuffers.h"

#include "OS/OS.h"
#include "Window/WindowSystem.h"
#include "OS/Sleep.h"
#include "Window/WindowManager.h"
#include "Graphics/Camera/Camera.h"
#include <cstdio>
#include <cstring>

#include "Audio/AudioManager.h"
#include "Audio/OpenAL.h"

#include "File/LogFile.h"

/// Windows includes and globals
#ifdef WINDOWS
#include <windows.h>
#include <process.h>
extern uintptr_t graphicsThread;	// Graphics thread pointer from Initializer.cpp
//extern HWND hWnd;			// Window handle
//extern HDC	hdc;			// Device context
//HGLRC		hRC = NULL;		// GL rendering context

/// Linux includes and globals
#elif defined USE_X11
// Global variables
extern pthread_t graphicsThread;
#include <GL/glx.h>     // connect X server with OpenGL
extern GLXContext       context; // OpenGL context
extern Display*         display; // connection to X server
extern XVisualInfo*     visual_info;
extern Window           window;
extern void testRender();

// OSX!
#elif defined OSX & 0
#include <AGL/AGL.h>
#endif // OSXx

int renderInstancesBegun = 0;
#include <fstream>

/// Main graphics manager processing thread
#ifdef WINDOWS
void GraphicsManager::Processor(void * vArgs){
#elif defined LINUX | defined OSX
void * GraphicsManager::Processor(void * vArgs){
#endif

    std::cout<<"\n=======================================================";
    std::cout<<"\nGraphicsProcessor begun!";
    renderInstancesBegun++;
    std::cout<<"\nInstancesBegun: "<<renderInstancesBegun;
    assert(renderInstancesBegun == 1);


	LogGraphics("Graphics processing thread started");

    int result;
#ifdef WINDOWS

	/// Wait until we have a valid window we can render onto.
	while(WindowMan.NumWindows() == 0)
	{
		LogGraphics("Waiting for windows");
		Sleep(50);
	}
	LogGraphics("Creating GL context");
	Window * mainWindow = WindowMan.GetWindow(0);
	mainWindow->CreateGLContext();
	mainWindow->MakeGLContextCurrent();

#elif defined USE_X11
    /// Create GL context! ^^
    std::cout<<"\n=======================================================";
    std::cout<<"\nCreating GLX context...";
    context = glXCreateContext(display, visual_info, None, true);
    if (context == NULL){
        std::cout<<"\n=======================================================";
        std::cout<<"ERROR: Could not create rendering context!";
        return NULL;
        assert(false && "could not create rendering context");
    }
    std::cout<<"\n=======================================================";
    std::cout<<"\nGLX context created!";
      // bind the rendering context to the window
    bool bound = glXMakeContextCurrent(display, window, window, context);
    if (bound == false)
    {
        std::cout<<"ERROR: Could not create rendering context!";
        return NULL;
        assert(false && "Failed to bind context");
    }
    std::cout<<"\n=======================================================";
    std::cout<<"\nGLX context made current!";

// OSX gl initialization.
#elif defined OSX & 0

    CGDirectDisplayID displayID = CGMainDisplayID();
    CGOpenGLDisplayMask openGLDisplayMask = CGDisplayIDToOpenGLDisplayMask(displayID);
    AGLPixelFormat pixelFormat = NULL;
    AGLContext context = NULL;
    GLint attributes[50] = {
        AGL_RGBA,
        AGL_DOUBLEBUFFER,
        AGL_DEPTH_SIZE, 16,
        AGL_DISPLAY_MASK,
        (GLint)openGLDisplayMask,
        AGL_NONE
    };

    pixelFormat = aglCreatePixelFormat(attributes);
    std::cout<<"\n=======================================================";
    std::cout<<"\nCreating AGL context...";
    context = aglCreateContext(pixelFormat, NULL);
    assert(context);
    result = aglSetCurrentContext(context);
    assert(result && "Failed to make gl context current");
#endif // OSX

	// Initialize glew
    std::cout<<"\nInitializing GLEW...";
	LogGraphics("Initializing GLEW");

//	glewExperimental = GL_TRUE;
	result = glewInit();
	if (result != GLEW_OK){
		/* Problem: glewInit failed, something is seriously wrong. */
		  std::cout<<"\nGL Error: "<<glewGetErrorString(result)<<"\n";
	}
	/// Check GL version
	const GLubyte * data;
	data = glGetString(GL_VERSION);
	GL_VERSION_MAJOR = data[0]-48;//, (char*)&GL_VERSION_MAJOR, 10);
	GL_VERSION_MINOR = data[2]-48;
	fprintf(stdout, "\nGL Status: Using GL %s", data);
	/// Check GLSL version
	data = glGetString(GL_SHADING_LANGUAGE_VERSION);
	int GL_SL_VERSION_MAJOR = data[0];
	int GL_SL_VERSION_MINOR = data[2];
	fprintf(stdout, "\nGL Status: Using GLSL %s", data);
	/// Check GLEW version
	data = glewGetString(GLEW_VERSION);
//	int GLEW_VERSION_MAJOR = data[0];
//	int GLEW_VERSION_MINOR = data[2];
	fprintf(stdout, "\nGL Status: Using GLEW %s", data);

	// Print other details of the implementation
	fprintf(stdout, "\nGL Status: GL Vendor %s", glGetString(GL_VENDOR));
	fprintf(stdout, "\nGL Status: GL Renderer %s\n", glGetString(GL_RENDERER));

	// Check framebuffer object extension
	char * gl_extensions = (char*)glGetString(GL_EXTENSIONS);
#ifdef PRINT_EXTENSIONS
	std::cout<<"\nSupported GL_EXTENSIONS: "<<gl_extensions;
#endif
	Graphics.support_framebuffer_via_ext;
	// Detect framebuffer object support via ARB (for OpenGL version < 3) - also uses non-EXT names
	if (strstr(gl_extensions, "ARB_framebuffer_object") != 0){
		std::cout<<"\nFramebuffer objects supported.";
		Graphics.support_framebuffer_via_ext = false;
	}
	// Detect framebuffer object support via EXT (for OpenGL version < 3) - uses the EXT names
	else if (strstr(gl_extensions, "EXT_framebuffer_object") != 0){
		std::cout<<"\nFramebuffer objects supported but require EXT.";
		Graphics.support_framebuffer_via_ext = true;
	}
	if (GL_VERSION_MAJOR < 4)
		Graphics.useDeferred = false;

	 // If support is via EXT (OpenGL version < 3), add the EXT suffix; otherwise functions are core (OpenGL version >= 3)
    // or ARB without the EXT suffix, so just get the functions on their own.
    std::string suffix = (Graphics.support_framebuffer_via_ext ? "EXT" : "");
	// Bind functions
/*	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress((std::string("glGenFramebuffers") + suffix).c_str());
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress((std::string("glDeleteFramebuffers") + suffix).c_str());
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress((std::string("glBindFramebuffer") + suffix).c_str());
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress((std::string("glFramebufferTexture2D") + suffix).c_str());
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress((std::string("glCheckFramebufferStatus") + suffix).c_str());
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress((std::string("glGenerateMipmap") + suffix).c_str());
*/
	/// Setup context
#ifdef WINDOWS
	// Context should already current, as set up above.
	/*HGLRC hRC2 = wglGetCurrentContext();
	wglMakeCurrent(hdc, hRC);
	if (glGetError() != GL_NO_ERROR)
		throw 3;*/
#endif

	/// Test.
//	mainWindow->MemLeakTest();


    std::cout<<"\nCreating shaders...";
	// Load shaders!
	ShadeMan.CreateDefaultShaders();

	// Display the window ^^
#ifdef WINDOWS
	/*ShowWindow(hWnd, 1);
	UpdateWindow(hWnd);*/
#endif


	/// Bufferize the rendering box
	Graphics.OnBeginRendering();


    // Some times.
    long long lastOptimization = Timer::GetCurrentTimeMs();
    long long now;
	// To use when not rendering, since I'm failing with it at the moment.
	long long timeNotRendered = lastOptimization;

	// Pointarr.
	GraphicsState * graphicsState = Graphics.graphicsState;

	LogGraphics("Beginning main rendering/physics/multimedia loop");
	// Then begin the main rendering loop
	while(Graphics.shouldLive)
	{
		try 
		{
			/// Timing
			Timer physicsProcessingTimer, renderTimer, total;
			Timer gmTimer, guTimer, pmTimer;

			total.Start();
			now = Timer::GetCurrentTimeMs();
			graphicsState->frametimeStartMs = now;

			Timer sleepTimer;
			sleepTimer.Start();
			/// Sleep at least a bit...
			int sleepTime = Graphics.sleepTime;
			Sleep(sleepTime);
			/// Sleep more when not in focus.
			if (!WindowMan.InFocus())
				Sleep(Graphics.outOfFocusSleepTime);
			sleepTimer.Stop();
	//		std::cout<<"\nSlept for "<<sleepTimer.GetMs()<<" ms";

		//	std::cout<<"\nProcessing physics messages... ";
			
			Timer totalPhysics;
			FrameStats.ResetPhysics();
			totalPhysics.Start();
			pmTimer.Start();
			/// Process any available physics messages first
			Physics.ProcessMessages();
			pmTimer.Stop();
			FrameStats.physicsMessages = pmTimer.GetMs();
			physicsProcessingTimer.Start();
			// Process physics from here in order to avoid graphical issues
			Physics.ProcessPhysics();
			physicsProcessingTimer.Stop();
			FrameStats.physicsProcessing = physicsProcessingTimer.GetMs();

			totalPhysics.Stop();
			FrameStats.totalPhysics = totalPhysics.GetMs();

			Timer multimedia;
			multimedia.Start();
			
			// Process Active Multimedia-streams if available.
			MultimediaMan.Update();

			// Process audio
			AudioMan.Update();
			multimedia.Stop();
			FrameStats.multimedia = multimedia.GetMs();

			/// Process graphics if we can claim the mutex within 10 ms. If not, skip it this iteration.
			if (GraphicsMan.graphicsProcessingMutex.Claim(100))
			{
				Graphics.processing = true;
				Timer graphicsTotal;
				graphicsTotal.Start();
				// Process graphics messages
				gmTimer.Start();
				Graphics.ProcessMessages();
				gmTimer.Stop();
				FrameStats.graphicsMessages = gmTimer.GetMs();

				/// Check if we should render or not.
				bool renderOnQuery = Graphics.renderOnQuery;
				bool renderQueried = Graphics.renderQueried;
				bool shouldRender = (renderOnQuery && renderQueried) || !renderOnQuery;

			//	std::cout<<"\n- Processing graphics messages time taken: "<<gmTimer.GetMs();
				if (Graphics.renderingEnabled && shouldRender && !Graphics.renderingStopped)
				{
					Timer timer;
					timer.Start();
					
					guTimer.Start();
					// Update the lights' positions as needed.
					Graphics.UpdateLighting();
					timer.Stop();
					FrameStats.updateLighting = timer.GetMs();

					// Reposition all entities that are physically active (dynamic entities) in the octrees and other optimization structures!
					timer.Start();
					Graphics.RepositionEntities();
					timer.Stop();
					FrameStats.graphicsRepositionEntities = timer.GetMs();

					/// Process particles systems, etc.
					timer.Start();
					Graphics.Process();
					Graphics.graphicsUpdatingFrameTime = guTimer.GetMs();
				//	std::cout<<"\n- Updating entities frame time: "<<guTimer.GetMs();
					/// Process movement for cameras.
					CameraMan.Process();
					timer.Stop();
					FrameStats.graphicsProcess = timer.GetMs();

					// Render
					renderTimer.Start();
					Graphics.RenderWindows();
					renderTimer.Stop();
					FrameStats.renderTotal = renderTimer.GetMs();
					Graphics.renderQueried = false;
				
				}
			
				else {
					if (now > timeNotRendered + 1000)
					{
		//				std::cout<<"\nPrint if rendering is disabled?";
						timeNotRendered = now;
					}
				}
				graphicsTotal.Stop();
				FrameStats.totalGraphics = graphicsTotal.GetMs();

				Graphics.processing = false;
				Graphics.graphicsProcessingMutex.Release();
			}
			else {
				// Busy.
				std::cout<<"\nUnable to claim graphicsProcessing mutex within given time frame, skipping processing this iteration";
			}
		//	std::cout<<"\nGraphicsFrameTime: "<<graphicsTimer.GetMs();

			total.Stop();
			/// Push frame time and increase optimization once a second if needed.
			FrameStats.PushFrameTime(total.GetMs());
			int fps = FrameStats.FPS();
			if (FrameStats.printQueued)
				FrameStats.Print();

			// How long the last frame took. Used for updating some mechanisms next frame.
			graphicsState->frameTime = Graphics.frameTime * 0.001;

			/// Enable the below via some option maybe, it just distracts atm.
			if (graphicsState->optimizationLevel < 5 && fps < 20 && now > lastOptimization + 100)
			{
				graphicsState->optimizationLevel++;
				lastOptimization = now;
			//	std::cout<<"\nFPS low, increasing graphics optimization level to: "<<graphicsState->optimizationLevel;
			}
			
			else if (graphicsState->optimizationLevel > 0 && fps > 40 && now > lastOptimization + 500)
			{
				graphicsState->optimizationLevel--;
				lastOptimization = now;
			//	std::cout<<"\nFPS high again, decreasing graphics optimization level to: "<<graphicsState->optimizationLevel;
			}
		}
		catch(...)
		{
			LogGraphics("An unexpected error occurred");
			std::cout<<"\nAn unexpected error occurred";
		}
	}
	LogGraphics("Ending main rendering/physics/multimedia loop");

	/// Shut down all remaining music.
	AudioMan.StopAndRemoveAll();

	/// Deallocate audio stufs, since that loop is here still..
	AudioBuffer::FreeAll();
	
	// Macro to free all audio OpenAL resources.
	AL_FREE_ALL

//	std::cout<<"\nExiting rendering loop. Beginning deallocating graphical resources.";

	/// Remove flags and begin deallocating all GL data.
	/// Deallocate textures before we remove any context!
	Graphics.OnEndRendering();

	// Free textures, delete buffers and delete VAOs (if any) before de-allocatin the GL context!
	// https://www.opengl.org/discussion_boards/showthread.php/125908-wglDeleteContext-causing-problems
	TexMan.FreeTextures();
	/// Free buffers
	GLBuffers::FreeAll();

	/// Unbind vertex array so that all may be freed correctly?
	glBindVertexArray(0);
	GLVertexArrays::FreeAll();
	// Unbind frame buffer so they can be freed.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLFrameBuffers::FreeAll();
	// Free all textures.
	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLTextures::FreeAll();

	List<Window*> windows = WindowMan.GetWindows();
	for (int i = 0; i < windows.Size(); ++i)
	{
		windows[i]->DeleteGLContext();
	}

    /// Delete rendering context
    time_t timeTaken, timeStart = clock();
#ifdef WINDOWS
	// Moved to Window::DeleteGLContext()
#elif defined USE_X11
    glXDestroyContext(display, context);
#elif defined OSX & 0
    result = aglDestroyContext(context);
    assert(false);
#endif
	timeTaken = clock() - timeStart;
	std::cout<<"\nDeleting rendering context... "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

	std::cout<<"\nGraphicsThread ending...";
	/// Mark as ready for window-destruction!
	Graphics.finished = true;
#ifdef WINDOWS
	graphicsThread = NULL;
	// May not be needed, and hinders destructors from being called.
//	_endthread();
#elif defined LINUX
    graphicsThread = NULL;
    return 0;
#endif
}
