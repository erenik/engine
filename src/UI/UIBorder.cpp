/// Emil Hedemalm
/// 2020-12-12
/// Border elements and specific rendering around, inside, and outside edges of existing UI elements.

#include "UIBorder.h"

#include "Mesh/Square.h"

UIBorder::~UIBorder() {

}

/// Adjusts the UI element size and position relative to new AppWindow size
void UIBorder::AdjustToWindow(int w_left, int w_right, int w_bottom, int w_top) {
	
	/// Reset text-variables so that they are re-calculated before rendering again.
	currentTextSizeRatio = -1.0f;

	// Extract some attributes before we begin
	left = -1, right = 1, bottom = -1, top = 1;
	int parentLeft, parentRight, parentBottom, parentTop;
	if (!lockSizeX)
		sizeX = 1;
	if (!lockSizeY)
		sizeY = 1;

	float z = 0;
	// If parent is list, and has a scrollbar, for example, reduce available size
	int parentSizeX = parent->AvailableParentSizeX();
	int parentPosX = parent->CenteredContentParentPosX();
	
	// Extract values from parent.
	left = parentLeft = parentPosX - parentSizeX / 2;
	right = parentRight = parentPosX + parentSizeX / 2;
	bottom = parentBottom = parent->posY - parent->sizeY / 2;
	top = parentTop = parent->posY + parent->sizeY / 2;

	//	std::cout<<"\nUIElement::AdjustToWindow called for element "<<name<<": L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top;

		/// Center of the UI that we place, only to be used for relative alignments!
	float centerX;
	float centerY;
	/// Default for non-movable objects: place them where they should be.
	if (!moveable) {
		centerX = ((float)parentRight - parentLeft) * alignmentX + parentLeft,
			centerY = ((float)parentTop - parentBottom) * alignmentY + parentBottom;
	}
	// Default for movable objects: place them at 0 and render them dynamically relative to their parents!
	else {
		centerX = (float)parentLeft;
		centerY = (float)parentBottom;
	}

	// X not locked - default
	float sizeRatioXwithConstraints = sizeRatioX;
	if (!lockSizeX) {
		sizeX = (int)(parentRight - parentLeft);
	}
	else { // X locked.
		sizeRatioXwithConstraints = 1.0f;
	}
	// Y not locked - default
	if (!lockSizeY) {
		sizeY = (int)(parentTop - parentBottom);
	}
	else { // Y size locked
	}

	// Borders ALLWAYS have parents
	z = parent->ZDepth() + 0.1f;
	zDepth = z;

	float parentBorderOffset = parent->borderOffset;

	/// Check alignment
	switch (alignment) {
	case LEFT:
		/// Do nothing, we start off using regular centering
		left = RoundInt(left);
		right = RoundInt(left + sizeX * sizeRatioXwithConstraints);
		bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
		top = RoundInt(centerY + sizeY * sizeRatioY / 2);
		break;

	case CENTER:
		/// Do nothing, we start off using regular centering
		left = RoundInt(centerX - sizeX * sizeRatioXwithConstraints / 2);
		right = RoundInt(centerX + sizeX * sizeRatioXwithConstraints / 2);
		bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
		top = RoundInt(centerY + sizeY * sizeRatioY / 2);
		break;

	case RIGHT:
		left = RoundInt(right - sizeX * sizeRatioXwithConstraints);
		right = RoundInt(right);
		bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
		top = RoundInt(centerY + sizeY * sizeRatioY / 2);
		break;

	case TOP:
		left = RoundInt(centerX - sizeX * sizeRatioXwithConstraints / 2);
		right = RoundInt(centerX + sizeX * sizeRatioXwithConstraints / 2);
		bottom = RoundInt(parentTop - sizeY * sizeRatioY + parentBorderOffset);
		top = RoundInt(parentTop + parentBorderOffset);
		break;
	case BOTTOM:

		left = RoundInt(centerX - sizeX * sizeRatioXwithConstraints / 2);
		right = RoundInt(centerX + sizeX * sizeRatioXwithConstraints / 2);
		bottom = RoundInt(parentBottom) - parentBorderOffset;
		top = RoundInt(parentBottom + sizeY * sizeRatioY) - parentBorderOffset;
		break;

	case TOP_RIGHT:
		left = RoundInt(right - sizeX * sizeRatioXwithConstraints);
		right = RoundInt(right);
		bottom = RoundInt(top - sizeY * sizeRatioY);
		top = RoundInt(top);
		break;

	default:
		assert(false && "implement? or bad alignment in UIBorder::");
		break;
	}

	/// Set the new dimensions
	if (mesh)
		GetMesh()->SetDimensions((float)left, (float)right, (float)bottom, (float)top, z);

	// Save away the new sizes
	sizeX = (int)(right - left);
	sizeY = (int)(top - bottom);
	posX = sizeX / 2 + left;
	posY = sizeY / 2 + bottom;
	position = Vector3f((float)posX, (float)posY, zDepth);
	// Correct the position for the movable objects! ^^
	if (moveable)
	{
		posX += RoundInt(parent->sizeX * alignmentX);
		posY += RoundInt(parent->sizeY * alignmentY);
		left = RoundInt(posX - sizeX / 2.0f);
		right = RoundInt(posX + sizeX / 2.0f);
		top = RoundInt(posY + sizeY / 2.0f);
		bottom = RoundInt(posY - sizeY / 2.0f);
	}

	/// Adjust all children too
	/// Assume no children of border elements.
}


