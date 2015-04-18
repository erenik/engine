// Emil Hedemalm
// 2013-06-28

#include "EditorState.h"

#include "Graphics/GraphicsManager.h"
#include "Maps/MapManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"
#include "Message/MessageManager.h"
#include "TextureManager.h"
#include "ModelManager.h"

#include "../Physics/PhysicsProperty.h"
#include "../Physics/Messages/PhysicsMessage.h"
#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"
#include "Message/VectorMessage.h"
#include "Actions.h"
#include "OS/Sleep.h"
#include "File/FileUtil.h"

#include <iomanip>
#include "Graphics/Messages/GMNavMesh.h"
#include "Graphics/GraphicsProperty.h"
#include "EntityStates/StateProperty.h"
#include "../../EntityStates/RacingShipGlobal.h"
#include "../../ShipManager.h"
#include "../../AI/ThrusterTester.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "UI/UIFileBrowser.h"
#include "Message/FileEvent.h"
#include "String/StringUtil.h"

#define EDITOR_MAP "EditorMap"
#define SHIP_EDITOR_MAP	"ShipEditor"

EditorState::EditorState()
: SpaceRaceGameState()
{
	id = GameStateID::GAME_STATE_EDITOR;
	mouseCameraState = 0;
	editMode = ENTITIES;
	activePath = NULL;
	checkPointWaypointInterval = 5;
	checkpointSize = 10.0f;
    name = "Editor state";

	activeShipEntity = NULL;
	activeShip = NULL;
	lastActiveMap = NULL;
	keyPressedCallback = true;
}
EditorState::~EditorState(){
	std::cout<<"\nEdhitor Destruchtor :3";
}

Selection EditorState::GetActiveSelection(){
    return editorSelection;
};

void EditorState::CreateUserInterface(){
	if (ui)
		delete ui;
	this->ui = new UserInterface();
	ui->Load("gui/Editor.gui");
}

void EditorState::OnEnter(GameState * previousState){
	// Load initial texture and set it to render over everything else
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

	if (!MapMan.Exists(EDITOR_MAP))
		MapMan.CreateMap(EDITOR_MAP);
	if (!MapMan.Exists(SHIP_EDITOR_MAP))
		MapMan.CreateMap(SHIP_EDITOR_MAP);

	// Ship edhitor
	if (editMode == SHIP_EDITOR){
		MapMan.MakeActive(SHIP_EDITOR_MAP);
	}
	/// Regular map editor.
	else {
		ReturnToLastActiveMap();
	}

	/// Disable AI-rendering
	Graphics.renderAI = false;
	Graphics.renderNavMesh = false;
	Graphics.renderGrid = true;
	Graphics.renderNavMesh = true;
	Graphics.renderPhysics = true;
	Graphics.renderFPS = true;
	Graphics.renderLights = true;

	/// Set editor selection as the renderable one!
	Graphics.selectionToRender = &editorSelection;

	// Set camera
	SetCameraProjection3D();
	
	mainCamera->SetRatio(Graphics.width, Graphics.height);

	// Load the AI scene
	// Don't load anytheing o-o;
/*	if (!MapMan.MakeActive(&editorMap)){
		Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
		Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
	}
*/
	/// Reset physics settings
	Physics.QueueMessage(new PhysicsMessage(PM_RESET_SETTINGS));

	Graphics.QueueMessage(new GMSet(FOG_BEGIN, 5000.0f));
	Graphics.QueueMessage(new GMSet(FOG_END, 50000.0f));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	Input.ForceNavigateUI(false);
	Input.NavigateUI(true);

}

void EditorState::OnExit(GameState *nextState){
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	MapMan.MakeActive(NULL);
	std::cout<<"\nLeaving Editor state.";
}



/// Last update time in ms
#include <ctime>
clock_t lastTime = 0;
void EditorState::Process(float time){
	/// Process key input for navigating the 3D - Space

	// Calculate time since last update
	int64 newTime = Timer::GetCurrentTimeMs();
	int64 timeDiff = newTime - lastTime;
	lastTime = newTime;

	/*
	// Print the input buffer if applicable
	//if (Input.IsInTextEnteringMode())
	{
		String inputBuf = Input.GetInputBuffer();
		if (Input.IsInTextEnteringMode()){
			if (newTime % 1000 > 500)
				inputBuf += "_";
		}
		Graphics.QueueMessage(new GMSetUIs("InputBuffer", GMUI::TEXT, inputBuf));
	}
	*/

	// Update for previously entered commands to the log AppWindow
	/*
	{
		List<String> inputs;
		for (int i = 1; i < InputManager::INPUT_BUFFERS; ++i){
			String input = Input.GetInputBuffer(i);
			if (input.Length() != 0)
				inputs.Add(input);
		}
		String s = "";
		for (int i = 0; i < inputs.Size() && i < 5; ++i){
			s = s + inputs[i] + "\n";
		}
		Graphics.QueueMessage(new GMSetUIs("OutputData", GMUI::TEXT, s));
	}*/


	/* Deprecated, should be handled within the camera and graphics thread now!
	/// Fly! :D
	/// Rotate first, yo o.O
	/// Rotation multiplier.
	float rotMultiplier = 0.05f;
	mainCamera->rotation += mainCamera->rotationVelocity * mainCamera->rotationSpeed * (float)timeDiff;
	// Check input for moving camera
	if (mainCamera->velocity.Length() > 0){
		Vector4d moveVec;
		moveVec = Vector4d(mainCamera->velocity);
		/// Flight-speed multiplier.
		float multiplier = 0.5f * mainCamera->flySpeedMultiplier;
		moveVec = moveVec * multiplier * (float)timeDiff;
		Matrix4d rotationMatrix;
		rotationMatrix.InitRotationMatrixY(-mainCamera->rotation.y);
		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-mainCamera->rotation.x));
		moveVec = rotationMatrix.product(moveVec);
		mainCamera->position += Vector3f(moveVec);
	}
	*/
};

/// Callback function that will be triggered via the MessageManager when messages are processed.
void EditorState::ProcessMessage(Message * message){
//	std::cout<<"\nEditorState::ProcessMessage: ";
	String msg = message->msg;
	switch(message->type){
		case MessageType::FILE_EVENT: 
		{
			FileEvent * fileEvent = (FileEvent *) message;
			List<String> files = fileEvent->files;
			if (msg == "LoadMap(this)")
			{
				LoadMap(files[0]);
			}
		}
		case MessageType::SET_STRING: 
		{
			if (!editorSelection.Size())
				return;
			SetStringMessage * ssm = (SetStringMessage*)message;
			if (msg == "SetEntityName"){
				editorSelection[0]->name = ssm->value;
			}
			break;
		}
		case MessageType::TEXTURE_MESSAGE: 
		{
			if (!editorSelection.Size())
				return;
			TextureMessage * tm = (TextureMessage*) message;
			if (msg == "SetEntityDiffuseTexture"){
				Graphics.QueueMessage(new GMSetEntityTexture(editorSelection[0], DIFFUSE_MAP, tm->texSource));
			}
			break;
		}
		case MessageType::FLOAT_MESSAGE:
		{
			String string = message->msg;
			FloatMessage * fm = (FloatMessage*) message;
			if (string == "SetRestitution"){
				Physics.QueueMessage(new PMSetEntity(RESTITUTION, this->editorSelection, fm->value));
			}
			else if (string == "SetFriction"){
				Physics.QueueMessage(new PMSetEntity(FRICTION, this->editorSelection, fm->value));
			}
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			String string = message->msg;
			VectorMessage * vm = (VectorMessage*) message;
			if (string == "SetPosition"){
				Physics.QueueMessage(new PMSetEntity(POSITION, this->editorSelection, vm->vec3f));
			}
			else if (string == "SetRotation"){
				Physics.QueueMessage(new PMSetEntity(SET_ROTATION, this->editorSelection, vm->vec3f));
			}
			else if (string == "SetScale"){
				Physics.QueueMessage(new PMSetEntity(SET_SCALE, this->editorSelection, vm->vec3f));
			}
			else if (string == "SetThrusterPosition"){
				Graphics.PauseRendering();
				activeShip->thrusterPosition = vm->vec3f;
				RacingShipGlobal * rsg = (RacingShipGlobal*)activeShipEntity->state->GlobalState();
				rsg->ReloadFromShip();

				Graphics.ResumeRendering();
			}
		}
		case MessageType::COLLISSION_CALLBACK: {
			return;
		}
		case MessageType::CONSOLE_COMMAND:
		{
			String command = message->msg;
			// Check if null-string
			if (!command)
				break;
			if (command.Type() == String::WIDE_CHAR)
				command.ConvertToChar();
			List<String> token = command.Tokenize(" ");
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			int tokensFound = token.Size();
			/// Entity manip
			if (command.Contains("scale")){
				if (token.Size() < 2)
					return;
				float value = token[1].ParseFloat();
				if (value == 0)
					return;
				this->ScaleActiveEntities(Vector3f(value,value,value));
			}
			/// Map save/load
			if (command.Contains("save map")){
				String fileName = "file";
				bool force = false;
				for (int i = 0; i < token.Size(); ++i){
					std::cout<<"\nToken "<<i<<": "<<token[i];
				}
				if (token.Size() > 2)
					fileName = token[2];
				if (token.Size() > 3)
					force = token[3].Contains("force");
				bool success = SaveMap(fileName, force);
			}
			else if (command.Contains("load map")){
				String fileName = "file";
				bool force = false;
				for (int i = 0; i < token.Size(); ++i){
					std::cout<<"\nToken "<<i<<": "<<token[i];
				}
				if (token.Size() > 2)
					fileName = token[2];
				if (token.Size() > 3)
					force = token[3].Contains("force");
				bool success = LoadMap(fileName);
			}
			//// Lights!
			else if (command.Contains("create") && tokensFound >= 2){
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
				Physics.QueueMessage(new PMSetEntity(COLLISIONS_ENABLED, editorSelection, true));
			}
			else if (command.Contains("collissions disabled")){
				Physics.QueueMessage(new PMSetEntity(COLLISIONS_ENABLED, editorSelection, false));
			}
			else if(command == "clear lights"){
				Lighting light = MapMan.GetLighting();
				light.DeleteAllLights();
				MapMan.SetLighting(light);
//				Graphics.QueueMessage(new GMSetLighting(light));
			}
			else if (command == "create light" || command == "add light"){
				Lighting light = MapMan.GetLighting();
				light.CreateLight();
				MapMan.SetLighting(light);
//				Graphics.QueueMessage(new GMSetLighting(light));
			}
			else if (command.Contains("set light pos") && tokensFound >= 6){
				Vector3f position;
				position.x = token[3].ParseFloat();
				position.y = token[4].ParseFloat();
				position.z = token[5].ParseFloat();
				Lighting light = MapMan.GetLighting();
			//	light.SetPosition(position.x,position.y,position.z);
				MapMan.SetLighting(light);
	//			Graphics.QueueMessage(new GMSetLighting(light));
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
//				Graphics.QueueMessage(new GMSetLighting(lighting));
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
//				lighting.VerifyData();
				lighting.SetAmbient(ambient);
//				Graphics.QueueMessage(new GMSetLighting(lighting));
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
//				Entity * newEntity = MapMan.CreateEntity(ModelMan.GetModel(model), TexMan.GetTextureBySourcetexture));

				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Centre house
					Entity * entity = MapMan.CreateEntity(model, texture);
					// Dynamic bouncer
					entity = MapMan.CreateEntity(model, texture);
					entity->SetPosition(5, 100, 0);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					// Right-side house
					entity = MapMan.CreateEntity(model, texture);
					entity->SetPosition(100, 0, 0);
					// Left-side house
					entity = MapMan.CreateEntity(model, texture);
					entity->SetPosition(-60, -20, 0);
					// Dynamic bouncer 2
					entity = MapMan.CreateEntity(model, texture);
					entity->SetPosition(2, 250, 0);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));

					Selection selection = MapMan.GetEntities();
					int amount = selection.Size();
					for (int i = 0; i < amount; ++i){
						selection[i]->Translate(0, 50, 0);
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
				//	entity->position(101, 50, 101);
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
//					entity->position(101, 50, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(-101, 100, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(-101, 175, -101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(101, 250, -101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
				}
			}
			else if (command.Contains("spawn edges")){
				Model * model = ModelMan.GetModel("Awesome haus_uv.obj");
				Texture * texture = TexMan.GetTextureByName("mainmenu_bg.png");
				if (model){
					// Create 1 corner bouncer
					Entity * entity = MapMan.CreateEntity(model, texture);
//					entity->position(51, 50, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(-51, 100, 101);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(-101, 175, -51);
					Physics.QueueMessage(new PMSetPhysicsType(Selection(entity), PhysicsType::DYNAMIC));
					entity = MapMan.CreateEntity(model, texture);
//					entity->position(101, 250, -51);
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
//						entity->position((float)posX, (float)posY, (float)posZ);
						balls.Add(entity);
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
		case MessageType::STRING:
		{
			String string = message->msg;
			if (HandleCameraMessages(string))
				return;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
				return;
			}
			else if (msg == "LoadMap")
			{
				UIFileBrowser * mapBrowser = new UIFileBrowser("Load map", "LoadMap(this)", ".map");
				mapBrowser->CreateChildren();
				mapBrowser->SetPath("./map/racing/");
				mapBrowser->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(mapBrowser));
				Graphics.QueueMessage(new GMPushUI(mapBrowser, ui));
			}
			else if (string == "SaveNavMesh"){
				String toPath = ui->GetElementByName("SaveNavMesh")->text;
				const NavMesh * nm = MapMan.GetNavMesh();
				nm->SaveToFile(toPath);
				return;
			}
			else if (string == "LoadNavMesh"){
				UIElement * element = ui->GetElementByName("LoadNavMesh");
				assert(element);
				if (element == NULL){
					std::cout<<"\nNo UIElement \"LoadNavMesh\" available to extract path from?";
					return;
				}
				String fromPath = element->text;
				NavMesh * nm = WaypointMan.LoadNavMesh(fromPath);
				if (!nm){
					ConsoleLog("Unable to load navmesh.");
					return;
				}
				MapMan.AssignNavMesh(nm);
				WaypointMan.MakeActive(nm);
				return;
			}
			else if (string == "SavePath"){
				UIElement * element = ui->GetElementByName("SavePath");
				String toFile = element->text;
				assert(activePath);
				activePath->Save(toFile);
			}
			else if (string == "LoadPath"){
				UIElement * element = ui->GetElementByName("LoadPath");
				String fromFile = element->text;
				Path * path = new Path();
				if (!path->Load(fromFile))
					return;
				activePath = path;
				OnActivePathUpdated();
				MapMan.AddPath(path);
			}
			else if (string == "SetNMWPProximity"){
				float prox = ui->GetElementByName("NMWPProximity")->text.ParseFloat();
				WaypointManager::SetMinimumWaypointProximity(prox);
			}
			else if (string == "SetNMMinimumNeighbours"){
				int neighbours = ui->GetElementByName("NMMinimumNeighbours")->text.ParseInt();
				WaypointManager::SetMinimumNeighbours(neighbours);
			}
			else if (string == "GenerateNavMesh"){
				waypointSelection.Clear();
				OnWaypointSelectionUpdated();
				bool result = MapMan.CreateNavMesh(editorSelection);
				assert(result);
				if (!result)
					std::cout<<"\nERROR: Unable to create navmesh!";
			}
			else if (string == "SetNMMinimumInclination"){
				float inclination = ui->GetElementByName("NMMinimumInclinationValue")->text.ParseFloat();
				WaypointManager::SetMinimumInclination(inclination);
			}
			else if (string == "LoadMap(this)"){
				LoadMap(ui->GetElementByName("LoadMap")->text);
			}
			else if (string.Contains("SetMode(")){
				String mode = string.Tokenize("()")[1];
				mode.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (mode == "NavMesh"){
					SetMode(NAVMESH);
				}
				else if (mode == "Paths"){
					SetMode(PATHS);
				}
				else if (mode == "Entities"){
					SetMode(ENTITIES);
				}
				else if (mode == "ShipEditor"){
					SetMode(SHIP_EDITOR);
				}
				else {
					assert(false && "Bad mode given in SetMode()! E.g. NavMesh, Entities, etc.");
				}
				return;
			}
			//// Ship editor
			else if (string == "NewShip"){
				NewShip();
			}
			else if (string == "LoadShip"){
				UIElement * element = ui->GetElementByName("LoadShip");
				LoadShip(element->text);
			}
			else if (string == "SaveShip"){
				UIElement * element = ui->GetElementByName("SaveShip");
				SaveShip(element->text);
			}
#define ASSERT_ELEMENT if (!element){ \
                    std::cout<<"\nERROR: No UIelement in the message?"; \
                    return; \
                }
#define ASSERT_ACTIVE_SHIP if (!activeShip){ \
                    std::cout<<"\nERROR: No active ship."; \
                    return; \
                } \
                if (!activeShipEntity){\
                std::cout<<"\nERROR: No active ship entity."; \
                return; \
                }
			else if (string == "SetShipDiffuse"){
                UIElement * element = message->element;
                ASSERT_ELEMENT
                ASSERT_ACTIVE_SHIP
                std::cout<<"\nSetting ship diffuse source to: "<<element->text;
                activeShip->diffuseSource = element->text;
                Graphics.QueueMessage(new GMSetEntityTexture(activeShipEntity, DIFFUSE_MAP, element->text));
			}
			else if (string == "SetShipSpecular"){
                UIElement * element = message->element;
                ASSERT_ELEMENT
                ASSERT_ACTIVE_SHIP
                std::cout<<"\nSetting ship diffuse source to: "<<element->text;
                activeShip->diffuseSource = element->text;
                Graphics.QueueMessage(new GMSetEntityTexture(activeShipEntity, DIFFUSE_MAP, element->text));
			}

			//// Paths n navmesh below
			else if (string == "GenerateTrackPath"){
				GenerateTrackPath();
			}
			else if (string == "SetCheckpointWaypointInterval"){
				int interval = ui->GetElementByName("CheckpointWaypointInterval")->text.ParseInt();
				assert(interval > 0);
				checkPointWaypointInterval = interval;
			}
			else if (string == "SetCheckpointSize"){
				float size = ui->GetElementByName("CheckpointSize")->text.ParseInt();
				assert(size > 0);
				checkpointSize = size;
			}
			else if (string == "SetCheckpointModel"){
				checkpointModelSource = ui->GetElementByName("CheckpointModel")->text;
			}
			else if (string == "PrintWaypoints"){
				if (activePath == NULL)
					return;
				for (int i = 0; i < activePath->Waypoints(); ++i){
					Waypoint * wp = activePath->GetWaypoint(i);
					Vector3f pos = wp->position;
					std::cout<<"\nWP"<<i<<": "<<pos;
				}
			}
			else if (string == "CreateCheckpoints"){
				CreateCheckpoints();
			}
			else if (string == "ConnectWaypoints"){
				ConnectWaypoints();
			}
			else if (string == "load_map"){
				StateMan.ActiveState()->InputProcessor(LOAD_MAP);
			}
			else if (string == "save_map"){
				StateMan.ActiveState()->InputProcessor(SAVE_MAP);
			}
			else if (string == "translate_entity"){
				StateMan.ActiveState()->InputProcessor(TRANSLATE_ENTITY);
			}
			else if (string == "scale_entity"){
				StateMan.ActiveState()->InputProcessor(SCALE_ENTITY);
			}
			else if (string == "rotate_entity"){
				StateMan.ActiveState()->InputProcessor(ROTATE_ENTITY);
			}
			else if (string == "create_Entity"){
				StateMan.ActiveState()->InputProcessor(CREATE_ENTITY);
			}
			else if (string == "set_texture"){
				StateMan.ActiveState()->InputProcessor(SET_TEXTURE);
				return;
			}
			else if (string == "delete_entity"){
				StateMan.ActiveState()->InputProcessor(DELETE_ENTITY);
				return;
			}
			else if (string.Contains("set_entity_pos")){
				std::cout<<"set_entity_posx";
				// If no argument provided, use active input
				String input = Input.GetInputBuffer();
				float f;
				if (input.Length() > 1)
					f = input.ParseFloat();
				// MODES, X = 1, Y = 2, Z = 3, ALL = 0
				int mode;
				if (string.Contains("posx"))
					mode = 1;
				else if (string.Contains("posy"))
					mode = 2;
				else if (string.Contains("posz"))
					mode = 3;
				// Get selected entity
				Selection selection = GetActiveSelection();
				for (int i = 0; i < selection.Size(); ++i){
					Entity * e = selection[i];
					Vector3f newPosition = Vector3f(
						mode == 1? f : e->position.x,
						mode == 2? f : e->position.y,
						mode == 3? f : e->position.z);
					Physics.QueueMessage(new PMSetEntity(POSITION, e, newPosition));
				}
				OnSelectionUpdated();
			}
		}
	}
	GameState::ProcessMessage(message);
}

/// Called every time the current selection is updated.
void EditorState::OnSelectionUpdated(){
	std::cout<<"\nEditorState::OnSelectionUpdated()";
#define ENTITY_MANIP_UI "EntityManipList"
	if (editorSelection.Size() == 0){
		Graphics.QueueMessage(new GMSetUIb(ENTITY_MANIP_UI, GMUI::VISIBILITY, false));
		return;
	}
	Entity * entity = editorSelection[0];
	if (entity)
	{
		// Reveal the UI
		Graphics.QueueMessage(new GMSetUIb(ENTITY_MANIP_UI, GMUI::VISIBILITY, true));

		Graphics.QueueMessage(new GMSetUIs("EntityName", GMUI::STRING_INPUT_TEXT, entity->name));

		Graphics.QueueMessage(new GMSetUIv3f("Position", GMUI::VECTOR_INPUT, entity->position));
		Graphics.QueueMessage(new GMSetUIv3f("Rotation", GMUI::VECTOR_INPUT, entity->rotation));
		Graphics.QueueMessage(new GMSetUIv3f("Scale", GMUI::VECTOR_INPUT, entity->scale));

		if (entity->physics){
			Graphics.QueueMessage(new GMSetUIf("Friction", GMUI::FLOAT_INPUT, entity->physics->friction));
			Graphics.QueueMessage(new GMSetUIf("Restitution", GMUI::FLOAT_INPUT, entity->physics->restitution));
		}

		Graphics.QueueMessage(new GMSetUIs("EntityDiffuseTexture", GMUI::TEXTURE_INPUT_SOURCE, entity->GetTextureSource(DIFFUSE_MAP)));
		Graphics.QueueMessage(new GMSetUIs("EntityModel", GMUI::STRING_INPUT_TEXT, entity->model->Source()));
	}
}

/// Input functions for the various states
void EditorState::MouseClick(bool down, int x, int y, UIElement * elementClicked){
	/// If new state (should be, but yeah)
	if (down != lButtonDown){
		/// Mouse press
		if (down){
			startMouseX = (float)x;
			startMouseY = (float)y;
		}
		/// Mouse release
		else {
			// Simple "click"-selection if the coordinates haven't moved more than 2 pixels in either direction ^.^
			if (abs(startMouseX - mouseX) < 2.0f &&
				abs(startMouseY - mouseY) < 2.0f &&
				elementClicked == NULL)
			{
				if (!Input.KeyPressed(KEY::CTRL))
					editorSelection.Clear();

				Ray clickRay = mainCamera->GetRayFromScreenCoordinates(mouseX, mouseY);
				std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;

				switch(editMode){
					case NAVMESH: case PATHS:{
						/// Try select waypoint closest to the ray?
						NavMesh * nm = WaypointMan.ActiveNavMesh();
						std::cout<<"\nNo active NavMesh! Generate one first!";
						if (nm == NULL)
							return;
						Waypoint * wp = nm->GetClosestToRay(clickRay);
						if (waypointSelection.Exists(wp))
							waypointSelection.Remove(wp);
						else
							waypointSelection.Add(wp);
						OnWaypointSelectionUpdated();
						break;
					}
					case ENTITIES:{

						// Sort, then select closest entity that is within the ray.
						Selection entities = MapMan.GetEntities();
						Vector3f camPos = mainCamera->Position();
						entities.SortByDistance(camPos);
						for (int i = 0; i < entities.Size(); ++i){
							Entity * entity = entities[i];
							Vector3f camToEntity = entity->position - camPos;
							Vector3f camToEntityNormalized = camToEntity.NormalizedCopy();
							float dotProductEntityToVector = clickRay.direction.DotProduct(camToEntityNormalized);
							if (dotProductEntityToVector < 0){
								std::cout<<"\nCulling entity "<<entity->name<<" as it is beind the camera direction.";
								continue;
							}
							float distanceProjectedOntoClickRay = clickRay.direction.DotProduct(camToEntity);
							Vector3f projectedPointOnVector = camPos + distanceProjectedOntoClickRay * clickRay.direction;
							float distanceToVector = (entity->position - projectedPointOnVector).Length();
							float radius = entity->radius;
							if (entity->physics){
								radius = entity->physics->physicalRadius;
								switch(entity->physics->physicsShape){
								/*
									/// Check with mesh instead
									break;

									/// Do regular stuff as shown below
									break;
									*/
								case ShapeType::QUAD:{
									/// Compare with plaaaane
									std::cout<<"\nQuad/Tri";
									Quad quad = *(Quad*)entity->physics->shape;
									quad.Transform(entity->transformationMatrix);
									if (!RayQuadIntersection(clickRay, quad)){
										continue;
									}
									break;
								}
								case ShapeType::TRIANGLE:
								case ShapeType::PLANE: {
									std::cout<<"\nPlane";
									Plane plane = Plane(*((Plane*)entity->physics->shape));
									plane.Transform(entity->transformationMatrix);
									if(!RayPlaneIntersection(clickRay, plane))
										continue;
									break;
								}
								default: case ShapeType::SPHERE: case ShapeType::MESH:
									if (distanceToVector > radius){
									//	std::cout<<"\nEntity not intersecting, distance: "<<distanceToVector<<" radius: "<<std::setw(6)<<entity->radius<<" "<<entity->name;
										continue;
									}
									break;
								}
							}

							editorSelection.Add(entity);
							/// TODO: Add option that switches bettn continue and break here!
							break;
						}
						OnSelectionUpdated();
					}

				}
			}
			else {


				/// Find entities in a frustum in target location.
				// Begin by extracting the end point in the editorCamera frustum for where we clicked
				Vector4f startPoint = mainCamera->Position();
				Vector4f endPoint = Vector4f(0, 0, -mainCamera->farPlane, 1);
				endPoint = Matrix4f(mainCamera->Projection()) * endPoint;
				endPoint = Matrix4f(mainCamera->View()) * endPoint;



				std::cout<<"\nStartPoint: "<<startPoint<<" EndPoint1: "<<endPoint;
				//		std::cout<<" EndPoint2: "<<endPoint2;
			}
		}

	}
	mouseX = (float)x;
	mouseY = (float)y;
	lButtonDown = down;
}
void EditorState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

	mouseX = x;
	mouseY = y;
	rButtonDown = down;
}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void EditorState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * element){
	Camera * camera = Graphics.cameraToTrack;
	float diffX = mouseX - x;
	float diffY = mouseY - y;
	if (lDown){
		if(camera){
			if (Input.KeyPressed(KEY::CTRL)){
				Vector3f left = camera->LeftVector();
				Vector3f up = camera->UpVector();
				camera->position += camera->LeftVector() * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
				camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
			}
			else {
				camera->rotation.x += diffY / 100.0f;
				camera->rotation.y -= diffX / 100.0f;
			}
		}
	}
	else if (rDown){
		if(camera){
			if (Input.KeyPressed(KEY::CTRL)){
				float camDist = AbsoluteValue(camera->distanceFromCentreOfMovement);
				camera->distanceFromCentreOfMovement += diffY * log(camDist);
		/*		if (diffY > 0){
					camera->distanceFromCentreOfMovement *= 0.8f;
				}
				else if (diffY < 0){
					camera->distanceFromCentreOfMovement *= 1.25f;
				}
				if (camera->distanceFromCentreOfMovement > 0)
					camera->distanceFromCentreOfMovement = 0;
					*/
			}
			else {
				camera->position += camera->LeftVector() * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
				camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
			}
		}
	}

	mouseX = x;
	mouseY = y;
}

void EditorState::MouseWheel(float delta){
	Camera * camera = Graphics.cameraToTrack;
	camera->distanceFromCentreOfMovement += delta / 100.0f;
	if (delta > 0){
		camera->distanceFromCentreOfMovement *= 0.8f;
	}
	else if (delta < 0){
		camera->distanceFromCentreOfMovement *= 1.25f;
	}
	if (camera->distanceFromCentreOfMovement > 0)
		camera->distanceFromCentreOfMovement = 0;
}

/// Callback from the Input-manager, query it for additional information as needed.
void EditorState::KeyPressed(int keyCode, bool downBefore)
{
	if (Input.HoverElement())
		return;
	switch(keyCode){
		case KEY::ESCAPE:
			editorSelection.Clear();
			OnSelectionUpdated();
			break;
		case KEY::ENTER:
		{
			/// If nothing is active, make active the command console input.
			MesMan.QueueMessages("BeginInput(CommandLine)");
			break;
		}
	}
}


// Loads target map o-o
bool EditorState::LoadMap(String fromFile){
	editorSelection.Clear();
	if (!fromFile.Contains(".map"))
		fromFile = fromFile + ".map";
	if (!fromFile.Contains("map/racing")){
		fromFile = "map/racing/" + fromFile;
	}
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTextureBySource("img/loading_map.png")));
	std::cout<<"\nLoadMap called for file: "<<fromFile;
	String filename = fromFile;
	Map * loadedMap = MapMan.LoadMap(filename.c_str());
	// Set map to be active!
	if (loadedMap){
		MapMan.MakeActive(loadedMap);
		ConsoleLog("Map loaded successfully.");
	}
	else {
		ConsoleLog("Unable to load map. Reason: "+MapMan.GetLastErrorString());
	}
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	lastActiveMap = loadedMap;
	// Clear any selected entities and navmeshstuffs too, yo.
	waypointSelection.Clear();
	OnWaypointSelectionUpdated();
	return loadedMap;
}


void EditorState::TranslateActiveEntities(Vector3f distance){
	Physics.QueueMessage(new PMSetEntity(TRANSLATE, editorSelection, distance));
}
void EditorState::SetScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, editorSelection, scale));
}
void EditorState::ScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SCALE, editorSelection, scale));
	ConsoleLog("Scaling "+String::ToString(editorSelection.Size())+" entities by scale: "+VectorString(scale));
	SleepThread(100);
	// Update UI accordingly.
	OnSelectionUpdated();
}
void EditorState::RotateActiveEntities(Vector3f rotation)
{
	for (int i = 0; i < editorSelection.Size(); ++i){
		if (!editorSelection[i])
			continue;
		editorSelection[i]->Rotate(rotation);
	}
}

/// Handle drag-n-drop files.
void EditorState::HandleDADFiles(List<String> & files){
	for (int i = 0; i < files.Size(); ++i){
		String file = files[i];
		file.ConvertToChar();
		file.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (file.Contains(".obj")){
			// Load it
			Model * model = ModelMan.LoadObj(file);
			if (editMode == ENTITIES){
				Entity * e = MapMan.CreateEntity(model, NULL);
			}
			else if (editMode == SHIP_EDITOR){
				Graphics.QueueMessage(new GMSetEntity(activeShipEntity, MODEL, model));
				activeShip->modelSource = model->Source();
			}
		}
	}
}

/// Attempts to save this map into given filename.
bool EditorState::SaveMap(String fileName, bool force)
{
	Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/saving_map.png"));
	bool physicsWasPaused = Physics.IsPaused();
	Physics.Pause();
	std::cout<<"\nInput>>SAVE_MAP";
	String path = fileName;
	if (!path.Contains("map/racing/"))
		path = "map/racing/" + path;

	if (!path.Contains(".map"))
	{
		path += ".map";
	}
	/// Perform check if file exists.
	if (!force && FileExists(path)){
		ConsoleLog("Unable to save. File already exists. Override with \'force\'");
		Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
		return false;
	}

	bool result = MapMan.SaveMap(path);
	if (result){
		ConsoleLog("Map saved successfully to "+path);
	}
	else {
		ConsoleLog("Unable to save. Reason: \n"+MapMan.GetLastErrorString());
	}
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	if (!physicsWasPaused)
		Physics.Resume();

	return result;
}

/// Outputs target string to graphical log.
void EditorState::ConsoleLog(String str)
{
	Graphics.QueueMessage(new GMSetUIs("OutputData", GMUI::TEXT, str));
}

void EditorState::ReturnToLastActiveMap(){
	// Last map if we were editing something earlier..!
	if (lastActiveMap)
		MapMan.MakeActive(lastActiveMap);
	else {
		MapMan.MakeActive(EDITOR_MAP);
		lastActiveMap = MapMan.ActiveMap();
	}
}

/// And update UI and stuff!
void EditorState::SetMode(int mode){
	/// Clear UI of previous mode!
#define HIDE(b) Graphics.QueueMessage(new GMSetUIb(b, GMUI::VISIBILITY, false));
	switch(editMode){
		case ENTITIES:
			HIDE("EntitiesUI");
			HIDE("EntityManipList");
			break;
		case SHIP_EDITOR:
			HIDE("ShipTestUI");
		//	Graphics.QueueMessage(new GMSetUIb("ShipTestUI", GMUI::VISIBILITY, false));
			ReturnToLastActiveMap();
			Physics.Resume();
			break;
		case PATHS:
			Graphics.QueueMessage(new GMSetUIb("PathsUI", GMUI::VISIBILITY, false));
			break;
		case NAVMESH:
			Graphics.QueueMessage(new GMSetUIb("NavMeshUI", GMUI::VISIBILITY, false));
			Graphics.QueueMessage(new GMSetUIb("NavMeshModeList", GMUI::VISIBILITY, false));
			break;
	}

#define MAKE_VISIBLE(b) Graphics.QueueMessage(new GMSetUIb(b, GMUI::VISIBILITY, true));
	switch(mode){
		case ENTITIES:
			MAKE_VISIBLE("EntitiesUI");
			MAKE_VISIBLE("EntityManipList");
			break;
		case SHIP_EDITOR: {
			Physics.Pause();
			MAKE_VISIBLE("ShipTestUI");
			bool result = MapMan.MakeActive(SHIP_EDITOR_MAP);
			assert(result);
			break;
		}
		case PATHS:
			Graphics.QueueMessage(new GMSetUIb("PathsUI", GMUI::VISIBILITY, true));
			break;
		case NAVMESH:
			Graphics.QueueMessage(new GMSetUIb("NavMeshUI", GMUI::VISIBILITY, true));
			Graphics.QueueMessage(new GMSetUIb("NavMeshModeList", GMUI::VISIBILITY, true));
			break;
		default:
			std::cout<<"\nERROR: Bad mode supplied in EditorState::SetMode, returning...";
			return;
	}
	editMode = mode;
};


// Ship Editor
void EditorState::NewShip(){
	MapMan.DeleteEntities();
	Entity * e = MapMan.CreateEntity(NULL, NULL);
	StateProperty * s = new StateProperty(e);
	Ship * ship = ShipMan.CreateShipType("New shipType");
	RacingShipGlobal * rsg = new RacingShipGlobal(e, ship);
	s->SetGlobalState(rsg);
	e->state = s;
	activeShip = ship;
	activeShipEntity = e;
	rsg->AssignAI(new ThrusterTester(e, rsg));
	/// Update UI with the new stats and names of the ship.
	OnActiveShipUpdated();
}
void EditorState::SaveShip(String toPath){
	if (!toPath.Contains("Ships/"))
		toPath = "Ships/" + toPath;
	if (!toPath.Contains("SpaceRace/"))
		toPath = "SpaceRace/" + toPath;
	if (!toPath.Contains("data/"))
		toPath = "data/" + toPath;

	assert(editMode == SHIP_EDITOR);
	assert(activeShip && activeShipEntity);
	ShipMan.SaveShip(activeShip, toPath);
}
void EditorState::LoadShip(String fromFile){
	if (!fromFile.Contains("Ships/"))
		fromFile = "Ships/" + fromFile;
	if (!fromFile.Contains("SpaceRace/"))
		fromFile = "SpaceRace/" + fromFile;
	if (!fromFile.Contains("data/"))
		fromFile = "data/" + fromFile;

	Ship * ship = ShipMan.LoadShip(fromFile);
	if (ship == NULL){
		std::cout<<"\nERROR: Unable to load ship! Aborting stuffies.";
		return;
	}
	MapMan.DeleteEntities();
	Entity * e = MapMan.CreateEntity(ModelMan.GetModel(ship->modelSource), NULL);
	ship->AssignTexturesToEntity(e);
	StateProperty * s = new StateProperty(e);
	RacingShipGlobal * rsg = new RacingShipGlobal(e, ship);
	s->SetGlobalState(rsg);
	e->state = s;
	rsg->AssignAI(new ThrusterTester(e, rsg));

	activeShipEntity = e;
	activeShip = ship;
	OnActiveShipUpdated();
}

/// Update UI with the new stats and names of the ship.
void EditorState::OnActiveShipUpdated(){
	assert(activeShip);
	Graphics.QueueMessage(new GMSetUIs("ShipName", GMUI::TEXT, activeShip->name));
	Graphics.QueueMessage(new GMSetUIv3f("ThrusterPosition", GMUI::VECTOR_INPUT, activeShip->thrusterPosition));
}


/// For waypoint-pathy-pathing and updating what iz rendurrred o-o;
void EditorState::OnActivePathUpdated(){
	Graphics.QueueMessage(new GMSetPathToRender(*activePath));
}

/// Update what's rendered and stuff in UI too maybe?
void EditorState::OnWaypointSelectionUpdated(){
	Graphics.QueueMessage(new GMSetSelectedWaypoints(waypointSelection));
	//
	Graphics.QueueMessage(new GMSetUIs("NumSelectedWaypoints", GMUI::TEXT, String::ToString(waypointSelection.Size())));
}

/// Connects all selected waypoints :3
void EditorState::ConnectWaypoints(){
	if (waypointSelection.Size() <= 1)
		return;
	for (int i = 0; i < waypointSelection.Size(); ++i){
		Waypoint * wp = waypointSelection[i];
		for (int j = i+1; j < waypointSelection.Size(); ++j){
			Waypoint * wp2 = waypointSelection[j];
			wp->AddNeighbour(wp2);
			wp2->AddNeighbour(wp);
		}
	}
}


/// Attempts to generate a track path using only the selected waypoints and current navMesh as help.
void EditorState::GenerateTrackPath(){
	Path * trackPath, tempPath;
	trackPath = new Path();
	if (waypointSelection.Size() > 1){
		for (int i = 0; i < waypointSelection.Size()-1; ++i){
			PathMan.GetPath(waypointSelection[i], waypointSelection[i+1], tempPath);
			(*trackPath) += tempPath;
		}
		PathMan.GetPath(waypointSelection[waypointSelection.Size()-1], waypointSelection[0], tempPath);
	}
	(*trackPath)+= tempPath;
	trackPath->name = MAIN_TRACK_PATH_NAME;
	activePath = trackPath;
	OnActivePathUpdated();
	MapMan.AddPath(trackPath);
}

void EditorState::CreateCheckpoints(){
	assert(activePath);
	if (activePath == NULL){
		std::cout<<"\nERROR: Active path is NULL. Create or load one first, yaow!";
		return;
	}

	/// Delete any existing entities that are called "CheckPoint" first.
	Map * map = MapMan.ActiveMap();
	List<Entity*> entities = map->GetEntities();
	for (int i = 0; i < entities.Size(); ++i){
		Entity * e = entities[i];
		String name = e->name;
		name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (name.Contains("CheckPoint")){
			MapMan.DeleteEntity(e);
		}
	}

	if (activePath->Waypoints() <= 0)
		return;
	int checkpointsGenerated = 0;
	for (int i = 0; i < activePath->Waypoints()-1; i += checkPointWaypointInterval){
		Waypoint * wp = activePath->GetWaypoint(i);
		assert(wp);
		Model * model = NULL;
		if (checkpointModelSource.Length()){
			model = ModelMan.GetModel(checkpointModelSource);
		}
		if (model == NULL)
			model = ModelMan.GetModel("Checkpoints/FlatQuadCheckpoint");
		Texture * tex = TexMan.GetTextureBySource("Checkpoints/test");
		Entity * entity = MapMan.CreateEntity(model, tex);
		/// Set entity rotation and position.
		++checkpointsGenerated;
		entity->name = "CheckPoint" + String::ToString(checkpointsGenerated);
		Vector3f position = wp->position;
		Physics.QueueMessage(new PMSetEntity(POSITION, entity, position));
		/// Scale up it in Y and X
		Vector3f scale =  Vector3f(checkpointSize, checkpointSize, checkpointSize * 0.1f);
		Physics.QueueMessage(new PMSetEntity(SCALE, entity, scale));


		GraphicsProperty* gs = new GraphicsProperty();
		// Better add depth-sorting for alpha-entities..!
		// RenderFlags::DISABLE_DEPTH_WRITE |
		gs->flags = RenderFlags::DISABLE_BACKFACE_CULLING | RenderFlags::REQUIRES_DEPTH_SORTING;
		entity->graphics = gs;

		/// Get rotation via direction to next waypoint
		Waypoint * wp2 = activePath->GetWaypoint(i+1);
		Vector3f toNext = wp2->position - wp->position;
		toNext.Normalize();

		/// Prepare a vector to calculate rotation needed
		Vector3f toNextXZ = toNext;
		toNextXZ.y = 0;
		toNextXZ.Normalize();
		// Now get rotation needed, compared to Z = -1
		// cos angle = X;
		// angle = cos-1 X;
		float angle = asin(toNextXZ.x);
		// This yields two results, which will have to be checked depending on le z!
		if (toNextXZ.x > 0 && toNextXZ.z < 0){
			angle = (PI - angle);
		}
		else if (toNextXZ.x < 0 && toNextXZ.z < 0){
			angle = (-PI - angle);
		}
		std::cout<<"\nToNextXZ: "<<toNextXZ<<" Angle: "<<angle;
		/// Get rotational coordinates?

		Vector3f rotation = Vector3f();
		rotation.y = angle;
		Physics.QueueMessage(new PMSetEntity(ROTATE, entity, rotation));
		Physics.QueueMessage(new PMSetPhysicsShape(entity, ShapeType::CUBE));
		Physics.QueueMessage(new PMSetEntity(COLLISSION_CALLBACK, entity, true));
		Physics.QueueMessage(new PMSetEntity(NO_COLLISSION_RESOLUTION, entity, true));

		assert(entity);
		if (entity == NULL){
			return;
		}
	}
}
