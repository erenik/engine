/// Emil Hedemalm
/// 2014-08-25
/// Some constants and variables relating to UI-handling in general.

#include "UI.h"

/** Default true. Change in ApplicationDefaults() as needed. 
	Will make all inputs based on numbers accept arbitrary mathematical expressions.
		
	If true, the inputs themselves must allow text to be entered to some degree, in order to access
	constants and other variables by name.
*/
bool UI::defaultAcceptMathematicalExpressions = true;
