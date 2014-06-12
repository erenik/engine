// Emil Hedemalm
// 2013-06-06

#include "GraphicsState.h"
#include "GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "TextureManager.h"
#include "Multimedia/MultimediaManager.h"
#include "FrameStatistics.h"

#include "OS/OS.h"
#include "Window/WindowSystem.h"
#include "OS/Sleep.h"
#include "Window/WindowManager.h"
#include <cstdio>
#include <cstring>

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

    int result;
#ifdef WINDOWS

	/// Wait until we have a valid window we can render onto.
	while(WindowMan.NumWindows() == 0)
		Sleep(5);

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
//	glewExperimental = GL_TRUE;
	result = glewInit();
	if (result != GLEW_OK){
		/* Problem: glewInit failed, something is seriously wrong. */
		  std::cout<<"\nGL Error: "<<glewGetErrorString(result)<<"\n";
	}
	/// Check GL version
	const GLubyte * data;
	data = glGetString(GL_VERSION);
	Graphics.GL_VERSION_MAJOR = data[0]-48;//, (char*)&Graphics.GL_VERSION_MAJOR, 10);
	Graphics.GL_VERSION_MINOR = data[2]-48;
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
	if (Graphics.GL_VERSION_MAJOR < 4)
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

    std::cout<<"\nCreating shaders...";
	// Load shaders!
	Graphics.CreateShaders();

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

	// Then begin the main rendering loop
	while(Graphics.shouldLive){

		/// Timing
		Timer physicsTimer, graphicsTimer, total;
		Timer gmTimer, guTimer;

		total.Start();
        now = Timer::GetCurrentTimeMs();
		graphicsState.currentFrameTime = now;

		Timer sleepTimer;
		sleepTimer.Start();
		/// Sleep at least a bit...
		Sleep(Graphics.sleepTime);
		/// Sleep more when not in focus.
		if (!WindowMan.InFocus())
			Sleep(Graphics.outOfFocusSleepTime);
		sleepTimer.Stop();
//		std::cout<<"\nSlept for "<<sleepTimer.GetMs()<<" ms";

		physicsTimer.Start();
	//	std::cout<<"\nProcessing physics messages... ";
		/// Process any available physics messages first
		Physics.ProcessMessages();
		// Process physics from here in order to avoid graphical issues
		Physics.ProcessPhysics();
		physicsTimer.Stop();


		// Process Active Multimedia-streams if available.
		MultimediaMan.Update();

		graphicsTimer.Start();
		// Process graphics messages
		gmTimer.Start();
		try {
			Graphics.ProcessMessages();
		} catch(...)
		{
			std::cout<<"Errors thrown while processing graphics messages.";
		}
		Graphics.graphicsMessageProcessingFrameTime = gmTimer.GetMs();
		/// Check if we should render or not.
		bool renderOnQuery = Graphics.renderOnQuery;
		bool renderQueried = Graphics.renderQueried;
		bool shouldRender = (renderOnQuery && renderQueried) || !renderOnQuery;
	//	std::cout<<"\n- Processing graphics messages time taken: "<<gmTimer.GetMs();
		if (Graphics.renderingEnabled && shouldRender)
		{
		    guTimer.Start();
			// Update the lights' positions as needed.
			Graphics.UpdateLighting();
			// Reposition all entities that are physically active (dynamic entities) in the octrees and other optimization structures!
			Graphics.RepositionEntities();
			Graphics.Process();
			Graphics.graphicsUpdatingFrameTime = guTimer.GetMs();
		//	std::cout<<"\n- Updating entities frame time: "<<guTimer.GetMs();
			Timer timer;
			timer.Start();
			// Render
			Graphics.RenderWindows();
			timer.Stop();
	//		std::cout<<"\n- Render frame time: "<<timer.GetMs();
			Graphics.renderQueried = false;
		}
		else {
			if (now > timeNotRendered + 1000)
			{
//				std::cout<<"\nPrint if rendering is disabled?";
				timeNotRendered = now;
			}
		}
		graphicsTimer.Stop();
	//	std::cout<<"\nGraphicsFrameTime: "<<graphicsTimer.GetMs();

		total.Stop();
		Graphics.frameTime = total.GetMs();
		long renderFrameTime = graphicsTimer.GetMs();
		Graphics.renderFrameTime = renderFrameTime;
		Graphics.physicsFrameTime = physicsTimer.GetMs();
		graphicsState.frameTime = Graphics.frameTime * 0.001;
	//	std::cout<<"\nTotalFrameTime: "<<renderFrameTime;

        /// Push frame time and increase optimization once a second if needed.
		FrameStats.PushFrameTime(total.GetMs());
		int fps = FrameStats.FPS();
		/// Enable the below via some option maybe, it just distracts atm.
	//	graphicsState.optimizationLevel = 3;

        if (graphicsState.optimizationLevel < 5 && fps < 20 && now > lastOptimization + 100){
            graphicsState.optimizationLevel++;
            lastOptimization = now;
            std::cout<<"\nFPS low, increasing graphics optimization level to: "<<graphicsState.optimizationLevel;
        }
		/*
		else if (graphicsState.optimizationLevel > 0 && fps > 55 && now > lastOptimization + 500){
#define DECREASE_ENABLED
        #ifdef DECREASE_ENABLED
			graphicsState.optimizationLevel--;
			lastOptimization = now;
			std::cout<<"\nFPS high again, decreasing graphics optimization level to: "<<graphicsState.optimizationLevel;
        #endif
		}
		*/
	}

//	std::cout<<"\nExiting rendering loop. Beginning deallocating graphical resources.";

	/// Remove flags and begin deallocating all GL data.
	Graphics.OnEndRendering();

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
	_endthread();
#elif defined LINUX
    graphicsThread = NULL;
    return 0;
#endif
}
