/// Emil Hedemalm
/// 2014-08-25
/// Free-form text field.

#ifndef UI_TEXT_FIELD_H
#define UI_TEXT_FIELD_H

#include "UIInput.h"

/// Class for 1 string input with label before it.
class UITextField : public UIInput 
{
public:
	UITextField();
	UITextField(String name, String onTrigger);
	virtual ~UITextField();

private:
};

#endif
