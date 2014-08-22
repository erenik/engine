// Emil Hedemalm
// 2013-07-02

#ifndef AE_TEXT_H
#define AE_TEXT_H

#include "AEString.h"

// An extension of the String class, made for handling and displaying input, colors, bold, etc.

class Text : public String {
public:
	Text();
	virtual ~Text();
	// Sets default caret positions etc. from copy constructors.
	void Nullify();
	/// Copy constructor and..
	Text(const String & string);
	Text(const char * string);
	Text(const wchar_t * string);

	/// Pastes text into this text. How it will behave depends on the caret positions.
	void Paste(String text);

	/** If any text is selected, remove it and start inserting characters where it was. If no selection exists, nothing is removed and the caret is left untouched.
		Returns true if anything was deleted.
	*/
	bool DeleteSelection();


	/// Places the previous caret position a the beginning and the caret position at the end. Any regular input after that will replace all the text.
	void SelectAll();

	/** For rendering active input location. The caret position lies by default
		on the left "side" of the index!
		A value of -1 indicates that it should not be rendered/is not active.
	*/
	int caretPosition;
	int previousCaretPosition;
private:

};

#endif