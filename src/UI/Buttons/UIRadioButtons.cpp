/// Emil Hedemalm
/// 2014-08-31
/// Aggregate input class which handles a number of buttons

#include "UIRadioButtons.h"
#include "UI/Buttons/UIToggleButton.h"
#include "UI/UITypes.h"
#include "Message/MessageManager.h"
#include "Message/MathMessage.h"
#include "UI/UILabel.h"
// #include "Graphics/GraphicsManager.h"

Color UIRadioButtons::toggledTextColor = Color::ColorByHexName("0xffffffff");
Color UIRadioButtons::notToggledTextColor = Color::ColorByHexName("0xaaaaaaff");

/// o.o
UIRadioButtons::UIRadioButtons(int numberOfButtons, String name, String action)
: UIColumnList(), numButtons(numberOfButtons), action(action)
{
	this->name = name;
	this->type = UIType::RADIO_BUTTONS;
	interaction.activateable = true;
	interaction.hoverable = true;
	interaction.navigatable = true;
	toggledIndex = 0;
}

UIRadioButtons::~UIRadioButtons()
{
}


/// Creates the actual buttons.
void UIRadioButtons::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	assert(children.Size() == 0);
	int elements; 
	float spaceLeft;
	float spacePerElement;
	float labelSize;
	labelSize = divider.x;
	elements = numButtons;
	if (noLabel)
		labelSize = 0;
	spaceLeft = (1.0f - labelSize) - layout.padding * elements;
	spacePerElement = spaceLeft / elements;
	// Add label!
	if (!noLabel)
	{
		label = new UILabel(GetText());
		InheritDefaults(label);
		label->layout.sizeRatioX = labelSize;
		label->SetText(displayText);
		label->visuals.textureSource = "";
		label->interaction.hoverable = true;
		AddChild(nullptr, label);
	}
	for (int i = 0; i < numButtons; ++i)
	{
		/// Create 3 children
		UIToggleButton * button = new UIToggleButton();
		this->InheritDefaults(button);
		/// Set them to only accept floats?
		button->name = name + "Button"+ String(i);
		button->SetText(buttonTexts.Size() > i ? buttonTexts[i] : "");
		button->UpdateTextColor(toggledTextColor, notToggledTextColor);
		button->layout.sizeRatioX = spacePerElement;
		button->topRightCornerTextureSource = topRightCornerTextureSource;
		button->visuals.textureSource = "";
		if (textureSourcesOrNames.Size()) {
			button->visuals.textureSource = textureSourcesOrNames[i];
			button->visuals.retainAspectRatioOfTexture = true;
		}
		else 
			button->visuals.textureSource = visuals.textureSource;
		/// Pre-select first one always.
		if (i == toggledIndex)
			button->SetToggled(true);
	//	input->onTrigger = "UIFloatInput("+name+")";
		AddChild(nullptr, button);
		buttons.Add(button);
	}
	childrenCreated = true;
}


/// Sets the texts of the children.
void UIRadioButtons::SetTexts(List<String> texts)
{
	buttonTexts = texts;

	for (int i = 0; i < buttons.Size() && i < texts.Size(); ++i)
	{
		UIToggleButton * button = buttons[i];
		String text = texts[i];
		button->SetText(text);
	}
}

// Set texture for all elements.
void UIRadioButtons::SetTextureSource(String source)
{
	for (int i = 0; i < buttons.Size(); ++i)
	{
		UIToggleButton * button = buttons[i];
		button->visuals.textureSource = source;
	}
}

// Sets textures to be used for the elements, assuming one texture per button.
void UIRadioButtons::SetTextures(List<String> newTextureSourcesOrNames) {
	textureSourcesOrNames = newTextureSourcesOrNames;
}


/// Sent when a child checkbox is toggled. 
void UIRadioButtons::OnToggled(UIElement * element)
{
	assert(element->type == UIType::TOGGLE_BUTTON);
	UIToggleButton * toggleButton = (UIToggleButton*) element;
	assert(buttons.Exists(toggleButton));
	/// Send a message based on which button it was.
	toggledIndex = buttons.GetIndexOf(toggleButton);
	/// Set a message!
	IntegerMessage * im = new IntegerMessage(action, toggledIndex);
	MesMan.QueueMessage(im);
	bool somethingToggled = false;
	/// De-toggle the other elements.
	if (toggleButton->IsToggled()) {
		for (int i = 0; i < buttons.Size(); ++i)
		{
			UIToggleButton * button = buttons[i];
			if (button == toggleButton)
				continue;
			button->SetToggledSilently(false);
		}
	}
	/// Ensure always one radio button is toggled.
	//else 
	//	toggleButton->SetToggledSilently(true);
}

// Returns true if it adjusted any UI state.
bool UIRadioButtons::OnNavigate(GraphicsState* graphicsState, NavigateDirection navigateDirection) {
	int value = toggledIndex;
	switch (navigateDirection) {
	case NavigateDirection::Left:
		--value;
		break;
	case NavigateDirection::Right:
		++value;
		break;
	default:
		return false;
	}
	// Clamp it
	if (value >= buttons.Size())
		value = 0;
	if (value < 0)
		value = buttons.Size() - 1;

	SetValue(graphicsState, value);
	return true;
}

bool UIRadioButtons::OnProceed(GraphicsState* graphicsState) {
	int value = toggledIndex + 1;
	if (value >= buttons.Size())
		value = 0;
	if (value < 0)
		value = buttons.Size() - 1;
	SetValue(graphicsState, value);
	return true;
}

// Sets color for the toggle-buttons/selections
void UIRadioButtons::SetSelectionsTextColor(Color color) {
	selectionsTextColor = color;
	for (int i = 0; i < buttons.Size(); ++i) {
		UIToggleButton * button = buttons[i];
		button->SetTextColors(color);
	}
}

void UIRadioButtons::OnStateAdded(GraphicsState* graphicsState, int state) {
	label->AddState(graphicsState, state, true);
}

/// Toggles appropriately.
void UIRadioButtons::SetValue(GraphicsState* graphicsState, int v)
{
	if (buttons.Size() == 0) // JIT creation?
		CreateChildren(graphicsState);

	if (v >= buttons.Size() || v < 0)
	{
		assert(false);
		return;
	}
	for (int i = 0; i < buttons.Size(); ++i)
	{
		bool value = i == v;
		if (graphicsState != nullptr)
			buttons[i]->SetToggled(value);
		else
			buttons[i]->SetToggledSilently(value);
	}
	toggledIndex = v;
}

/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
void UIRadioButtons::SetText(CTextr newText, bool force) {
	if (!childrenCreated)
		CreateChildren(nullptr);
	label->SetText(newText, force);
}
