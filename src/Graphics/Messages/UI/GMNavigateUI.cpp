// Emil Hedemalm
// 2020-08-12
// Analog stick menu navigation

#include "GMNavigateUI.h"

#include "UI/UserInterface.h"
#include "Window/AppWindowManager.h"
#include "File/LogFile.h"
#include "UI/UIInputs.h"
#include "Direction.h"
#include "UI/Buttons/UIRadioButtons.h"

GMNavigateUI::GMNavigateUI(int direction)
	: GMUI(GM_NAVIGATE_UI)
{
	switch (direction) {
	case UP: navigateDirection = NavigateDirection::Up; break;
	case DOWN: navigateDirection = NavigateDirection::Down; break;
	case LEFT: navigateDirection = NavigateDirection::Left; break;
	case RIGHT: navigateDirection = NavigateDirection::Right; break;
	default:
		assert(false);

	}
}

GMNavigateUI::GMNavigateUI(NavigateDirection direction)
	: GMUI(GM_NAVIGATE_UI)
	, navigateDirection(direction)
{

}
GMNavigateUI::~GMNavigateUI(){

}

void GMNavigateUI::Process(GraphicsState* graphicsState)
{
	bool ok = GetUI();
	if (!ok) {
		LogGraphics("No UI found in GMProceedUI", INFO);
		return;
	}

	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement) {
		// No hover element? Find a suitable one.	
		UIElement * element = ui->GetStackTop()->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (element != nullptr)
			element->AddState(UIState::HOVER);
		return;
	}

	if (hoverElement->HasStateRecursive(UIState::ACTIVE)) {
		hoverElement->Navigate(navigateDirection);
		return;		
	}

	List<UIElement*> relevantElements = ui->GetRelevantElements();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * desiredElement;
	if (hoverElement == NULL)
	{
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i) {
			UIElement * e = relevantElements[i];
			if (e->posY > element->posY)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		if (!element) {
			if (hoverElement->type == UIType::RADIO_BUTTONS) {
				UIRadioButtons * radioButtons = (UIRadioButtons*)hoverElement;
				bool reacted =  radioButtons->OnNavigate(graphicsState, navigateDirection);
				if (reacted)
					return;
			}

			bool searchChildrenOnly = false;
			switch (navigateDirection) {
			case NavigateDirection::Down:
				desiredElement = hoverElement->GetDownNeighbour(nullptr, nullptr, searchChildrenOnly);
				break;
			case NavigateDirection::Up:
				desiredElement = hoverElement->GetUpNeighbour(nullptr, NULL, searchChildrenOnly);
				break;
			case NavigateDirection::Left:
				desiredElement = hoverElement->GetLeftNeighbour(nullptr, searchChildrenOnly);
				break;
			case NavigateDirection::Right:
				desiredElement = hoverElement->GetRightNeighbour(nullptr, searchChildrenOnly);
				break;

			default:
				assert(false && "Implement");
			}
			
			if (ui->IsNavigatable(desiredElement))
			{
				element = desiredElement;
			}
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;

	LogGraphics("Navigating fromn " + hoverElement->name + " to " + desiredElement->name, INFO);

	ui->SetHoverElement(graphicsState, element);
}
