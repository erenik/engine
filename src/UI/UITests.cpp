/// Emil Hedemalm
/// 2020-12-10
/// Tests to verify integrity of UI updates.

#include "UIElement.h"
#include "Globals.h"
#include "UI/Input/UIIntegerInput.h"
#include "UI/Lists/UIColumnList.h"
#include "UI/Lists/UIList.h"
#include "UI/Buttons/UICheckbox.h"
#include "UI/Buttons/UIToggleButton.h"

void UIElement::UnitTest() {

	wchar_t a = 'a';
	wchar_t b = 'b';
	String aStr = a;
	aStr += b;
	assert(aStr == "ab");
	char c = 'c';
	char d = 'd';
	String dcString = aStr + c;
	dcString += d;
	assert(dcString == "abcd");

	Color * colorAlloc = new Color();
	SAFE_DELETE(colorAlloc);

	// Test Text allocation as it has TextColors*
	{
		Text t = Text("Hello", 0xFF112233);
	}

	{
		Text t = Text("HelloWorld", 0x112233FF);
		Text t2 = t;
	}


	UIElement * element = new UIElement();
	auto color = Color(Vector4f(1, 1, 1, 1));
	element->SetTextColors(color);
	SAFE_DELETE(element);

	UIIntegerInput * integerInput = new UIIntegerInput("Hello", "World");
	integerInput->SetTextColors(color);
	integerInput->CreateChildren(nullptr);
	SAFE_DELETE(integerInput);

	UIList* list = new UIList();
	SAFE_DELETE(list);

	UIColumnList * cl = new UIColumnList();
	UIElement* aCopy = cl->Copy();
	SAFE_DELETE(cl);
	SAFE_DELETE(aCopy);

	UIToggleButton* toggleButton = new UIToggleButton();
	SAFE_DELETE(toggleButton);

	UICheckbox* checkBox = new UICheckbox();
	checkBox->CreateChildren();
	SAFE_DELETE(checkBox);
}
