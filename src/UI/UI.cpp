/// Emil Hedemalm
/// 2014-08-25
/// Some constants and variables relating to UI-handling in general.

#include "UI.h"
#include "InputState.h"
#include "OS/Sleep.h"
#include "StateManager.h"
#include "Graphics/GraphicsManager.h"

/** Default true. Change in ApplicationDefaults() as needed. 
	Will make all inputs based on numbers accept arbitrary mathematical expressions.
		
	If true, the inputs themselves must allow text to be entered to some degree, in order to access
	constants and other variables by name.
*/
bool UI::defaultAcceptMathematicalExpressions = true;

/// Pauses execution in main thread. Disables input for the time being.
void PrepareForUIRemoval()
{
	inputState->acceptInput = false;
	SleepThread(10);
	/// Pause execution of the main thread, so that it doesn't try to access any dying UI elements while reloading.
	StateMan.Pause();
}

/// Resumes execution of the main thread. Enables input again.
void OnUIRemovalFinished()
{
	StateMan.Resume();
	Graphics.renderQueried = true;
	inputState->acceptInput = true;
}
