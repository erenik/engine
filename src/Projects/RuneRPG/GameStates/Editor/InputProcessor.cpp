// Emil Hedemalm
// 2013-06-28

#include "RuneEditor.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Maps/2D/TileMap2D.h"
#include "Maps/Grids/TileTypeManager.h"
#include "RuneRPG/GameStates/Map/MapState.h"
#include "EntityStates/StateProperty.h"
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


void RuneEditor::InputProcessor(int action, int inputDevice/* = 0*/){
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
		case INTERPRET_CONSOLE_COMMAND: {
			String command = Input.GetInputBuffer();

			// Check if null-string
			if (command.Length() < 2)
				break;

			List<String> token = command.Tokenize(" ");
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			int tokensFound = token.Size();
			if (command.Contains("save") && tokensFound >= 2){
				String toFile = "map/RuneRPG/"+token[1];
				if (!toFile.Contains(".tmap"))
					toFile += ".tmap";
				map->Save(toFile);
			}
			else if (command.Contains("load") && tokensFound >= 2){
				String fromFile = token[1];
				LoadMap(fromFile);

			}
			else if (command.Contains("clear entities") ||command.Contains("delete entities")){
				/// Clears ALL entities o-o
				MapMan.DeleteEntities();
			}
			else if (command.Contains("play") || command.Contains("test")){
				Playtest();
			}
			else if (command.Contains("load tiletypes")){
				ReloadTiles();
			}
			else if (command.Contains("set mapsize")){
				int xSize = 0;
				int ySize = 0;
				if (tokensFound >= 4){
					xSize = token[2].ParseInt();
					ySize = token[3].ParseInt();
				}
				else if (tokensFound == 3){
					xSize = ySize = token[2].ParseInt();
				}
				SetMapSize(xSize, ySize);
			}
			else if (command.Contains("set viewrange") && tokensFound >= 3){
				int x = token[2].ParseInt();
				map->SetViewRange(x);
			}
			else if (command.Contains("randomize tiles")){
				Graphics.PauseRendering();
				map->RandomizeTiles();
				Graphics.ResumeRendering();
			}
			else if (command == "toggle debugrenders"){
				Graphics.EnableAllDebugRenders(!Graphics.renderGrid);
			}
			else if (command.Contains("render light") || command.Contains("toggle light")){
				Graphics.renderLights = !Graphics.renderLights;
			}
			/// Light stuffs !
			else if (command.Contains("set ambient") || command.Contains("set ambience")){
				if (token.Size() < 5){
					std::cout<<"\nERROR: Lacking arguments to setting ambient light.";
					return;
				}
				Vector3f ambience;
				ambience.x = token[2].ParseFloat();
				ambience.y = token[3].ParseFloat();
				ambience.z = token[4].ParseFloat();
				Graphics.QueueMessage(new GMSetAmbience(ambience));
			}
			else if (command.Contains("render lights") || command.Contains("toggle lights")){
				Graphics.renderLights = !Graphics.renderLights;
			}

			if (tokensFound >= 4 && token[0] == "set" && token[1] == "ambient"){
				Vector3f ambient;
				ambient.x = token[2].ParseFloat();
				ambient.y = token[3].ParseFloat();
				ambient.z = token[4].ParseFloat();
				Graphics.QueueMessage(new GMSetAmbience(ambient));
			}
			else if (tokensFound >= 3 && command.Contains("set gridspacing")){
				float gridSpacing = token[2].ParseFloat();
				Graphics.QueueMessage(new GMSetf(GRID_SPACING, gridSpacing));
			}
			else if (tokensFound >= 3 && command.Contains("set gridsize")){
				float gridSpacing = token[2].ParseFloat();
				Graphics.QueueMessage(new GMSetf(GRID_SIZE, gridSpacing));
			}
			if (strcmp(token[0], "reload") == 0 && strcmp(token[1], "map") == 0){
				/// Clear selection first!
				runeEditorSelection.Clear();
				std::cout<<"\nInput>>Reloading map...";
				Map * map = MapMan.ReloadFromFile();
				// Set map to be active!
				if (map){
					MapMan.MakeActive(map);
				}
				else
					std::cout<<"\nERROR: Unable to reload map!";
			}
			if (strcmp(token[0], "select") == 0 && token[1] != ""){
				Selection select = MapMan.SelectEntitiesByName(token[1]);
				if (select.Size() > 0)
					runeEditorSelection = select;
			}
			if (strcmp(token[0], "register") == 0 && strcmp(token[1], "for") == 0 && strcmp(token[2], "physics") == 0){
				for (int i = 0; i < runeEditorSelection.Size(); ++i){
					Entity * entity = runeEditorSelection[i];
					Physics.AttachPhysicsTo(entity);
				}
				Physics.QueueMessage(new PMRegisterEntities(runeEditorSelection));
				// RegisterEntities(runeEditorSelection);

			}
			if (strcmp(token[0], "attach") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "property") == 0){
				std::cout<<"\nAttaching physics properties to selected entities.";
				for (int i = 0; i < runeEditorSelection.Size(); ++i){
					Entity * entity = runeEditorSelection[i];
					Physics.AttachPhysicsTo(entity);
				}
			}

			if (tokensFound > 2 &&  strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "dynamic") == 0){
				std::cout<<"\nSetting selected entities' physics type to dynamic.";
				Physics.QueueMessage(new PMSetPhysicsType(runeEditorSelection, PhysicsType::DYNAMIC));
			}
			else if (tokensFound > 2 && strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "static") == 0){
				std::cout<<"\nSetting selected entities' physics type to dynamic.";
				Physics.QueueMessage(new PMSetPhysicsType(runeEditorSelection, PhysicsType::STATIC));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "plane") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Plane.";
				Physics.QueueMessage(new PMSetPhysicsShape(runeEditorSelection, ShapeType::PLANE));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "tri") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Triangle.";
				Physics.QueueMessage(new PMSetPhysicsShape(runeEditorSelection, ShapeType::TRIANGLE));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "quad") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Quad.";
				Physics.QueueMessage(new PMSetPhysicsShape(runeEditorSelection, ShapeType::QUAD));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "sphere") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Sphere.";
				Physics.QueueMessage(new PMSetPhysicsShape(runeEditorSelection, ShapeType::SPHERE));
			}
			if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "mesh") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Mesh.";
				Physics.QueueMessage(new PMSetPhysicsShape(runeEditorSelection, ShapeType::MESH));
			}
			if (strcmp(token[0], "set") == 0 && strcmp(token[1], "gravity") == 0){
				float x = 0.0f, y = 0.0f, z = 0.0f;
				if (tokensFound == 3){
					y = token[2].ParseFloat();
				}
				Physics.QueueMessage(new PMSetGravity(Vector3f(x,y,z)));
			}
			if (tokensFound >= 2 && strcmp(token[0], "toggle") == 0 && strcmp(token[1], "physics") == 0){
				Graphics.renderPhysics = !Graphics.renderPhysics;
			}
			if (strcmp(token[0], "toggle") == 0 && strcmp(token[1], "grid") == 0){
				Graphics.renderGrid = !Graphics.renderGrid;
			}
			if (strcmp(token[0], "toggle") == 0 && strcmp(token[1], "navmesh") == 0){
				Graphics.renderNavMesh = !Graphics.renderNavMesh;
			}
			if (strcmp(token[0], "toggle") == 0 && strcmp(token[1], "fps") == 0){
				Graphics.renderFPS = !Graphics.renderFPS;
			}
			if (tokensFound > 1 && strcmp(token[0], "delete") == 0 && strcmp(token[1], "all") == 0){
				MapMan.DeleteEntities();
				runeEditorSelection.Clear();
			}
#ifdef USE_NETWORK
			if (strcmp(token[0], "host") == 0 && strcmp(token[1], "RuneEditor") == 0){
				int port = DEFAULT_PORT;
				char password[MAX_PASSWORD] = "";
				if(tokensFound > 2)
					port = atoi(token[2]);
				if(tokensFound > 3)
					strncpy(password, token[3], MAX_PASSWORD);

				Network.StartServer(port, password);

			}
#endif	// USE_NETWORK
			break;
		}
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
			Input.EnterTextInputMode("LOAD_MAP");
			break;
		case LOAD_MAP: {
			/// Clear selection first!
			runeEditorSelection.Clear();
			Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/loading_map.png"));
			std::cout<<"\nInput>>LOAD_MAP";
			String filename = Input.GetInputBuffer();
			Map * loadedMap = MapMan.LoadMap(filename.c_str());
			// Set map to be active!
			if (loadedMap)
				MapMan.MakeActive(loadedMap);
			Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
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
	// ************************************************* //
	/// Entity manipulation
	// ************************************************* //
		case TRANSLATE_ENTITY_PROMPT: { /// Begins prompt for entity translation
			if (runeEditorSelection.Size() == 0)
				return;
			Entity * entity = runeEditorSelection[0];
			std::cout<<"\nTranslate Entity prompt: Current translation: "<<entity->position;
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
			if (runeEditorSelection.Size() == 0)
				return;
			Entity * entity = runeEditorSelection[0];
			std::cout<<"\nScale Entity prompt: Current scale: "<<entity->scale;
			Input.EnterTextInputMode("SCALE_ENTITY");
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
			if (runeEditorSelection.Size() == 0)
				return;
			Entity * entity = runeEditorSelection[0];
			std::cout<<"\nRotate Entity prompt: Current Rotation: "<<entity->rotation;
			Input.EnterTextInputMode("ROTATE_ENTITY");
			break;
		}
		case ROTATE_ENTITY: 			/// Rotates active entity/entities
			float rotation[3];
			if (Input.ParseFloats(rotation, 3) == 3)
				RotateActiveEntities(Vector3f(rotation));
			break;
		case SET_ENTITY_NAME_PROMPT: {	/// Begins a prompt to set active entity's name.
			if (runeEditorSelection.Size() == 0)
				return;
			std::cout<<"\nSet Entity Name prompt: ";
			Entity * entity = runeEditorSelection[0];
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
			Entity * entity = runeEditorSelection[0];
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
		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		runeEditorCamera->zoom *= 0.8f;
		Graphics.UpdateProjection();
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
