/// Emil Hedemalm
/// 2013-07-03

#include "OS/OS.h"
#include "OS/Sleep.h"

#ifdef WINDOWS
	#include <process.h>
	#include <windows.h>
#endif

/// OpenGL!
#include "Graphics/OpenGL.h"


// All the includes!
#include "Mesh/Square.h"
#include "Fonts/TextFont.h"
#include "Window/Window.h"

#include "UI/UserInterface.h"
#include "Graphics/Messages/GMUI.h"
#include "RenderSettings.h"
#include "GraphicsState.h"
#include "TextureManager.h"
#include "Physics/PhysicsManager.h"
#include "StateManager.h"
#include "Graphics/Camera/Camera.h"
#include "Viewport.h"

#include "Render/RenderPipeline.h"
#include "Render/RenderRay.h"
#include "Render/Renderable.h"
#include "Render/RenderPipelineManager.h"

#include "GraphicsManager.h"
#include "GraphicsProperty.h"

//#include "Texture/Texture.h"
//#include "Texture/TextureManager.h"

/// OS-dependent Window management + threads externed variables, probably from main.cpp
#ifdef WINDOWS
	extern uintptr_t graphicsThread;	// Graphics thread pointer from Initializer.cpp
//	extern HWND		hWnd;			// Window handle
//	extern HDC		hdc;			// Device context
//	extern HGLRC	hRC;		// GL rendering context

#elif defined LINUX
    // Render-thread pointer.
	extern pthread_t graphicsThread;
	// Externed from main.cpp
    extern Display*                display; // connection to X server
    extern Window                  window;
#endif

/// Singleton initialization to null. Require manual allocation of it!
GraphicsManager * GraphicsManager::graphicsManager = NULL;

/// Allocate
void GraphicsManager::Allocate()
{
	assert(graphicsManager == NULL);
	graphicsManager= new GraphicsManager();

	RenderPipelineManager::Allocate();
	RenderPipeMan.LoadFromPipelineConfig();
	Graphics.graphicsState->renderPipe = RenderPipeMan.activePipeline;
}
GraphicsManager * GraphicsManager::Instance()
{
	return graphicsManager;
}

void GraphicsManager::Deallocate()
{
    int waitTime = 0;
    while(!graphicsManager->finished){
        if (waitTime > 1000){
            std::cout<<"\nWaiting for GraphicsThread to end before deallocating manager...";
            waitTime -= 1000;
        }
        Sleep(10);
        waitTime += 10;
    }

	RenderPipelineManager::Deallocate();

	assert(graphicsManager);
	delete(graphicsManager);
	graphicsManager= NULL;
}

/// Prints the values of the error code in decimal as well as hex and the literal meaning of it.
void PrintGLError(const char * text){
	if (text == NULL)
		return;
	GLuint error = glGetError();
	switch(error){
		case 0:
			break;
		case 1280:
			std::cout<<"\n"<<text<<": "<<error<<" GL_INVALID_ENUM";
			break;
		case 1281:
			std::cout<<"\n"<<text<<": "<<error<<" GL_INVALID_VALUE";
			break;
		case 1282:
			std::cout<<"\n"<<text<<": "<<error<<" GL_INVALID_OPERATION";
			break;
		case 1283:
			std::cout<<"\n"<<text<<": "<<error<<" GL_OUT_OF_MEMORY";
			break;
		case 1286:
			std::cout<<"\n"<<text<<": "<<error<<" GL_INVALID_FRAMEBUFFER_OPERATION: Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete, as defined here. : Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete, as defined here. ";
			break;
		default:
			std::cout<<"\nUnknown GLerror "<<error;
			break;
	}
}

/// Constructor which anulls all relevant variables.
GraphicsManager::GraphicsManager()
{
	pauseProcessing = false;
	optimizationStructure = NONE;
//	this->defaultLighting.CreateDefaultSetup();
	frustum = new Frustum();
	// Biggar frustuuum default plz
	vfcOctree = new VFCOctree(50000, *frustum);
	// Default camera values
	defaultCamera = NULL;
	cameraToTrack = NULL;
	selectionToRender = NULL;
	overlayTexture = NULL;
	queuedOverlayTexture = NULL;
	mapToRender = NULL;
	/// Current full-screen flag
	isFullScreen = false;
	/// Sizes pre-fullscreen
	shouldLive = true;
	finished = false;
	renderingStopped = false;
	enteringMainLoop = false;
	shouldFullScreen = false;
	useDeferred = true;

	renderOnQuery  = false;
	renderQueried = true;

	// Nullify GL items
	frameBufferObject = 0;
	depthBuffer = 0;

	diffuseTexture = 0;
	depthTexture = 0;
	normalTexture = 0;
	positionTexture = 0;
	specularTexture = 0;
	tangentTexture = 0;
	normalMapTexture = 0;
	pickingTexture = 0;

	renderingEnabled = true;
	backfaceCullingEnabled = false;

	// Allocate o-o
	deferredRenderingBox = new Square();
	float size = 1.0f;
	deferredRenderingBox->SetDimensions(-size, size, -size, size);


	graphicsState = new GraphicsState();
	renderSettings = new RenderSettings();

	paused = false;
	processing = false;

	ResetSleepTimes();

	// Create mutex for threading
	graphicsMessageQueueMutex.Create("graphicsMessageQueueMutex");
	graphicsProcessingMutex.Create("graphicsProcessingMutex");
}

/// Queries render of 1 frame by posting a GMRender message to the graphics message queue. Should be used instead of setting the boolean below straight away!
void GraphicsManager::QueryRender()
{
	if (Graphics.renderOnQuery)
		Graphics.QueueMessage(new GMRender());
}

/// Set default values to the sleep-times.
void GraphicsManager::ResetSleepTimes(){
	sleepTime = 0;
	outOfFocusSleepTime = 15;
}

void GraphicsManager::Pause()
{
	paused = true;
	while(!graphicsProcessingMutex.Claim(-1))
		;
	assert(!processing);
}
void GraphicsManager::Resume()
{
	paused = false;
	graphicsProcessingMutex.Release();
}


GraphicsManager::~GraphicsManager()
{

	/// Process any remaining messages?
//	ProcessMessages();

	delete renderSettings;
	renderSettings = NULL;
	double timeStart = clock();
	double timeTaken;
	/// Notify the thread to stop processing.
	shouldLive = false;
	/// Wait for the thread in-case it's slow
	int waits = 0;
	while (!finished){
        Sleep(10);
        ++waits;
        if (waits > 100){
            std::cout<<"\nWaiting for render-thread to end..";
            waits = 0;
        }
    }
	graphicsMessageQueueMutex.Destroy();
	graphicsProcessingMutex.Destroy();

	// De-allocate any remaining messages?
	while(messageQueue.Length())
		delete messageQueue.Pop();

	/// Delete stuff
	if (deferredRenderingBox){
		delete deferredRenderingBox;
		deferredRenderingBox = NULL;
	}

	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for graphics thread to end... "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

	// Delete fonts
	fonts.ClearAndDelete();

    /// Delete clickrays if any
    rays.ClearAndDelete();
    renderShapes.ClearAndDelete();

	delete graphicsState;
	graphicsState = 0;

	SAFE_DELETE(frustum);
	SAFE_DELETE(vfcOctree);

	std::cout<<"\n>>> GraphicsManager deallocated successfully.";

}

/// If true, it is still OK to queue messages.
bool GraphicsManager::GraphicsProcessingActive()
{
	return graphicsThread;
}

int GraphicsManager::RegisteredEntities()
{
	return this->registeredEntities.Size();
}


void GraphicsManager::Initialize()
{
	/// Create cameras post-allocation.
	defaultCamera = CameraMan.DefaultCamera();
	cameraToTrack = defaultCamera;
	

#ifdef WINDOWS
	scrWidth = GetSystemMetrics(SM_CXSCREEN);
	scrHeight = GetSystemMetrics(SM_CYSCREEN);
#elif defined LINUX
    xWindow::GetScreenSize(scrWidth, scrHeight);
#endif
	/// Update camera projection for the first time!
	graphicsState->camera = this->defaultCamera;
	Texture * tex = TexMan.GetTextureBySource("img/initializing.png");
	SetOverlayTexture(tex);
}

/// Returns the active camera for the given viewport
Camera * GraphicsManager::ActiveCamera(int viewport /* = 0*/) const 
{
    if (viewport == 0){
		return graphicsState->camera;
    }

  //  else {
		//WindowMan.
  //      assert(viewport < renderViewports.Size());
  //      Viewport * rv = renderViewports[viewport];
  //      return rv->camera;
  //  }
    return NULL;
}

/// Returns the active lighting. 
Lighting * GraphicsManager::ActiveLighting()
{	
	return &lighting;
}


void GraphicsManager::PauseRendering()
{
	GraphicsMan.QueueMessage(new GraphicsMessage(GM_PAUSE_RENDERING));
	while (Graphics.renderingEnabled)
		Sleep(5);
	/// Catch the mutex?
}

/// Stops rendering permanentlty. Should only be called upon application shutdown.
void GraphicsManager::StopRendering()
{
	Graphics.QueueMessage(new GraphicsMessage(GM_PAUSE_RENDERING));
	while (Graphics.renderingEnabled)
		;
	renderingStopped = true;
}

void GraphicsManager::ResumeRendering()
{
	if (renderingStopped)
		return;
	Graphics.QueueMessage(new GraphicsMessage(GM_RESUME_RENDERING));
}


/// Sets screen resolution in pixels
bool GraphicsManager::SetResolution(int i_width, int i_height)
{
	assert(false && "Deprecated");
	/// If we've flagged for no adjusting after start, return here!
	//bool screenSizeAdjustableAfterStart = false;
	//
	//// Check if we should resize or not
	//if (!screenSizeAdjustableAfterStart){
	//	// Screen size is NOT adjustable after start!
	//	// Check if we've initialized width and height
	//	if (width && height)
	//		if (i_width != width || i_height != height)
	//			return false;
	//}

	/*GraphicsMessage * resize = new GMResize(i_width, i_height);
	messageQueue.Push(resize);
	*/return true;
}

// For toggling all debug renders.
void GraphicsManager::EnableAllDebugRenders(bool enabled/* = true*/)
{
	assert(false);
	/*
	renderAI = renderFPS = renderGrid =
		renderPhysics = renderNavMesh = renderLights = enabled;
		*/
}

///** Returns a pointer to the active system-global UI. If it has not been created earlier it will be created upon calling it.
//	If argument is true and no global has been constructed, it will be constructed.
//*/
//UserInterface * GraphicsManager::GetGlobalUI(bool createIfNeeded /*= false*/)
//{
//	// Move this creatoin elsewhere.. wtf man?
//	if (createIfNeeded && !globalUI)
//	{
//		globalUI = new UserInterface();
//		globalUI->CreateRoot();
//		globalUI->name = "System-global UI";
//	}
//	return globalUI; 
//}


/// Sets active UserInterface to be rendered
void GraphicsManager::SetUI(UserInterface * i_ui){
	assert(false);
/*
	if (ui)
		ui->OnExitScope();
	ui = i_ui;
	if (ui)
		ui->OnEnterScope();*/
}

/// Sets system-global ui.
void GraphicsManager::SetGlobalUI(UserInterface * newGlobalUI)
{
	assert(false);

	//globalUI = newGlobalUI;
}


/// Sets overlay texture to be rendered on top of everything else
void GraphicsManager::SetOverlayTexture(String source, int fadeInTime)
{
	/// Check if source is null
	if (source.Length() == 0){
		queuedOverlayTexture = overlayTexture = NULL;
		return;
	}
	/// Check if it's preloaded
	Texture * texture = TexMan.GetTexture(source);
	SetOverlayTexture(texture, fadeInTime);
}

/// Sets overlay texture to be rendered on top of everything else
void GraphicsManager::SetOverlayTexture(Texture * texture, int fadeInTime /* = 0*/)
{
	if (fadeInTime == 0)
	{
		queuedOverlayTexture = overlayTexture = texture;
	}
	else 
	{
		// Log in fade-time after the texture is buffered!
		overlayFadeInTime = fadeInTime;
		queuedOverlayTexture = texture;
		overlayFadeInStart = 0; // But reset it here, or things will not work.
	}
	renderQueried = true;
}

void GraphicsManager::RepositionEntities()
{
	graphicsThreadDetails = "GraphicsManager::RepositionEntities";
	if (optimizationStructure == VFC_OCTREE)
	{
		List<Entity*> dynamicEntities = Physics.GetDynamicEntities();
		for (int i = 0; i < dynamicEntities.Size(); ++i){
			if (!dynamicEntities[i]->registeredForRendering)
				continue;
			vfcOctree->RepositionEntity(dynamicEntities[i]);
		}
	}
}

/// Called before the main rendering loop is begun, after initial GL allocations
void GraphicsManager::OnBeginRendering()
{
    Graphics.enteringMainLoop = true;
	Graphics.deferredRenderingBox->Bufferize(false, false);
};

/// Called after the main rendering loop has ended, before general deallcoations of resources is done.
void GraphicsManager::OnEndRendering()
{
	/// Wait a short while before processing final messages, so that everything is done before exiting.
	Sleep(100);
	/// Process final messages?
	ProcessMessages();

	std::cout<<"\nGraphicsManager::OnEndRendering";
	glDeleteBuffers(1, &deferredRenderingBox->vertexBuffer);
	deferredRenderingBox->vertexBuffer = NULL;

	/// Delete the frame-buffer after we're done with it...
	clock_t timeStart = clock();
	this->DeallocFrameBuffer();
	clock_t	timeTaken = clock() - timeStart;
	std::cout<<"\nDeallocating frame buffer..."<<timeTaken / CLOCKS_PER_SEC<<" seconds";

	/// Delete all shaders
	timeStart = clock();
	ShadeMan.DeleteShaders();
	timeTaken = clock() - timeStart;
	std::cout<<"\nDeleting shaders... "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

	
    /// Deallocate textures before we remove any context!
	timeStart = clock();
	StateMan.DeallocateUserInterfaces();
	timeTaken = clock() - timeStart;
	std::cout<<"\nDeallocating UserInterfaces... "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

};

/// Updates the Projection matrices of the GraphicsState relative to the active camera and ratio to device resolution.
void GraphicsManager::UpdateProjection(float relativeWidth /* = 1.0f */, float relativeHeight /* = 1.0f */)
{
	assert(false && "Do this when you know the viewport to render in");
	/*
	float widthRatio = 1.0f, heightRatio = 1.0f;
	float requestedWidth = relativeWidth;
	float requestedHeight = relativeHeight;
	/// If ratios provided are big numbers, assume they are absolute-values for the window.
	if (requestedWidth > 1 && requestedHeight > 1){

	}
	/// If ratios provided were around 1, multiply by window size
	else {
		requestedWidth = Graphics.Width() * relativeWidth;
		requestedHeight = Graphics.Height() * relativeHeight;
	}
	if (requestedWidth > requestedHeight)
		widthRatio *= (float) requestedWidth / requestedHeight;
	else if (requestedHeight > requestedWidth)
		heightRatio *= (float) requestedHeight / requestedWidth;


	Camera * cameraToTrack = graphicsState->camera;
	/// Update cameras with these ratios too!
	graphicsState->camera->SetRatio(requestedWidth, requestedHeight);

	// Set up projection matrix before sending it in
	graphicsState->projectionMatrixD.InitProjectionMatrix(
		-cameraToTrack->zoom * widthRatio,
		cameraToTrack->zoom * widthRatio,
		-cameraToTrack->zoom * heightRatio,
		cameraToTrack->zoom * heightRatio,
		cameraToTrack->nearPlane,
		cameraToTrack->farPlane
	);
	graphicsState->projectionMatrixF = graphicsState->projectionMatrixD;

	// Load the matrix into GLs built-in projection matrix for testing purposes too.
//	glMatrixMode(GL_PROJECTION);
//	glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
//	if (glGetError() != GL_NO_ERROR)
//		std::cout<<"\nERROR: Unable to load projection matrix to GL";
	// And do the same for the projection matrix! o-o
//	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState->projectionMatrixF.getPointer());
//	if (glGetError() != GL_NO_ERROR)
//		int b = 14;
//	glGetError();
	// Update graphicsState viewFrustum internal variables
	graphicsState->viewFrustum.SetCamInternals(-cameraToTrack->zoom * widthRatio,
		cameraToTrack->zoom * widthRatio,
		-cameraToTrack->zoom * heightRatio,
		cameraToTrack->zoom * heightRatio,
		cameraToTrack->nearPlane,
		cameraToTrack->farPlane);
		*/
}

// Enters a message into the message queue
void GraphicsManager::QueueMessage(GraphicsMessage *msg){
//	graphicsMessageQueueMutex;
	while(!graphicsMessageQueueMutex.Claim(-1))
		;
	messageQueue.Push(msg);
	graphicsMessageQueueMutex.Release();
	return;
}

/** Processes given message without delay, claiming the mutex and blocking all rendering and processing in the thread until the finished. 
	Use with caution! May be used at start-up and shutdown as well as for other scripted events if a certain timing is required.
*/
void GraphicsManager::ProcessMessage(GraphicsMessage * msg)
{
	while(!graphicsMessageQueueMutex.Claim(-1))
		;
	msg->Process();
	delete msg;
	graphicsMessageQueueMutex.Release();
}

// Processes queued messages
void GraphicsManager::ProcessMessages()
{
	graphicsThreadDetails = "GraphicsManager::ProcessMessages";
	// Wait until claimable.
	while(!graphicsMessageQueueMutex.Claim(-1));
	/// Spend only max 10 ms of time processing messages each frame!
	long long messageProcessStartTime = Timer::GetCurrentTimeMs();
	long long now;
	List<GraphicsMessage*> graphicsMessages;
	// Process queued messages
	while (!messageQueue.isOff())
	{
		GraphicsMessage * msg = messageQueue.Pop();
		graphicsMessages.Add(msg);
	}
	graphicsMessageQueueMutex.Release();

	/// Process each message.
	int numMessages = graphicsMessages.Size();
	if (numMessages > 1000)
	{
		std::cout<<"\n"<<numMessages<<" messages to process.";
	}
	// Claim ui Mutex while processing the messages.
	uiMutex.Claim();
	for (int i = 0; i < numMessages; ++i)
	{
		GraphicsMessage * gm = graphicsMessages[i];
		gm->Process();
		/// Only process 10 ms of messages each frame!
		if (i % 1000 == 0 && i > 0)
			std::cout<<"\n"<<numMessages - i<<" of "<<numMessages<<" messages to process in GraphicsManager::ProcessMessages...";
	}
	uiMutex.Release();
	// Clear and delete all at once...
	graphicsMessages.ClearAndDelete();
}

void GraphicsManager::ToggleFullScreen(Window * window)
{
	window->ToggleFullScreen();
}

/// Wooo. Font-handlin'
TextFont * GraphicsManager::GetFont(String byName)
{
	String source = byName;
	if (!source.Contains(".png"))
		source += ".png";
	if (!source.Contains("/"))
		source = "img/fonts/" + source;
	
	for (int i = 0; i < fonts.Size(); ++i)
	{
		TextFont * tf = fonts[i];
		if (tf->source == source)
			return tf;
	}
	/// Try load it.
	TextFont * tf = new TextFont();
	bool result = tf->Load(source);
	if (!result){
		delete tf;
		return NULL;
	}
	fonts.Add(tf);
	return tf;
}

/// Allocates the frame buffer objects
void GraphicsManager::InitFrameBuffer()
{
	assert(false && "This would also have to take into consideration the multiple windows which are being added now. Disabling until further notice. // 2014-06-12, Emil.");
	/*

	/// OpenGL specific data
	/// Frame buffer object for deferred shading
	//GLuint frameBufferObject;	// Main frame buffer object to use
	//GLuint depthBuffer;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
	//GLuint positionTexture;		// World coordinate position texture
	//GLuint diffuseTexture;		// Diffuse texture
	//GLuint depthTexture;		// Depth texture
	//GLuint normalTexture;		// Normal texture
	//
	GLuint error;
	/// Setup main Frame buffer object
	if (!frameBufferObject)
		glGenFramebuffers(1, &frameBufferObject);
	error = glGetError();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	PrintGLError("glBindFramebuffer");
	/// Establish some variables before we try tweaking properties..
	int textureWidth = width;
	int textureHeight = height;
	/// Try oversampling o-o
	bool overSampling = false;
	if (overSampling){
		glEnable( GL_MULTISAMPLE );
		textureWidth = width * 2;
		textureHeight = height * 2;
	}
	else
		glDisable(GL_MULTISAMPLE);
	/// Setup Render buffers
	error = glGetError();
	if (!depthBuffer){
		glGenRenderbuffers(1, &depthBuffer);
		glGenRenderbuffers(1, &diffuseBuffer);
		glGenRenderbuffers(1, &positionBuffer);
		glGenRenderbuffers(1, &normalBuffer);
		glGenRenderbuffers(1, &specularBuffer);
		glGenRenderbuffers(1, &tangentBuffer);
		glGenRenderbuffers(1, &normalMapBuffer);
		glGenRenderbuffers(1, &pickingBuffer);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");
	glBindRenderbuffer(GL_RENDERBUFFER, diffuseBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuseBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");
	glBindRenderbuffer(GL_RENDERBUFFER, normalBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB16F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, normalBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");
	glBindRenderbuffer(GL_RENDERBUFFER, positionBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, positionBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");

	glBindRenderbuffer(GL_RENDERBUFFER, specularBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, specularBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");

	glBindRenderbuffer(GL_RENDERBUFFER, tangentBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_RENDERBUFFER, tangentBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");

	glBindRenderbuffer(GL_RENDERBUFFER, normalMapBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_RENDERBUFFER, normalMapBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("GLError");

	glBindRenderbuffer(GL_RENDERBUFFER, pickingBuffer);
	error = glGetError();
	if (error == 1282){
		std::cout<<"\nGL_ERROR: 1282 GL_INVALID_OPERATION: renderbuffer is not zero or the name of a renderbuffer previously returned from a call to glGenRenderbuffers.";
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F,textureWidth, textureHeight); // Set the render buffer storage to be a depth component, with a width and height of the window
	PrintGLError("glRenderbufferStorage");
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_RENDERBUFFER, pickingBuffer);	// Attach the depth buffer fbo_depth to our frame buffer
	PrintGLError("glFramebufferRenderbuffer");

	glBindRenderbuffer(GL_RENDERBUFFER, 0);	// Bind to 0 when done.
	/// Set up textures
	if (!diffuseTexture)
		glGenTextures(1, &diffuseTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, diffuseTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	if (!normalTexture)
		glGenTextures(1, &normalTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, normalTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!positionTexture)
		glGenTextures(1, &positionTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, positionTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	PrintGLError("Error initializing frame buffer textures");

	if (!depthTexture)
		glGenTextures(1, &depthTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, depthTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureWidth, textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture
	PrintGLError("Error initializing depth texture");

	/// Set up textures
	if (!specularTexture)
		glGenTextures(1, &specularTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, specularTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!tangentTexture)
		glGenTextures(1, &tangentTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, tangentTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!normalMapTexture)
		glGenTextures(1, &normalMapTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, normalMapTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	if (!pickingTexture)
		glGenTextures(1, &pickingTexture); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, pickingTexture); // Bind the texture fbo_texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);	// Unbind the texture

	// Bind framebuffer first?

	// Try bidning color 0 to null.
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

	// Attach textures to be rendered to
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuseTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, positionTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, specularTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tangentTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, normalMapTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, pickingTexture, 0); // Attach the texture to the color buffer in our frame buffer
	PrintGLError("glFramebufferTexture2D");

	frameBufferColorAttachmentsSet = 7;

	/// Attach depth texture too
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depthTexture, 0); // Attach the texture to the color buffer in our frame buffer
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, depthTexture, 0); /// Attach depth texture o.O
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0); /// Attach depth texture o.O
	PrintGLError("GLError");

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	PrintGLError("glCheckFramebufferStatusEXT");

	// Check that frame buffer is okay to work on.
	int result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch(result) {
		case GL_FRAMEBUFFER_COMPLETE: // yay :3
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout<<"\nINFO: Framebuffer incomplete attachment.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout<<"\nINFO: Framebuffer incomplete, missing attachment. Attach an image!";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout<<"\nINFO: Framebuffer incomplete draw buffer.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout<<"\nINFO: Framebuffer incomplete read buffer.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout<<"\nINFO: Framebuffer incomplete multisample.";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout<<"\nINFO: Framebuffer incomplete layer targets.";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout<<"\nINFO: Framebuffer unsupported.";
			break;
		default:
			std::cout<<"\nINFO: Unknown error in framebuffer ...";
			break;
	}
	if (result != GL_FRAMEBUFFER_COMPLETE){
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);
		std::cout<<"\nINFO: FrameBuffer not ready to be used.";
		Sleep(10);
	}

	/// Only have frame buffer parameters in OpenGL 4.3 core and above...
	if (this->GL_VERSION_MAJOR >= 4 && this->GL_VERSION_MINOR >= 3){
		/// Set frame buffer parameters
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 512);
	//	glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4);
		error = glGetError();
	}

	// Unbind our frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	*/
}

/// Cleans up the frame buffer objects
void GraphicsManager::DeallocFrameBuffer(){
	/// OpenGL specific data
	/// Frame buffer object for deferred shading
	if (frameBufferObject)
		glDeleteFramebuffers(1, &frameBufferObject);
	/// Depth buffer
	if (depthBuffer){
		glDeleteBuffers(1, &depthBuffer);
		glDeleteBuffers(1, &normalBuffer);
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &specularBuffer);
		glDeleteBuffers(1, &tangentBuffer);
		glDeleteBuffers(1, &normalMapBuffer);
		glDeleteBuffers(1, &pickingBuffer);
	}

	/// Position
	if (positionTexture){
		glDeleteTextures(1, &positionTexture);
		glDeleteTextures(1, &diffuseTexture);
		glDeleteTextures(1, &depthTexture);
		glDeleteTextures(1, &normalTexture);
		glDeleteTextures(1, &specularTexture);
		glDeleteTextures(1, &tangentTexture);
		glDeleteTextures(1, &normalMapTexture);
		glDeleteTextures(1, &pickingTexture);
	}

	frameBufferObject = 0;
	depthBuffer = 0;
	positionTexture = 0;
}

/// Updates the graphicsState's lighting to include dynamic lights' new positions as well.
void GraphicsManager::UpdateLighting()
{
	graphicsThreadDetails = "GraphicsManager::ProcessMessages";
	if (graphicsState->lighting == NULL)
	{
		graphicsState->lighting = new Lighting();
	}
	// Reload light from the active setup, before dynamic-entity lights are being taken into account.
	*graphicsState->lighting = lighting;

	// Reset the lights in the lighting to be first only the static ones.
	for (int i = 0; i < graphicsState->dynamicLights.Size(); ++i)
	{
		Light * light = graphicsState->dynamicLights[i];
		// Update it's position relative to the entity.. important!
		Entity * owner = light->owner;
		if (!owner)
			continue;

		assert(owner);
		light->spotDirection = owner->rotationMatrix.Product(light->relativeSpotDirection);
		light->position = owner->transformationMatrix.Product(light->relativePosition);
		
		// Add the dynamic light to be rendered.
		graphicsState->lighting->Add(light);
	}
}


#include "Particles/ParticleSystem.h"

void GraphicsManager::Process()
{
	if (pauseProcessing)
		return;
	graphicsThreadDetails = "GraphicsManager::Process";
	static Time lastTime = Time::Now();
	Time now = Time::Now();

	int milliseconds = (now - lastTime).Milliseconds();
	// Max 200 milliseconds per batch, 5 fps is needed, yo.
	milliseconds = milliseconds % 200;
	lastTime = now;

	/// Process particle effects.
	for (int i = 0; i < particleSystems.Size(); ++i)
	{
		ParticleSystem * ps = particleSystems[i];
		ps->Process(milliseconds * 0.001f);
	}
	/// Process entity specific controls and systems
	for (int i = 0; i < registeredEntities.Size(); ++i)
	{
		Entity * entity = registeredEntities[i];
		entity->graphics->Process(milliseconds);
		// If not registered for physics, update transformation matrix now (since parents may be moving, etc.)
		if (!entity->registeredForPhysics)
			entity->RecalculateMatrix();
	}
}


/// Lists active camera data in the console.
void GraphicsManager::ListCameras(){
	const Camera * c = ActiveCamera();
	if (c)
		c->PrintData();
}


/// Returns active map being rendered.
TileMap2D * GraphicsManager::ActiveTileMap2D(){
	return this->mapToRender;
}
