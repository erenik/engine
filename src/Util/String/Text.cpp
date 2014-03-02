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