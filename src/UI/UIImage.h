/// Emil Hedemalm
/// 2014-02-25
/// UI class to show images, often with restrictions if they should scale, tile, etc.

#include "UIElement.h"

class UIImage : public UIElement {
public:
	UIImage(String nameAndTextureSource);
	UIImage(String name, String textureSource);
	virtual ~UIImage();
	/// Subclassing in order to control rendering.
	virtual void RenderSelf();

	Texture * GetTexture();
	String GetTextureSource();
	void SetTextureSource(String src);
private:

};
