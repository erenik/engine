// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "Viewport.h"
#include "GraphicsMessages.h"
#include "UI/UserInterface.h"
#include "Window/AppWindow.h"

GMResize::GMResize(AppWindow * window, short width, short height) 
: GraphicsMessage(GM_RESIZE), height(height), width(width), window(window) 
{
};

void GMResize::Process(GraphicsState* graphicsState)
{
	// TODO: Move this to the render-code.
	/*
//    std::cout << "Width="<< width << " Height= " << height << " -> " << std::endl;
	// Make UI invisible <- Not needed since messages are processed in the graphics thread..!
//	UserInterface * currentUI = Graphics.GetUI();
//	Graphics.SetUI(NULL);

	// Set up glViewport
	glViewport(0, 0, Graphics.width, Graphics.height);
	if (glGetError() != GL_NO_ERROR)
		throw 3;
	// Update projection matrix
	Graphics.UpdateProjection();
	if (glGetError() != GL_NO_ERROR)
		throw 3;

	// Resize all UI
	UserInterface * ui = Graphics.GetUI();
	if (ui){
		if (ui->AdjustToWindow(Graphics.width, Graphics.height)){
			if (ui->IsBuffered())
				ui->Unbufferize();
			if (ui->IsGeometryCreated())
				ui->Bufferize();
		}
	}
	List<Viewport *> rvl = Graphics.GetViewports();
	for (int i = 0; i < rvl.Size(); ++i){
		Viewport * rv = rvl[i];
		/// Update aboslute-values of the viewport first.
		rv->AdjustToWindow(Graphics.width, Graphics.height);
		/// Then update it's internal UI
	//	ui = rv[i]->viewPortUI;

	//	rv[i]->
	}

	// Make UI visible again
//	Graphics.SetUI(currentUI);
	Graphics.renderQueried = true;
	*/
}
