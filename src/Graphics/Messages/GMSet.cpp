// Emil Hedemalm
// 2013-06-28

#include "Graphics/GraphicsManager.h"
#include "GMSet.h"
#include "GraphicsMessages.h"
#include "UI/UserInterface.h"
#include "GraphicsState.h"
#include "../RenderSettings.h"
#include "Input/InputManager.h"
#include "TextureManager.h"
#include "Window/WindowManager.h"
#include "Viewport.h"

GMSets::GMSets(int t, String s): GraphicsMessage(GM_SET_STRING) {
	this->str = s;
	this->target = t;
};


GMSet::GMSet(int target, float floatValue)
: GraphicsMessage(GM_SET), target(target), floatValue(floatValue)
{
	switch(target){
		case GT_FOG_BEGIN:
		case GT_FOG_END:
			break;
		default:
			assert(false && "Bad target in GMSet");
			break;
	}
}

GMSet::GMSet(int target, Vector3f value)
: GraphicsMessage(GM_SET), target(target), vec3fValue(value)
{
    switch(target){
        case GT_CLEAR_COLOR:
            break;
        default:
            assert(false && "Bad target om GMSet");
            break;
    }
}

GMSet::GMSet(int t, void * data)
: GraphicsMessage(GM_SET), target(t), pData(data)
{
	switch(t) {
		case GT_MAIN_CAMERA:
		case GT_OVERLAY_TEXTURE:
		case GT_ACTIVE_2D_MAP_TO_RENDER:
			break;
		default:
			assert(false);
	}
};

void GMSet::Process()
{
	GraphicsState * graphicsState = Graphics.graphicsState;
	switch(target)
	{
		case GT_MAIN_CAMERA:
			Graphics.cameraToTrack = (Camera*)pData;
			break;
		case GT_FOG_BEGIN:
			graphicsState->fogBegin = floatValue;
			break;
		case GT_FOG_END:
			graphicsState->fogEnd = floatValue;
			break;
	    case GT_CLEAR_COLOR:
            graphicsState->clearColor = vec3fValue;
            break;
		case GT_ACTIVE_USER_INTERFACE:
		{
			assert(false && "Deprecated?");
			/*
			/// Notify the previous ui that they are going out of scope.
			UserInterface * previousUI = Graphics.GetUI();
			if (previousUI)
				previousUI->OnExitScope();

			UserInterface * ui = (UserInterface*) pData;
			Graphics.ui = ui;
			if (ui == NULL)
				return;
			// Activate things as necessary.
			ui->OnEnterScope();
			bool needToResize = ui->AdjustToWindow(Graphics.width, Graphics.height);
			// If we haven't created the geoms, do it and buffer it straight away
			if (!ui->IsGeometryCreated()){
				ui->CreateGeometry();
				ui->Bufferize();
			}
			// If not, check if we need to resize and re-buffer the geoms
			else if (needToResize){
				if (ui->IsBuffered())
					ui->Unbufferize();
				ui->ResizeGeometry();
				ui->Bufferize();
			}
			// Notify the input-manager that a new UI is active, so that it may update it's input-thingies
			Input.OnSetUI(ui);
			// No bufferization was needed apparantly ^^
			break;
			*/
		}
		case GT_OVERLAY_TEXTURE: {
			Texture * t = (Texture*) pData;
			Graphics.SetOverlayTexture(t);
			break;
		}
		case GT_ACTIVE_2D_MAP_TO_RENDER: {
			TileMap2D * map = (TileMap2D*) pData;
			Graphics.mapToRender = map;
			break;
		}
		default:
			assert(false && "Bad target in GMSet(GM_SET)");
			break;
	}
}


GMSetf::GMSetf(int target, float value): GraphicsMessage(GM_SET_FLOAT) {
	this->target = target;
	this->floatValue = value;
}

void GMSetf::Process()
{	
	GraphicsState * graphicsState = Graphics.graphicsState;
	switch (target){
		case GT_GRID_SPACING: {
			graphicsState->gridSpacing = floatValue;
			break;
		}
		case GT_GRID_SIZE: {
			graphicsState->gridSize = (int)floatValue;
			break;
		}
		default: {
			assert(false && "Bad target in GMSetf()");
		}

	}
}

void GMSets::Process(){
	switch(target){
		case GT_OVERLAY_TEXTURE: {
			Graphics.SetOverlayTexture(str);
			break;
		}
		default: {
			assert(false && "Bad target in GMSets()");
			break;
		}
	}

}


GMSetGlobalUI::GMSetGlobalUI(UserInterface *ui, Window * window)
: GraphicsMessage(GM_SET_GLOBAL_UI), ui(ui), window(window)
{
}

void GMSetGlobalUI::Process()
{
	/// Allow single windows for all projects with them no more.
	if (!window)
	{
		window = WindowMan.MainWindow();
	}
	UserInterface * oldGlobalUI = window->GetGlobalUI();
	// Re-setting same?
	if (oldGlobalUI == ui)
		return;
	if (oldGlobalUI)
	{
		oldGlobalUI->OnExitScope();
		// Unbufferize it
		if (oldGlobalUI->IsBuffered())
			oldGlobalUI->Unbufferize();
		if (oldGlobalUI->IsGeometryCreated())
			oldGlobalUI->DeleteGeometry();
	}
	window->globalUI = ui;
	ui->OnEnterScope();
}

/// Regular UI setter for the main window (Assumes 1 main window)
GMSetUI::GMSetUI(UserInterface * ui)
: GraphicsMessage(GM_SET_UI), ui(ui), window(WindowMan.MainWindow()), viewport(0)
{
}
// Regular UI setter per window.
GMSetUI::GMSetUI(UserInterface * ui, Window * forWindow /*= NULL*/)
: GraphicsMessage(GM_SET_UI), ui(ui), window(forWindow), viewport(0)
{
}
// If viewport is unspecified (NULL) the global UI will be swapped.
GMSetUI::GMSetUI(UserInterface * ui, Viewport * i_viewport /* = NULL */)
: GraphicsMessage(GM_SET_UI), ui(ui), viewport(i_viewport)
{
}
void GMSetUI::Process()
{
	/// First remove the old UI.
	UserInterface * oldUI = NULL;
	if (window)
	{
		oldUI = window->GetUI();		
	}
	else if (viewport)
	{
		oldUI = viewport->ui;
	}
	// Default. Assume ui of main window?
	else {
		window = WindowMan.MainWindow();
		oldUI = window->GetUI();
	}
	// Make sure its not re-setting. Skip it if so.
	if (oldUI == ui)
	{
		std::cout<<"\nSetting same UI. Doing nothing then!";
		return;
	}
	/// Remove references to old ui and enter the new one.
	if (window)
	{
		window->ui = ui;
	}
	else if (viewport)
	{
		viewport->ui = ui;
	}
	/// And finally remove the old one.
	if (oldUI)
	{
		oldUI->OnExitScope();
		/// Unbufferize it too as needed.
		if (oldUI->IsBuffered())
			oldUI->Unbufferize();
		if (oldUI->IsGeometryCreated())
			oldUI->DeleteGeometry();
//		delete oldUI;
	}

	/// If ui..
	if (ui)
		ui->OnEnterScope();

	Graphics.renderQueried = true;
}


GMSetOverlay::GMSetOverlay(String textureName, int fadeInTimeInMs)
: GraphicsMessage(GM_SET_OVERLAY), textureName(textureName), fadeInTimeInMs(fadeInTimeInMs)
{	
	tex = 0;
}

GMSetOverlay::GMSetOverlay(Texture * tex, int fadeInTimeInMs)
: GraphicsMessage(GM_SET_OVERLAY), tex(tex), fadeInTimeInMs(fadeInTimeInMs)
{
}


void GMSetOverlay::Process()
{
	if (tex)
		Graphics.SetOverlayTexture(tex, fadeInTimeInMs);
	else
		Graphics.SetOverlayTexture(textureName, fadeInTimeInMs);
}
