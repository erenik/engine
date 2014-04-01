// Emil Hedemalm
// 2013-07-14

#include "GraphicsMessage.h"
#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "UI/UserInterface.h"
#include "Texture.h"
#include "Graphics/Render/RenderViewport.h"
#include "GraphicsMessages.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Render/RenderFrustum.h"
#include "Input/InputManager.h"
#include "OS/Sleep.h"
#include "Message/MessageManager.h"

GraphicsMessage::GraphicsMessage(int i_type){
	type = i_type;
}
GraphicsMessage::~GraphicsMessage(){
}

void GraphicsMessage::Process(){
	switch(type){
	    case GM_RENDER_FRUSTUM: {
	        Graphics.renderShapes.ClearAndDelete();
            Frustum frustum = Graphics.cameraToTrack->GetFrustum();
            RenderFrustum * rf = new RenderFrustum(frustum);
            Graphics.renderShapes.Add(rf);
            break;
        }
        case GM_RESET_CAMERA:
            *Graphics.cameraToTrack = Camera();
            break;
		case GM_PAUSE_RENDERING:
			Graphics.renderingEnabled = false;
			break;
		case GM_RESUME_RENDERING:
			Graphics.renderingEnabled = true;
			break;
		case GM_RECOMPILE_SHADERS:
			Graphics.shadeMan.RecompileAllShaders();
			break;
		case GM_CLEAR_OVERLAY_TEXTURE:
			Graphics.SetOverlayTexture(NULL);
			break;
		case GM_RELOAD_UI: 
		{
			Input.acceptInput = false;
			Sleep(10);

			/// Reset global UI
		//	Graphics.SetGlobalUI(NULL);

			GameState * state = StateMan.ActiveState();
			UserInterface * ui = state->GetUI();
			bool wasActive = (ui == Graphics.GetGlobalUI());
			if (ui){
				if (ui->IsBuffered())
					ui->Unbufferize();
				if (ui->IsGeometryCreated())
					ui->DeleteGeometry();
			}
			state->CreateUserInterface();

			
			ui = state->GetUI();
			std::cout<<"\nRecreating ui: "<<ui->Source();
			/// Always re-adjust to window and re-create geometry/re-bufferize
			ui->AdjustToWindow(Graphics.width, Graphics.height);
			ui->CreateGeometry();
			ui->ResizeGeometry();
			ui->Bufferize();

			/// Set the ui again, since the state probably de-allocated and re-allocated it.
			Graphics.SetGlobalUI(ui);

			// Update for all render-viewports UIs too!
			for (int i = 0; i < Graphics.renderViewports.Size(); ++i){
				if (!Graphics.renderViewports[i]->GetUI())
					continue;
				ui = Graphics.renderViewports[i]->GetUI();
				ui->Unbufferize();
				ui->DeleteGeometry();
				ui->Load(ui->Source());
				ui->AdjustToWindow(Graphics.width, Graphics.height);
				ui->CreateGeometry();
				ui->ResizeGeometry();
				ui->Bufferize();
			}
			Input.acceptInput = true;
			Graphics.renderQueried = true;
			/// Inform game-states etc. that a reload has completed.
			MesMan.QueueMessages("OnReloadUI");
			break;
		}
		case GM_UNREGISTER_ALL_ENTITIES:
			Graphics.UnregisterAll();
			break;
		case GM_CLEAR_UI:
			Graphics.SetGlobalUI(NULL);
			break;
		default:
			std::cout<<"\nERROR: Unhandled message in GraphicsMessage::Process with ID/type: "<<type;
//			assert(false && "Unhandled message in GraphicsMessage::Process");
			break;
	}
}

GMDeleteVBOs::GMDeleteVBOs(UserInterface * ui) : GraphicsMessage(GM_DELETE_VBOS){
	assert(ui->IsBuffered());
	this->ui = ui;
}
void GMDeleteVBOs::Process(){
	ui->Unbufferize();
}

// Unbuffers and deletes (i.e. all data) related to a UserInterface-object, including the object itself!
GMDelete::GMDelete(UserInterface * ui) : GraphicsMessage(GM_DELETE_UI){
	assert(ui->IsCreated());
	assert(ui->IsBuffered());
	this->ui = ui;
}
void GMDelete::Process(){
	ui->Unbufferize();
	ui->DeleteGeometry();
	delete ui;
}
