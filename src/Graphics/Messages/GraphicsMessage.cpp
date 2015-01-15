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
		case GM_SHUTDOWN:
		{
			GraphicsMan.shouldLive = false;
			break;
		}
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
			
			/// Pause execution of the main thread, so that it doesn't try to access any dying UI elements while reloading.
			StateMan.Pause();

			/// Reloads 'em all.
			UserInterface::ReloadAll();

			StateMan.Resume();

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
GMDelete::GMDelete(UserInterface * ui) 
	: GraphicsMessage(GM_DELETE_UI)
{
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
