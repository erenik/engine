/// Emil Hedemalm
/// 2014-01-14
/// Enumeration list of all types of UI. Separated from UIElement.h for logical purposes and reducing compilation time upon changing here.

/// Default types that .. do stuff? Not sure if this is even used much anymore?
namespace UIType
{
	enum UITypes
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
		CHECKBOX,
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
		MESSAGEBOX,		// A Message box that will prompt the user before the program continues
		VIDEO,		// Element integrating MultimediaStream playback into another texture which it renders within with proper aspect ratio, etc.
		IMAGE, // UIImage
		MATRIX,	// For data-manipulation.
		DROP_DOWN_MENU,	DROP_DOWN_LIST = DROP_DOWN_MENU,

	};
};
