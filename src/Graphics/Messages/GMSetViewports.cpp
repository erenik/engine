// Emil Hedemalm
// 2013-06-09

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "Viewport.h"
#include "GraphicsMessages.h"
#include "UI/UserInterface.h"
#include "Window/AppWindow.h"

GMSetViewports::GMSetViewports(List<Viewport*> i_viewports, AppWindow * inWindow)
: GraphicsMessage(GM_SET_VIEWPORTS), viewports(i_viewports), window(inWindow)
{
	bool viewportFound = false;
	for (int i = 0; i < viewports.Size(); ++i)
	{
		Viewport * viewport = viewports[i];
		if (viewport == NULL)
		{
			assert(false && "NULL viewport. What are you doing?");
		}
	}
	if (viewports.Size() == 0)
	{
		std::cout<<"Always send at least one viewport or gtfo!";
		assert(false && "Bad programmer! At least 1 viewport shoudl be sent always!");
	}
};

GMSetViewports::~GMSetViewports(){
}

void GMSetViewports::Process()
{
	// Unbuffer old viewports as needed.
	for (int i = 0; i < window->viewports.Size(); ++i)
	{
		Viewport * rv = window->viewports[i];
		UserInterface * ui = rv->ui;
		if (ui){
			ui->Unbufferize();
			ui->DeleteGeometry();
		}
		/// Remove it from window.
		window->viewports.Remove(rv);
		rv->window = NULL;
	}

	window->viewports.ClearAndDelete();
	
	// Buffer new UIs if applicable
	for (int i = 0; i < viewports.Size(); ++i)
	{
		Viewport * rv = viewports[i];
		/// Update size based on window they reside in.
		rv->window = window;
		rv->UpdateSize();
		window->viewports.Add(rv);
		if (!rv)
			continue;
		if (rv->uiSource && rv->ui == NULL)
		{
			UserInterface * ui = new UserInterface();
			ui->Load(rv->uiSource);
			ui->CreateGeometry();
			ui->AdjustToWindow(rv->size);
			ui->Bufferize();
			rv->ui = ui;
		}
	}
}
