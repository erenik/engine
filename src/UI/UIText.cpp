/// Emil Hedemalm
/// 2021-05-21
/// Grouping of text-related variables and functions for UI elements.

#include "UI/UIText.h"

#include "GraphicsState.h"
#include "Graphics/Fonts/TextFont.h"
#include "UI/UILayout.h"
#include "Graphics/GraphicsManager.h"

TextColors * UIText::defaultTextColors = nullptr; // Color(Vector4f(1, 1, 1, 1));
bool UIText::defaultForceUpperCase = false;

UIText::UIText() {}
UIText::~UIText() {}

void UIText::Nullify() {
	onHoverTextColor = nullptr;
	lineSizeRatio = -1.f;
	if (defaultTextColors) {
		SetColors(*defaultTextColors);
	}
	sizeRatio = 1.0f;
	forceUpperCase = false;
	paddingPixels = 4;
}


void UIText::FormatText(GraphicsState& graphicsState, const UILayout& layout)
{
	/// Resize to fit.
	Text& textToRender = text;

	TextFont * currentFont = fontDetails.font;
	if (currentFont == nullptr)
		currentFont = graphicsState.currentFont;
	if (currentFont == nullptr) {
		return;
	}
	assert(currentFont);

	/// Rows available
	int rowsAvailable = 1;
	currentTextSizeRatio = 1.0f;
	/// Returns the size required by a call to RenderText if it were to be done now. In... pixels? or units
	Vector2f size = currentFont->CalculateRenderSizeUnits(textToRender);

	// SizeY but taking into consideration textPadding.
	textSizeY = layout.sizeY * (sizeRatio) - paddingPixels * 2;
	textSizeX = layout.sizeX * (sizeRatio) - paddingPixels * 2;

	textToRender.offsetY = (float)paddingPixels;

	/// Pixels per unit, defined by the relative Y size the text should have.
	float pixelsPerUnit = textSizeY;
	/// Total pixels required if rendering right now.
	Vector2f pixelsRequired = size * pixelsPerUnit;
	/// Check how much X is available, if we need to center it, etc.
	int xAvailable = textSizeX - pixelsRequired.x;
	float xRatio = 1.f, yRatio = 1.f;
	if (pixelsRequired.x > rowsAvailable * textSizeX) {
		xRatio = textSizeX / pixelsRequired.x;
	}
	if (pixelsRequired.y > textSizeY) {
		yRatio = textSizeY / pixelsRequired.y;
	}
	if (xRatio < yRatio)
		currentTextSizeRatio = xRatio;
	else
		currentTextSizeRatio = yRatio;

	// Take into consideration the padding both above and below.
	{
		int pixelsAfterPadding = textSizeY;
		float ratioPixelsAfterPaddingDividedByWholeHeight = textSizeY / (float)layout.sizeY;
		// Don't decrease further if it is already auto-scaled down.
		if (currentTextSizeRatio > ratioPixelsAfterPaddingDividedByWholeHeight)
			currentTextSizeRatio = ratioPixelsAfterPaddingDividedByWholeHeight;
	}

	/// Check previous ratio. Use lower one.
	if (previousTextSizeRatio < currentTextSizeRatio)
		currentTextSizeRatio = previousTextSizeRatio;

	previousTextSizeRatio = currentTextSizeRatio;

	/// Re-align.
	switch (alignment) {
	case CENTER: {
		pixelsPerUnit = currentTextSizeRatio * textSizeY;
		pixelsRequired = size * pixelsPerUnit;
		int offsetX = textSizeX * 0.5f - pixelsRequired.x * 0.5f + paddingPixels;
		if (offsetX < 0)
			offsetX = 0;
		textToRender.offsetX = offsetX;
		break;
	}
	case RIGHT: {
		pixelsPerUnit = currentTextSizeRatio * textSizeY;
		pixelsRequired = size * pixelsPerUnit;
		int offsetX = textSizeX - pixelsRequired.x + paddingPixels;
		if (offsetX < 0)
			offsetX = 0;
		textToRender.offsetX = offsetX;
		break;
	}
	default:
	case LEFT:
		textToRender.offsetX = paddingPixels;
		break;
	}

}

void UIText::RenderText(
	GraphicsState & graphicsState,
	const UILayout& layout,
	bool isDisabled,
	bool isHover,
	bool isToggled,
	bool isActive,
	bool highlightOnHover
) {
	if (this->text.Length() == 0 && this->text.caretPosition < 0)
		return;

	if (this->text.caretPosition >= 0) {
		//LogGraphics("Hello", INFO);
	}

	Text& toRender = this->text;
	if (this->textToRender.Length() > 0)
		toRender = textToRender;

	/// Bind correct font if applicable.
	auto font = this->fontDetails.font;
	auto fontSource = this->fontDetails.source;
	if (toRender.Length()) {
		if (font) {
			graphicsState.currentFont = font;
		}
		else if (fontSource && !font) {
			font = GraphicsMan.GetFont(fontSource);
			if (font)
				graphicsState.currentFont = font;
		}
		// If still no font, use default font.
		if (!font)
		{
			graphicsState.currentFont = GraphicsMan.GetFont(TextFont::defaultFontSource);
		}
	}
	// Render text if applicable!
	if ((toRender.Length() || toRender.caretPosition > -1)
		&& graphicsState.currentFont)
	{
	}
	else
		return;

	if (currentTextSizeRatio <= 0) {
		FormatText(graphicsState, layout);
	}
	float pixels = textSizeY * currentTextSizeRatio; // Graphics.Height();


	TextFont * currentFont = graphicsState.currentFont;
	Vector4f modelPositionOffset = graphicsState.modelMatrix * Vector4f(0, 0, 0, 1); // E.g. due to scrolling in a list.
	Vector3f fontRenderOffset = Vector3f(layout.left + toRender.offsetX, layout.top - toRender.offsetY, layout.zDepth + 0.05f)
		+ modelPositionOffset;

	// If disabled, dull the color! o.o
	if (isDisabled)
		currentFont->disabled = true;
	else
		currentFont->disabled = false;
	if (isHover && highlightOnHover)
		currentFont->hoveredOver = true;
	else
		currentFont->hoveredOver = false;

	Color * overrideColor = nullptr;
	TextState textState = TextState::Idle;
	// Does it have colors assigned for toggle-states? Then treat it differentely.
	if (text.colors && text.colors->toggledIdle != nullptr) {
		if (isToggled)
			overrideColor = text.colors->toggledIdle;
		else
			overrideColor = text.colors->notToggledIdle;
		//textState = TextState::Hover; // Permanently hovered over if toggled good enough..?
	}
	else if (isDisabled) {
		textState = TextState::DisabledIdle;
		if (isHover)
			textState = TextState::DisabledHover;
	}
	else if (isActive) {
		textState = TextState::Active;
	}
	else if (isHover) {
		textState = TextState::Hover;
		if (onHoverTextColor != nullptr)
			overrideColor = onHoverTextColor;
	}

	// Set right shader as well.
	if (fontDetails.shader) {
		graphicsState.fontShaderName = fontDetails.shader;
	}

	// What should be rendered? If not specified, then use the 'text'
	if (this->textToRender.Length() == 0)
		this->textToRender = text;

	graphicsState.currentFont->RenderText(
		this->textToRender,
		textState,
		overrideColor,
		graphicsState,
		fontRenderOffset,
		pixels);
}

// Sets it to override.
void UIText::SetColors(const TextColors& overrideColors) {
	text.SetColors(overrideColors);
}

void UIText::SetText(CTextr newText, bool force) {
	TextColors * oldColors = text.colors;
	text = newText;
	if (oldColors)
		text.SetColors(*oldColors);
	textToRender = "";
	if (forceUpperCase) {
		this->text.ToUpperCase();
	}
	/// Reset text-variables so that they are re-calculated before rendering again.
	currentTextSizeRatio = -1.0f;
}
