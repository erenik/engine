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
#include "Window/AppWindowManager.h"
#include "Viewport.h"

GMSets::GMSets(int t, String s): GraphicsMessage(GM_SET_STRING) {
	this->str = s;
	this->target = t;
};

GMSet::GMSet(int target)
	: GraphicsMessage(GM_SET), target(target){}

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

GMSet::GMSet(int target, const Vector3f & value)
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

GMSet::GMSet(int t, bool bValue)
	: GraphicsMessage(GM_SET), target(t), bValue(bValue)
{
	switch(t)
	{
		case GT_RENDER_GRID:
		case GT_ANTIALIASING:
			break;
		default:
			assert(false);
	}
}

GMSet * GMSet::Antialiasing(bool bValue)
{
	GMSet * gt = new GMSet(GT_ANTIALIASING, bValue);
	return gt;
}
GMSet * GMSet::FarPlane(int newValue)
{
	GMSet * gt = new GMSet(GT_FAR_PLANE);
	gt->iValue = newValue;
	return gt;
}

void GMSet::Process(GraphicsState* graphicsState)
{
	switch(target)
	{
		case GT_ANTIALIASING:
			graphicsState->antialiasing = bValue;
			break;
		case GT_FAR_PLANE:
			graphicsState->farPlane = iValue;
			break;
		case GT_RENDER_GRID:
			WindowMan.MainWindow()->MainViewport()->renderGrid = bValue;
			break;
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

GMSeti::GMSeti(int target, int iValue)
	: GraphicsMessage(GM_SET_INTEGER), target(target), iValue(iValue)
{
	switch(target)
	{
		case GM_SET_OUT_OF_FOCUS_SLEEP_TIME:
			break;
		default:
			assert(false);
	}
}

void GMSeti::Process(GraphicsState* graphicsState)
{
	switch(target)
	{
		case GM_SET_OUT_OF_FOCUS_SLEEP_TIME:
			GraphicsMan.outOfFocusSleepTime = iValue;
			break;
	}
}


GMSetf::GMSetf(int target, float value): GraphicsMessage(GM_SET_FLOAT) {
	this->target = target;
	this->floatValue = value;
}

void GMSetf::Process(GraphicsState* graphicsState)
{	
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

void GMSets::Process(GraphicsState* graphicsState){
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

GMSetData::GMSetData(List<Vector3f> * targetList, List<Vector3f> newData)
	: GraphicsMessage(GM_SET_DATA), targetList(targetList), newData(newData)
{
}
void GMSetData::Process(GraphicsState* graphicsState)
{
	*targetList = newData;
}


GMSetGlobalUI::GMSetGlobalUI(UserInterface *ui, AppWindow * window)
: GraphicsMessage(GM_SET_GLOBAL_UI), ui(ui), window(window)
{
}

void GMSetGlobalUI::Process(GraphicsState* graphicsState)
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
	if (ui)
		ui->OnEnterScope();
}

/// Regular UI setter for the main AppWindow (Assumes 1 main AppWindow)
GMSetUI::GMSetUI(UserInterface * ui)
: GraphicsMessage(GM_SET_UI), ui(ui), window(WindowMan.MainWindow()), viewport(0)
{
}
// Regular UI setter per AppWindow.
GMSetUI::GMSetUI(UserInterface * ui, AppWindow * forWindow /*= NULL*/)
: GraphicsMessage(GM_SET_UI), ui(ui), window(forWindow), viewport(0)
{
}
// If viewport is unspecified (NULL) the global UI will be swapped.
GMSetUI::GMSetUI(UserInterface * ui, Viewport * i_viewport /* = NULL */)
: GraphicsMessage(GM_SET_UI), ui(ui), viewport(i_viewport)
{
}
void GMSetUI::Process(GraphicsState* graphicsState)
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
	// Default. Assume ui of main AppWindow?
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


void GMSetOverlay::Process(GraphicsState* graphicsState)
{
	if (tex)
		Graphics.SetOverlayTexture(tex, fadeInTimeInMs);
	else
		Graphics.SetOverlayTexture(textureName, fadeInTimeInMs);
}


GMSetResolution::GMSetResolution()
	: GraphicsMessage(GM_SET_RESOLUTION)
{
	Nullify();
}
GMSetResolution::GMSetResolution(Vector2i newRes, bool reqLock)
	: GraphicsMessage(GM_SET_RESOLUTION)
{
	Nullify();
	res = newRes; 
	lock = reqLock;
}
void GMSetResolution::Nullify()
{
	naturalRatio = false;
	staticRatio = false;
	lock = false;
}

// To set ratio to follow window, but without auto-scaling.
GMSetResolution * GMSetResolution::ToStaticRatio(Vector2f staticRatio)
{
	GMSetResolution * gm = new GMSetResolution();
	gm->staticRatio = true;
	gm->ratio = staticRatio;
	return gm;
}
GMSetResolution * GMSetResolution::ToNaturalRatio()
{
	GMSetResolution * gm = new GMSetResolution();
	gm->naturalRatio = true;
	return gm;
}

void GMSetResolution::Process(GraphicsState* graphicsState)
{
	if (naturalRatio)
	{
		graphicsState->renderResolution = MainWindow()->ClientAreaSize();
		graphicsState->resolutionLocked = false;
		return;
	}
	if (staticRatio)
	{
		graphicsState->relativeResolution = ratio;
		graphicsState->renderResolution = MainWindow()->ClientAreaSize() * ratio;
		graphicsState->resolutionLocked = false;
		return;
	}
	if (graphicsState->resolutionLocked && !lock)
		return;
	graphicsState->renderResolution = res * graphicsState->relativeResolution;
	graphicsState->resolutionLocked = lock;
}
