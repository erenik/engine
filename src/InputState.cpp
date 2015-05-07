/// Emil Hedemalm
/// 2015-05-07
/// Consider renaming to InputState to have functionality similarly available as with GraphicsState and PhysicsState

#include "InputState.h"

InputState mainInputState;
InputState * inputState = &mainInputState;

InputState::InputState()
{
	// States
	mousePosition = Vector2i(-1,-1);
	lButtonDown = false;

	// Settings
	acceptInput = true;
	demandActivatableForHoverElements = false;
	demandHighlightOnHoverForHoverElements = false;
	demandHoverElement = false;
}
