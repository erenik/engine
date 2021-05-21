/// Emil Hedemalm
/// 2021-05-21
/// Helper/holder of all layout variables and utility functions.

#include "UI/UILayout.h"
#include "UI/UITypes.h"
#include "UI/UIElement.h"


int GetAlignment(String byName)
{
	byName.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (byName == "NULL_ALIGNMENT")
		return NULL_ALIGNMENT;
	else if (byName.Contains("MAXIMIZE"))
		return CENTER;
	else if (byName == "CENTER")
		return CENTER;
	else if (byName == "TOP_LEFT")
		return TOP_LEFT;
	else if (byName == "Left")
		return LEFT;
	else if (byName == "Right")
		return RIGHT;
	else if (byName == "BOTTOM_LEFT")
		return BOTTOM_LEFT;
	else {
		assert(false && "Unknown symbol in UIElement::GetAlignment(byName)");
	}
	return NULL_ALIGNMENT;
}


void UILayout::Nullify() {
	alignment = NULL_ALIGNMENT;	// Alignment relative to parent
	lockSizeY = false;
	lockSizeX = false;
	alignmentX = 0.5;
	alignmentY = 0.5;

	sizeRatioX = 1.0f;		// Size ratio compared to parent(!)
	sizeRatioY = 1.0f;		// Size ratio compared to parent(!)

	posX = posY = 0;
	sizeX = sizeY = 0;
	padding = 0;
	zDepth = 0;

	parentLayout = nullptr;
}

void UILayout::AdjustToWindow(
	const UILayout windowLayout,
	const UILayout * parentLayout,
	int availableParentSizeX,
	int parentCenteredContentPosX,
	bool movable
) {
	// Extract some attributes before we begin
	left = -1, right = 1, bottom = -1, top = 1;
	if (!lockSizeX)
		sizeX = 1;
	if (!lockSizeY)
		sizeY = 1;

	float z = 0;
	if (parentLayout) {
		// If parent is list, and has a scrollbar, for example, reduce available size
		int parentSizeX = availableParentSizeX; // parent->AvailableParentSizeX();
		int parentPosX = parentCenteredContentPosX; // parent->CenteredContentParentPosX();
		switch (alignment) {
		case SCROLL_BAR_Y:
			left = parentPosX + parentSizeX / 2;
			right = parentLayout->posX + parentLayout->sizeX / 2;
			bottom = parentLayout->posY - parentLayout->sizeY / 2;
			top = parentLayout->posY + parentLayout->sizeY / 2;
			break;
		default:
			left = parentPosX - parentSizeX / 2;
			right = parentPosX + parentSizeX / 2;
			bottom = parentLayout->posY - parentLayout->sizeY / 2;
			top = parentLayout->posY + parentLayout->sizeY / 2;
			break;
		}
	}
	else {
		left = windowLayout.left;
		right = windowLayout.right;
		bottom = windowLayout.bottom;
		top = windowLayout.top;
	}

	//	std::cout<<"\nUIElement::AdjustToWindow called for element "<<name<<": L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top;

		/// Center of the UI that we place, only to be used for relative alignments!
	float centerX;
	float centerY;
	/// Default for non-movable objects: place them where they should be.
	if (!movable) {
		centerX = ((float)right - left) * alignmentX + left,
			centerY = ((float)top - bottom) * alignmentY + bottom;
	}
	// Default for movable objects: place them at 0 and render them dynamically relative to their parents!
	else {
		centerX = (float)left;
		centerY = (float)bottom;
	}

	// X not locked - default
	float sizeRatioXwithConstraints = sizeRatioX;
	if (!lockSizeX) {
		sizeX = (int)(right - left);
		// If limiting width.
		float wouldBeWidth = sizeX * sizeRatioX;
		sizeRatioXwithConstraints = sizeRatioX;
		if (maxWidth != 0) {
			if (wouldBeWidth > maxWidth)
				sizeRatioXwithConstraints = maxWidth / (float)sizeX;
		}
	}
	else { // X locked.
		sizeRatioXwithConstraints = 1.0f;
	}
	// Y not locked - default
	if (!lockSizeY) {
		sizeY = (int)(top - bottom);
	}
	else { // Y size locked
	}

	if (parentLayout)
		z = parentLayout->zDepth + 0.1f;
	zDepth = z;

	float parentBorderOffset = 0;
	if (parentLayout != nullptr)
		parentBorderOffset = parentLayout->borderOffset * 2;

	/// Check alignment
	switch (alignment) {
	case SCROLL_BAR_Y:
	{
		/// Default behavior, just scale out from our determined center.
		left = RoundInt(left);
		right = RoundInt(right);
		bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
		top = RoundInt(centerY + sizeY * sizeRatioY / 2);
		break;
	}
	case NULL_ALIGNMENT: {
		/// Default behavior, just scale out from our determined center.
		left = RoundInt(centerX - sizeX * sizeRatioXwithConstraints / 2);
		right = RoundInt(centerX + sizeX * sizeRatioXwithConstraints / 2);
		bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
		top = RoundInt(centerY + sizeY * sizeRatioY / 2);
		break;
	}
	case MAXIMIZE:
		if (this->keepRatio) {
			//float screenRatio = (w_right - w_left) / (float)(w_top - w_bottom);
			float newRatio = 1;//screenRatio / ratio;
			left = RoundInt(centerX - sizeX * newRatio / 2);
			right = RoundInt(centerX + sizeX * newRatio / 2);
			bottom = RoundInt(centerY - sizeY * newRatio / 2);
			top = RoundInt(centerY + sizeY * newRatio / 2);
		}
		break;

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
		bottom = RoundInt(top - sizeY * sizeRatioY);
		top = RoundInt(top);
		break;
	case BOTTOM:

		left = RoundInt(centerX - sizeX * sizeRatioXwithConstraints / 2);
		right = RoundInt(centerX + sizeX * sizeRatioXwithConstraints / 2);
		bottom = RoundInt(bottom) - parentBorderOffset;
		top = RoundInt(bottom + sizeY * sizeRatioY) - parentBorderOffset;
		break;

	case TOP_RIGHT:
		left = RoundInt(right - sizeX * sizeRatioXwithConstraints);
		right = RoundInt(right);
		bottom = RoundInt(top - sizeY * sizeRatioY);
		top = RoundInt(top);
		break;

	default:
		break;
	}

	// Save away the new sizes
	sizeX = (int)(right - left);
	sizeY = (int)(top - bottom);
	posX = sizeX / 2 + left;
	posY = sizeY / 2 + bottom;
	position = Vector3f((float)posX, (float)posY, zDepth);
	// Correct the position for the movable objects! ^^
	if (movable)
	{
		posX += RoundInt(parentLayout->sizeX * alignmentX);
		posY += RoundInt(parentLayout->sizeY * alignmentY);
		left = RoundInt(posX - sizeX / 2.0f);
		right = RoundInt(posX + sizeX / 2.0f);
		top = RoundInt(posY + sizeY / 2.0f);
		bottom = RoundInt(posY - sizeY / 2.0f);
	}

}

void UILayout::AdjustToParent(const UILayout& parentLayout, const bool& moveable) {
	
	// Extract some attributes before we begin
	left = -1, right = 1, bottom = -1, top = 1;
	int parentLeft, parentRight, parentBottom, parentTop;
	if (!lockSizeX)
		sizeX = 1;
	if (!lockSizeY)
		sizeY = 1;

	float z = 0;
	// If parent is list, and has a scrollbar, for example, reduce available size
	int parentSizeX = parentLayout.sizeX;
	int parentPosX = parentLayout.posX;

	// Extract values from parent.
	left = parentLeft = parentPosX - parentSizeX / 2;
	right = parentRight = parentPosX + parentSizeX / 2;
	bottom = parentBottom = parentLayout.posY - parentLayout.sizeY / 2;
	top = parentTop = parentLayout.posY + parentLayout.sizeY / 2;

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
	z = parentLayout.zDepth + 0.1f;
	zDepth = z;

	float parentBorderOffset = parentLayout.borderOffset;

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

	// Save away the new sizes
	sizeX = (int)(right - left);
	sizeY = (int)(top - bottom);
	posX = sizeX / 2 + left;
	posY = sizeY / 2 + bottom;
	position = Vector3f((float)posX, (float)posY, zDepth);
	// Correct the position for the movable objects! ^^
	if (moveable)
	{
		posX += RoundInt(parentLayout.sizeX * alignmentX);
		posY += RoundInt(parentLayout.sizeY * alignmentY);
		left = RoundInt(posX - sizeX / 2.0f);
		right = RoundInt(posX + sizeX / 2.0f);
		top = RoundInt(posY + sizeY / 2.0f);
		bottom = RoundInt(posY - sizeY / 2.0f);
	}

	/// Adjust all children too
	/// Assume no children of border elements.
}

bool UILayout::IsOutside(int mouseX, int mouseY) {
	return (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom);
}

// Movement functions. These take into consideration the parent element and UIType
void UILayout::MoveX(UIType type, int distance, int parentSizeX) {
	switch (type) {
	case UIType::SLIDER_HANDLE:
		posX += distance;
		if (posX > parentSizeX - sizeX)
			posX = (int)parentSizeX - sizeX;
		if (posX < 0)
			posX = 0;
		break;
	case UIType::SCROLL_HANDLE:
		break;
	default:
		posX += distance;
	}
}

void UILayout::MoveY(UIType type, int distance, int parentSizeY) {
	switch (type) {
	case UIType::SLIDER_HANDLE:
		break;
	case UIType::SCROLL_HANDLE:
		posY += distance;
		if (posY > parentSizeY - sizeY)
			posY = (int)parentSizeY - sizeY;
		if (posY < 0)
			posY = 0;
		break;
	default:
		posY += distance;
	}
}

void UILayout::MoveTo(UIType type, int * x, int * y) {
	assert(parentLayout != nullptr);
	if (this->parentLayout == nullptr)
		return;
	int absX = 0, absY = 0;
	GetAbsolutePos(&absX, &absY);
	if (x)
		posX = *x - (absX - posX);
	if (y)
		posY = *y - (absY - posY);
	// If certain UITypes, constrain movement to parent node
	switch (type) {
	case UIType::SLIDER_HANDLE: {
		posX -= sizeX / 2;
		if (posX > parentLayout->sizeX - sizeX)
			posX = (int)parentLayout->sizeX - sizeX;
		if (posX < 0)
			posX = 0;
		break;
	}
	case UIType::SCROLL_HANDLE: {
		posY -= sizeY / 2;
		if (posY > parentLayout->sizeY - sizeY)
			posY = (int)parentLayout->sizeY - sizeY;
		if (posY < 0)
			posY = 0;
		break;
	}
	}
}

void UILayout::GetAbsolutePos(int * i_posX, int * i_posY) {
	int resX = (int)floor(posX + 0.5f);
	int resY = (int)floor(posY + 0.5f);
	UILayout * pl = parentLayout;
	// While parent pointer is non-NULL
	while (pl) {
		// Increment result
		resX += (int)floor(pl->posX + 0.5f);
		resY += (int)floor(pl->posY + 0.5f);
		pl = pl->parentLayout;
	}
	// Increment positions
	if (i_posX)
		(*i_posX) = resX;
	if (i_posY)
		(*i_posY) = resY;
}

void UILayout::CalcPosition() {
	posX = (right + left) / 2;
	posY = (top + bottom) / 2;
	sizeX = right - left;
	sizeY = top - bottom;
}
