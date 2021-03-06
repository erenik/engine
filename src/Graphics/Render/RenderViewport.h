#ifndef RENDER_VIEWPORT_H
#define RENDER_VIEWPORT_H

#include "Viewport.h"
#include "MathLib.h"
//#include "PhysicsLib.h"

class GraphicsState;
class Camera;
class UserInterface;

/** A struct to properly divide AppWindow screen-space.
	Can include settings/options that act on a per-viewport basis.
*/
class RenderViewport : public Viewport 
{
	friend class GraphicsManager;
	friend class GMSetViewports;
public:
	/// Default constructor, pass optional gui source to attach a gui to it on creation.
	RenderViewport(String uiSource);
	RenderViewport(int leftX, int bottomY, int width, int height);
	virtual ~RenderViewport();

	/// Sets the viewport to use relative coordinates or absolute.
	void SetRelative(float leftX, float bottomY, float width, float height);
	void SetCameraToTrack(Camera * camera);
	void AdjustToWindow(int width, int height);
	/// Per-viewport render
	void Render();
	UserInterface * GetUI() { return viewPortUI; };
	/// For distinguation and later alteration
	String viewportName;
private:
	/** If the provided sizes to x/y/width/height should be relative to AppWindow size (instead of absolute).
		This may require more updates to keep correct!
	*/
	bool relative;
	// Set initial default/NULL-values.
	void Initialize(); 

	// Camera to render from each frame
	Camera * camera;
	// UI specific to this viewport! 
	UserInterface * viewPortUI;
	String uiSource;
	int renderOptions;

	/// Absolute-values of the AppWindow.
	Vector2i absMin, absMax;
};

#endif
