// Emil Hedemalm
// 2013-06-28

#include <cstring>

#include "PhysicsDemoState.h"
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
#include "Graphics/Render/RenderFrustum.h"

void PhysicsDemoState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode()){
		std::cout<<"\nIs in text-entering mode..";
		return;
	}
	switch(action){
		case TOGGLE_PAUSE: {
			if (Physics.IsPaused())
				Physics.Resume();
			else
				Physics.Pause();
			break;
		}
	    case RENDER_FRUSTUM:{
	        Graphics.QueueMessage(new GraphicsMessage(GM_RENDER_FRUSTUM));
	        break;
        }
		case GO_TO_MAIN_MENU:
			std::cout<<"\nInput>>GO_TO_MAIN_MENU";
			StateMan.QueueState(GAME_STATE_MAIN_MENU);
			break;
		/// Opening the general console for multi-purpose commands of more difficult nature
		case OPEN_CONSOLE:
			std::cout<<"\nOpening multi-purpose console: ";
			Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			break;
		case INTERPRET_CONSOLE_COMMAND: {
			String command = Input.GetInputBuffer();

			// Check if null-string
			if (!command)
				break;

			command.ConvertToChar();
			List<String> token = command.Tokenize(" ");
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			int tokensFound = token.Size();

			//// Lights!
			if (command.Contains("create") && tokensFound >= 2){
                Model * model = ModelMan.GetModel(token[1]);
                Texture * tex = NULL;
                if (tokensFound >= 3)
                    tex = TexMan.GetTexture(token[2]);
                MapMan.CreateEntity(model, tex);
			}
			else if (command == "gen navmesh"){
				WaypointMan.GenerateNavMesh(MapMan.ActiveMap());
			}
			else if (command.Contains("collissions enabled")){
				Physics.QueueMessage(new PMSetEntity(COLLISSIONS_ENABLED, editorSelection, true));
			}
			else if (command.Contains("collissions disabled")){
				Physics.QueueMessage(new PMSetEntity(COLLISSIONS_ENABLED, editorSelection, false));
			}
			else if(command == "clear lights"){
				Lighting light = MapMan.GetLighting();
				light.DeleteAllLights();
				MapMan.SetLighting(light);
				Graphics.QueueMessage(new GMSetLighting(light));
			}
			else if (command == "create light" || command == "add light"){
				Lighting light = MapMan.GetLighting();
				light.CreateLight();
				MapMan.SetLighting(light);
				Graphics.QueueMessage(new GMSetLighting(light));
			}
			else if (command.Contains("set light pos") && tokensFound >= 6){
				Vector3f position;
				position.x = token[3].ParseFloat();
				position.y = token[4].ParseFloat();
				position.z = token[5].ParseFloat();
				Lighting light = MapMan.GetLighting();
				light.SetPosition(position.x,position.y,position.z);
				MapMan.SetLighting(light);
				Graphics.QueueMessage(new GMSetLighting(light));
			}
			else if (tokensFound >= 3 && editorSelection.Size() > 0
				&& (command.Contains("set diffuse") || command.Contains("set texture") ||
				command.Contains("set specular") ||
				command.Contains("set normal")) )
			{
				int target;
				if (command.Contains("diffuse") || command.Contains("set texture"))
					target = DIFFUSE_MAP;
				if (command.Contains("specular"))
					target = SPECULAR_MAP;
				if (command.Contains("normal"))
					target = NORMAL_MAP;
				Graphics.QueueMessage(new GMSetEntityTexture(editorSelection[0], target, token[2]));
			}
			else if (command.Contains("collissiontriangles")){
				Graphics.renderCollissionTriangles = ! Graphics.renderCollissionTriangles;
			}
			else if (command.Contains("load") && !command.Contains("navmesh")){
				LoadMap(token[1]);
			}
			else if (command == "toggle debugrenders"){
				Graphics.EnableAllDebugRenders(!Graphics.renderGrid);
			}
			else if (command.Contains("render light") || command.Contains("toggle light")){
				Graphics.renderLights = !Graphics.renderLights;
			}
			/// Light stuffs !
			else if (command.Contains("set ambient") || command.Contains("set ambience")){
				Vector3f ambience;
				if (tokensFound == 3){
					ambience.x = ambience.y = ambience.z = token[2].ParseFloat();
				}
				else if (token.Size() == 5){
					ambience.x = token[2].ParseFloat();
					ambience.y = token[3].ParseFloat();
					ambience.z = token[4].ParseFloat();
				}
				else
					std::cout<<"\nBad arguments! Give 1 or 3 floats for setting ambience!";
				Lighting lighting = MapMan.GetLighting();
				lighting.SetAmbient(ambience);
				Graphics.QueueMessage(new GMSetLighting(lighting));
			}
			else if (command.Contains("render lights") || command.Contains("toggle lights")){
				Graphics.renderLights = !Graphics.renderLights;
			}

			if (tokensFound >= 4 && token[0] == "set" && token[1] == "ambient"){
				Vector3f ambient;
				ambient.x = token[2].ParseFloat();
				ambient.y = token[3].ParseFloat();
				ambient.z = token[4].ParseFloat();
				Lighting lighting = MapMan.GetLighting();
				lighting.VerifyData();
				lighting.SetAmbient(ambient);
				Graphics.QueueMessage(new GMSetLighting(lighting));
			}
			else if (tokensFound >= 3 && command.Contains("set gridspacing")){
				float gridSpacing = token[2].ParseFloat();
				Graphics.QueueMessage(new GMSetf(GRID_SPACING, gridSpacing));
			}
			else if (tokensFound >= 3 && command.Contains("set gridsize")){
				float gridSpacing = token[2].ParseFloat();
				Graphics.QueueMessage(new GMSetf(GRID_SIZE, gridSpacing));
			}
			if (command.Length() < 2)
				return;
			if (command.Contains("generateNavMeshFromWorld")){
				/// Do awesome stuff
				std::cout<<"\nAWESOME,  GOOOOOOOOOOOOOOOO!";
				WaypointMan.GenerateNavMeshFromWorld(editorSelection[0]);
			}
			if (strcmp(token[0], "mergeWaypointsByProximity") == 0 && tokensFound >= 2){
				WaypointMan.GetActiveNavMeshMutex();
				/// Get float
				float proximity = 0.0f;
				proximity = token[1].ParseFloat();
				WaypointMan.ActiveNavMesh()->MergeWaypointsByProximity(proximity);
				WaypointMan.ReleaseActiveNavMeshMutex();
			}
			if (tokensFound > 2 && strcmp(token[0], "scale") == 0 && strcmp(token[1], "navmesh") == 0){
				float amount = 1.0f;
				Input.ParseFloats(&amount, 1);
				WaypointMan.ActiveNavMesh()->Scale(amount);
			}
			if (strcmp(token[0], "play") == 0 && strcmp(token[1], "physics") == 0){
				std::cout<<"\nResuming physics";
				Physics.Resume();
			}
			if (strcmp(token[0], "pause") == 0 && strcmp(token[1], "physics") == 0){
				std::cout<<"\nPausing physics";
				Physics.Pause();
			}
			if (strcmp(token[0], "reload") == 0 && strcmp(token[1], "map") == 0){
				/// Clear selection first!
				editorSelection.Clear();
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
					editorSelection = select;
			}
			if (strcmp(token[0], "register") == 0 && strcmp(token[1], "for") == 0 && strcmp(token[2], "physics") == 0){
				for (int i = 0; i < editorSelection.Size(); ++i){
					Entity * entity = editorSelection[i];
					Physics.AttachPhysicsTo(entity);
				}
				Physics.QueueMessage(new PMRegisterEntities(editorSelection));
				// RegisterEntities(editorSelection);

			}
			if (strcmp(token[0], "attach") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "property") == 0){
				std::cout<<"\nAttaching physics properties to selected entities.";
				for (int i = 0; i < editorSelection.Size(); ++i){
					Entity * entity = editorSelection[i];
					Physics.AttachPhysicsTo(entity);
				}
			}

			if (tokensFound > 2 &&  strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "dynamic") == 0){
				std::cout<<"\nSetting selected entities' physics type to dynamic.";
				Physics.QueueMessage(new PMSetPhysicsType(editorSelection, PhysicsType::DYNAMIC));
			}
			else if (tokensFound > 2 && strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "static") == 0){
				std::cout<<"\nSetting selected entities' physics type to dynamic.";
				Physics.QueueMessage(new PMSetPhysicsType(editorSelection, PhysicsType::STATIC));
			}
			else if (command == "select all checkpoints"){
				editorSelection.Clear();
				List<Entity*> entities = MapMan.GetEntities();
				for (int i = 0; i < entities.Size(); ++i){
					Entity * e = entities[i];
					String entityName = e->name;
					entityName.SetComparisonMode(String::NOT_CASE_SENSITIVE);
					if (entityName.Contains("Checkpoint"))
						editorSelection.Add(entities[i]);
				}
				OnSelectionUpdated();
			}
			else if (command == "set physics cube"){
				std::cout<<"\nSetting selected entities' physics shape to Cube.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::CUBE));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "plane") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Plane.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::PLANE));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "tri") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Triangle.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::TRIANGLE));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "quad") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Quad.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::QUAD));
			}
			else if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "sphere") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Sphere.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::SPHERE));
			}
			if (strcmp(token[0], "set") == 0 && strcmp(token[1], "physics") == 0 && strcmp(token[2], "mesh") == 0){
				std::cout<<"\nSetting selected entities' physics shape type to Mesh.";
				Physics.QueueMessage(new PMSetPhysicsShape(editorSelection, ShapeType::MESH));
			}
			if (strcmp(token[0], "set") == 0 && strcmp(token[1], "gravity") == 0){
				float x = 0.0f, y = 0.0f, z = 0.0f;
				if (tokensFound == 3){
					y = token[2].ParseFloat();
				}
				Physics.QueueMessage(new PMSetGravity(Vector3f(x,y,z)));
			}
			if (tokensFound >= 2 && strcmp(token[0], "save") == 0 && strcmp(token[1], "navmesh") == 0){
				WaypointMan.GetActiveNavMeshMutex();
				if (tokensFound >= 3)
					WaypointMan.SaveNavMesh(token[2]);
				else
					WaypointMan.SaveNavMesh();
				WaypointMan.ReleaseActiveNavMeshMutex();
			}
			if (tokensFound >= 2 && strcmp(token[0], "load") == 0 && strcmp(token[1], "navmesh") == 0){
				WaypointMan.GetActiveNavMeshMutex();
				if (tokensFound >= 3)
					WaypointMan.LoadNavMesh(token[2]);
				else
					WaypointMan.LoadNavMesh();
				WaypointMan.ReleaseActiveNavMeshMutex();
			}
			if (tokensFound >= 2 && strcmp(token[0], "find") == 0 && strcmp(token[1], "path") == 0){
				PathMan.SetSearchAlgorithm("AStar");
				Path path;
				NavMesh * nm = WaypointMan.ActiveNavMesh();
				Waypoint * from, * to;
				from = nm->waypoints[rand()%nm->waypoints];
				while (!from->passable)
					from = nm->waypoints[rand()%nm->waypoints];
				to = nm->waypoints[rand()%nm->waypoints];
				while (!to->passable || to->IsAerial() != from->IsAerial()){
					to = nm->waypoints[rand()%nm->waypoints];
				}
				PathMan.GetPath(from, to, path);
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
				editorSelection.Clear();
			}
			if (command.Contains("test physics")){
//				Entity * newEntity = MapMan.CreateEntity(ModelMan.GetModel(model), TexMan.GetTexture(texture));

				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Centre house
					Entity * entity = MapMan.CreateEntity(model, texture);
					// Dynamic bouncer
					entity = MapMan.CreateEntity(model, texture);
					entity->position(5, 100, 0);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					// Right-side house
					entity = MapMan.CreateEntity(model, texture);
					entity->position(100, 0, 0);
					// Left-side house
					entity = MapMan.CreateEntity(model, texture);
					entity->position(-60, -20, 0);
					// Dynamic bouncer 2
					entity = MapMan.CreateEntity(model, texture);
					entity->position(2, 250, 0);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));

					Selection selection = MapMan.GetEntities();
					int amount = selection.Size();
					for (int i = 0; i < amount; ++i){
						selection[i]->translate(0, 50, 0);
					}
					// Floor
					entity = MapMan.CreateEntity(ModelMan.GetModel("plane.obj"), texture);
					entity->Scale(200.0f);
					Physics.QueueMessage(new PMSetPhysicsShape(Selection(entity), ShapeType::MESH));

				}
			}
			if (command.Contains("test tri")){
				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Create 1 corner bouncer
					Entity * entity = MapMan.CreateEntity(model, texture);
					entity->position(101, 50, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));

					// Floor
					entity = MapMan.CreateEntity(ModelMan.GetModel("plane.obj"), texture);
					entity->Scale(200.0f);
					Physics.QueueMessage(new PMSetPhysicsShape(Selection(entity), ShapeType::MESH));
				}
			}
			else if (command.Contains("spawn corners")){
				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Create 1 corner bouncer
					Entity * entity = MapMan.CreateEntity(model, texture);
					entity->position(101, 50, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(-101, 100, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(-101, 175, -101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(101, 250, -101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
				}
			}
			else if (command.Contains("spawn edges")){
				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Create 1 corner bouncer
					Entity * entity = MapMan.CreateEntity(model, texture);
					entity->position(51, 50, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(-51, 100, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(-101, 175, -51);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
					entity->position(101, 250, -51);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
				}
			}
			if (strcmp(token[0], "ball") == 0 && strcmp(token[1], "hell") == 0){
				Model * model = ModelMan.GetModel("racing/ship_UVd.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					/// Pause physics
					Physics.Pause();
					/// Create shit
#define BALLS	25
					Selection balls;
					Entity * entity = NULL;
					for (int i = 0; i < BALLS; ++i){
						int posX = rand()%(11 * BALLS) - 5 * BALLS;
						int posY = rand()%(51 * BALLS) + 10;
						int posZ = rand()%(11 * BALLS) - 5 * BALLS;
						// Dynamic bouncer
						entity = MapMan.CreateEntity(model, texture);
						entity->position((float)posX, (float)posY, (float)posZ);
						balls.Add(entity);
						SleepThread(1000 / BALLS);
					}
					/// SEt all balls' physics type to dynamic
					Physics.QueueMessage(new PMSetPhysicsType(balls, PhysicsType::DYNAMIC));

					/// Resume physics
					Physics.Resume();
				}
			}
#ifdef USE_NETWORK
			if (strcmp(token[0], "host") == 0 && strcmp(token[1], "editor") == 0){
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
			std::cout<<"\nSave Map prompt: Enter file name. If no extension is added .map will be added. ";
			Input.EnterTextInputMode("SAVE_MAP");
			break;
	//	case CHECK_EXISTING_FILE:
	//		break;
		case SAVE_MAP: {
			Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/saving_map.png"));
			bool physicsWasPaused = Physics.IsPaused();
			Physics.Pause();
			std::cout<<"\nInput>>SAVE_MAP";
			SleepThread(100);
			String filename = Input.GetInputBuffer();
			if (!filename.Contains("racing/"))
				filename = "racing/" + filename;
			MapMan.SaveMap(filename);
			Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
			if (!physicsWasPaused)
				Physics.Resume();
			break; }
		case LOAD_MAP_PROMPT:
			std::cout<<"\nLoad Map prompt: ";
			Input.EnterTextInputMode("LOAD_MAP");
			break;
		case LOAD_MAP: {
			LoadMap(Input.GetInputBuffer());
			break;}
		case LOAD_MODEL:
			std::cout<<"\nInput>>LOAD_MODEL";
			std::cout<<"\nIMPLEMENT";
			SleepThread(10000);
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
			inView = inView.CullByCamera(editorCamera);
			inView.SortByDistance(Vector3f(editorCamera.Position()));
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
			inView = inView.CullByCamera(editorCamera);
			inView.SortByDistance(Vector3f(editorCamera.Position()));
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
				    assert(false);
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
		case DELETE_ENTITY: {
			/// Check input
			String reply = Input.GetInputBuffer();
			/// Compare to all valid replies
			reply.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (!reply.Contains("yes"))
				break;
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
		editorCamera.velocity = Vector3f(0, 0, 0);
		editorCamera.rotationVelocity = Vector3f(0, 0, 0);
		break;
	/// Navigation
	case FORWARD: 	editorCamera.Begin(DIRECTION::FORWARD);		break;
	case FORWARD_S:	editorCamera.End(DIRECTION::FORWARD);		break;
	case BACKWARD: 	editorCamera.Begin(DIRECTION::BACKWARD);	break;
	case BACKWARD_S:editorCamera.End(DIRECTION::BACKWARD);		break;
	case LEFT:		editorCamera.Begin(DIRECTION::LEFT);		break;
	case LEFT_S:	editorCamera.End(DIRECTION::LEFT);			break;
	case RIGHT:		editorCamera.Begin(DIRECTION::RIGHT);		break;
	case RIGHT_S:	editorCamera.End(DIRECTION::RIGHT);			break;
	case UP:		editorCamera.Begin(DIRECTION::UP);			break;
	case UP_S:		editorCamera.End(DIRECTION::UP);			break;
	case DOWN:		editorCamera.Begin(DIRECTION::DOWN);		break;
	case DOWN_S:	editorCamera.End(DIRECTION::DOWN);			break;
	/// Rotation
	case TURN_LEFT:		editorCamera.BeginRotate(DIRECTION::LEFT); break;
	case TURN_LEFT_S:	editorCamera.EndRotate(DIRECTION::LEFT); break;
	case TURN_RIGHT:	editorCamera.BeginRotate(DIRECTION::RIGHT); break;
	case TURN_RIGHT_S:	editorCamera.EndRotate(DIRECTION::RIGHT); break;
	case TURN_UP:		editorCamera.BeginRotate(DIRECTION::UP); break;
	case TURN_UP_S:		editorCamera.EndRotate(DIRECTION::UP); break;
	case TURN_DOWN:		editorCamera.BeginRotate(DIRECTION::DOWN); break;
	case TURN_DOWN_S:	editorCamera.EndRotate(DIRECTION::DOWN); break;
	/// Camera Distance
	case COME_CLOSER:
		editorCamera.distanceFromCentreOfMovement *= 0.8f;
		break;
	case BACK_AWAY:
		editorCamera.distanceFromCentreOfMovement *= 1.25f;
		break;
	case ZOOM_IN: 		/// Zoom
		editorCamera.zoom *= 1.25f;
		Graphics.UpdateProjection();
		break;
	case ZOOM_OUT:
		editorCamera.zoom *= 0.8f;
		Graphics.UpdateProjection();
		break;
	case INCREASE_SPEED:
		editorCamera.flySpeedMultiplier *= 1.25f;
		break;
	case DECREASE_SPEED:
		editorCamera.flySpeedMultiplier *= 0.8f;
		break;
	case RESET_CAMERA:
		editorCamera = Camera();
		editorCamera.SetRatio(Graphics.width, Graphics.height);
		editorCamera.Update();
		break;
	default:
		std::cout<<"\nINFO: Default case for action: "<<action<<"!";
		break;
	}
}
