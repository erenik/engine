/// Emil Hedemalm
/// 2014-06-12
/// Merge of the previously divided Viewport and RenderViewport classes.

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "MathLib.h"
#include "PhysicsLib.h"
#include "Graphics/OpenGL.h"

class FrameBuffer;
class Camera;
class UserInterface;
class Window;

/** A struct to properly divide window screen-space.
	Can include settings/options that act on a per-viewport basis.
*/
class Viewport 
{
public:
	/// Default constructor, pass optional gui source to attach a gui to it on creation.
	Viewport();
	Viewport(String uiSource);
	Viewport(Vector2i bottomLeftCorner, Vector2i size);
	virtual ~Viewport();

	/// Unique ID for this viewport.
	int ID();

	/// Sets the viewport to use relative coordinates.
	void SetRelative(Vector2f bottomLeftCorner, Vector2f size);
	void SetCameraToTrack(Camera * camera);
	/// Update size based on window it resides in.
	void UpdateSize();
	UserInterface * GetUI();

	/// Sets up frame-buffers for this viewport, resizing them as needed. Creates frame buffer if needed. Returns false if something failed along the way.
	bool BindFrameBuffer();
	/// Creates a default set of ish 8 render buffers joint to a FrameBuffer object.
	void CreateFrameBuffer();

	FrameBuffer * frameBuffer;

	/// For distinguation and later alteration
	String name;
	/// Parent window it resides in.
	Window * window;



	/** If the provided sizes to x/y/width/height should be relative to window size (instead of absolute).
		This may require more updates to keep correct!
	*/
	bool relative;
	// Set initial default/NULL-values.
	void Initialize(); 

	// Camera to render from each frame
	Camera * camera;
	// UI specific to this viewport! 
	UserInterface * ui;
	String uiSource;
	int renderOptions;

	/// Absolute-values of the window... meaning?
	Vector2i absMin, absMax;



	Vector4f backgroundColor;
	
	// Size details
	Vector2i bottomLeftCorner, size;
	Vector2f relativeOffset;
	Vector2f relativeSize;

	// For toggling all debug renders.
	void EnableAllDebugRenders(bool enabled = true);

	/// Flag for this viewport.
	bool renderNavMesh;

	/// Dragged from GraphicsManager
	/// Booleans for extra rendering tools
	bool renderPhysics;
	bool renderCollisions;
	bool renderCollisionTriangles;
	bool renderSeparatingAxes;
	bool renderGrid;
	bool renderFPS;
	bool renderAI;
	bool renderUI;
	bool renderLights; // For 3D-representations of the light-sources!
	bool renderMap; // For le 2D-map-crap
	bool renderLookAtVectors;

protected:
	
	/// Unique ID
	static int idEnumerator;
	int id;
};

/// Screen-to-world space functions, defined by input variables.
/** Returns a ray in 3D space using the given mouse and camera data.
	Mouse coordinates are assumed to be in screen-pixel space (i.e. 0.0 to 800.0 or similar)
*/
Ray GetRayFromScreenCoordinates(int mouseX, int mouseY, Viewport& viewport);


#endif
