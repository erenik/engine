// Emil Hedemalm
// 2021-04-25
// For Mouse input, messages sent to graphics thread for processing in the UI.

#include "GMMouse.h"

#include "GraphicsMessages.h"
#include "UI/UserInterface.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "UI/GMProceedUI.h" // For GMCancelUI

GMMouse::GMMouse(int interaction, AppWindow * window, Vector2i coords)
	: GraphicsMessage(GM_MOUSE), window(window), coords(coords), interaction(interaction)
{}
GMMouse * GMMouse::Move(AppWindow * window, Vector2i coords)
{
	return new GMMouse(MOVE, window, coords);
}
GMMouse * GMMouse::LDown(AppWindow * window, Vector2i coords)
{
	return new GMMouse(LDOWN, window, coords);
}
GMMouse * GMMouse::RDown(AppWindow * window, Vector2i coords)
{
	return new GMMouse(RDOWN, window, coords);
}
GMMouse * GMMouse::LUp(AppWindow * window, Vector2i coords)
{
	return new GMMouse(LUP, window, coords);
}
GMMouse * GMMouse::RUp(AppWindow * window, Vector2i coords)
{
	return new GMMouse(RUP, window, coords);
}
void GMMouse::Process(GraphicsState * graphicsState)
{
	UserInterface * userInterface = GetRelevantUIForWindow(window);
	UIElement * element = NULL;
	if (userInterface)
	{
		switch (interaction)
		{
		case LUP:
		{
			UIElement * activeElement = NULL;
			activeElement = userInterface->GetActiveElement();
			if (activeElement)
			{
				activeElement->Activate(graphicsState);
			}
			UIElement * hoverElement = userInterface->Hover(graphicsState, coords.x, coords.y, true);
			userInterface->SetHoverElement(graphicsState, hoverElement);

			MouseMessage* mm = new MouseMessage(LUP, window, coords);
			mm->element = hoverElement;
			MesMan.QueueMessage(mm);
			break;
		}
		case LDOWN:
		{
			UIElement * activeElement = NULL;
			activeElement = userInterface->GetActiveElement();
			userInterface->Click(graphicsState, coords.x, coords.y);
			if (activeElement)
			{
				activeElement->RemoveState(UIState::ACTIVE);
			}

			MouseMessage* mm = new MouseMessage(LDOWN, window, coords);
			mm->element = activeElement;
			MesMan.QueueMessage(mm);
			break;
		}
		case RDOWN:
		{
			UIElement * element = NULL;
			UserInterface * userInterface = RelevantUI();
			if (userInterface)
			{
				element = userInterface->Hover(graphicsState, coords.x, coords.y, true);
				if (element == nullptr)
					return;
				//QueueGraphics(new GMSetHoverUI(element->name));
			}
			// If navigating UI, interpret right-click as cancel/exit?
			//if (this->navigateUI && !down) {
			/// Only activate the cancel function if we are currently within a UI?
			if (element) {
				GMCancelUI cancelUI;
				cancelUI.Process(graphicsState);
				//this->UICancel(); // earlier sent a GMCancelUI from the InputManager
			}
			//}
			break;
		}
		case MOVE:
			// If we had any active element since earlier, notify it of our mouse move.
			UIElement * activeElement = userInterface->GetActiveElement();
			if (activeElement)
				activeElement->OnMouseMove(coords);
			// If we have an active element, don't hover, to retain focus on the active element (e.g. the scroll-bar).
			else
			{
				UIElement * stackTop = userInterface->GetStackTop();
				if (stackTop == nullptr)
					return;
				stackTop->RemoveState(UIState::HOVER, true);
				// Save old hover element...? wat
				UIElement * hoverElement = userInterface->Hover(graphicsState, coords.x, coords.y, true);
			}
			/// Inform app state of the movement perhaps?
			auto mm = new MouseMessage(MOVE, window, coords);
			mm->element = userInterface->GetHoverElement();
			MesMan.QueueMessage(mm);
			break;
		}
	}
}

