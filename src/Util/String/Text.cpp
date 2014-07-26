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
void Text::Nullify(){
	caretPosition = -1;
	previousCaretPosition = -1;
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

/// Pastes text into this text. How it will behave depends on the caret positions.
void Text::Paste(String text)
{
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

