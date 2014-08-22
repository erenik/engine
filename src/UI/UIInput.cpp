/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UITypes.h"
#include "UIInput.h"
#include "UIList.h"
#include "UIImage.h"

#include "Input/InputManager.h"
#include "Input/Keys.h"

#include "Message/Message.h"
#include "Message/VectorMessage.h"
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

/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
	If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
*/
bool UIInput::HandleDADFiles(List<String> files)
{
	// Grab first file.
	if (files.Size() == 0)
		return false;
	String firstFile = files[0];
	// Enter it as input.
	editText = firstFile;
	// Pause graphics and just set it, or it won't work.
	Graphics.Pause();
	editText = text = firstFile;
	// Remove active flag if it was active.
	state &= ~UIState::ACTIVE;
	Graphics.Resume();
	// Activate the messages this element had.
	MesMan.QueueMessages(onTrigger, this);		
	return true;
}

// When clicking/Enter pressed on keyboard.
UIElement * UIInput::Click(int mouseX, int mouseY)
{
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

// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
void UIInput::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::DRAG_AND_DROP:
		{
			DragAndDropMessage * dadm = (DragAndDropMessage*) message;
			if (dadm->dataType == DataType::STRING)
			{
				Graphics.Pause();
				editText = dadm->string;
				SetText(dadm->string);
				Graphics.Resume();
				// Activate the messages this element had.
				MesMan.QueueMessages(onTrigger, this);
			}
			break;
		}
		case MessageType::PASTE:
		{
			PasteMessage * pm = (PasteMessage*) message;
			String pasteText = pm->text;
			// Insert it into wherever the caret is..?
			editText.Paste(pasteText);
			// Store caret position from the text, as it should be able to handle this all.
			this->caretPosition = editText.caretPosition;
			// Update thingy so text is updated also..
			this->OnTextUpdated();
			break;
		}
	}
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

	int oldCaretPosition = editText.caretPosition;

	bool moveCommand = false;
	switch(keyCode)
	{
		case KEY::BACKSPACE: {
	#ifndef WINDOWS
			// Double trigger at the moment..
			OnBackspace();
	#endif
			break;
		}
		case KEY::ESCAPE:
		{
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
			// Activate the messages this element had, if any. If using as a compound e.g. inside a StringInput, then this onTrigger may be omitted.
			if (onTrigger.Length())
				MesMan.QueueMessages(onTrigger, this);
			else 
			{
				
			}
			/// Notify of the update to self and then parents, so that extra actions may be taken.
			this->OnInputUpdated(this);
			break;
		}
		// Delete
		case KEY::DELETE_KEY:
		{
			// Delete selection if any
			if (editText.DeleteSelection())
			{
				caretPosition = editText.caretPosition;
				break;
			}
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
			moveCommand = true;
			break;
		case KEY::HOME:
			caretPosition = 0;
			editText.caretPosition = caretPosition;
			moveCommand = true;
			break;
		case KEY::UP: 
		{
			break;
		}
		case KEY::DOWN: 
		{
			break;
		}
		case KEY::LEFT:
			if (caretPosition > 0)
				--caretPosition;
			// Update the text to render.
			editText.caretPosition = caretPosition;
			moveCommand = true;
			break;
		case KEY::RIGHT:
			if (caretPosition < editText.Length())
				++caretPosition;
			// Update the text to render.
			editText.caretPosition = caretPosition;
			moveCommand = true;
			break;
	}

	// If was trying to move.. 
	if (moveCommand && oldCaretPosition != editText.caretPosition)
	{
		if (!Input.KeyPressed(KEY::SHIFT))
		{
			// Reset the "previous caret"!
			editText.previousCaretPosition = -1;
		}
		// But if shift is pressed, and the previous caret is -1, then set it!
		else if (editText.previousCaretPosition == -1)
		{
			editText.previousCaretPosition = oldCaretPosition;
		}
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
	/// If control is held, only evaluate it as a special command.
	if (Input.KeyPressed(KEY::CTRL))
	{
		switch(asciiCode)
		{
			/// Generated by CTRL+A on windows.. o.O "Start of heading"
			case 1:
			{
				std::cout<<"ERKA:REKA";
				// Select all text! o.o
				editText.SelectAll();
				OnTextUpdated();
				break;
			}
			case 'A':
			case 'a':
			{
				std::cout<<"ERKA:REKA";
				break;
			}
		}
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
			case 22:	// Synchrous idle, CTRL+V, dunno..
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
		/// If any text is selected, remove it and start inserting characters where it was.
		if (editText.DeleteSelection())
			caretPosition = editText.caretPosition;

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
void UIInput::OnBackspace()
{
	// Delete selection if any
	editText.DeleteSelection();
    if (caretPosition > 0)
	{
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
void UIInput::BeginInput()
{
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

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIStringInput::OnInputUpdated(UIInput * inputElement)
{
	// Only logical thing should be our input calling us straight away.
	assert(inputElement == input);
	// Post a SetString message accordingly.
	SetStringMessage * m = new SetStringMessage(action, GetValue());
	MesMan.QueueMessage(m);
	return;

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
//	input->onTrigger = "UIStringInput("+name+")";
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

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIFloatInput::OnInputUpdated(UIInput * inputElement)
{
	// Only logical thing should be our input calling us straight away.
	assert(inputElement == input);
	// Post a SetString message accordingly.
	FloatMessage * m = new FloatMessage(action, GetValue());
	MesMan.QueueMessage(m);
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
//	input->onTrigger = "UIFloatInput("+name+")";
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

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIIntegerInput::OnInputUpdated(UIInput * inputElement)
{
	assert(inputElement == input);
	IntegerMessage * m = new IntegerMessage(action, GetValue());
	MesMan.QueueMessage(m);
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
//	input->onTrigger = "UIIntegerInput("+name+")";
	box->AddChild(input);
}

/// Getter/setter for the input element.
int UIIntegerInput::GetValue()
{
	return input->text.ParseInt();
}
void UIIntegerInput::SetValue(int value)
{
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

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIVectorInput::OnInputUpdated(UIInput * inputElement)
{
	assert(this->inputs.Exists(inputElement));
	/// Fetch vector data from the input first.
	VectorMessage * m = NULL;
	switch(numInputs)
	{
		case 2:
			m = new VectorMessage(action, GetValue2i());
			break;
		case 3:
			m = new VectorMessage(action, GetValue3f());
			break;
		case 4:
			m = new VectorMessage(action, GetValue4f());
			break;
		default:
			assert(false && "implement");
			break;
	}
	if (m)
		MesMan.QueueMessage(m);
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
	//	input->onTrigger = "UIVectorInput("+name+")";
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
void UIVectorInput::SetValue3f(Vector3f vec)
{
	for (int i = 0; i < inputs.Size() && i < 3; ++i){
		String s = String::ToString(vec[i], maxDecimals);
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
