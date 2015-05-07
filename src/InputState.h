/// Emil Hedemalm
/// 2015-05-07
/// Consider renaming to InputState to have functionality similarly available as with GraphicsState and PhysicsState

#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include "MathLib/Vector2i.h"

class UIElement;

/// It should be noted that all variables here should only be changed from the main input and processing thread, or if said thread(s) are locked first.
class InputState
{
public:
	InputState();
	
	/// Current mouse position.
	Vector2i mousePosition;
	/// Current state
	bool lButtonDown;

	/// If false, will refuse or discard any given input until further notice. Defaul true.
	bool acceptInput;
	/** If true, all hovering operations (highlighting an element) whether due to mouse or keyboard input will demand that the element 
		has the activatable flag set to true. This will in practice remove possibilities to add tooltips etc. from UI such as labels, images, etc.
		Default false. Recommended for simple games.
	*/
	bool demandActivatableForHoverElements;
	/// If true, all elements which want to be hovered over need to have highlightOnHover set to true.
	bool demandHighlightOnHoverForHoverElements;
	/** If true, demands that there is always a valid element which is the current hover element. This means that hoverElement may never be NULL, unless no single valid element is present.
		Default false. Recommended for simple games.
	*/
	bool demandHoverElement;
};
/// Public global main state.
extern InputState * inputState;


#endif
