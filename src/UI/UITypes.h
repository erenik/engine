/// Emil Hedemalm
/// 2014-01-14
/// Enumeration list of all types of UI. Separated from UIElement.h for logical purposes and reducing compilation time upon changing here.
#pragma once

/// Default types that .. do stuff? Not sure if this is even used much anymore?
enum class UIType
{
	NULL_TYPE,
	BASIC,
	LABEL,
	TEXT_FIELD,
	BUTTON,
	INPUT_FIELD,
	SLIDER_HANDLE,
	SLIDER_BAR,
	SCROLL_HANDLE,
	SCROLL_FIELD,
	SCROLL_BAR,
	CHECKBOX, // Compound of text and a toggling button
	TOGGLE_BUTTON, // Button which will change text or appearance when clicked.
	RADIOBUTTON,
	DROPDOWNMENU,
	LABON,
	CANVAS,
	LIST,
	COLUMN_LIST,
	TEXTURE_INPUT,	// Combined input that displays both source and a small image preview using an UIInput, UILabel and UIImage.
	STRING_INPUT,	// Single short string input.
	FLOAT_INPUT,	// Single number input.
	INTEGER_INPUT,	// Single int input.
	VECTOR_INPUT,	// Triple input made for numbers.
	RADIO_BUTTONS,	// Aggregate input consiting of several radio-buttons.
	QUERY_DIALOGUE,	// Dialogue that requires input.
	FILE_BROWSER,	// General file-system navigator.
	FILE_INPUT,		// String input with button for pushing File browsers.
	MESSAGEBOX,		// A Message box that will prompt the user before the program continues
	VIDEO,		// Element integrating MultimediaStream playback into another texture which it renders within with proper aspect ratio, etc.
	IMAGE, // UIImage
	MATRIX,	// For data-manipulation.
	DROP_DOWN_MENU,	DROP_DOWN_LIST = DROP_DOWN_MENU,
	LOG, // For lists of texts that update/scroll dynamically, may have filtering functions, etc.
	BAR, // For HP, shield, etc., Default horizontal

};
