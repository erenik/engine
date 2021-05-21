/// Emil Hedemalm
/// 2021-05-21
/// Helper/holder of all layout variables and utility functions.

#pragma once

class UIElement;
enum class UIType;

#include "MathLib.h"

// Positional variables (pre-resizing)
enum generalAlignments {
	NULL_ALIGNMENT,
	MAXIMIZE, MAXIMIZED = MAXIMIZE,	// Have maximization as an alignment too to simplify it all
	TOP_LEFT, TOP, TOP_RIGHT,
	LEFT, CENTER, MIDDLE = CENTER, RIGHT,
	BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT,
	SCROLL_BAR_Y, // Default to the right of the centered content.
};

int GetAlignment(String byName);

struct UILayout {

	void Nullify();


// FUNCS
	void AdjustToWindow(
		const UILayout windowLayout,
		const UILayout * parentLayout,
		int availableParentSizeX,
		int parentCenteredContentPosX,
		bool movable);
	void AdjustToParent(const UILayout& parentLayout, const bool& moveable);

	int AvailableParentSizeX() { return sizeX; };

	bool IsOutside(int mouseX, int mouseY);

	// Movement functions. These take into consideration the parent element and UIType
	void MoveX(UIType uiType, int distance, int parentSizeX);
	void MoveY(UIType uiType, int distance, int parentSizeY);
	void MoveTo(UIType type, int * x, int * y);

	void GetAbsolutePos(int * i_posX, int * i_posY);
	void CalcPosition();

// VARS

	UILayout* parentLayout;

	// Alignment relative to parent. 0.5 = in the center/middle of parent, 0 = left/up.
	float alignmentX;
	float alignmentY;

	// Size ratios for relative sizing within UI.
	float sizeRatioX;		// Size ratio compared to parent(!)
	float sizeRatioY;		// Size ratio compared to parent(!)

	// If 0, not used. Used to limit stretching beyond some mark to make display nicer on bigger displays.
	int maxWidth = 0, maxHeight = 0;

	// Position, usage depends on the alightment type, scalability, etc.
	Vector3f position; // Vector, try and migrate to them instead, ya?
	int posX, posY;					// Position of the element, relative to parent and origin.
	int sizeX, sizeY;					// Dimensions of the element.
	bool lockSizeY, lockSizeX;
	int left, right, top, bottom;		// Dimensions in screen-space, from 0 to width and 0 to height, 0,0 being in the bottom-left.

	// Position-adjustment variables for advanced UI elements like UIList
	float padding;

	// For rendering, but also layout.
	float zDepth = 0.0f;

	int borderOffset = 0;

	/// Alignment relative to parent. If this is set all other alignment* variables will be ignored.
	char alignment;


	bool keepRatio;
};
