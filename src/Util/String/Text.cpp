// Emil Hedemalm
// 2013-07-02

#include "Text.h"
#include "Globals.h"

#define disabledMultiplier 0.5f
#define disabledHoverMultiplier 0.85f

TextColors::TextColors(Color baseIdleColor)
	: idle(baseIdleColor)
	, hover(baseIdleColor * 1.30f + Vector4f(0.1f, 0.1f, 0.1f, 1.0f))
	, active(baseIdleColor * 1.50f + Vector4f(0.2f, 0.2f, 0.2f, 1.0f))
	, disabledIdle(baseIdleColor * disabledMultiplier)
	, disabledHover(baseIdleColor * disabledHoverMultiplier) 
{
	NullifyPointers();
}

TextColors::TextColors(Color idle, Color hover, Color active)
	: idle(idle)
	, hover(hover)
	, active(active)
	, disabledIdle(idle * disabledMultiplier)
	, disabledHover(hover * disabledHoverMultiplier) 
{
	NullifyPointers();
}

TextColors::TextColors(const TextColors& other) {
	NullifyPointers();
	other.CopyDataInto(*this);
}

void TextColors::operator = (const TextColors& other) {
	other.CopyDataInto(*this);
}

void TextColors::NullifyPointers() {
}


void TextColors::CopyDataInto(TextColors& other) const {

	other.idle = idle;
	other.hover = hover;
	other.active = active;
	other.disabledIdle = disabledIdle;
	other.disabledHover = disabledHover;
	other.toggledIdle = toggledIdle;
	other.toggledHover = toggledHover;

}

TextColors::~TextColors() {
}

TextColors TextColors::WithAlpha(float alpha) {
	TextColors newColors = *this;
	newColors.idle.a = alpha;
	newColors.hover.a = alpha;
	newColors.active.a = alpha;
	newColors.disabledIdle.a = alpha;
	newColors.disabledHover.a = alpha;
	return newColors;
}

Color TextColors::Get(TextState byState) {
	switch (byState) {
	case TextState::Idle:
		return idle;
	case TextState::Hover:
		return hover;
	case TextState::Active:
		return active;
	case TextState::DisabledIdle:
		return disabledIdle;
	case TextState::DisabledHover:
		return disabledHover;
	case TextState::ToggledIdle:
		return toggledIdle;
	case TextState::ToggledHover:
		return toggledHover;
	case TextState::NotToggledIdle:
		return notToggledIdle;
	default:
		assert(false);
	}
}


void Text::SetColors(const TextColors& textColors) {
	if (colors == nullptr) {
		colors = new TextColors(textColors);
	}
	else 
		*colors = textColors;
}

Text::Text()
: String()
{
	Nullify();
}

#include "File/LogFile.h"
#include "Util/String/StringUtil.h"

Text::~Text()
{
	//LogMain("Destructing text: " + *this+" with Color: "+(colors? VectorString(colors->idle) : "null"), INFO);
	//std::cout<<"\nText destructor for: "<<*this;	
	SAFE_DELETE(colors);
}

// Sets default caret positions etc. from copy constructors.
void Text::Nullify()
{
	caretPosition = -1;
	previousCaretPosition = -1;
	offsetX = 0;
	offsetY = 0;
	colors = nullptr; // = Color::defaultTextColor;
}

// Copy constructor
Text::Text(const Text& text)
	: String(text)
	, colors(nullptr)
{
	text.CopyTextVariables(*this);
}

const Text& Text::operator = (const Text & otherText) {
	// Copy the strings parts
	String::operator =(otherText);
	otherText.CopyTextVariables(*this);
	return *this;
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
	colors = new TextColors(Color::ColorByHex32(colorHex));
}

// Returns a copy with given color
Text Text::WithColor(const Color& color) const {
	Text text = *this;
	text.colors = new TextColors(color);
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

void Text::CopyTextVariables(Text& intoText) const {
	intoText.caretPosition = caretPosition;
	intoText.previousCaretPosition = previousCaretPosition;
	if (colors) {
		if (intoText.colors) // Copy over data if already allocated.
			*intoText.colors = *colors;
		else { // Otherwise allocated it.
			SAFE_DELETE(intoText.colors);
			intoText.colors = new TextColors(*colors);
		}
	}
}
