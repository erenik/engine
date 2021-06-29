/// Emil Hedemalm
/// 2021-05-21
/// Helper/holder of all interaction variables and utility functions.

#pragma once

#include "String/AEString.h"

class UIElement;

struct UIInteraction {

	// Sets selectable, hoverable, navigatable and activateable = true;
	void DefaultTrue() {
		selectable = hoverable = navigatable = activateable = true;
	};
	// Sets default state to all variables, based on current defaults.
	void Nullify(); 
	void InheritNeighbours(const UIInteraction& from);

// VARS 

	bool selected;
	bool selectable;
	bool visible;						// Visibility flag
	bool axiomatic;						// If flagged: return straight away if hovered on without looking at children.
	bool hoverable;						// Toggles if the element responds to hover-messages
	bool activateable;					// Toggles whether it is CURRENTLY activatable.
	bool togglable; // For toggle-buttons, checkboxes, Radiobuttons
	bool navigatable;					// Toggles whether this element should be navigatable with arrow-keys/gamepad.
	/// Defines if the element is moveable in runtime, for example slider-handles
	bool moveable;
	/// If in the UI-interaction/navigation stack, so that navigation commands don't go outside it or to a parent-node.
	bool inStack;
	/// Says that when pressing ESC (or similar) this menu/element can/will be closed. This should work recursively until an element that is closable is found.
	bool exitable;
	/// Message to be processed when exiting this menu (could be anything, or custom stuff).
	String onExit;

	/// Wether NavigateUI should be enabled when this element is pushed. Default is false.
	bool navigateUIOnPush;
	bool disableNavigateUIOnPop;
	/// If force navigate UI should be applied for this element.
	bool forceNavigateUI;
	/// Previous state before pushing this UI. 0 for none. 1 for regular, 2 for force.
	int previousNavigateUIState;

	/** In order to override some annoying UI movements, you can specify the following.
		If an entry exists here target element will be hovered to if it exists before using the general distance-dotproduct nearest algorithm.
	*/
	String leftNeighbourName, rightNeighbourName, upNeighbourName, downNeighbourName;
	/// Pointer-equivalents of the above. Should only be set and used with care!
	UIElement * leftNeighbour, *rightNeighbour, *upNeighbour, *downNeighbour;

	/** Will enable/disable cyclicity of input navigation when this element is pushed. When popped, the next element in the stack will determine cyclicity. */
	bool cyclicY;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing. Must be subclass of UIInput as extra functions there are used for this.
	bool demandInputFocus;
};
