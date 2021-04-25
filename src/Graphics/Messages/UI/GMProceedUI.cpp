// Emil Hedemalm
// 2020-08-02
// Clicking a button with Enter or pressing the A-button.

#include "GMProceedUI.h"

#include "UI/UserInterface.h"
#include "UI/UIInputs.h"
#include "Window/AppWindowManager.h"
#include "File/LogFile.h"
#include "Message/MessageManager.h"

GMProceedUI::GMProceedUI()
	: GMUI(GM_PROCEED_UI) {
}
GMProceedUI::~GMProceedUI() {}

void GMProceedUI::Process(GraphicsState * gs) {

	bool ok = GetUI();
	if (!ok) {
		LogGraphics("No UI found in GMProceedUI", INFO);
		return;
	}

	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return;

	UIElement * stackTop = ui->GetStackTop();
	UIElement * activeElement = stackTop->GetElementByState(UIState::ACTIVE);

	// Don't add Active again if same element, then just de-activate it.
	if (hoverElement == activeElement ||
		hoverElement->HasStateRecursive(UIState::ACTIVE)) 
	{

		hoverElement->StopInput();

		stackTop->RemoveState(UIState::ACTIVE, true);
	}
	else {
		ui->GetStackTop()->RemoveState(UIState::ACTIVE, true);
		if (hoverElement->OnProceed(gs))
			return;

		hoverElement->AddState(gs, UIState::ACTIVE);
		// Remove active state immediately, as we want to continue navigating.
		switch (hoverElement->type) {
		case UIType::BUTTON:
			hoverElement->RemoveState(UIState::ACTIVE);
			break;
		}
		
	}
}

GMCancelUI::GMCancelUI() : GMUI(GM_CANCEL_UI) {
}
GMCancelUI::~GMCancelUI() {}
void GMCancelUI::Process(GraphicsState* graphicsState) {

	bool ok = GetUI();
	if (!ok) {
		LogGraphics("No UI found in GMProceedUI", INFO);
		return;
	}

	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return;

	UIElement * stackTop = ui->GetStackTop();
	UIElement * activeElement = stackTop->GetElementByState(UIState::ACTIVE);

	if (hoverElement == activeElement ||
		hoverElement->HasStateRecursive(UIState::ACTIVE))
	{
		hoverElement->StopInput();
		stackTop->RemoveState(UIState::ACTIVE, true);
	}
	else {
		/// Fetch active ui in stack.
		UIElement * element = ui->GetStackTop();
		if (element->exitable == false)
			return;
		/// Queue a message to remove it!
		ui->PopFromStack(graphicsState, element);

		// Remove active state from stuff in the new stack as well!
		ui->GetStackTop()->RemoveState(UIState::ACTIVE, true);

		// Post onExit only when exiting it via UICancel for example!
		MesMan.QueueMessages(element->onExit);
	}
}

