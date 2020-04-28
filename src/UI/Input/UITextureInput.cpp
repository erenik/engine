/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UITextureInput.h"
#include "UI/UITypes.h"
#include "UI/UIImage.h"
#include "UI/UILists.h"
#include "UIInput.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

UITextureInput::UITextureInput(String name, String onTrigger)
{
	type = UIType::TEXTURE_INPUT;
	this->name = name;
	action = onTrigger;
	label = NULL;
	input = NULL;
	uiImage = NULL;
}
UITextureInput::~UITextureInput()
{

}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UITextureInput::OnInputUpdated(UIInput * inputElement, GraphicsState& graphicsState)
{
	Graphics.QueueMessage(new GMSetUIs(uiImage->name, GMUI::TEXTURE_SOURCE, inputElement->text));
	// Generate a message to send to the game-state too.
	MesMan.QueueMessage(new TextureMessage(action, inputElement->text));
	return;
}

/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
	If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
*/
bool UITextureInput::HandleDADFiles(List<String> files){
	if (!files.Size())
		return false;
	String file = files[0];
	SetTextureSource(file);
	MesMan.QueueMessage(new TextureMessage(action, uiImage->GetTextureSource()));
	return true;
}



/// Creates the label and input.
void UITextureInput::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(nullptr, box);

	int elements = 1 + 2;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = 0.3f;
	box->AddChild(nullptr, label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->text = "";
	input->sizeRatioX = 0.55f;
//	input->onTrigger = "UIStringInput("+name+")";
	box->AddChild(nullptr, input);

	uiImage = new UIImage("Black");
	uiImage->sizeRatioX = 0.1f;
	box->AddChild(nullptr, uiImage);
	childrenCreated = true;
}

/// Getter/setter for the input element.
Texture * UITextureInput::GetTexture()
{
	uiImage->GetTexture();
	return NULL;
}
String UITextureInput::GetTextureSource()
{
	return input->text;
}
/// To be called only from the RenderThread! Use GMSetUIs with TEXTURE_INPUT_SOURCE target!
void UITextureInput::SetTextureSource(String source)
{
	input->SetText(source, true);
	uiImage->SetTextureSource(source);
}

