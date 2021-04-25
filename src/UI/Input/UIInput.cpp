/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "InputState.h"
#include "UIInput.h"
#include "UI/UI.h"
#include "UI/UITypes.h"

#include "Input/InputManager.h"
#include "Input/Keys.h"
#include "UI/Lists/UIColumnList.h"

#include "File/LogFile.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"
	
Color UIInput::defaultInputTextColor = Color(1,0,0,1);

UIInput::UIInput(String name /*= ""*/)
: UIElement()
{
	rememberPreviousInputs = false;
	this->name = name;
	type = UIType::INPUT_FIELD;
	selectable = hoverable = activateable = true;
	navigatable = true;
	activationMessage = "BEGIN_INPUT(this)";
	
	inputTextureSource = UIElement::defaultTextureSource;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = true;
	caretPosition = 0;
	inputActive = false;
	numbersOnly = false;
	mathematicalExpressionsOnly = false;
	concealCharacters = false;

	min = -INT_MAX;
	max = INT_MAX;

	label = nullptr;
	input = nullptr;
}

UIInput::~UIInput()
{
//	std::cout<<"\nUIInput destructor";
}

void UIInput::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) {
	UIElement::OnInputUpdated(graphicsState, inputElement);
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
	SetText(firstFile);
	editText = GetText();
	
	// Remove active flag if it was active.
	RemoveState(UIState::ACTIVE);
	Graphics.Resume();
	// Activate the messages this element had.
	MesMan.QueueMessages(onTrigger, this);		
	return true;
}

// When clicking/Enter pressed on keyboard... no?
UIElement * UIInput::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	UIElement * e = UIElement::Click(graphicsState, mouseX, mouseY);
	if (e == this){
		// BeginInput();
	}
	return e;
}
// When button is released.
UIElement* UIInput::Activate(GraphicsState* graphicsState)
{
	for (int i = 0; i < activationActions.Size(); ++i) {
		activationActions[i].Process(graphicsState, this);
		return this;
	}
	if (BeginInput(graphicsState))
		return this;
	return nullptr;
}

// When navigating, either via control, or arrow keys or whatever.
void UIInput::Navigate(NavigateDirection direction) {
	if (HasState(UIState::ACTIVE)) {
		if (numbersOnly) {
			switch (direction) {
			case NavigateDirection::Right:
			case NavigateDirection::Up:
				IncrementValue();
				break;
			case NavigateDirection::Left:
			case NavigateDirection::Down:
				DecrementValue();
				break;
			}
		}
	}
}


/// Default calls parent class RemoveState. If the Active flag is removed, input is also halted/cancelled.
void UIInput::RemoveState(int state, bool recursive /*= false*/)
{
	bool wasActive = HasState(UIState::ACTIVE);
	UIElement::RemoveState(state, recursive);
	// And restore old string!
	if (wasActive && (state & UIState::ACTIVE))
	{
		editText = previousText;
		StopInput();	
	}
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIInput::OnStateAdded(GraphicsState* graphicsState, int state) {
	if (state == UIState::ACTIVE) {
		for (int i = 0; i < activationActions.Size(); ++i) {
			activationActions[i].Process(graphicsState, this);
			return;
		}
		BeginInput(graphicsState);
	}
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

/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
void UIInput::SetText(CTextr newText, bool force /*= false*/)
{
	UIElement::SetText(newText, force);
}
/// Sets edit text. Resets text size so that it should be visible straight away.
void UIInput::SetEditText(CTextr newText)
{
	editText = newText;
	textSize = -1;
}

/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
void UIInput::OnExitScope(bool forced)
{
	/// Call it for children too.
	UIElement::OnExitScope(forced);
	if (inputActive)
	{
		StopInput();
	}
}

/// Used by input-captuing elements. Should not be called for any base UI elements(?)
UIInputResult UIInput::OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore)
{
	bool isActive = HasState(UIState::ACTIVE);
	assert(inputActive == isActive);
	if (!inputActive)
		return UIInputResult::NoUpdate;

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
				return UIInputResult::NoUpdate;
			StopInput();
			// Activate the messages this element had, if any. If using as a compound e.g. inside a StringInput, then this onTrigger may be omitted.
			for (int i = 0; i < onTriggerActions.Size(); ++i) {
				onTriggerActions[i].Process(graphicsState, this);
			}
			if (onTrigger.Length()) {
				MesMan.QueueMessages(onTrigger, this);
			}
			/// Notify of the update to self and then parents, so that extra actions may be taken.
			this->OnInputUpdated(graphicsState, this);
			return UIInputResult::InputStopped;
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
			if (this->numbersOnly) {
				IncrementValue();
			}
			else
				parent->OnKeyDown(graphicsState, keyCode, downBefore);
			break;
		}
		case KEY::DOWN: 
		{
			if (this->numbersOnly) {
				DecrementValue();
			}
			else
				parent->OnKeyDown(graphicsState, keyCode, downBefore);
			break;
		}
		case KEY::LEFT:
			if (this->numbersOnly)
				DecrementValue();
			else if (caretPosition > 0)
			{
				if (InputMan.KeyPressed(KEY::CTRL))
				{
					caretPosition = editText.CaretPositionAtPreviousWord();
				}
				else {
					--caretPosition;
				}
				// Update the text to render.
				editText.caretPosition = caretPosition;
				moveCommand = true;
			}
			break;
		case KEY::RIGHT:
			if (this->numbersOnly)
				IncrementValue();
			else if (caretPosition < editText.Length())
			{
				if (InputMan.KeyPressed(KEY::CTRL))
				{
					caretPosition = editText.CaretPositionAtNextWord();
				}
				else {
					++caretPosition;
				}
				// Update the text to render.
				editText.caretPosition = caretPosition;
				moveCommand = true;
			}
			break;
	}

	// If was trying to move.. 
	if (moveCommand /*&& oldCaretPosition != editText.caretPosition*/)
	{
		if (!InputMan.KeyPressed(KEY::SHIFT))
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
	return UIInputResult::TextUpdated;
}

/// Used for getting text. This will be local translated language key codes?
UIInputResult UIInput::OnChar(int asciiCode)
{
	LogGraphics("OnChar for element " + name + " activatable: " + activateable + " inputActive: " + HasState(UIState::ACTIVE) + " asciiCode: "+ asciiCode + " as Char: "+ (char) asciiCode + " CtrlPressed: "+ InputMan.KeyPressed(KEY::CTRL), INFO);
	assert(activateable);
	if (!activateable) {
		return UIInputResult::NoUpdate;
	}

	bool isActive = HasState(UIState::ACTIVE);
//	assert(inputActive == isActive);
	if (!isActive)
		return UIInputResult::NoUpdate;
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
		return UIInputResult::NoUpdate;
	}
	/// If control is held, only evaluate it as a special command.
	if (InputMan.KeyPressed(KEY::CTRL))
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
		return UIInputResult::TextUpdated;
	}
	// Escape, cancel input
	else if (asciiCode == 0x1B)
	{
		// And restore old string!
		editText = previousText;
		StopInput();
		return UIInputResult::InputStopped;
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
				return UIInputResult::NoUpdate;
			default:
				#ifdef _DEBUG_ASCII
		//		std::cout<<"\nAsciiCode: "<<(int)asciiCode<<" "<<(unsigned char)asciiCode;
				#endif
				break;
		}
		/// Accept only alpha-numeric + other accepted signs implemented in Expression 
		bool ok = false;
		if (mathematicalExpressionsOnly)
		{
			if (isalnum(asciiCode))
				ok = true;
			switch(asciiCode)
			{
				case '.':
				case ',':
				case '(':
				case ')':
				case '*':
				case '+':
				case '-':
				case '/':
				case '%':
				case '^':
					ok = true;
					break;
			}
		}
		/// If only accept numbers, skip all except a few ascii..
		else if (numbersOnly)
		{
			if (isdigit(asciiCode))
				ok = true;
			switch(asciiCode)
			{
				case '.':
				case '-':
					ok = true;
					break;
			}
		}
		/// No limit defined? Then automatically accept all characters.
		else 
		{
			ok = true;
		}
		if (!ok)
			return UIInputResult::NoUpdate;
		/// If any text is selected, remove it and start inserting characters where it was.
		if (editText.DeleteSelection())
			caretPosition = editText.caretPosition;

		String firstHalf = editText.Part(0, caretPosition);
		String secondHalf = editText.Part(caretPosition);
		editText = firstHalf + (char)asciiCode + secondHalf;
		// inputBuffers[selectedInputBuffer][caretPosition] = asciiCode;
		++caretPosition;
		editText.caretPosition = caretPosition;
		if (numbersOnly) {
			int value = editText.ParseInt();
			ClampInt(value, min, max);
			editText = String::ToString(value);
			editText.caretPosition = caretPosition;
		}

		LogGraphics("OnChar for element " + name + " caretPos " + caretPosition+" editText: "+editText, INFO);

		OnTextUpdated();
	}
	return UIInputResult::TextUpdated;
}


/// For handling text-input
void UIInput::OnBackspace()
{
	// Delete selection if any
	bool deletedAnything = editText.DeleteSelection();
	/// If anything was deleted, don't take any more.. yet!
    if (caretPosition > 0 && !deletedAnything)
	{
        --caretPosition;	// Move back caret
        // ...and move back the rest one step
		String first = editText.Part(0, caretPosition);
		String second = editText.Part(caretPosition+1);
		editText = first + second;
		// Update the text to render.
		editText.caretPosition = caretPosition;
    }
	OnTextUpdated();
} 
  


/// Begins input! >)
bool UIInput::BeginInput(GraphicsState* graphicsState)
{
	if (!activateable) {
		LogMain("Attempting to input on non-activatable input", ERROR);
		return false;
	}
	if (inputActive)
		return true;

	LogMain("Beginning input on " + name, INFO);
	inputActive = true;
	// Set active state if not done so already.
	if (!HasState(UIState::ACTIVE)) {
		AddState(graphicsState, UIState::ACTIVE);
	}
	editText = text; // GetText();
	caretPosition = editText.Length();
	editText.caretPosition = caretPosition;
	// sends message to update the ui with new caret and stuff.
	OnTextUpdated();
	previousText = editText;
	InputMan.DisableKeyBindings();
	return true;
}

// Creates default elements for a label and input one-liner input element. Used by Integer, String, Float inputs.
UIColumnList * UIInput::CreateDefaultColumnList(UIElement * parent) {
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->textureSource = "0x00000000";
	box->padding = parent->padding;
	parent->InheritDefaults(box);
	parent->AddChild(nullptr, box);
	return box;
}
UILabel * UIInput::CreateDefaultLabel(UIElement * box, String text, float sizeX) {
	UILabel * label = new UILabel();
	box->InheritDefaults(label);
	label->textureSource = "0x00000000";
	//label->hoverable = true;
	label->SetText(text);
	label->sizeRatioX = sizeX;
	label->rightBorderTextureSource = box->rightBorderTextureSource;
	label->activateable = false;
	box->AddChild(nullptr, label);
	return label;
}
UIInput * UIInput::CreateDefaultInput(UIElement * box, String inputName, float sizeX) {
	/// Create 3 children
	UIInput * input = new UIInput();
	input->textureSource = UIElement::defaultTextureSource;
	input->name = inputName + "Input";
	input->sizeRatioX = sizeX;
	box->InheritDefaults(input);
	box->AddChild(nullptr, input);

	// If activating this meta-Input element, start adjusting this sub-field.
	box->activationMessage = "BEGIN_INPUT("+ input->name +")";

	return input;
}

void UIInput::SetInputTexture(String source) {
	if (source == "null")
		source = "";
	if (input)
		input->textureSource = source;
	inputTextureSource = source;
}

float UIInput::DefaultSpacePerElement(float padding) {
	int elements = 1 + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;
	return spacePerElement;
}

// Used for numbersOnly input fields.
void UIInput::IncrementValue() {
	int value = editText.ParseInt() + 1;
	ClampInt(value, min, max);
	editText = String::ToString(value);
	editText.caretPosition = -1; //  editText.Length();
	OnTextUpdated();
}
void UIInput::DecrementValue() {
	int value = editText.ParseInt() - 1;
	ClampInt(value, min, max);
	editText = String::ToString(value);
	editText.caretPosition = -1;// editText.Length();
	OnTextUpdated();
}

// Parses int value from this element's text.
const int UIInput::ParseInt() {
	return text.ParseInt();
}

// Parses float value from this element's text.
const float UIInput::ParseFloat() {
	return text.ParseFloat();
}

// For setting static colors.
void UIInput::SetTextColors(TextColors newOverrideTextColors) {
	UIElement::SetTextColors(newOverrideTextColors); // Set for self, in-case elements are not yet created.

	if (label)
		label->SetTextColors(newOverrideTextColors);
	if (input)
		input->SetTextColors(newOverrideTextColors);
}

void UIInput::SetRange(int newMin, int newMax) {
	min = newMin;
	max = newMax;
}

/// Halts input and removes Active state.
void UIInput::StopInput()
{
	if (inputActive)
	{
		LogMain("Stopping input on " + name, INFO);
		inputActive = false;
		/// Remove caret
		editText.caretPosition = -1;
		OnTextUpdated();
		// o.o
		UIElement::RemoveState(UIState::ACTIVE);
		InputMan.EnableKeyBindings();
	}
}

// sends message to update the ui with new caret and stuff.
void UIInput::OnTextUpdated()
{
	if (!ui)
		ui = this->GetRoot()->ui;
//	Graphics.Pause();
	SetText(editText, true);
//	assert(ui);
//	Graphics.QueueMessage(new GMSetUIs(name, GMUI::TEXT, editText, true, this->ui));
//	Graphics.Resume();
}


