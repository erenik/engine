/// Emil Hedemalm
/// 2021-05-21
/// Helper/holder of all interaction variables and utility functions.

#include "UI/UIInteraction.h"

// Sets default state to all variables, based on current defaults.
void UIInteraction::Nullify() {
	selected = false;
	visible = true;
	inStack = false;
	hoverable = false;
	navigatable = false;
	axiomatic = false;
	selectable = false;
	activateable = false;
	moveable = false;

	/// Wether NavigateUI should be enabled when this element is pushed.
	navigateUIOnPush = false;
	disableNavigateUIOnPop = false;
	/// If force navigate UI should be applied for this element.
	forceNavigateUI = false;
	/// Previous state before pushing this UI. 0 for none. 1 for regular, 2 for force.
	previousNavigateUIState = 0;

	/// Exit-properties.
	exitable = true;

	/** Will enable/disable cyclicity of input navigation when this element is pushed. When popped, the next element in the stack will determine cyclicity. */
	cyclicY = true;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing when active. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = false;

	/// Neighbour pointers
	upNeighbour = rightNeighbour = leftNeighbour = downNeighbour = NULL;

}

void UIInteraction::InheritNeighbours(const UIInteraction& from) {
	if (!leftNeighbourName.Length())
		leftNeighbourName = from.leftNeighbourName;
	if (!rightNeighbourName.Length())
		rightNeighbourName = from.rightNeighbourName;
	if (!downNeighbourName.Length())
		downNeighbourName = from.downNeighbourName;
	if (!upNeighbourName.Length())
		upNeighbourName = from.upNeighbourName;
}
