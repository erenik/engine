/// Emil Hedemalm
/// 2021-05-21
/// Grouping of all textures, models, and states for visuals for UI elements.

#pragma once

#include "MathLib.h"
#include "Graphics/OpenGL.h"

class Square;
class Texture;
class GraphicsState;
struct UILayout;

struct UIVisuals {

	UIVisuals();
	~UIVisuals();

// FUNCS
	void Nullify();

	/// Recalculates and sets highlighting factors used when rendering the UI (dedicated shader settings)
	void UpdateHighlightColor(bool disabled, bool isActive, bool isHover);

	/** Fetches texture, assuming the textureSource has been set already. Binds and bufferizes, so call only from graphics thread.
	Returns false if no texture could be find, bind or bufferized. */
	bool FetchBindAndBufferizeTexture();
	/// Bufferizes the UIElement mesh into the vbo buffer
	void Bufferize();

	void FreeBuffers(GraphicsState& graphicsState);

	virtual void CreateGeometry(GraphicsState* graphicsState, const UILayout& layout);
	virtual void ResizeGeometry(GraphicsState* graphicsState, const UILayout& layout);
	void DeleteGeometry();

	/// Splitting up the rendering.
	void Render(GraphicsState & graphicsState);

	bool IsBuffered() const { return isBuffered; }
	bool IsGeometryCreated() const { return isGeometryCreated; }

// VARS 
	/// Colors for the element.
	Vector4f color;

	bool highlightOnHover;				// Toggles the UI-highlighting for currently hovered UI-elements.
	bool highlightOnActive;				// Toggles if the element should highlight when clicking or active. Default is True.

	// When clicking on it.
	static float onActiveHightlightFactor;
	// When hovering on it.
	static float onHoverHighlightFactor;
	// When not hovering, not clicking it.
	static float onIdleHighlightFactor;
	// If toggled (checkbox, radiobutton), additional highlight-factor
	static float onToggledHighlightFactor;

	String textureSource;	// Name of the texture the UIelement will use
	static String defaultTextureSource;
	// If true, will reduce width or height dynamically so that the texture's aspect ratio is retained when rendering.
	bool retainAspectRatioOfTexture = false;

	// Graphical members, relevant to buffering/rendering
	Square * mesh;		// Mesh Entity for this element
	Texture * texture;	// Texture to be mapped to the mesh
	GLuint vboBuffer;	// GL Vertex Buffer ID
	int vboVertexCount;	// Vertex buffer Entity vertex count

private:
	/// GL specific stuff
	GLuint vertexArray;
	/// Similar to UI, this checks if this particular element is buffered or not.
	bool isBuffered = false;
	bool isGeometryCreated = false;


};
