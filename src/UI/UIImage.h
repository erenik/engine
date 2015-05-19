/// Emil Hedemalm
/// 2014-02-25
/// UI class to show images, often with restrictions if they should scale, tile, etc.

#ifndef UI_IMAGE_H
#define UI_IMAGE_H

#include "UIElement.h"

class UIImage : public UIElement {
public:
	UIImage(String nameAndTextureSource);
	UIImage(String name, String textureSource);
	virtual ~UIImage();
	/// Subclassing in order to control rendering.
	virtual void RenderSelf(GraphicsState & graphicsState);
	/// Subclassed in order to change geometry based on image-dimensions (in addition to element size) and stretch conditions.
	virtual void CreateGeometry();
	virtual void ResizeGeometry();

	/// Called after FetchBindAndBufferizeTexture is called successfully. (may also be called other times). Only called from the graphics thread.
	virtual void OnTextureUpdated();

	Texture * GetTexture();
	String GetTextureSource();
	void SetTextureSource(String src);
private:
	// If true, navigation by zooming via scrolling and maybe other gestures will be enabled.
	bool navigationEnabled;
	// For painting over them.. not enabled by default.
	bool editable;

};

#endif
