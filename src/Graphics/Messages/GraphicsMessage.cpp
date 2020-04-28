// Emil Hedemalm
// 2013-07-14

#include "InputState.h"
#include "UI/UI.h"
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
#include "Window/AppWindow.h"

#include "Render/RenderPipelineManager.h"
#include "Render/RenderPipeline.h"
#include "TextureManager.h"

#include "Window/AppWindowManager.h"
#include "Window/AppWindow.h"
#include "Render/FrameBuffer.h"
#include "Message/Message.h"
#include "File/LogFile.h"

int GraphicsMessage::defaultMaxRetryAttempts = 3;

GraphicsMessage::GraphicsMessage(int i_type)
{
	type = i_type;
	retry = false;
	maxRetryAttempts = GraphicsMessage::defaultMaxRetryAttempts;
	retryTimeout = Time::Milliseconds(1000);
}
GraphicsMessage::~GraphicsMessage(){
}

void GraphicsMessage::Process(GraphicsState* graphicsState)
{
	switch(type)
	{
		case GM_DUMP_FRAMEBUFFER_TEXTURES:
		{
			List<AppWindow*> windows = WindowMan.GetWindows();
			for (int i = 0; i < windows.Size(); ++i)
			{
				List<Viewport*> viewports = windows[i]->viewports;
				for (int i = 0; i < viewports.Size(); ++i)
				{
					Viewport * vp = viewports[i];
					if (vp->shadowMapDepthBuffer)
					{
						vp->shadowMapDepthBuffer->DumpTexturesToFile();
					}
				}
			}
			break;
		}
		case GM_PAUSE_PROCESSING:
		{
			GraphicsMan.pauseProcessing = true;
			break;
		}
		case GM_RESUME_PROCESSING:
		{
			GraphicsMan.pauseProcessing = false;
			break;
		}
		case GM_SHUTDOWN:
		{
			GraphicsMan.shouldLive = false;
			break;
		}
		case GM_CYCLE_RENDER_PIPELINE:
		{
			RenderPipeline * pipe = RenderPipeMan.Next();
			String name = "NULL";
			if (pipe != NULL)
				name = pipe->name;
			LogGraphics("Cycling to next render pipeline: " + name, INFO);
			graphicsState->renderPipe = pipe;
			break;
		}
		case GM_CYCLE_RENDER_PIPELINE_BACK:	
		{
			RenderPipeline * pipe = RenderPipeMan.Previous();
			String name = "NULL";
			if (pipe != NULL)
				name = pipe->name;
			LogGraphics("Cycling to previous render pipeline: " + name, INFO);
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
			AppWindow * activeWindow = ActiveWindow();
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
			RenderPipeMan.LoadFromPipelineConfig(graphicsState);
			break;
		case GM_CLEAR_OVERLAY_TEXTURE:
			Graphics.SetOverlayTexture(NULL);
			break;
		case GM_RELOAD_UI: 
		{
			PrepareForUIRemoval();
			/// Reload 'em all.
			UserInterface::ReloadAll();
			OnUIRemovalFinished();
			/// Inform game-states etc. that a reload has completed.
			MesMan.QueueMessages("OnReloadUI");
			break;
		}
		case GM_RELOAD_TEXTURES: 
			TexMan.ReloadTextures();
			TexMan.RebufferizeTextures();
			break;
		case GM_UNREGISTER_ALL_ENTITIES:
			Graphics.UnregisterAll();
			break;
		case GM_CLEAR_UI:
		{
			AppWindow * window = MainWindow();
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

GMMouse::GMMouse(int interaction, AppWindow * window, Vector2i coords)
	: GraphicsMessage(GM_MOUSE), window(window), coords(coords), interaction(interaction)
{}
GMMouse * GMMouse::Move(AppWindow * window, Vector2i coords)
{
	return new GMMouse(MOVE, window, coords);
}
GMMouse * GMMouse::LDown(AppWindow * window, Vector2i coords)
{
	return new GMMouse(LDOWN, window, coords);
}
GMMouse * GMMouse::RDown(AppWindow * window, Vector2i coords)
{
	return new GMMouse(RDOWN, window, coords);
}
GMMouse * GMMouse::LUp(AppWindow * window, Vector2i coords)
{
	return new GMMouse(LUP, window, coords);
}
GMMouse * GMMouse::RUp(AppWindow * window, Vector2i coords)
{
	return new GMMouse(RUP, window, coords);
}
void GMMouse::Process(GraphicsState * graphicsState)
{
	UserInterface * userInterface = GetRelevantUIForWindow(window);
	UIElement * element = NULL;
	if (userInterface)
	{
		switch(interaction)
		{
		case LUP:
		{
			UIElement * activeElement = NULL;
			activeElement = userInterface->GetActiveElement();
			if (activeElement)
			{ 
//				activeElement->RemoveState(UIState::ACTIVE);
				activeElement->Activate(graphicsState);
			}
			UIElement * hoverElement = userInterface->Hover(coords.x, coords.y, true);
			userInterface->SetHoverElement(graphicsState, hoverElement);
			break;
		}
		case LDOWN:
		{
			UIElement * activeElement = NULL;
			activeElement = userInterface->GetActiveElement();
			userInterface->Click(coords.x, coords.y);
			if (activeElement)
			{ 
				activeElement->RemoveState(UIState::ACTIVE);
			}
			break;
		}
		case MOVE:
			// If we had any active element since earlier, notify it of our mouse move.
			UIElement * activeElement = userInterface->GetActiveElement();
			if (activeElement)
				activeElement->OnMouseMove(coords);
			// If we have an active element, don't hover, to retain focus on the active element (e.g. the scroll-bar).
			else 
			{
				// Save old hover element...? wat
				UIElement * hoverElement = userInterface->Hover(coords.x, coords.y, true);
/*				if (printHoverElement)
				{
					std::cout<<"\nHoverElement: "<<(hoverElement? hoverElement->name.c_str() : "NULL");
				}*/
			}
			/// Inform app state of the movement perhaps?
			MesMan.QueueMessage(new MouseMessage(MOVE, window, coords));
//				AppState * currentState = StateMan.ActiveState();
	//		if (currentState)
		//	{
			//	currentState->MouseMove(appWindow, x, y, lButtonDown, rButtonDown, element);
		//	}
			break;
		}
	}
}

GMChar::GMChar(AppWindow * window, char c)
	: GraphicsMessage(GM_CHAR), window(window), c(c)
{
}
void GMChar::Process(GraphicsState * graphicsState)
{
	UserInterface * ui = GetRelevantUIForWindow(window);
	UIElement * element = NULL;
	if (ui)
	{
		UIElement * inputFocusElement = ui->ActiveInputFocusElement();
		// Catch the codes there that don't get caught in WM_CHAR?
		if (inputFocusElement)
		{
			/// Use the result somehow to determine if other actions can be triggered, too.
			int result = inputFocusElement->OnChar(c);
			return;
		}
	}
}

GMKey::GMKey(AppWindow * window, int keyCode, bool down, bool downBefore)
	: GraphicsMessage(GM_KEY), window(window), keyCode(keyCode), down(down), up(!down), downBefore(downBefore)
{
}

GMKey * GMKey::Down(AppWindow * window, int keyCode, bool downBefore)
{
	return new GMKey(window, keyCode, true, downBefore);
}
GMKey * GMKey::Up(AppWindow * window, int keyCode)
{
	return new GMKey(window, keyCode, false, false);
}

void GMKey::Process(GraphicsState* graphicsState)
{
	/// Key down.
	if (down)
	{
		UserInterface * ui = RelevantUI();
		UIElement * activeElement = NULL;
		if (ui)
		{
			UIElement * inputFocusElement = RelevantUI()->ActiveInputFocusElement();
			// Catch the codes there that don't get caught in WM_CHAR?
			if (inputFocusElement)
			{
				activeElement = inputFocusElement;
				/// Use the result somehow to determine if other actions can be triggered, too.
				int result = inputFocusElement->OnKeyDown(graphicsState, keyCode, false);
				Graphics.QueryRender();
			}
		}
	}
	/// Key up.
}


GMRecordVideo::GMRecordVideo(AppWindow * fromWindow)
: GraphicsMessage(GM_RECORD_VIDEO), window(fromWindow)
{
}

void GMRecordVideo::Process(GraphicsState * graphicsState)
{
	window->recordVideo = !window->recordVideo;
}


GMDeleteVBOs::GMDeleteVBOs(UserInterface * ui) : GraphicsMessage(GM_DELETE_VBOS){
	assert(ui->IsBuffered());
	this->ui = ui;
}
void GMDeleteVBOs::Process(GraphicsState * graphicsState){
	ui->Unbufferize();
}

// Unbuffers and deletes (i.e. all data) related to a UserInterface-object, including the object itself!
GMDelete::GMDelete(UserInterface * ui) 
	: GraphicsMessage(GM_DELETE_UI)
{
	this->ui = ui;
}
void GMDelete::Process(GraphicsState * graphicsState)
{
	if (ui->IsBuffered())
		ui->Unbufferize();
	if (ui->IsGeometryCreated())
		ui->DeleteGeometry();
	delete ui;
}
