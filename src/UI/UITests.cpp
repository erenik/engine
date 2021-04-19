/// Emil Hedemalm
/// 2020-12-10
/// Tests to verify integrity of UI updates.

#include "UIElement.h"
#include "Globals.h"
#include "UI/Input/UIIntegerInput.h"
#include "UI/Lists/UIColumnList.h"

void UIElement::UnitTest() {
	UIElement * element = new UIElement();
	auto color = std::make_shared<Color>(Vector4f(1, 1, 1, 1));
	element->SetTextColor(color);
	SAFE_DELETE(element);

	UIIntegerInput * integerInput = new UIIntegerInput("Hello", "World");
	integerInput->SetTextColor(color);
	integerInput->CreateChildren(nullptr);
	SAFE_DELETE(integerInput);

	UIColumnList * cl = new UIColumnList();
	//UIElement* aCopy = cl->Copy();
	SAFE_DELETE(cl);
	//SAFE_DELETE(aCopy);
}
