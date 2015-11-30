/// Emil Hedemalm
/// 2014-08-25
/// Some constants and variables relating to UI-handling in general.

#ifndef UI_H
#define UI_H

class Message;

namespace UI 
{
	/** Default true. Change in ApplicationDefaults() as needed. 
		Will make all inputs based on numbers accept arbitrary mathematical expressions.
		
		If true, the inputs themselves must allow text to be entered to some degree, in order to access
		constants and other variables by name.
	*/
	extern bool defaultAcceptMathematicalExpressions;

	/// Processor for UI messages instead of having them all in MessageManager.cpp
	void ProcessMessage(Message * message);

};


/// Pauses execution in main thread. Disables input for the time being.
void PrepareForUIRemoval();
/// Resumes execution of the main thread. Enables input again.
void OnUIRemovalFinished();


#endif
