/// Emil Hedemalm
/// 2021-05-21
/// Grouping of text-related variables and functions for UI elements.

#pragma once

#include "Util/String/Text.h"

class GraphicsState;
class TextFont;
struct UILayout;
struct TextColors;

struct UIText {

	UIText();
	~UIText();

	void Nullify();

// FUNCS
	void ResetTextSizeRatio() { currentTextSizeRatio = -1.0f; };
	void FormatText(GraphicsState& graphicsState, const UILayout& layout);
	void RenderText(
		GraphicsState & graphicsState,
		const UILayout& layout,
		bool isDisabled,
		bool isHover,
		bool isToggled,
		bool isTogglable,
		bool isActive,
		bool highlightOnHover
	);
	void SetColors(const TextColors& overrideColors);
	TextColors* GetColors() const { return text.colors; }
	void SetPreviousTextSizeRatio(float value) { previousTextSizeRatio = value; }
	// Hide caret.
	void OnInputStopped() {
		text.caretPosition = -1;
	}
	Text GetText() const { return text; }
	void SetText(CTextr newText, bool force);

// VARS
	struct FontDetails {
		FontDetails() { font = nullptr; }
		String source;
		String shader; // By default 'Font', interhited from TextFont::defaultFontShader
		TextFont * font;
	} fontDetails;

	/// Text contents alignment relative to current size/etc. Defautlt left.
	char alignment;

	// Padding for text within elements. Default 0, or try 0.1?
	int paddingPixels;
	// 
	float textSizeY, textSizeX;

#define MINIMUM_TEXT_SIZE_PIXELS 12
	
	// Height of text glyphs in pixels. Calculated with textSizeY/X and currentTextSizeRatio.
	float textHeightPixels;

	// Size of text in pixels
	float textSize;
	/// Relative text size to AppWindow size
	float sizeRatio;

	// If set, this will override the text colors used by Idle,Hover,Active states.
	Color* onHoverTextColor = nullptr;

	/// Colors for the text, if null, default TextFont colors will be used when rendering the elements.
	static TextColors* defaultTextColors;
	/// If true, forces all string assignments to upper-case (used with some styles of games)
	static bool defaultForceUpperCase;

	bool forceUpperCase;

	/// Used by UILog, could be used by UIList also to over-ride child Y-ratio. If negative, does not override.
	float lineSizeRatio;


private:

	// Text-variable that will contain the version of the text adapted for rendering (inserted newlines, for example).
	Text textToRender;
	/// Re-calculated depending on active UI size and stuff. Reset this to 0 or below in order to re-calculate it.
	float currentTextSizeRatio, previousTextSizeRatio;

	// Text that will be rendered
	Text text;

};
