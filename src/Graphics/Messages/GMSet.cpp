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

GMSets::GMSets(int t, String s): GraphicsMessage(GM_SET_STRING) {
	this->str = s;
	this->target = t;
};


GMSet::GMSet(int target, float floatValue)
: GraphicsMessage(GM_SET), target(target), floatValue(floatValue)
{
	switch(target){
		case FOG_BEGIN:
		case FOG_END:
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
        case CLEAR_COLOR:
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
		case MAIN_CAMERA:
		case OVERLAY_TEXTURE:
		case ACTIVE_2D_MAP_TO_RENDER:
			break;
		default:
			assert(false);
	}
};

void GMSet::Process(){
	switch(target){
		case MAIN_CAMERA:
			Graphics.cameraToTrack = (Camera*)pData;
			break;
		case FOG_BEGIN:
			Graphics.renderSettings->fogBegin = floatValue;
			break;
		case FOG_END:
			Graphics.renderSettings->fogEnd = floatValue;
			break;
	    case CLEAR_COLOR:
            Graphics.renderSettings->clearColor = vec3fValue;
            break;
		case ACTIVE_USER_INTERFACE:{
			/// Notify the previous ui that they are going out of scope.
			UserInterface * previousUI = Graphics.globalUI;
			if (previousUI)
				previousUI->OnExitScope();

			UserInterface * ui = (UserInterface*) pData;
			Graphics.globalUI = ui;
			if (ui == NULL)
				return;
			// Activate things as necessary.
			Graphics.globalUI->OnEnterScope();
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
		}
		case OVERLAY_TEXTURE: {
			Texture * t = (Texture*) pData;
			Graphics.SetOverlayTexture(t);
			break;
		}
		case ACTIVE_2D_MAP_TO_RENDER: {
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

void GMSetf::Process(){
	switch (target){
		case GRID_SPACING: {
			Graphics.graphicsState->gridSpacing = floatValue;
			break;
		}
		case GRID_SIZE: {
			Graphics.graphicsState->gridSize = (int)floatValue;
			break;
		}
		default: {
			assert(false && "Bad target in GMSetf()");
		}

	}
}

void GMSets::Process(){
	switch(target){
		case OVERLAY_TEXTURE: {
			Graphics.SetOverlayTexture(str);
			break;
		}
		default: {
			assert(false && "Bad target in GMSets()");
			break;
		}
	}

}


// If viewport is unspecified (-1) the global UI will be swapped.
GMSetUI::GMSetUI(UserInterface * i_ui, int i_viewport /* = -1 */)
: GraphicsMessage(GM_SET_UI), ui(i_ui), viewport(i_viewport)
{
	
}
void GMSetUI::Process()
{
	/// First remove the old UI.
	switch(viewport)
	{
		/// Global UI
		case -1: 
		{
			UserInterface * oldUI = Graphics.globalUI;
			if (oldUI)
			{
				oldUI->OnExitScope();
				/// Unbufferize it too as needed.
				if (oldUI->IsBuffered())
					oldUI->Unbufferize();
				if (oldUI->IsGeometryCreated())
					oldUI->DeleteGeometry();
			}
			break;
		}
		default:
			assert(false && "Implement");
	}
	// If an empty UI, just enter it and then return.
	if (ui == NULL)
	{
		Graphics.globalUI = NULL;
		return;
	}
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

	// Assign it if all went well?
	switch(viewport){
		case -1:{
			Graphics.globalUI = ui;
			if (Graphics.globalUI)
				Graphics.globalUI->OnEnterScope();
			break;
		}
		default:
			assert(false && "Implement");
			break;
	}
	Graphics.renderQueried = true;
}


GMSetOverlay::GMSetOverlay(String textureName, int fadeInTimeInMs)
: GraphicsMessage(GM_SET_OVERLAY), textureName(textureName), fadeInTime(fadeInTimeInMs)
{
	
}
void GMSetOverlay::Process()
{
	Graphics.SetOverlayTexture(textureName, fadeInTime);
}
