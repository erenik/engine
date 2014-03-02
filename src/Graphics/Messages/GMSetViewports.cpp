// Emil Hedemalm
// 2013-06-09

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "Graphics/Render/RenderViewport.h"
#include "GraphicsMessages.h"
#include "UI/UserInterface.h"

GMSetViewports::GMSetViewports(List<RenderViewport*> i_viewports)
: GraphicsMessage(GM_SET_VIEWPORTS), viewports(i_viewports){
};

GMSetViewports::~GMSetViewports(){
}

void GMSetViewports::Process(){
	// Unbuffer old viewports as needed.
	for (int i = 0; i < Graphics.renderViewports.Size(); ++i){
		RenderViewport * rv = Graphics.renderViewports[i];
		if (rv->viewPortUI){
			rv->viewPortUI->Unbufferize();
			rv->viewPortUI->DeleteGeometry();
		}
	}

	List<RenderViewport*> oldViewports = Graphics.renderViewports;
	oldViewports.ClearAndDelete();
	Graphics.renderViewports = oldViewports;
	
	// Put in new viewports.
	Graphics.renderViewports = viewports;

	// Buffer new UIs if applicable
	for (int i = 0; i < viewports.Size(); ++i){
		RenderViewport * rv = viewports[i];
		if (!rv)
			continue;
		if (rv->uiSource && rv->viewPortUI == NULL){
			UserInterface * ui = new UserInterface();
			ui->Load(rv->uiSource);
			ui->CreateGeometry();
			ui->AdjustToWindow((int) (rv->width * Graphics.Width()), (int)(rv->height * Graphics.Height()));
			ui->Bufferize();
			rv->viewPortUI = ui;
		}
	}
}
