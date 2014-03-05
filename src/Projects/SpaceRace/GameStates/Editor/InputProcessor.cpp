// Emil Hedemalm
// 2013-06-28

#include <cstring>
#include "EditorState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Graphics/Messages/GMSetEntity.h"
#include "OS/Sleep.h"
#include "Input/InputManager.h"
#include "Maps/MapManager.h"
#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Pathfinding/WaypointManager.h"
#include "Physics/PhysicsManager.h"
#include "Pathfinding/PathManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GMSet.h"

void EditorState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode()){
		std::cout<<"\nIs in text-entering mode..";
		return;
	}
	switch(action){
		case GO_TO_MAIN_MENU:
			std::cout<<"\nInput>>GO_TO_MAIN_MENU";
			StateMan.QueueState(GAME_STATE_MAIN_MENU);
			break;
		/// Opening the general console for multi-purpose commands of more difficult nature
		/// Different modes
		/// Map save/loading
	//	case CHECK_EXISTING_FILE:
	//		break;
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
		case LIST_SELECTION: {
			editorSelection.ListEntities();
			break;			/// Lists currently selected entities
		}
		case LIST_ACTIONS: {
			Input.ListMappings();
			break;
		}
		case LIST_DATA: {	/// Lists entity data for all selected entities.
			for (int i = 0; i < editorSelection.Size(); ++i){
				Entity * entity = editorSelection[i];
				std::cout<<"\nEntity "<<i<<" name: "<<entity->name;
				if (entity->model)
					std::cout<<"\nModel: "<<entity->model->Name();
				if (entity->GetTexture(DIFFUSE_MAP))
					std::cout<<"\nTexture: "<<entity->GetTexture(DIFFUSE_MAP)->name;
			}
		}
	// ************************************************* //
	/// Selecting functions
	// ************************************************* //
		case SELECT_ALL: {		/// Selects all entities
			///  First deselect previous stuff
			editorSelection.Clear();
			Entity ** list = NULL;
			int amount = MapMan.GetEntities(editorSelection);
			OnSelectionUpdated();
			break;
		}
		case SELECT_NEXT: {
			if (editorSelection.Size() > 1)
				return;
			// Fetch map's all entities, cull with frustum, then sort from left to right (from camera).
			Selection inView = MapMan.GetEntities();
			if (!inView.Size())
				return;
			inView = inView.CullByCamera(mainCamera);
			inView.SortByDistance(Vector3f(mainCamera->Position()));
			if (editorSelection.Size())
				editorSelection = inView.SelectNext(editorSelection[0]);
			else
				editorSelection = inView.SelectNext(NULL);
			if (editorSelection.Size())
				std::cout<<"\nSelecting next Entity: "<<editorSelection[0]<<" "<<editorSelection[0]->name;
			OnSelectionUpdated();
			break;
		}
		case SELECT_PREVIOUS: {
			if (editorSelection.Size() > 1)
				return;
			// Fetch map's all entities, cull with frustum, then sort from left to right (from camera).
			Selection inView = MapMan.GetEntities();
			inView = inView.CullByCamera(mainCamera);
			inView.SortByDistance(Vector3f(mainCamera->Position()));
			if (editorSelection.Size())
				editorSelection = inView.SelectPrevious(editorSelection[0]);
			else
				editorSelection = inView.SelectPrevious(NULL);
			std::cout<<"\nSelecting prev Entity: "<<editorSelection[0]<<" "<<editorSelection[0]->name;
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
			switch(editMode){
				case ENTITIES:{
					editorSelection.Clear();
					OnSelectionUpdated();
					break;
				}
				case NAVMESH:{
					waypointSelection.Clear();
					OnWaypointSelectionUpdated();
					break;
				}
			}

			break;
	// ************************************************* //
	/// Entity Creation
	// ************************************************* //
		case CREATE_ENTITY_PROMPT:
			std::cout<<"\nCreate Entity prompt: ";
			Input.EnterTextInputMode("CREATE_ENTITY");
			break;
		case CREATE_ENTITY: {
			String command = Input.GetInputBuffer();
			command.ConvertToChar();
			List<String> tokens = command.Tokenize(" ,");
			std::cout<<"\nCreating entity...";
			/// Parse
			Texture * texture = NULL;
			Model * model = NULL;
			// Process depending on how many numbers we got
			switch(tokens.Size()){
				case 2:	texture = TexMan.GetTextureByIndex(tokens[1].ParseInt());
				case 1: model =	ModelMan.GetModel(tokens[0].ParseInt());
			}
			if (model == NULL){
				std::cout<<"\nERROR: Bad model index!";
				return;
			}
			Entity * newEntity = MapMan.CreateEntity(model, texture);
			/// Check that creation as successful
			if (newEntity){
				editorSelection.Add(newEntity);
			}
			return;
		}
		case DUPLICATE_ENTITY: {
			if (editorSelection.Size() == 0)
				return;
			std::cout<<"\nDuplicating selected entities...";
			Selection newSelection;
			for (int i = 0; i < editorSelection.Size(); ++i){
				Entity * entity = editorSelection[i];
				Entity * newEntity = MapMan.CreateEntity(entity);
				assert(newEntity);
				newSelection.Add(newEntity);
			}
			editorSelection = newSelection;
			break;
		}
	// ************************************************* //
	/// Entity Deletion
	// ************************************************* //
		case DELETE_ENTITY_PROMPT: {
			}
		case DELETE_ENTITY: {
			/// Delete them...
			editorSelection.DeleteEntities();
			break;
		}
	// ************************************************* //
	/// Entity manipulation
	// ************************************************* //
		case TRANSLATE_ENTITY_PROMPT: { /// Begins prompt for entity translation
			if (editorSelection.Size() == 0)
				return;
			Entity * entity = editorSelection[0];
			std::cout<<"\nTranslate Entity prompt: Current translation: "<<entity->positionVector;
			Input.EnterTextInputMode("TRANSLATE_ENTITY");
			break;
		}
		case TRANSLATE_ENTITY: {		/// Translates active entity/entities
			float translation[3];
			if (Input.ParseFloats(translation, 3) == 3)
				TranslateActiveEntities(Vector3f(translation));
			break;
		}
		case RESET_ENTITY_SCALE:	/// Resets scale to 1.0 on all lengths
			std::cout<<"\nResetting scale for target entities...";
			SetScaleActiveEntities(Vector3f(1.0f,1.0f,1.0f));
			break;
		case SCALE_ENTITY_PROMPT: {	/// Begins prompt for entity scaling
			if (editorSelection.Size() == 0)
				return;
			Entity * entity = editorSelection[0];
			std::cout<<"\nScale Entity prompt: Current scale: "<<entity->scaleVector;
		//	Input.EnterTextInputMode("SCALE_ENTITY");
			break;
		}
		case SCALE_ENTITY: {		/// Scales active entity/entities
			float scale[3];
			int floatsParsed = Input.ParseFloats(scale, 3);
			if (floatsParsed == 3)
				ScaleActiveEntities(Vector3f(scale));
			else if (floatsParsed == 1)
				ScaleActiveEntities(Vector3f(scale[0], scale[0], scale[0]));
			break;
		}
		case ROTATE_ENTITY_PROMPT: {	/// Begins prompt for entity rotation
			if (editorSelection.Size() == 0)
				return;
			Entity * entity = editorSelection[0];
			std::cout<<"\nRotate Entity prompt: Current Rotation: "<<entity->rotationVector;
			Input.EnterTextInputMode("ROTATE_ENTITY");
			break;
		}
		case ROTATE_ENTITY: 			/// Rotates active entity/entities
			float rotation[3];
			if (Input.ParseFloats(rotation, 3) == 3)
				RotateActiveEntities(Vector3f(rotation));
			break;
		case SET_TEXTURE_PROMPT: 		/// Begins a prompt to set texture of the active entity
			if (editorSelection.Size() == 0)
				return;
			std::cout<<"\nSet Entity Texture prompt: ";
			if (editorSelection[0] && editorSelection[0]->GetTexture(DIFFUSE_MAP)){
				std::cout<<"Current texture: "<<editorSelection[0]->GetTexture(DIFFUSE_MAP)->name;
			}
			Input.EnterTextInputMode("SET_TEXTURE");
			break;
		case SET_TEXTURE: { 			/// Sets texture to target entity by parsing input-buffer
			String input = Input.GetInputBuffer();
			int index = input.ParseInt();
			String source = input;
			Texture * newTexture = NULL;
			if (index == 0)
				newTexture = TexMan.GetTextureByName(Input.GetInputBuffer());
			else
				newTexture = TexMan.GetTextureByIndex(index);
			if (!newTexture){
				std::cout<<"\nERROR: No Texture by given name \""<<Input.GetInputBuffer()<<"\" exists. Aborting procedure";
				return;
			}
			assert(newTexture);
			std::cout<<"\nSetting texture to: "<<newTexture->name;
			for (int i = 0; i < editorSelection.Size(); ++i){
				Graphics.QueueMessage(new GMSetEntityTexture(editorSelection[i], DIFFUSE_MAP, newTexture));
			}
			break;
		}
		case SET_MODEL_PROMPT: {
			if (editorSelection.Size() == 0)
				return;
			std::cout<<"\nSet Model prompt: ";
			if (editorSelection[0] && editorSelection[0]->model){
				std::cout<<"Current model: "<<editorSelection[0]->model->Name();
			}

			Input.EnterTextInputMode("SET_MODEL");
			break;
		}
		case SET_MODEL: {
			Model * newModel = ModelMan.GetModel(Input.GetInputBuffer());
			if (!newModel){
				std::cout<<"\nERROR: No Model by given name \""<<Input.GetInputBuffer()<<"\" exists. Aborting procedure";
				return;
			}
			assert(newModel);
			std::cout<<"\nSetting Model to: "<<newModel->Name();
			for (int i = 0; i < editorSelection.Size(); ++i){
				editorSelection[i]->model = newModel;
			}
			// Get texture number
			// Query the entity manager to create an entity, buffer it and begin rendering it in the scene
			break;
		}
		case SET_ENTITY_NAME_PROMPT: {	/// Begins a prompt to set active entity's name.
			if (editorSelection.Size() == 0)
				return;
			std::cout<<"\nSet Entity Name prompt: ";
			Entity * entity = editorSelection[0];
			//  Copy past name to buffer
			Input.EnterTextInputMode("SET_ENTITY_NAME");
			if (entity->name){
				std::cout<<"Current name: "<<entity->name;
				Input.SetInputBuffer(entity->name);
			}
			Input.PrintBuffer();
			break;
		}
		case SET_ENTITY_NAME: {		/// Sets name to target entity by parsing input-buffer
			String name = Input.GetInputBuffer();
			Entity * entity = editorSelection[0];
			entity->SetName(name);
			if (name)
				std::cout<<"\nName set: "<<entity->name;
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
			}
			else {
				Physics.Pause();
			}
		}
	// ************************************************** //
	/// Camera
	// ************************************************** //
	case STOP:
		mainCamera->velocity = Vector3f(0, 0, 0);
		mainCamera->rotationVelocity = Vector3f(0, 0, 0);
		break;
	/// Navigation
	case FORWARD: 	mainCamera->Begin(Direction::FORWARD);		break;
	case FORWARD_S:	mainCamera->End(Direction::FORWARD);		break;
	case BACKWARD: 	mainCamera->Begin(Direction::BACKWARD);	break;
	case BACKWARD_S:mainCamera->End(Direction::BACKWARD);		break;
	case LEFT:		mainCamera->Begin(Direction::LEFT);		break;
	case LEFT_S:	mainCamera->End(Direction::LEFT);			break;
	case RIGHT:		mainCamera->Begin(Direction::RIGHT);		break;
	case RIGHT_S:	mainCamera->End(Direction::RIGHT);			break;
	case UP:		mainCamera->Begin(Direction::UP);			break;
	case UP_S:		mainCamera->End(Direction::UP);			break;
	case DOWN:		mainCamera->Begin(Direction::DOWN);		break;
	case DOWN_S:	mainCamera->End(Direction::DOWN);			break;
	/// Rotation
	case TURN_LEFT:		mainCamera->BeginRotate(Direction::LEFT); break;
	case TURN_LEFT_S:	mainCamera->EndRotate(Direction::LEFT); break;
	case TURN_RIGHT:	mainCamera->BeginRotate(Direction::RIGHT); break;
	case TURN_RIGHT_S:	mainCamera->EndRotate(Direction::RIGHT); break;
	case TURN_UP:		mainCamera->BeginRotate(Direction::UP); break;
	case TURN_UP_S:		mainCamera->EndRotate(Direction::UP); break;
	case TURN_DOWN:		mainCamera->BeginRotate(Direction::DOWN); break;
	case TURN_DOWN_S:	mainCamera->EndRotate(Direction::DOWN); break;
	/// Camera Distance
	case COME_CLOSER:
		mainCamera->distanceFromCentreOfMovement *= 0.8f;
		break;
	case BACK_AWAY:
		mainCamera->distanceFromCentreOfMovement *= 1.25f;
		break;
	case ZOOM_IN: 		/// Zoom
		mainCamera->zoom *= 1.25f;
		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		mainCamera->zoom *= 0.8f;
		Graphics.UpdateProjection();
		break;
	case INCREASE_SPEED:
		mainCamera->flySpeed *= 1.25f;
		break;
	case DECREASE_SPEED:
		mainCamera->flySpeed *= 0.8f;
		break;
	case RESET_CAMERA:
		*mainCamera = Camera();
		mainCamera->SetRatio(Graphics.width, Graphics.height);
		mainCamera->Update();
		break;
	default:
		std::cout<<"\nINFO: Default case for action: "<<action<<"!";
		break;
	}
}
