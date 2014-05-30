// Emil Hedemalm
// 2013-06-28

#include "RacingState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "../../SRPlayer.h"
#include "../../EntityStates/RacingShipGlobal.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Network/NetworkManager.h"
#include "../../Network/SRPacketTypes.h"
#include "Network/Packet/Packet.h"
#include "../../Network/SRPacket.h"

void Racing::InputProcessor(int action, int inputDevice){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;

	SRSession * srs = GetSession();
	List<SRPlayer*> players = GetPlayers();
	SRPlayer * player = NULL;
	int playerIndex = -1;
	/// Find player linked to this device, and also get index for networking
	for (int i = 0; i < players.Size(); ++i){
		if (!players[i]->isLocal)
			continue;
		if (players[i]->InputDevice() == inputDevice)
		{
			player = players[i];
			playerIndex = i;
		}
	}
	/// If we got a valid player, check stuff.
	if (player && player->entity){
		Entity * playerEntity = player->entity;
		RacingShipGlobal * rsg = (RacingShipGlobal*)playerEntity->state->GlobalState();
		bool noAction = false;
		/// Send the message straight to the player entity's global ship state!
		switch(action){
			case BEGIN_ACCELERATION: rsg->Accelerate(); break;
			case STOP_ACCELERATION: rsg->StopAccelerating();   break;
			case BEGIN_BREAKING:	rsg->Reverse();	break;
			case STOP_BREAKING:		rsg->StopReversing(); break;
			case BEGIN_TURNING_LEFT: rsg->TurnLeft(); break;
			case STOP_TURNING_LEFT:	rsg->StopTurnLeft(); break;
			case BEGIN_TURNING_RIGHT:	rsg->TurnRight(); break;
			case STOP_TURNING_RIGHT:	rsg->StopTurnRight();	break;
			case BEGIN_BOOST:		rsg->Boost();	break;
			case STOP_BOOST:		rsg->StopBoosting();	break;
			case RESET_POSITION:		rsg->ResetPosition();	break;
			case TOGGLE_AUTO_PILOT:		rsg->ToggleAI();	break;
			default: noAction = true;
		}
		if (!noAction){		
			/// If as action was taken, inform host of the new state.
			if (!srs->IsHost())
			{
				SRPlayerMovePacket movePacket(playerIndex);
				movePacket.SetMessage(rsg->GetStateAsString(false));
				srs->Send(&movePacket);	
			}
			return;
		}
	}

#define PLAYER_CAMERA (*cameras[0])

	// Regular switch
	switch(action){
		case TOGGLE_MENU: {
			ToggleMenu();
			break;
		}
		case SHOW_RESULTS: {
			FormatResults();
			Graphics.QueueMessage(new GMSetUIb("Results", GMUI::VISIBILITY, true));
			break;
		}
		case HIDE_RESULTS:{
			Graphics.QueueMessage(new GMSetUIb("Results", GMUI::VISIBILITY, false));
			break;
		}
		case INTERPRET_CONSOLE_COMMAND: {
#ifdef _DEBUG
			String command = Input.GetInputBuffer();

			// Check if null-string
			if (command == NULL)
				break;

			List<String> token = command.Tokenize(" ");
			int tokensFound = token.Size();
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (command.Contains("collissiontriangles")){
				Graphics.renderCollissionTriangles = ! Graphics.renderCollissionTriangles;
			}
			else if (command == "toggle backfaceculling"){
				Graphics.backfaceCullingEnabled = !Graphics.backfaceCullingEnabled;
			}
			else if (command == "toggle debug renders"){
				Graphics.EnableAllDebugRenders(!Graphics.renderGrid);
			}
			else if (command.Contains("render light") || command.Contains("toggle light")){
				Graphics.renderLights = !Graphics.renderLights;
			}
			else if (command == "toggle physics shapes"){
				Graphics.renderPhysics = !Graphics.renderPhysics;
			}
			else if (command == "toggle bfc"){
				Graphics.backfaceCullingEnabled = !Graphics.backfaceCullingEnabled;
			}
#endif
			break;
		}
		case GO_TO_MAIN_MENU:
			std::cout<<"\nInput>>GO_TO_MAIN_MENU";
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
			break;
		case CHANGE_CAMERA:
			switch(PLAYER_CAMERA.trackingMode){
				case TrackingMode::FROM_BEHIND:
					PLAYER_CAMERA.trackingMode = TrackingMode::THIRD_PERSON;
					break;
				case TrackingMode::THIRD_PERSON:
					PLAYER_CAMERA.trackingMode = TrackingMode::FROM_BEHIND;
					break;
			};
			break;

	/*
			/// Driving... :P
	BEGIN_ACCELERATION,	STOP_ACCELERATION,
	BEGIN_BREAKING,		STOP_BREAKING,
	BEGIN_TURNING_LEFT, STOP_TURNING_LEFT,
	BEGIN_TURNING_RIGHT, STOP_TURNING_RIGHT,

	/// Camera
	CHANGE_CAMERA,			/// Switch active camera
*/

		/// Opening the general console for multi-purpose commands of more difficult nature
		case OPEN_CONSOLE:
			std::cout<<"\nOpening multi-purpose console: ";
			// Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			break;

		/// Different modes
	// ************************************************* //
	/// Selecting functions
	// ************************************************* //
		case SELECT_ENTITY_PROMPT:	/// Begins prompt to select target entity/entities
			std::cout<<"\nIMPLEMENT";
			break;
		case SELECT_ENTITY:			/// Attempts to select specified entities
			std::cout<<"\nIMPLEMENT";
			break;
		case ADD_TO_SELECTION_PROMPT:/// Begins prompt to select more entities without deselecting the previous ones.
			std::cout<<"\nIMPLEMENT";
			break;
		case ADD_TO_SELECTION:
			std::cout<<"\nIMPLEMENT";
			break;
		case REMOVE_FROM_SELECTION_PROMPT:	///
			std::cout<<"\nIMPLEMENT";
			break;
		case REMOVE_FROM_SELECTION:	/// Attempts to remove from selection by parsing input
			std::cout<<"\nIMPLEMENT";
			break;
		case CLEAR_SELECTION:
			editorSelection.Clear();
			break;
	// ************************************************* //
	/// Entity Deletion
	// ************************************************* //
		case DELETE_ENTITY_PROMPT: {
			std::cout<<"\nDelete entities prompt: ";
			int amount = editorSelection.Size();
			if (amount == 0){
				std::cout<<"No entities selected, aborting.";
				break;
			}
			std::cout<<"All "<<amount<<" selected entities will be deleted. Are you sure? (yes/no)";
			Input.EnterTextInputMode("DELETE_ENTITY");
			break;
		}
	// ************************************************** //
	/// Entity Physics
	// ************************************************** //
		case TOGGLE_PHYSICS_SHAPES: {
			Graphics.renderPhysics = !Graphics.renderPhysics;
			break;
		}
		case PAUSE_SIMULATIONS: {
			if (Physics.IsPaused()){
				Physics.Resume();
		//		AI.Resume();
			}
			else {
				Physics.Pause();
		//		AI.Pause();
			}
		}
	// ************************************************** //
	/// Camera
	// ************************************************** //
	case STOP:
		PLAYER_CAMERA.velocity = Vector3f(0, 0, 0);
		PLAYER_CAMERA.rotationVelocity = Vector3f(0, 0, 0);
		break;
	/// Navigation
	case FORWARD: 	PLAYER_CAMERA.Begin(Direction::FORWARD);		break;
	case FORWARD_S:	PLAYER_CAMERA.End(Direction::FORWARD);		break;
	case BACKWARD: 	PLAYER_CAMERA.Begin(Direction::BACKWARD);	break;
	case BACKWARD_S:PLAYER_CAMERA.End(Direction::BACKWARD);		break;
	case LEFT:		PLAYER_CAMERA.Begin(Direction::LEFT);		break;
	case LEFT_S:	PLAYER_CAMERA.End(Direction::LEFT);			break;
	case RIGHT:		PLAYER_CAMERA.Begin(Direction::RIGHT);		break;
	case RIGHT_S:	PLAYER_CAMERA.End(Direction::RIGHT);			break;
	case UP:		PLAYER_CAMERA.Begin(Direction::UP);			break;
	case UP_S:		PLAYER_CAMERA.End(Direction::UP);			break;
	case DOWN:		PLAYER_CAMERA.Begin(Direction::DOWN);		break;
	case DOWN_S:	PLAYER_CAMERA.End(Direction::DOWN);			break;
	/// Rotation
	case TURN_LEFT:		PLAYER_CAMERA.BeginRotate(Direction::LEFT); break;
	case TURN_LEFT_S:	PLAYER_CAMERA.EndRotate(Direction::LEFT); break;
	case TURN_RIGHT:	PLAYER_CAMERA.BeginRotate(Direction::RIGHT); break;
	case TURN_RIGHT_S:	PLAYER_CAMERA.EndRotate(Direction::RIGHT); break;
	case TURN_UP:		PLAYER_CAMERA.BeginRotate(Direction::UP); break;
	case TURN_UP_S:		PLAYER_CAMERA.EndRotate(Direction::UP); break;
	case TURN_DOWN:		PLAYER_CAMERA.BeginRotate(Direction::DOWN); break;
	case TURN_DOWN_S:	PLAYER_CAMERA.EndRotate(Direction::DOWN); break;
	/// Camera Distance
	case COME_CLOSER:
		PLAYER_CAMERA.distanceFromCentreOfMovement *= 0.8f;
		break;
	case BACK_AWAY:
		PLAYER_CAMERA.distanceFromCentreOfMovement *= 1.25f;
		break;
	case ZOOM_IN: 		/// Zoom
		PLAYER_CAMERA.zoom *= 1.25f;
		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		PLAYER_CAMERA.zoom *= 0.8f;
		Graphics.UpdateProjection();
		break;
	case INCREASE_SPEED:
		PLAYER_CAMERA.flySpeed *= 1.25f;
		break;
	case DECREASE_SPEED:
		PLAYER_CAMERA.flySpeed *= 0.8f;
		break;
	case RESET_CAMERA:
		PLAYER_CAMERA = Camera();
		PLAYER_CAMERA.SetRatio(Graphics.width, Graphics.height);
		PLAYER_CAMERA.Update();
		break;
	default:
	///	std::cout<<"\nINFO: Default case for action: "<<action<<"!";
		break;
	}
}

// #endif
