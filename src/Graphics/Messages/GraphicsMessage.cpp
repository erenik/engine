// Emil Hedemalm
// 2013-07-14

#include "GraphicsMessage.h"
#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "UI/UserInterface.h"
#include "Texture.h"
#include "Viewport.h"
#include "GraphicsMessages.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Render/RenderFrustum.h"
#include "Input/InputManager.h"
#include "OS/Sleep.h"
#include "Message/MessageManager.h"
#include "GraphicsState.h"
#include "Window/Window.h"

#include "Render/RenderPipelineManager.h"
#include "Render/RenderPipeline.h"

GraphicsMessage::GraphicsMessage(int i_type){
	type = i_type;
}
GraphicsMessage::~GraphicsMessage(){
}

void GraphicsMessage::Process()
{
	GraphicsState * graphicsState = Graphics.graphicsState;
	switch(type)
	{
		case GM_CYCLE_RENDER_PIPELINE:
		{
			RenderPipeline * pipe = RenderPipeMan.Next();
			graphicsState->renderPipe = pipe;
			break;
		}
		case GM_CYCLE_RENDER_PIPELINE_BACK:	
		{
			RenderPipeline * pipe = RenderPipeMan.Previous();
			graphicsState->renderPipe = pipe;
			break;
		}
		case GM_RECORD_VIDEO: 
		{
			graphicsState->recording = !graphicsState->recording; 
			break;
		}
		case GM_PRINT_SCREENSHOT:
		{
			Window * activeWindow = ActiveWindow();
			activeWindow->saveScreenshot = true;
	//		graphicsState->promptScreenshot = true;
			break;
		}
	    case GM_RENDER_FRUSTUM: {
	        Graphics.renderShapes.ClearAndDelete();
            Frustum frustum = Graphics.cameraToTrack->GetFrustum();
            RenderFrustum * rf = new RenderFrustum(frustum);
            Graphics.renderShapes.Add(rf);
            break;
        }
        case GM_RESET_CAMERA:
            Graphics.cameraToTrack->Nullify();
            break;
		case GM_PAUSE_RENDERING:
			Graphics.renderingEnabled = false;
			break;
		case GM_RESUME_RENDERING:
			Graphics.renderingEnabled = true;
			break;
		case GM_RECOMPILE_SHADERS:
			ShadeMan.RecompileAllShaders();
			RenderPipeMan.LoadFromPipelineConfig();
			break;
		case GM_CLEAR_OVERLAY_TEXTURE:
			Graphics.SetOverlayTexture(NULL);
			break;
		case GM_RELOAD_UI: 
		{
			Input.acceptInput = false;
			Sleep(10);
			
			/// Reloads 'em all.
			UserInterface::ReloadAll();
/*
			/// Delete and re-create the globalUI
			UserInterface * globalUI = Graphics.GetGlobalUI();
			if (globalUI)
			{
				if (globalUI->IsBuffered())
					globalUI->Unbufferize();
				if (globalUI->IsGeometryCreated())
					globalUI->DeleteGeometry();
			}
			AppState * globalState = StateMan.GlobalState();
			if (globalState)
			{
				globalState->CreateUserInterface();
				Graphics.SetGlobalUI(globalState->GetUI());
			}

			/// Refresh current UI too.
			Graphics.SetUI(NULL);

			AppState * state = StateMan.ActiveState();
			UserInterface * ui = state->GetUI();
			bool wasActive = (ui == Graphics.GetUI());
			if (ui){
				if (ui->IsBuffered())
					ui->Unbufferize();
				if (ui->IsGeometryCreated())
					ui->DeleteGeometry();
			}
			state->CreateUserInterface();

			
			ui = state->GetUI();
			if (ui)
			{
				std::cout<<"\nRecreating ui: "<<ui->Source();
				/// Always re-adjust to window and re-create geometry/re-bufferize
				ui->AdjustToWindow(Graphics.width, Graphics.height);
				ui->CreateGeometry();
				ui->ResizeGeometry();
				ui->Bufferize();

				/// Set the ui again, since the state probably de-allocated and re-allocated it.
				Graphics.SetUI(ui);

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
			}
			*/
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
		{
			Window * window = MainWindow();
			UserInterface * ui = window->ui;
			if (ui)
			{
				ui->Unbufferize();
				ui->DeleteGeometry();
			}
			window->ui = NULL;
			break;
		}
		default:
			std::cout<<"\nERROR: Unhandled message in GraphicsMessage::Process with ID/type: "<<type;
//			assert(false && "Unhandled message in GraphicsMessage::Process");
			break;
	}
}

GMRecordVideo::GMRecordVideo(Window * fromWindow)
: GraphicsMessage(GM_RECORD_VIDEO), window(fromWindow)
{
}

void GMRecordVideo::Process()
{
	window->recordVideo = !window->recordVideo;
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
void GMDelete::Process()
{
	if (ui->IsBuffered())
		ui->Unbufferize();
	if (ui->IsGeometryCreated())
		ui->DeleteGeometry();
	delete ui;
}
