// Emil Hedemalm
// 2013-06-28

#include "MapState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Maps/2D/TileMap2D.h"
#include "EntityStates/StateProperty.h"
#include "Message/Message.h"
#include "RuneRPG/EntityStates/RREntityState.h"
#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "StateManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "Graphics/Camera/Camera.h"


void MapState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;


#define ASSERT_PLAYER {\
	if(!playerEntity || !playerEntity->state)\
		return;\
	}
	
	std::cout<<"\nMapState::InputProcessor";
	// Regular switch
	switch(action){
		case BEGIN_WALK_LEFT: {
			ASSERT_PLAYER
			playerState->ProcessMessage(new Message("WalkLeft"));
			return;
		}
		case BEGIN_WALK_RIGHT: {
			ASSERT_PLAYER
			playerState->ProcessMessage(new Message("WalkRight"));
			return;
		}
		case BEGIN_WALK_UP: {
			ASSERT_PLAYER
			playerState->ProcessMessage(new Message("WalkUp"));
			return;
		}
		case BEGIN_WALK_DOWN: {
			ASSERT_PLAYER
			playerState->ProcessMessage(new Message("WalkDown"));
			return;
		}
		case STOP_WALK_LEFT: {
			ASSERT_PLAYER;
			playerState->ProcessMessage(new Message("stopWalkLeft"));
			return;
		}
		case STOP_WALK_RIGHT: {
			ASSERT_PLAYER;
			playerState->ProcessMessage(new Message("stopWalkRight"));
			return;
		}
		case STOP_WALK_UP: {
			ASSERT_PLAYER;
			playerState->ProcessMessage(new Message("stopWalkUp"));
			return;
		}
		case STOP_WALK_DOWN: {
			ASSERT_PLAYER;
			playerState->ProcessMessage(new Message("stopWalkDown"));
			return;
		}
		case INTERACT: {
			/// Interact with ze environmeneeent.
		//	if (playerEntity && playerState){
		//		playerState->ProcessMessage(new Message("Interact"));
		//	}
			return;
		}
	/*	case STOP_WALK_LEFT: {
			assert(player && player->state);
			player->state->ProcessMessage(new Message("stopWalkLeft"));
			return;
		}
		case STOP_WALK_LEFT: {
			assert(player && player->state);
			player->state->ProcessMessage(new Message("stopWalkLeft"));
			return;
		}
	*/
		case INTERPRET_CONSOLE_COMMAND: {
#ifdef _DEBUG

			String command = Input.GetInputBuffer();

			// Check if null-string
			if (command == NULL)
				break;

			List<String> token = command.Tokenize(" ");
			int tokensFound = token.Size();
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if(command == "back" && enterMode == EnterMode::TESTING_MAP){
				ReturnToEditor();
			}
			else if (command.Contains("set movementSpeed")){
				if (tokensFound < 3)
					return;
				float newSpeed = token[2].ParseFloat();

				playerState->movementSpeed = newSpeed;
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
			switch(camera->trackingMode){
				case TrackingMode::FROM_BEHIND:
					camera->trackingMode = TrackingMode::THIRD_PERSON;
					break;
				case TrackingMode::THIRD_PERSON:
					camera->trackingMode = TrackingMode::FROM_BEHIND;
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
			Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			break;

		/// Different modes
		case LOAD_MODEL:
			std::cout<<"\nInput>>LOAD_MODEL";
			std::cout<<"\nIMPLEMENT";
            break;
		case LIST_MODELS:
			ModelMan.ListObjects();
			break;
		case LIST_TEXTURES:
			TexMan.ListTextures();
			break;
		case LIST_ENTITIES:
			MapMan.ListEntities();
			break;
		case LIST_ACTIONS: {
			Input.ListMappings();
			break;
		}
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
		camera->velocity = Vector3f(0, 0, 0);
		camera->rotationVelocity = Vector3f(0, 0, 0);
		break;
	/// Navigation
	case FORWARD: 	camera->Begin(Direction::FORWARD);		break;
	case FORWARD_S:	camera->End(Direction::FORWARD);		break;
	case BACKWARD: 	camera->Begin(Direction::BACKWARD);	break;
	case BACKWARD_S:camera->End(Direction::BACKWARD);		break;
	case LEFT:		camera->Begin(Direction::LEFT);		break;
	case LEFT_S:	camera->End(Direction::LEFT);			break;
	case RIGHT:		camera->Begin(Direction::RIGHT);		break;
	case RIGHT_S:	camera->End(Direction::RIGHT);			break;
	case UP:		camera->Begin(Direction::UP);			break;
	case UP_S:		camera->End(Direction::UP);			break;
	case DOWN:		camera->Begin(Direction::DOWN);		break;
	case DOWN_S:	camera->End(Direction::DOWN);			break;
	/// Rotation
	case TURN_LEFT:		camera->BeginRotate(Direction::LEFT); break;
	case TURN_LEFT_S:	camera->EndRotate(Direction::LEFT); break;
	case TURN_RIGHT:	camera->BeginRotate(Direction::RIGHT); break;
	case TURN_RIGHT_S:	camera->EndRotate(Direction::RIGHT); break;
	case TURN_UP:		camera->BeginRotate(Direction::UP); break;
	case TURN_UP_S:		camera->EndRotate(Direction::UP); break;
	case TURN_DOWN:		camera->BeginRotate(Direction::DOWN); break;
	case TURN_DOWN_S:	camera->EndRotate(Direction::DOWN); break;
	/// Camera Distance
	case COME_CLOSER:
		camera->distanceFromCentreOfMovement *= 0.8f;
		break;
	case BACK_AWAY:
		camera->distanceFromCentreOfMovement *= 1.25f;
		break;
	case ZOOM_IN: 		/// Zoom
		camera->zoom *= 1.25f;
		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		camera->zoom *= 0.8f;
		Graphics.UpdateProjection();
		break;
	case INCREASE_SPEED:
		camera->flySpeed *= 1.25f;
		break;
	case DECREASE_SPEED:
		camera->flySpeed *= 0.8f;
		break;
	case RESET_CAMERA:
		ResetCamera();
		break;
	default:
		std::cout<<"\nINFO: Default case for action: "<<action<<"!";
		break;
	}
}

