// Emil Hedemalm
// 2013-07-02

#include "Text.h"

Text::Text()
: String()
{
	Nullify();
}

Text::~Text()
{
//	std::cout<<"\nText destructor.";	
}

// Sets default caret positions etc. from copy constructors.
void Text::Nullify()
{
	caretPosition = -1;
	previousCaretPosition = -1;
	offsetX = 0;
	offsetY = 0;
	color = Color::defaultTextColor;
}
/// Copy constructor and..
Text::Text(const String & string)
: String(string)
{
	Nullify();
}
Text::Text(const char * string)
: String(string)
{
	Nullify();
}
Text::Text(const wchar_t * string)
: String(string)
{
	Nullify();
}

/// String and color in hex.
Text::Text(String str, int colorHex)
	: String(str)
{
	Nullify();
	color = std::make_shared<Color>(Color::ColorByHex32(colorHex));
}

// Returns a copy with given color
Text Text::WithColor(std::shared_ptr<Color> color) const {
	Text text = *this;
	text.color = color;
	return text;
}


/// Pastes text into this text. How it will behave depends on the caret positions.
void Text::Paste(String text)
{
	DeleteSelection();
	// Just add it for now..
	if (caretPosition == -1 || caretPosition >= this->Length())
	{
		this->Add(text);
		caretPosition = this->Length();
	}
	else {
		// Divide it.
		int caret = caretPosition;
		String firstPart = this->Part(0, caretPosition);
		String secondPart = this->Part(caretPosition, Length());
		*this = firstPart + text + secondPart;
		caretPosition = caret + text.Length();
	}
}

/// If any text is selected, remove it and start inserting characters where it was. If no selection exists, nothing is removed and the caret is left untouched.
bool Text::DeleteSelection()
{
	if (previousCaretPosition == -1)
		return false;

	// Fetch the parts before and after the selection.
	int min, max;
	if (caretPosition > previousCaretPosition)
	{
		max = caretPosition;
		min = previousCaretPosition;
	}
	else 
	{
		min = caretPosition;
		max = previousCaretPosition;
	}
	// Divide it.
	int caret = caretPosition;
	String firstPart = this->Part(0, min);
	String secondPart = this->Part(max, Length());
	*this = firstPart + secondPart;
	caretPosition = min;
	return true;
}


/// Places the previous caret position a the beginning and the caret position at the end. Any regular input after that will replace all the text.
void Text::SelectAll()
{
	previousCaretPosition = 0;
	caretPosition = Length() + 1;
}

/// Returns the position of the caret when moving it back one word (exactly how it moves will depend on some settings?)
int Text::CaretPositionAtPreviousWord()
{
	int prevPos = 0;
	for (int i = caretPosition - 1; i >= 0; --i)
	{
		char c = arr[i];
		if (isalnum(c))
			continue;
		prevPos = i;
		break;
	}
	return prevPos;
}

/// Returns the position of the caret when moving it forward one word (exactly how it moves will depend on some settings?)
int Text::CaretPositionAtNextWord()
{
	int prevPos = Length();
	for (int i = caretPosition + 1; i < Length(); ++i)
	{
		if (isalnum(arr[i]))
			continue;
		prevPos = i;
		break;
	}
	return prevPos;
}

