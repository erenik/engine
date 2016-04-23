/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "InputState.h"
#include "UIInput.h"
#include "UI/UI.h"
#include "UI/UITypes.h"

#include "Input/InputManager.h"
#include "Input/Keys.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"
	
UIInput::UIInput(String name /*= ""*/)
: UIElement()
{
	this->name = name;
	type = UIType::INPUT_FIELD;
	selectable = hoverable = activateable = true;
	activationMessage = "BEGIN_INPUT(this)";
	
	/// When true, re-directs all (or most) keyboard input to the target element for internal processing. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = true;
	caretPosition = 0;
	inputActive = false;
	numbersOnly = false;
	mathematicalExpressionsOnly = false;
	concealCharacters = false;
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

// When clicking/Enter pressed on keyboard... no?
UIElement * UIInput::Click(int mouseX, int mouseY)
{
	UIElement * e = UIElement::Click(mouseX, mouseY);
	if (e == this){
		// BeginInput();
	}
	return e;
}
// When button is released.
UIElement* UIInput::Activate()
{
	// Make this element active for input!
	BeginInput();
	// Skip this.
	return this;
}

/// Default calls parent class RemoveState. If the Active flag is removed, input is also halted/cancelled.
void UIInput::RemoveState(int state, bool recursive /*= false*/)
{
	bool wasActive = this->state & UIState::ACTIVE;
	UIElement::RemoveState(state, recursive);
	// And restore old string!
	if (wasActive && (state & UIState::ACTIVE))
	{
		editText = previousText;
		StopInput();	
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
	bool isActive = (state & UIState::ACTIVE);
	assert(inputActive == isActive);
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
			}
			moveCommand = true;
			break;
		case KEY::RIGHT:
			if (caretPosition < editText.Length())
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
			}
			moveCommand = true;
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
	return 0;
}

/// Used for getting text. This will be local translated language key codes?
int UIInput::OnChar(int asciiCode)
{
	bool isActive = (state & UIState::ACTIVE);
	assert(inputActive == isActive);
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
			return 0;
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
	InputMan.DisableKeyBindings();
}

/// Halts input and removes Active state.
void UIInput::StopInput()
{
	inputActive = false;
	/// Remove caret
	editText.caretPosition = -1;
	OnTextUpdated();
	// o.o
	UIElement::RemoveState(UIState::ACTIVE);
	InputMan.EnableKeyBindings();
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


