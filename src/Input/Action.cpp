/// Emil Hedemalm
/// 2015-01-16
/// An action, or response of any kind, based on input from the user. 
/// Added in order to make the messaging system more flexible when it 
/// comes to triggering Key-bindings where some data other than text 
/// might need transport, or an action wants to be coded into a simple 
/// integer-based value for quicker processing.
///
/// It should also alleviate from re-coding similar tasks for all 
/// projects as the actions include processing code for when they 
/// activate to some extent. E.g. taking a screenshot.

#include "Action.h"

#include "OS/OSUtil.h"

#include "Application/Application.h"

#include "Message/MessageManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/FrameStatistics.h"

#include "Graphics/Camera/Camera.h"

#include "Window/AppWindow.h"
#include "Window/AppWindowManager.h"
#include "Viewport.h"
#include "File/LogFile.h"

#include "Multimedia/MultimediaManager.h"

#include "StateManager.h"

#include "Player/PlayerManager.h"

#include "TextureManager.h"

#include "Model/ModelManager.h"


extern int debug;

/// Empty.
Action::Action()
{
	Nullify();
	type = NONE;
}

/// Default of queueing the message into the Message-manager.
Action::Action(const String & msg)
{
	Nullify();
	type = QUEUE_STRING_MESSAGE;
	startAction = msg;
}

/// Sets standard values.
void Action::Nullify()
{
	activateOnRepeat = false;
}

/** Creates an action featuring both a start- and stop-trigger.
	The start- and stop-actions will feature a prefix "Start" and "Stop" respectively, followed by the action's actual name. E.g. StartMoveCameraRight, StopMoveCameraRight
*/
Action * Action::CreateStartStopAction(String action)
{
	Action * ssa = new Action();
	ssa->type = QUEUE_STRING_START_STOP;
	ssa->startAction = "Start" + action;
	ssa->stopAction = "Stop" + action;
	return ssa;
}

/// See enum at the top of Action.h. Pre-programmed engine functionality, mostly.
Action * Action::FromEnum(int id)
{
	Action * action = new Action();
	action->type = id;
	return action; 
}

/// Default action based on a single start trigger string.
Action * Action::FromString(String str, int flags)
{
	Action * action = new Action();
	action->type = QUEUE_STRING_MESSAGE;
	action->startAction = str;
	if (flags & ACTIVATE_ON_REPEAT)
		action->activateOnRepeat = true;
	return action;
}


/// Called when the action is to be triggered.
void Action::TriggerStart()
{
	AppWindow * activeWindow = ActiveWindow();
	Viewport * mainViewport = activeWindow ? activeWindow->MainViewport() : 0;
	switch(type)
	{
		case LIGHTEN_BACKGROUND:
			extern float backgroundFactor;
			backgroundFactor += 0.05f;
			std::cout<<"\nBackground factor: "<<backgroundFactor;
			break;
		case DARKEN_BACKGROUND:
			backgroundFactor -= 0.05f;
			std::cout<<"\nBackground factor: "<<backgroundFactor;
			break;
		case DEBUG_NEXT:
			++debug;
			std::cout<<"\nDebug set to "<<debug;
			break;
		case DEBUG_PREVIOUS:
			--debug;
			std::cout<<"\nDebug set to "<<debug;
			break;
		case QUEUE_STRING_MESSAGE:
		case QUEUE_STRING_START_STOP:
			MesMan.QueueMessages(startAction);
			break;
		case PRINT_SHADOW_MAPS:
		{
			GraphicsMan.QueueMessage(new GraphicsMessage(GM_DUMP_FRAMEBUFFER_TEXTURES));
			break;
		}
		case RELOAD_UI:
			std::cout<<"\nInput>>RELOAD_UI";
			GraphicsMan.QueueMessage(new GraphicsMessage(GM_RELOAD_UI));
			// Notify people of zis
			break;
		case RECOMPILE_SHADERS:
			std::cout<<"\nInput>>RECOMPILE_SHADERS";
			GraphicsMan.QueueMessage(new GraphicsMessage(GM_RECOMPILE_SHADERS));
			break;
		case RELOAD_TEXTURES:
			GraphicsMan.QueueMessage(new GraphicsMessage(GM_RELOAD_TEXTURES));
			break;
		case QUIT_APPLICATION:
			if (Application::queryOnQuit)
				MesMan.QueueMessages("Query(QuitApplication)");
			else
				MesMan.QueueMessages("QuitApplication");
			break;
		case RECORD_VIDEO:
			Graphics.QueueMessage(new GMRecordVideo(activeWindow));
			break;
		case COPY:
			OSUtil::Copy();			
			break;
		case PASTE:
			OSUtil::Paste();
			break;

		case CYCLE_ACTIVE_CAMERA:
		{
			Camera * c = CameraMan.NextCamera();
			std::cout<<"\nActive camera: "<<c->name;
			break;
		}
			// Pasted shit from /Input/Bindings/General.cpp
		case CYCLE_RENDER_PIPELINE:
		{
			Graphics.QueueMessage(new GraphicsMessage(GM_CYCLE_RENDER_PIPELINE));	
			break;
		}
		case CYCLE_RENDER_PIPELINE_BACK:
		{
			Graphics.QueueMessage(new GraphicsMessage(GM_CYCLE_RENDER_PIPELINE_BACK));	
			break;
		}
		case CLOSE_WINDOW:
		{
			if (activeWindow->IsMain())
				MesMan.QueueMessages("Query(QuitApplication)");
			else
				activeWindow->Hide();	
			break;
		}
		case OPEN_LIGHTING_EDITOR:
		{
			/// This assumes only one lighting setup is used.
			Lighting * activeLighting = Graphics.ActiveLighting();
			activeLighting->OpenEditorWindow();
			break;
		}
		case TOGGLE_RENDER_LIGHTS:
			mainViewport->renderLights = !mainViewport->renderLights;
			break;
		case TOGGLE_RENDER_PHYSICS:
			mainViewport->renderPhysics = !mainViewport->renderPhysics;
			break;
		case TOGGLE_RENDER_WAYPOINTS:
			mainViewport->renderNavMesh = !mainViewport->renderNavMesh;
			break;
		case PRINT_TO_FILE:
			std::cout<<"\nPrint to file command issued.";
			MultimediaMan.SaveCurrentFramesToFile();
			break;
		case RELOAD_MODELS:
			std::cout<<"\nInput>>RELOAD_MODELS";
			Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_MODELS));
			break;
  		case GO_TO_EDITOR:
			std::cout<<"\nInput>>GO_TO_EDITOR";
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
			break;
		case GO_TO_MAIN_MENU: {
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
			break;
		}
		case PRINT_FRAME_TIME:
		{
			FrameStats.QueuePrint();
            break;
		}
		case PRINT_PLAYER_INPUT_DEVICES: {
			PlayerMan.PrintPlayerInputDevices();
			break;
		}
		case LIST_TEXTURES: {
            TexMan.ListTextures();
            break;
        }
        case LIST_MODELS: {
            ModelMan.ListObjects();
            break;
        }
        case LIST_CAMERAS: {
        	Graphics.ListCameras();
        	break;
        }
        case PRINT_UI_TREE:{
			if (StateMan.ActiveState()->GetUI())
				StateMan.ActiveState()->GetUI()->Print();
            break;
        }
		case TOGGLE_FULL_SCREEN: 
		{
			LogMain("TOGGLE_FULL_SCREEN", INFO);
			AppWindow * window = WindowMan.GetCurrentlyActiveWindow();
			if (window)
				window->ToggleFullScreen();
			break;
		}
		default:
			assert(false && "Implement");
	}
}

/// Called when the action is to be triggered again. (e.g. OS calls of keys being held down for a pro-longed period of time).
void Action::TriggerRepeat()
{
	if (activateOnRepeat)
	{
		TriggerStart();	
	}
	// Some enum-listed messages may want to be repeatable.
	switch(type)
	{
		case PASTE:
			TriggerStart();
			break;
	}
}

/// Called when the action is to be triggered.
void Action::TriggerStop()
{
	switch(type)
	{
		case QUEUE_STRING_START_STOP:
			MesMan.QueueMessages(stopAction);
			break;
	}
}


