// Emil Hedemalm
// 2013-06-28

#include "RuneEditor.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Maps/2D/TileMap2D.h"
#include "Maps/Grids/TileTypeManager.h"
#include "RuneRPG/AppStates/Map/MapState.h"
#include "Entity/EntityProperty.h"
#include "RuneRPG/EntityStates/RREntityState.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "Maps/MapManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMLight.h"
#include "Message/MessageManager.h"

void RuneEditor::InputProcessor(int action, int inputDevice/* = 0*/)
{
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;
	switch(action){
		case RELOAD_TILES:
			ReloadTiles();
			break;
		case SET_MODE: case SET_MODE+1: case SET_MODE+2: case SET_MODE+3: {
			int previousMode = editMode;
			editMode = action - SET_MODE;
			switch(editMode){
				case EditMode::TILES:
					brushType = tileBrushType;
					brushSize = tileBrushSize;
					break;
				case EditMode::TERRAIN:
					brushType = terrainBrushType;
					brushSize = terrainBrushSize;
					break;
			}
			OnEditModeUpdated(previousMode);
			break;
		}
		case SET_BRUSH_TYPE: {
			int newMode = brushType+1;
			if (newMode == MAX_BRUSH_TYPES)
				newMode = SQUARE;
			switch(editMode){
				case EditMode::TILES:
					tileBrushType = newMode; break;
				case EditMode::TERRAIN:
					terrainBrushType = newMode; break;
			}
			brushType = newMode;
			break;
		}
		case SET_BRUSH_SIZE: case SET_BRUSH_SIZE+1: case SET_BRUSH_SIZE+2: case SET_BRUSH_SIZE+3:
		case SET_BRUSH_SIZE+4: case SET_BRUSH_SIZE+5: case SET_BRUSH_SIZE+6: case SET_BRUSH_SIZE+7:
		case SET_BRUSH_SIZE+8: case SET_BRUSH_SIZE+9:{
			switch(editMode){
				case EditMode::TILES:
					brushSize = tileBrushSize = action - SET_BRUSH_SIZE;
					break;
				case EditMode::TERRAIN:
					brushSize = terrainBrushSize = action - SET_BRUSH_SIZE;
					break;
			}
			break;
		}
		/// Opening the general console for multi-purpose commands of more difficult nature
		case OPEN_CONSOLE:
			std::cout<<"\nOpening multi-purpose console: ";
			Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			break;
			/// Different modes
		/// Map save/loading
		case SAVE_MAP_PROMPT:
			/*
			std::cout<<"\nSave Map prompt: Enter file name. If no extension is added .map will be added. ";
			Input.EnterTextInputMode("SAVE_MAP");
			*/
			break;
	//	case CHECK_EXISTING_FILE:
	//		break;
		case SAVE_MAP: {
			std::cout<<"\nInput>>SAVE_MAP";
			assert(false && "Deprecated");
			/*
			String filename = Input.GetInputBuffer();
			SaveMap(filename);
			*/
			break; }
		case LOAD_MAP_PROMPT:
			std::cout<<"\nLoad Map prompt: ";
			MesMan.QueueMessages("OpenFileBrowser(\"Load map\",LoadMap,\".tmap\")");
			break;
		case LOAD_MAP: {
			/// Clear selection first!
			runeEditorSelection.Clear();
			Graphics.QueueMessage(new GMSetOverlay("img/loading_map.png"));
			std::cout<<"\nInput>>LOAD_MAP";
			String filename = Input.GetInputBuffer();
			Map * loadedMap = MapMan.LoadMap(filename.c_str());
			// Set map to be active!
			if (loadedMap)
				MapMan.MakeActive(loadedMap);
			Graphics.QueueMessage(new GMSetOverlay(NULL));
			break;}
		case LOAD_MODEL:
			std::cout<<"\nInput>>LOAD_MODEL";
			std::cout<<"\nIMPLEMENT";
			break;
		case LIST_SELECTION: {
			runeEditorSelection.ListEntities();
			break;			/// Lists currently selected entities
		}
		case LIST_ACTIONS: {
			Input.ListMappings();
			break;
		}
	// ************************************************* //
	/// Selecting functions
	// ************************************************* //
		case SELECT_ALL: {		/// Selects all entities
			///  First deselect previous stuff
			runeEditorSelection.Clear();
			Entity ** list = NULL;
			int amount = MapMan.GetEntities(runeEditorSelection);
			OnSelectionUpdated();
			break;
		}
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
			runeEditorSelection.Clear();
			OnSelectionUpdated();
			break;
	// ************************************************* //
	/// Entity Creation
	// ************************************************* //
	// ************************************************* //
	/// Entity Deletion
	// ************************************************* //
		case DELETE_ENTITY_PROMPT: {
			std::cout<<"\nDelete entities prompt: ";
			int amount = runeEditorSelection.Size();
			if (amount == 0){
				std::cout<<"No entities selected, aborting.";
				break;
			}
			std::cout<<"All "<<amount<<" selected entities will be deleted. Are you sure? (yes/no)";
			Input.EnterTextInputMode("DELETE_ENTITY");
			break;
		}
		case DELETE_ENTITY: {
			/// Check input
			String reply = Input.GetInputBuffer();
			/// Compare to all valid replies
			reply.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (!reply.Contains("yes"))
				break;
			/// Delete them...
			runeEditorSelection.DeleteEntities();
			break;
		}
	// ************************************************** //
	/// Entity Physics
	// ************************************************** //
		case PAUSE_SIMULATIONS: {
			if (Physics.IsPaused()){
				Physics.Resume();
			}
			else {
				Physics.Pause();
			}
		}
	// ************************************************** //
	/// Camera
	// ************************************************** //
	case STOP:
		runeEditorCamera->velocity = Vector3f(0, 0, 0);
		runeEditorCamera->rotationVelocity = Vector3f(0, 0, 0);
		break;
	/// Navigation
	case FORWARD: 	runeEditorCamera->Begin(Direction::UP);		break;
	case FORWARD_S:	runeEditorCamera->End(Direction::UP);		break;
	case BACKWARD: 	runeEditorCamera->Begin(Direction::DOWN);	break;
	case BACKWARD_S:runeEditorCamera->End(Direction::DOWN);		break;
	case LEFT:		runeEditorCamera->Begin(Direction::LEFT);		break;
	case LEFT_S:	runeEditorCamera->End(Direction::LEFT);			break;
	case RIGHT:		runeEditorCamera->Begin(Direction::RIGHT);		break;
	case RIGHT_S:	runeEditorCamera->End(Direction::RIGHT);			break;
	case UP:		runeEditorCamera->Begin(Direction::UP);			break;
	case UP_S:		runeEditorCamera->End(Direction::UP);			break;
	case DOWN:		runeEditorCamera->Begin(Direction::DOWN);		break;
	case DOWN_S:	runeEditorCamera->End(Direction::DOWN);			break;
	/// Rotation
/*	case TURN_LEFT:		runeEditorCamera->BeginRotate(Direction::LEFT); break;
	case TURN_LEFT_S:	runeEditorCamera->EndRotate(Direction::LEFT); break;
	case TURN_RIGHT:	runeEditorCamera->BeginRotate(Direction::RIGHT); break;
	case TURN_RIGHT_S:	runeEditorCamera->EndRotate(Direction::RIGHT); break;
	case TURN_UP:		runeEditorCamera->BeginRotate(Direction::UP); break;
	case TURN_UP_S:		runeEditorCamera->EndRotate(Direction::UP); break;
	case TURN_DOWN:		runeEditorCamera->BeginRotate(Direction::DOWN); break;
	case TURN_DOWN_S:	runeEditorCamera->EndRotate(Direction::DOWN); break;
	*/
	/// Camera Distance
	case COME_CLOSER:
		runeEditorCamera->distanceFromCentreOfMovement *= 0.8f;
		break;
	case BACK_AWAY:
		runeEditorCamera->distanceFromCentreOfMovement *= 1.25f;
		break;
	case ZOOM_IN: 		/// Zoom
		runeEditorCamera->zoom *= 1.25f;
//		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		runeEditorCamera->zoom *= 0.8f;
//		Graphics.UpdateProjection();
		break;
	case INCREASE_SPEED:
		runeEditorCamera->flySpeed *= 1.25f;
		break;
	case DECREASE_SPEED:
		runeEditorCamera->flySpeed *= 0.8f;
		break;
	case RESET_CAMERA:
		ResetCamera();
		/*
		RuneEditorCamera = Camera();
		runeEditorCamera->SetRatio(Graphics.width, Graphics.height);
		runeEditorCamera->Update();
		*/
		break;
	default:
		std::cout<<"\nINFO: Default case for action: "<<action<<"!";
		break;
	}
}
