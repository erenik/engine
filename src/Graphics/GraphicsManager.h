/// Emil Hedemalm
/// 2013-03-01

#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "OS/OS.h"
#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "Mutex/Mutex.h"
#include "Queue/Queue.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "ShaderManager.h"
#include "VFCOctree.h"
#include "System/DataTypes.h"

struct RenderSettings;
class Texture;
class UserInterface;
class TextFont;
class Viewport;
class TileMap2D;
class Camera;
class UserInterface;
class ParticleSystem;
class RenderRay;
class Window;

#define MAX_TEXTURES	250

#define Graphics	(*GraphicsManager::Instance())


// Defines a class that handles textures, rendering and it's contexts and settings
class GraphicsManager {
	friend class GraphicsMessage;
	friend class GMRegisterEntity;
	friend class GMRegisterEntities;
	friend class GMUnregisterEntity;
	friend class GMUnregisterEntities;
	friend class GMSetLighting;
	friend class GMSet;
	friend class GMSetUI;
	friend class GMSets;
	friend class GMSetf;
	friend class GMSetUIb;
	friend class GMSetUIs;
	friend class GMSetOverlay;
	friend class GMSetViewports;
	friend class Viewport;
	friend class TileMap2D;
	friend class GMRegister;
	friend class GMClear;
	friend class GMRender;
	friend class GMAttachLight;
//	friend class ;
private:
	/// Constructor which anulls all relevant variables.
	GraphicsManager();
	static GraphicsManager * graphicsManager;
public:
	static void Allocate();
	static GraphicsManager * Instance();
	static void Deallocate();
	~GraphicsManager();

	/// Loads all settings from files and OS
	void Initialize();
	/// Loads and compiles all relevant shaders
	void CreateShaders();
	/// Sets screen resolution in pixels. Returns false if the query is not valid.
	bool SetResolution(int width, int height);
	/// Sets up GL for rendering with the current context.
//	void InitializeGL();

	// For toggling all debug renders.
	void EnableAllDebugRenders(bool enabled = true);

	/// Returns a pointer to active GUI
//	UserInterface * GetUI() { return ui; };
	/** Returns a pointer to the active system-global UI. If it has not been created earlier it will be created upon calling it.
		If argument is true and no global has been constructed, it will be constructed.
	*/
//	UserInterface * GetGlobalUI(bool createIfNeeded = false);
//	List<Viewport*> GetViewports() { return renderViewports; };
	/// Sets active UserInterface to be rendered
	void SetUI(UserInterface * ui);
	/// Sets system-global ui.
	void SetGlobalUI(UserInterface * ui);

	void RepositionEntities();

    /// Returns the active camera for the given viewport
	Camera * ActiveCamera(int viewport = 0) const;
	/// Returns the active lighting. 
	Lighting * ActiveLighting();

	/// For updating graphical effects before rendering takes place.
	void Process();

	/// Pauses rendering and returns only after it has paused fully.
	void PauseRendering();
	/// Reumes the rendering immediately.
	void ResumeRendering();

	/// Main graphics manager processing thread
#ifdef WINDOWS
	static void Processor(void * vArgs);
#elif defined LINUX | defined OSX
	static void * Processor(void * vArgs);
#endif

	/// Lists active camera data in the console.
	void ListCameras();

	/// Updates the Projection matrices of the GraphicsState relative to the active camera and ratio to device resolution.
	void UpdateProjection(float relativeWidth = 1.0f, float relativeHeight = 1.0f);

	// Enters a message into the message queue
	void QueueMessage(GraphicsMessage * msg);
	// Processes queued messages
	void ProcessMessages();


	void ToggleFullScreen(Window * forWindow);
	

//	int DeviceWidth() const { return scrWidth; };
//	int DeviceHeight() const { return scrHeight; };

	/// Called before the main rendering loop is begun, after initial GL allocations
	void OnBeginRendering();
	/// Called after the main rendering loop has ended, before general deallcoations of resources is done.
	void OnEndRendering();

	bool rendering;
	/// For main rendering.
	bool backfaceCullingEnabled;

	/// Booleans for extra rendering tools
	bool renderPhysics;
	bool renderCollissions;
	bool renderCollissionTriangles;
	bool renderSeparatingAxes;
	bool renderGrid;
	bool renderFPS;
	bool renderAI;
	bool renderNavMesh;
	bool renderUI;
	bool renderLights; // For 3D-representations of the light-sources!
	bool renderMap; // For le 2D-map-crap

	/// Queries render of 1 frame by posting a GMRender message to the graphics message queue. Should be used instead of setting the boolean below straight away!
	void QueryRender();
	/// To disable real-time rendering. If true will only render once the renderQueried variable is set to true, which will reset after each successful render. Default is false.
	bool renderOnQuery;
	/// Set this to true to render 1 frame. Resets to false after each successful render.
	bool renderQueried;

	/// Set default values to the sleep-times.
	void ResetSleepTimes();
	/// Sleep-times for the render-thread.
	int sleepTime;
	int outOfFocusSleepTime;

	// Entity render stuff
	bool renderLookAtVectors;

	/// Current full-screen flag
	bool isFullScreen;
	/// Sizes pre-fullscreen
	int oldWidth;
	int oldHeight;
	/// Bool for the main graphicsManager loop
	bool shouldLive;
	bool enteringMainLoop; // Flagged once entering main render-loop!
	bool finished; // For deallocation readiness!
	/// Bool for if we should full-screen upon startup
	bool shouldFullScreen;
	bool useDeferred;

	/// Manager for all shader programs
	ShaderManager shadeMan;

	/// Keeps track of GL version for the client
	int GL_VERSION_MAJOR;
	int GL_VERSION_MINOR;

	/// Camera to track for changes/updates.
	Camera * cameraToTrack;
	Camera * defaultCamera;
	/// Selection to track for changes/updates and rendering.
	Selection * selectionToRender;

	const long FrameTime() const { return frameTime; };
	const long RenderFrameTime() const { return renderFrameTime; };
	const long PhysicsFrameTime() const { return physicsFrameTime; };
    float renderViewportsFrameTime;
    float renderViewportFrameTime[4];
    int64 graphicsMessageProcessingFrameTime,
        graphicsUpdatingFrameTime,
        preRenderFrameTime,
        swapBufferFrameTime,
        postViewportFrameTime;
	/// For checking .
	bool RenderingEnabled() const { return renderingEnabled; };

	/// CBA friending all message-functions...
	RenderSettings * renderSettings;


	/** Sets selected shader to be active. Prints out error information and does not set to new shader if it fails.
		Returns the active shading program or NULL if it fails.
		WARNING: Should only be called from a render-thread, yaow.
	*/
	Shader * SetShaderProgram(const char * shaderName);

	/// Wooo. Font-handlin'
	TextFont * GetFont(String byName);
	/// Returns active map being rendered.
	TileMap2D * ActiveTileMap2D();
private:
    /// Wosh. For like Frustum and other custom stuff.
    List<Renderable*> renderShapes;
	/// List of all particle systems that will be updated each frame.
	List<ParticleSystem*> particleSystems;

	/// For deferred.
	void BindFrameBuffer();
	/// Process the lighting and render the quad.
	void RenderFrameBuffer();

	/// Should definitely be private..
	bool renderingEnabled;

	/// Time statistics
	long renderFrameTime, physicsFrameTime, frameTime;

	/// Mutex to be used for accessing the message queue.
	Mutex graphicsMessageQueueMutex;

	/// Sets overlay texture to be rendered on top of everything else
	void SetOverlayTexture(Texture * t, int fadeInTime = 0);
	void SetOverlayTexture(String source, int fadeInTime = 0);


	/// Box to be used for rendering deferred shading onto the window, placed here for eased usage.
	Square * deferredRenderingBox;

	/// Active camera to be used when rendering the 3D-scenes!
//	Camera camera;

	/// Default lighting if the current map lacks one
	Lighting defaultLighting;
	/// Current lighting, which is editable. Will be copied to the graphicsState for rendering together with all dynamic lights.
	Lighting lighting;

	/// Handle them here in the graphics manager.
	List<TextFont*> fonts;

	/// List of viewports to render to.
//	List<Viewport *> renderViewports;
//	Viewport * defaultViewPort;

    /// Clickrays to render
	List<RenderRay*> rays;


	/// To properly render to each window.
	void RenderWindows();
	void RenderWindow();
	/// Renders contents in target viewport.
	void RenderViewport(Viewport * vp);
	/** Renders additional data pertaining to the AI.
		Settings for these should be placed somewhere well thought out!
	*/
	void RenderAI();
	/// Renders target Navigation-meshes used for pathfinding
	void RenderNavMesh();
	/// Renders target paths
	void RenderPath();
	/// Renders physical bounds for all entities within the frustum. Does a local frustum culling that will slow down the system more ^^
	void RenderPhysics();
	/// Renders a bar for FPS, with marks at 0, 15, 30 and 60 fps.
	void RenderFPS();
	/// Renders a simple grid with 10x10 grids, each 10x10 big, as well as an RGB XYZ triangle for orientation.
	void RenderGrid();
	// Renders the scene normally using the active camera using frustum culling. Also queues up alpha-entities and effects for later rendering.
	void RenderScene();
	/// Sorts and renders the alpha-entities that were queued up earlier.
	void RenderAlphaEntities();
	/// Renders nicelish graphical effectslies! :#
	void RenderEffects();
	/// Renders wireframe's around all entities that are currently selected, without caring about depth for clarity.
	void RenderSelection();
	/// Renders the provided UI.
	void RenderUI(UserInterface * ui);
	/// Renders overlay pictures or videos for loading screens, cinematics, etc.
	void RenderOverlay();
	/// Renders 3D-representations of all light-sources.
	void RenderLights();
	/// For rendering LookAt's, velocities? et al.
	void RenderEntityVectors();
	/// Renders all generic simple shapes
	void RenderShapes();
	/// Capaturing rendered contents into file or buffers. (e.g. print-screen)
	void RenderCapture();
	/// Renders target texture to the screen.
	void RenderFullScreen(Texture * tex, float alpha = 1.0f);

	/// Adds an Entity to be rendered to the vfcOctree.
	bool RegisterEntity(Entity * entity);
	/// Registers all entities in the selection for rendering. Returns the number of faield registrations.
	int RegisterEntities(List<Entity*> & toRegister);
	/// Removes an Entity from the rendering vfcOctree.
	bool UnregisterEntity(Entity * entity);
	/// Unregisters all entities in the selection from rendering. Returns the number of failed unregistrations.
	int UnregisterEntities(List<Entity*> & toUnregister);
	/// Unregisters all entities possible from rendering.
	int UnregisterAll();

	/// Yup.
	bool RegisterParticleSystem(ParticleSystem * ps);

	/// Screen size
	int scrWidth, scrHeight;


	/// Overlay texture to be displayed at start-up and eventual splash-screens later on.
	Texture * overlayTexture, * queuedOverlayTexture;
	int overlayFadeInTime;
	long long overlayFadeInStart;

	TileMap2D * mapToRender;

	/// Updates the graphicsState's lighting to include dynamic lights' new positions as well.
	void UpdateLighting();

	// Main state for rendering
//	GraphicsState * graphicsState;
	// Octree vfcOctree
	VFCOctree * vfcOctree;
	// Main frustum pointer for VFC. This should be updated to correspond to the active camera at all times.
	Frustum * frustum;

	// Queue for messages to be processed between renders
	Queue<GraphicsMessage*> messageQueue;

	/// Number of registered entities
	List<Entity*> registeredEntities;

	/// Allocates the frame buffer objects
	void InitFrameBuffer();
	/// Initializes a texture to be used with the frame buffer
	void InitFrameBufferTexture(GLuint &texture);
	/// Cleans up the frame buffer objects
	void DeallocFrameBuffer();

	/// OpenGL specific data
	/// Frame buffer object for deferred shading
	GLuint frameBufferObject;	// Main frame buffer object to use

	/// Buffers
	GLuint depthBuffer;			// Depth buffer to act as Z-buffer for when parsing the input to the frame buffer
	GLuint diffuseBuffer;
	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint specularBuffer;
	GLuint tangentBuffer;
	GLuint normalMapBuffer;
	GLuint pickingBuffer;

	/// Texture
	GLuint diffuseTexture;		// Diffuse texture
	GLuint depthTexture;		// Depth texture
	GLuint normalTexture;		// Normal texture
	GLuint positionTexture;		// World coordinate position texture
	GLuint specularTexture;
	GLuint tangentTexture;
	GLuint normalMapTexture;
	GLuint pickingTexture;

	int frameBufferColorAttachmentsSet;

	/// Boolean for handling framebuffer functions
	bool support_framebuffer_via_ext;
};

/// Prints the values of the error code in decimal as well as hex and the literal meaning of it.
void PrintGLError(const char * text);

#endif
