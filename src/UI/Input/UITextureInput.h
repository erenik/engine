/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#ifndef UI_TEXTURE_INPUT_H
#define UI_TEXTURE_INPUT_H

#include "UI/UIElement.h"

class UIImage;

class UITextureInput : public UIElement {
public:
	UITextureInput(String name, String onTrigger);
	virtual ~UITextureInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	virtual bool HandleDADFiles(List<String> files) override;


	/// Creates the label and input.
	void CreateChildren(GraphicsState* graphicsState) override;
	/// Getter/setter for the input element.
	Texture * GetTexture();
	String GetTextureSource();
	
	/// To be called only from the RenderThread! Use GMSetUIs with TEXTURE_INPUT_SOURCE target!
	void SetTextureSource(String source);
	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	UIInput * input;
	UIImage * uiImage;
private:
};


#endif
