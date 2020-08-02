// Emil Hedemalm
// 2020-08-02
// Clicking a button with Enter or pressing the A-button.

#include "GMProceedUI.h"

#include "UI/UserInterface.h"
#include "Window/AppWindowManager.h"
#include "File/LogFile.h"

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

	hoverElement->AddState(UIState::ACTIVE);
}
