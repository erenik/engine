/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UITypes.h"
#include "UIInput.h"
#include "UIList.h"
#include "UIImage.h"
#include "Input/Keys.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

UIInput::UIInput(String name /*= ""*/)
: UIElement()
{
	this->name = name;
	type = UIType::INPUT_FIELD;
	selectable = hoverable = activateable = true;
	activationMessage = "BEGIN_INPUT(this)";
	this->textureSource = "80gray.png";
	textColor = defaultTextColor;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = true;
	caretPosition = 0;
	inputActive = false;
	numbersOnly = false;
}

UIInput::~UIInput()
{
//	std::cout<<"\nUIInput destructor";
}

// When clicking/Enter pressed on keyboard.
UIElement* UIInput::Click(float & mouseX, float & mouseY){
	UIElement * e = UIElement::Click(mouseX,mouseY);
	if (e == this){
		BeginInput();
	}
	return e;
}
// When button is released.
UIElement* UIInput::Activate(){
	// Skip this.
	return NULL;
}

/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
void UIInput::OnExitScope()
{
	/// Call it for children too.
	UIElement::OnExitScope();
	/// stop input here then!
	inputActive = false;
	this->RemoveState(UIState::ACTIVE);
	/// Remove caret
	editText.caretPosition = -1;
}

/// Used by input-captuing elements. Should not be called for any base UI elements(?)
int UIInput::OnKeyDown(int keyCode, bool downBefore)
{
	if (!inputActive)
		return 0;

	switch(keyCode){
		case KEY::BACKSPACE: {
	#ifndef WINDOWS
			// Double trigger at the moment..
			OnBackspace();
	#endif
			break;
		}
		case KEY::ESCAPE:{
			std::cout<<"\nCanceling input.";
			editText = previousText;
			// Make inactive.
			StopInput();
			break;
		}
		case KEY::ENTER: {
			// Don't evaluate Enter and certain other keys if they were down before
			if (downBefore)
				return 0;
			StopInput();
			// Activate the messages this element had.
			MesMan.QueueMessages(onTrigger, this);
			/// Notify of the update to self and then parents, so that extra actions may be taken.
			this->OnInputUpdated(this);
	/*
			OnStopActiveInput();

			if (wcslen(inputBuffers[selectedInputBuffer]) == 0)
				return;
			// Copy to old buffer ^^
			wcscpy(inputBuffers[0], inputBuffers[selectedInputBuffer]);
			// Move back previous buffers first! And reset the most recent used old buffer to 0
			for (int i = INPUT_BUFFERS - 1; i > 0; --i){
				wcscpy(inputBuffers[i], inputBuffers[i-1]);
			}
			memset(inputBuffers[0], 0, sizeof(wchar_t) * BUFFER_SIZE);
			std::cout<<"\nExiting text-entering mode.";
		//	std::cout<<"\nSending input to state input-binding processor.";
	*/
			break;
		}
		// Delete
		case KEY::DELETE_KEY:
		{
			String left = editText.Part(0, caretPosition);
			String right = editText.Part(caretPosition+1);
			editText = left + right;
			// Update the text to render.
			editText.caretPosition = caretPosition;
			break;
		}
		case KEY::END:
			caretPosition = editText.Length();
			editText.caretPosition = caretPosition;
			break;
		case KEY::HOME:
			caretPosition = 0;
			editText.caretPosition = caretPosition;
			break;
		case KEY::UP: {
	/*		/// Get one of the old previous buffers ^^
			selectedInputBuffer++;
			if (selectedInputBuffer > INPUT_BUFFERS)
				selectedInputBuffer = INPUT_BUFFERS;
			int length = wcslen(inputBuffers[selectedInputBuffer]);
			if (length > 0){
			//	_tcscpy(inputBuffers[0], inputBuffers[selectedInputBuffer-1]);
				caretPosition = length;
			}
			OnTextInputUpdated();
			PrintBuffer();
	*/
		}
		case KEY::DOWN: {
	/*		/// Get one of the old previous buffers ^^
			selectedInputBuffer--;
			if (selectedInputBuffer < 0)
				selectedInputBuffer = 0;

			int length = wcslen(inputBuffers[selectedInputBuffer]);
			if (length > 0){
			//	_tcscpy(inputBuffers[0], inputBuffers[selectedInputBuffer-1]);
				caretPosition = length;
			}
			OnTextInputUpdated();
			PrintBuffer();
			return;
	*/	}
		case KEY::LEFT:
			if (caretPosition > 0)
				--caretPosition;
			// Update the text to render.
			editText.caretPosition = caretPosition;
			break;
		case KEY::RIGHT:
			if (caretPosition < editText.Length())
				++caretPosition;
			// Update the text to render.
			editText.caretPosition = caretPosition;
			break;
	}
	OnTextUpdated();

	// Return from here no matter what now, since we don't want any hot-key
	// whatsoever to be triggered while entering any input!
	return 0;
}

/// Used for getting text. This will be local translated language key codes?
int UIInput::OnChar(int asciiCode)
{
	if (!this->inputActive)
		return 0;
	/// Make sure the buffer period has passed ^^
/*	clock_t currentTime = clock();
	if (currentTime < textInputStartTime + 10)	/// 100 ms delay before input can be done o-o
		return;
*/

	// NOTE: Caret is visualized as being right behind the new letter, or right after the letter being added.
	// So caret 3 in the word Ashwood would be:  Ash|wood
	// Backspace
	if (asciiCode == 0x08){
	    OnBackspace();
		return 0;
	}

	// Escape, cancel input
	else if (asciiCode == 0x1B)
	{
		// And restore old string!
		editText = previousText;
		StopInput();
		return 0;
	}
	else {
#define _DEBUG_ASCII
		/// Ignore crap letters
		switch(asciiCode){
        case 0:
		case 4:		// End of transmission, not the same as ETB
		case 13:	// Vertical tab, whatever that is
		case 19:	// XOFF, with XON is TERM=18 flow control
#ifdef _DEBUG_ASCII
	//		std::cout<<"\nSkipping crap letters.. :3";
#endif
			return 0;
		default:
#ifdef _DEBUG_ASCII
			std::cout<<"\nAsciiCode: "<<(int)asciiCode<<" "<<asciiCode;
#endif
			break;
		}
		/// If only accept numbers, skip all except a few ascii..
		if (numbersOnly){
			switch(asciiCode){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '.':
				case '-':
					break;
				default:
					return 0;
			}
		}

		String firstHalf = editText.Part(0, caretPosition);
		String secondHalf = editText.Part(caretPosition);
		editText = firstHalf + (char)asciiCode + secondHalf;
		// inputBuffers[selectedInputBuffer][caretPosition] = asciiCode;
		++caretPosition;
		editText.caretPosition = caretPosition;
		OnTextUpdated();
	}
	return 0;
}


/// For handling text-input
void UIInput::OnBackspace(){
    if (caretPosition > 0){
        --caretPosition;	// Move back caret
        // ...and move back the rest one step
		String first = editText.Part(0, caretPosition);
		String second = editText.Part(caretPosition+1);
		editText = first + second;
		// Update the text to render.
		editText.caretPosition = caretPosition;
        OnTextUpdated();
    }
}


/// Begins input! >)
void UIInput::BeginInput(){
	inputActive = true;
	// Set active state if not done so already.
	this->AddState(UIState::ACTIVE);
	editText = text;
	caretPosition = editText.Length();
	editText.caretPosition = caretPosition;
	// sends message to update the ui with new caret and stuff.
	OnTextUpdated();
	previousText = editText;
}

/// Halts input and removes Active state.
void UIInput::StopInput()
{
	inputActive = false;
	this->RemoveState(UIState::ACTIVE);
	/// Remove caret
	editText.caretPosition = -1;
	OnTextUpdated();
}

// sends message to update the ui with new caret and stuff.
void UIInput::OnTextUpdated()
{
	if (!ui)
		ui = this->GetRoot()->ui;
	assert(ui);
	Graphics.QueueMessage(new GMSetUIs(name, GMUI::TEXT, editText, true, this->ui));
}



UIStringInput::UIStringInput(String name, String onTrigger)
: UIElement(),action(onTrigger)
{
	this->type = UIType::STRING_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
}
UIStringInput::~UIStringInput()
{
}
/// Creates the label and input.
void UIStringInput::CreateChildren()
{
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = 1 + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = 0.3f;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->text = "";
	input->sizeRatioX = 0.65f;
	input->onTrigger = "UIStringInput("+name+")";
	box->AddChild(input);
}
/// Getter/setter for the input element.
String UIStringInput::GetValue()
{
	return input->text;
}
void UIStringInput::SetValue(String value)
{
	Graphics.QueueMessage(new GMSetUIs(input->name, GMUI::TEXT, value));
}


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
void UITextureInput::OnInputUpdated(UIInput * inputElement)
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
void UITextureInput::CreateChildren()
{
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = 1 + 2;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = 0.3f;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->text = "";
	input->sizeRatioX = 0.55f;
	input->onTrigger = "UIStringInput("+name+")";
	box->AddChild(input);

	uiImage = new UIImage("Black");
	uiImage->sizeRatioX = 0.1f;
	box->AddChild(uiImage);
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

/// Class for 1 float inputs.
UIFloatInput::UIFloatInput(String name, String onTrigger)
: UIElement(), action(onTrigger)
{
	this->type = UIType::FLOAT_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
	maxDecimals = -1;
}
UIFloatInput::~UIFloatInput(){
	// Nothing special, let base class handle children.
}
/// Creates the label and input.
void UIFloatInput::CreateChildren(){
/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = 1 + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = spacePerElement;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->numbersOnly = true;
	input->text = "0";
	input->sizeRatioX = spacePerElement;
	input->onTrigger = "UIFloatInput("+name+")";
	box->AddChild(input);
}

/// Getter/setter for the input element.
float UIFloatInput::GetValue(){
	return input->text.ParseFloat();
}
void UIFloatInput::SetValue(float value){
	input->SetText(String::ToString(value, maxDecimals));
}



/// Class for 1 Integer inputs.
UIIntegerInput::UIIntegerInput(String name, String onTrigger)
: UIElement(), action(onTrigger)
{
	this->type = UIType::INTEGER_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
}
UIIntegerInput::~UIIntegerInput()
{
	// Nothing special, let base class handle children.
}
/// Creates the label and input.
void UIIntegerInput::CreateChildren()
{
/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = 1 + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = spacePerElement;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->numbersOnly = true;
	input->text = "0";
	input->sizeRatioX = spacePerElement;
	input->onTrigger = "UIIntegerInput("+name+")";
	box->AddChild(input);
}

/// Getter/setter for the input element.
int UIIntegerInput::GetValue(){
	return input->text.ParseInt();
}
void UIIntegerInput::SetValue(int value){
	input->SetText(String::ToString(value));
}


///
UIVectorInput::UIVectorInput(int numInputs, String name, String onTrigger)
: UIElement()
{
	type = UIType::VECTOR_INPUT;
	this->numInputs = numInputs;
	this->name = name;
	this->action = onTrigger;
	maxDecimals = 3;
}
UIVectorInput::~UIVectorInput()
{

}
/// Creates ze children!
void UIVectorInput::CreateChildren()
{
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = numInputs + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = spacePerElement;
	box->AddChild(label);

	/// Create 3 children
	for (int i = 0; i < numInputs; ++i){
		UIInput * input = new UIInput();
		/// Set them to only accept floats?
		input->name = name + "Input"+String::ToString(i);
		input->numbersOnly = true;
		input->text = "0";
		input->sizeRatioX = spacePerElement;
		input->onTrigger = "UIVectorInput("+name+")";
		box->AddChild(input);
		inputs.Add(input);
	}
}	
/// Getters
Vector2i UIVectorInput::GetValue2i()
{
	int arr[2];
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		arr[i] = inputs[i]->text.ParseInt();
	}
	return Vector2i(arr);
}
Vector3f UIVectorInput::GetValue3f()
{
	float arr[3];
	for (int i = 0; i < inputs.Size() && i < 3; ++i){
		arr[i] = inputs[i]->text.ParseFloat();
	}
	return Vector3f(arr);
	
}
Vector4f UIVectorInput::GetValue4f()
{
	float arr[4];
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		arr[i] = inputs[i]->text.ParseFloat();
	}
	return Vector4f(arr);
}	
/// Setters
void UIVectorInput::SetValue2i(Vector2i vec)
{
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		String s = String::ToString(vec[i]);
		inputs[i]->SetText(s);
	}
}
void UIVectorInput::SetValue4f(Vector4f vec)
{
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}


UITextField::UITextField()
: UIInput()
{
	type = UIType::TEXT_FIELD;
	// Override controls so that ENTER can be pressed too.
};
