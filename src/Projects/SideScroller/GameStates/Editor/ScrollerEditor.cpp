// Emil Hedemalm
// 2013-06-28

#include "Game/GameType.h"

#ifdef RUNE_RPG

#include "Graphics/Messages/GMSetEntity.h"
#include "OS/Sleep.h"
#include "Maps/2D/TileMapLevel.h"
#include "Maps/2D/TileMap2D.h"
#include "Maps/Grids/Tile.h"
#include "Maps/Grids/TileTypeManager.h"
#include "../Physics/PhysicsProperty.h"
#include "../Physics/Messages/PhysicsMessage.h"
#include "../UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "UI/UIImage.h"
#include "../Graphics/Messages/GMUI.h"
#include "Message/Message.h"
#include "Message/FileEvent.h"
#include "Message/VectorMessage.h"
#include "File/FileUtil.h"
#include "GameStates/GameStates.h"
#include "Maps/Grids/GridObject.h"
extern UserInterface * ui[MAX_GAME_STATES];

#include "Graphics/GraphicsManager.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"
#include "ModelManager.h"
#include "Message/MessageManager.h"
#include "UI/UIFileBrowser.h"

#include <iomanip>
/// Flum Microsoft function...
#undef CreateEvent
#include "ScrollerEditor.h"
#include "Event/Event.h"
#include "../Map/MapState.h"
#include "EntityStates/StateProperty.h"
#include "../../EntityStates/Player/ScrollerPlayerState.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"
#include "../../RuneDirectories.h"
#include "Entity/EntityFlags.h"
#include "Graphics/Messages/GMSet.h"
#include "../RuneGameStatesEnum.h"
#include <ctime>

/// For sprintf in Linux
#include <cstdio>


ScrollerEditor::ScrollerEditor()
: SideScrollerGameState() 
{
	std::cout<<"\nScrollerEditor constructor";
	id = GAME_STATE_EDITOR;
	stateName = "ScrollerEditorState";
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "ScrollerEditorState");
	mouseCameraState = 0;
	scrollerEditorCamera = new Camera();
	scrollerEditorCamera->flySpeed *= 20.0f;
	rootMapDir = "map/";
	// We want keypressed.
	keyPressedCallback = true;
	map = NULL;
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "ScrollerEditorState");
}

ScrollerEditor::~ScrollerEditor(){
	SAFE_DELETE(scrollerEditorCamera);
}

/// Set defaults!
void ScrollerEditor::ResetCamera(){
	std::cout<<"\nScrollerEditor::ResetCamera";
	scrollerEditorCamera->projectionType = Camera::PROJECTION_3D;
	scrollerEditorCamera->rotation = Vector3f();
	/// Don't call resetCamera if there's nothing to look at...
	scrollerEditorCamera->position = Vector3f();
	scrollerEditorCamera->zoom = 10.f;
	scrollerEditorCamera->farPlane = -5000.0f;
	scrollerEditorCamera->Update();
}

void ScrollerEditor::CreateUserInterface(){
	std::cout<<"\nScrollerEditor::CreateUserInterface: "<<stateName;
	assert(stateName == "ScrollerEditorState");
	
	if (ui)
		delete ui;
	this->ui = new UserInterface();
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "ScrollerEditorState");	
	String source = "gui/Editor.gui";
	std::cout<<"ScrollerEditor::CreateUserInterface: "<<source;
	ui->Load(source);
}

void ScrollerEditor::OnEnter(GameState * previousState){

	std::cout<<"\nScrollerEditor::OnEnter";
	/// Create a map upon entering instead?
	if (!map){
		std::cout<<"\nCreating map";
		map = MapMan.CreateMap("default.map");
		ResetCamera();
	}

	SetCameraFreeFly();
//	SetCameraOrthogonal();

	// Load initial texture and set it to render over everything else
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));
	
	/// Turn off UI-navigation.
	Input.ForceNavigateUI(false);

	/// Turn off any requirement to have an active UI element as it interferes with the mouse-hover-detection mechanism currently in place?
	

	// Close the menu if it's up.
	Graphics.QueueMessage(new GMPopUI("MainMenu"));

	/// Check amount of tiles.
	MapMan.processOnEnter = false;
	MapMan.MakeActive(map);
	MapMan.ClearEventSpawnedEntities();
	MapMan.ClearPlayerEntities();
	MapMan.processOnEnter = true;

	/// Disable AI-rendering
	Graphics.renderAI = false;
	Graphics.renderNavMesh = false;
	Graphics.renderGrid = true;
	Graphics.renderNavMesh = true;
	Graphics.renderPhysics = false;
	Graphics.renderFPS = true;
	Graphics.renderLights = false;

	std::cout<<"\nrrrr5";
	/// Set ScrollerEditor selection as the renderable one!
	Graphics.selectionToRender = &editorSelection;

	// And set it as active

	/// Make ScrollerEditor map active!
	assert(MapMan.MakeActive(map));

	std::cout<<"\nrrrr6";
	// Load the AI scene
	// Don't load anytheing o-o;
/*	if (!MapMan.MakeActive(&ScrollerEditorMap)){
		Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
		Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
	}
*/
	/// Reset physics settings
	Physics.QueueMessage(new PhysicsMessage(PM_RESET_SETTINGS));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));

	std::cout<<"\nrrrr";
	OnSelectionUpdated();
}

void ScrollerEditor::OnExit(GameState *nextState){
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	if (nextState->GetID() == GAME_STATE_MAIN_MENU)
		MapMan.MakeActive(NULL);
	std::cout<<"\nLeaving ScrollerEditor state.";
}

/// For key-bindings.
void ScrollerEditor::CreateDefaultBindings()
{
	SideScrollerGameState::CreateCameraBindings();
	inputMapping.CreateBinding("Next entity", KEY::TAB);
	inputMapping.CreateBinding("Previous entity", KEY::CTRL, KEY::TAB);
	inputMapping.CreateBinding("Delete", KEY::DELETE_KEY);
}


/// For key-bindings.
void ScrollerEditor::InputProcessor(int action, int inputDevice /*= 0*/)
{
	
}

/// Last update time in ms
clock_t lastTime = 0;
void ScrollerEditor::Process(float time){
	/// Process key input for navigating the 3D - Space
	Sleep(20);

	/// Set camera position! D:
	if (mainCamera)
		Graphics.QueueMessage(new GMSetUIv3f("CameraPosition", GMUI::VECTOR_INPUT, mainCamera->Position()));

    time_t currentTime = Timer::GetCurrentTimeMs();

	/// Fly! :D
	/// Rotate first, yo o.O
	/// Rotation multiplier.
	float rotMultiplier = 0.05f;
	scrollerEditorCamera->rotation += scrollerEditorCamera->rotationVelocity * scrollerEditorCamera->rotationSpeed * (float)time;
	// Check input for moving camera
	if (scrollerEditorCamera->velocity.Length() > 0){
		Vector4d moveVec;
		moveVec = Vector4d(scrollerEditorCamera->velocity);
		/// Flight-speed multiplier.
		float multiplier = 0.5f * scrollerEditorCamera->flySpeed;
		moveVec = moveVec * multiplier * (float)time * scrollerEditorCamera->zoom;
		Matrix4d rotationMatrix;
		rotationMatrix.InitRotationMatrixY(-scrollerEditorCamera->rotation.y);
		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-scrollerEditorCamera->rotation.x));
		moveVec = rotationMatrix.product(moveVec);
		scrollerEditorCamera->position += Vector3f(moveVec);
	}
};

/// Callback function that will be triggered via the MessageManager when messages are processed.
void ScrollerEditor::ProcessMessage(Message * message){
	std::cout<<"\nScrollerEditor::ProcessMessage: ";
	switch(message->type)
	{
		case MessageType::DATA_MESSAGE:
		{
			break;
		}
		case MessageType::TEXTURE_MESSAGE: 
		{
			String msg = message->msg;
			TextureMessage * tm = (TextureMessage*) message;
			if (msg == "SetEntityDiffuseTexture")
			{
				/// Derp.
				if (!editorSelection.Size())
					return;
				Entity * entity = editorSelection[0];
				Graphics.QueueMessage(new GMSetEntityTexture(entity, DIFFUSE_MAP, tm->texSource));
			}
			break;
		}
		case MessageType::SET_STRING: 
		{
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			String msg = message->msg;
			VectorMessage * vec = (VectorMessage*) message;
			if (msg == "SetEntityScale")
				Physics.QueueMessage(new PMSetEntity(SET_SCALE, editorSelection, vec->vec3f));
			else if (msg == "SetEntityPosition")
				Physics.QueueMessage(new PMSetEntity(SET_POSITION, editorSelection, vec->vec3f));
			else if (msg == "SetEntityRotation")
				Physics.QueueMessage(new PMSetEntity(SET_ROTATION, editorSelection, vec->vec3f));
			break;
		}
		case MessageType::FILE_EVENT: 
		{
			FileEvent * fe = (FileEvent*)message;
			String action = fe->msg;
			List<String> files = fe->files;
			String mainFile = files[0];
			if (action == "SaveMap"){
				std::cout<<"Stuff";
				// Save path to save anyway.
				this->mapFilePath = mainFile;
				// Check for overriting and present a query message if so?
				if (FileExists(mainFile)){
					MesMan.QueueMessages("Query(SaveMap,Overrite file,File already exists. Overwrite it?)");
				}
				// If not, save straight away.
				else 
					SaveMap();
			}
			else if (action == "LoadMap"){
				LoadMap(mainFile);
			}
			break;							  
		}
		case MessageType::STRING: {
			String string = message->msg;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (SideScrollerGameState::HandleCameraMessages(string))
				return;
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "Save map")
			{
				// Open file dialogue for it?
				UIFileBrowser * browser = new UIFileBrowser("Save map", "SaveMap", ".map");
				browser->CreateChildren();
				browser->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(browser));
				Graphics.QueueMessage(new GMPushUI("Save map"));
			}
			else if (string == "Next entity")
			{
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
			}
			else if (string == "Previous entity")
			{
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
			}
			else if (string == "Clear selection")
			{
				editorSelection.Clear();
				std::cout<<"\nClearing selection";
				OnSelectionUpdated();
			}
			else if (string == "Delete")
			{
				while(editorSelection.Size()){
					Entity * e = editorSelection[0];
					MapMan.DeleteEntity(e);
					editorSelection.Remove(e);
				}
				OnSelectionUpdated();
			}
			else if (string == "NewEntity")
			{
				/// Create one
				Entity * entity = MapMan.CreateEntity(ModelMan.GetModel("standingPlane"), TexMan.GetTexture("default.png"));
				Physics.QueueMessage(new PMSetPhysicsShape(entity, ShapeType::CUBE));
				Vector3f pos = mainCamera->Position();
				pos.z = 0;
				Physics.QueueMessage(new PMSetEntity(POSITION, entity, pos));
				std::cout<<"\nEntity created";
				if (entity){
					editorSelection.Clear();
					editorSelection.Add(entity);
					OnSelectionUpdated();
				}
				Graphics.QueueMessage(new GMSetUIs("NumEntitiesInMap", GMUI::TEXT, "Entities in map: "+String::ToString(MapMan.ActiveMap()->NumEntities())));
			}
			else if (string == "OnReloadUI"){

			}
			else if (string == "SaveMap"){
				bool success = SaveMap();
				if (!success){
					std::cout<<"ERROR: Failed to save.";
				}
			}
			else if (string == "LoadMap"){
				UIElement * e = ui->GetElementByName("LoadMap");
				String mapName = e->text;
				LoadMap(mapName);
			}
			else if (string == "TestLevel"){
				Playtest();
				return;
			}
			else if (string.Contains("SetEventName")){
				assert(selectedEvent);
				String name = ui->GetElementByName("EventName")->text;
				assert(name.Length() > 2);
				selectedEvent->name = name;
			}
			else if (string.Contains("SetEventSource")){
				assert(selectedEvent);
				String source = ui->GetElementByName("EventSource")->text;
				assert(source.Length() > 2);
				selectedEvent->source = source;
			}
		}
	}
	GameState::ProcessMessage(message);
}

/// Called every time the current selection is updated.
void ScrollerEditor::OnSelectionUpdated(){
	std::cout<<"\nScrollerEditor::OnSelectionUpdated()";
	if (editorSelection.Size() == NULL){
		Graphics.QueueMessage(new GMSetUIb("EntityManipList", GMUI::VISIBILITY, false));
		return;
	}
	Entity * entity = editorSelection[0];

	// Reveal the UI
	Graphics.QueueMessage(new GMSetUIb("EntityManipList", GMUI::VISIBILITY, true));

	Graphics.QueueMessage(new GMSetUIs("EntityName", GMUI::STRING_INPUT_TEXT, entity->name));

	Graphics.QueueMessage(new GMSetUIv3f("EntityPosition", GMUI::VECTOR_INPUT, entity->positionVector));
	Graphics.QueueMessage(new GMSetUIv3f("EntityRotation", GMUI::VECTOR_INPUT, entity->rotationVector));
	Graphics.QueueMessage(new GMSetUIv3f("EntityScale", GMUI::VECTOR_INPUT, entity->scaleVector));

	if (entity->physics){
		Graphics.QueueMessage(new GMSetUIf("Friction", GMUI::FLOAT_INPUT, entity->physics->friction));
		Graphics.QueueMessage(new GMSetUIf("Restitution", GMUI::FLOAT_INPUT, entity->physics->restitution));
	}
	Graphics.QueueMessage(new GMSetUIs("EntityDiffuseTexture", GMUI::TEXTURE_INPUT_SOURCE, entity->GetTextureSource(DIFFUSE_MAP)));
}

/// Input functions for the various states
void ScrollerEditor::MouseClick(bool down, int x, int y, UIElement * elementClicked){
	/// Don't do anything if we're over a UI-element, yo.
	if (elementClicked)
		return;

	// If left mouse button was just clicked down...
	if (down){
		/// Select entities and start movement as well		
		if (!Input.KeyPressed(KEY::CTRL))
			editorSelection.Clear();

		Ray clickRay = GetRayFromScreenCoordinates(mouseX, mouseY, *mainCamera);
		std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;
		Vector3f pos = clickRay.start;
		moveStartPosition = pos;
		/// Fetch closest entity? or just first one?
		List<Entity*> entities = map->GetEntities();
		for (int i = 0; i < entities.Size(); ++i)
		{
			/// Assume rectangular selection for all entities.
			Entity * entity = entities[i];
			Vector3f ePos = entity->positionVector;
			Vector3f eScale = entity->scaleVector * 0.5f;
			if (ePos.x + eScale.x < pos.x ||
				ePos.y + eScale.y < pos.y ||
				ePos.x - eScale.x > pos.x ||
				ePos.y - eScale.y > pos.y)
				continue;
			editorSelection.Add(entity);
		}
		OnSelectionUpdated();
	}
	/// If released the left mouse button.
	else {
		
	}
	/// If new state (should be, but yeah)
	if (down != lButtonDown){
		/// Mouse press
		if (down){
			startMouseX = (float)x;
			startMouseY = (float)y;
		}
		/// Mouse release
		else {

		}

	}
	mouseX = (float)x;
	mouseY = (float)y;
	lButtonDown = down;
}
void ScrollerEditor::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
	/// Don't do anything if we're over a UI-element, yo.
	if (elementClicked)
		return;
	mouseX = (float)x;
	mouseY = (float)y;
	rButtonDown = down;
}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void ScrollerEditor::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){

//	std::cout<<"\nMouse move "<<x<<y;
	/// Don't do anything if we're over a UI-element, yo.
	if (elementOver)
		return;

	Camera * camera = Graphics.cameraToTrack;
	float diffX = mouseX - x;
	float diffY = mouseY - y;

	if (!map)
		return;

	/// If mouse down, move active selection!
	if (lDown){
		Ray clickRay = GetRayFromScreenCoordinates(mouseX, mouseY, *mainCamera);
		Vector3f newPos = clickRay.start;
		Vector3f diff = newPos - moveStartPosition;
		Physics.QueueMessage(new PMSetEntity(TRANSLATE, editorSelection, diff));
		moveStartPosition = newPos;
	}

	/*
	Plane plane;
	plane.Set3Points(Vector3f(0,-1,0), Vector3f(1,0,0), Vector3f(0,0,0));
	Vector3f collissionPoint;
	Ray clickRay = GetRayFromScreenCoordinates(x, y, *ScrollerEditorCamera);
	if (RayPlaneIntersection(clickRay, plane, &collissionPoint))
	{
		cursorPosition = cursorPositionPreRounding = collissionPoint;
		cursorPosition.Round();
	}
	*/
	/// Update cursor position!
	int xi = (int)cursorPosition.x, yi = (int)cursorPosition.y;
	String posString = "x: " + String::ToString(xi) +
		" y:" + String::ToString(yi);
	Graphics.QueueMessage(new GMSetUIs("CursorPosition", GMUI::TEXT, posString));

	if (lDown){
		/// Drag map if holding ctrl or space?
		if (Input.KeyPressed(KEY::CTRL) || Input.KeyPressed(KEY::SPACE)){
			if(camera){
				if (Input.KeyPressed(KEY::CTRL)){
					Vector3f dist = camera->UpVector().CrossProduct(camera->LookingAt()) * diffX / 100.0f * PAN_SPEED_MULTIPLIER 
						- camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
					camera->position += dist * camera->zoom * 0.5f;
				}
				else if (Input.KeyPressed(KEY::SPACE)){
					camera->rotation.x += diffY / 100.0f;
					camera->rotation.y -= diffX / 100.0f;
				}
			}
		}
		else {
		}
	}
	else if (rDown){
		if(camera){
			if (Input.KeyPressed(KEY::CTRL)){
				camera->distanceFromCentreOfMovement += diffY;
				if (camera->distanceFromCentreOfMovement > 0)
					camera->distanceFromCentreOfMovement = 0;
			}
			else {
			}
		}
	}
	else {
		// Neither l nor r down, reset previousCursorPosition?
		previousCursorPosition = cursorPosition;
	}

	mouseX = x;
	mouseY = y;
}

void ScrollerEditor::MouseWheel(float delta){

	/// Camera if CTRL is held
	if (!Input.KeyPressed(KEY::CTRL)){
		Camera * camera = Graphics.cameraToTrack;
		camera->zoom -= delta / 100.0f;
		if (delta < 0){
			camera->zoom *= 1.25f;
		}
		else if (delta > 0){
			camera->zoom *= 0.8f;
		}
		if (camera->zoom < 1)
			camera->zoom = 1;
	}
	else {
	}
}

/// Callback from the Input-manager, query it for additional information as needed.
void ScrollerEditor::KeyPressed(int keyCode, bool downBefore){
//	std::cout<<"\nOpHening Mhenu.";
	switch(keyCode){
		case KEY::ESCAPE: {
//			std::cout<<"\nOpHening Mhenu.";
			if (editorSelection.Size())
			{
				editorSelection.Clear();
				OnSelectionUpdated();
				return;
			}
			UIElement * menu = ui->GetElementBySource("gui/EditorMenu.gui");
			if (!menu || menu->visible == false)
				Graphics.QueueMessage(new GMPushUI("gui/EditorMenu.gui"));
			break;
		}
	}
}

// For rendering extra editor-specific content like the current cursor position and stuff... doesn't feel fitting to add that to TileMap2D nor in the GraphicsManager.
void ScrollerEditor::Render(GraphicsState & graphicsState){

    glUseProgram(0);
	// Set projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix4d mat;
	mat.InitOrthoProjectionMatrix();
	glLoadMatrixd(mat.getPointer());
	// Load identity to model matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	glMatrixMode(GL_PROJECTION);
	graphicsState.camera->SetRatio(Graphics.Width(), Graphics.Height());
	graphicsState.camera->Update();
	Matrix4f projectionMatrix = graphicsState.camera->ProjectionMatrix4d();
	glLoadMatrixd(graphicsState.camera->ProjectionMatrix4d().getPointer());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(graphicsState.camera->ViewMatrix4d().getPointer());

	/// Fill mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/// Bind texture depending on mode
}

/// Transitions to the level test state!
void ScrollerEditor::Playtest(){

	GameState * gs = StateMan.GetState(RUNE_GAME_STATE_MAP);
	MapState * ms = (MapState*) gs;
	ms->SetEnterMode(EnterMode::TESTING_MAP);
	ms->SetCamera(*scrollerEditorCamera);
	ms->activeMap = map;
	/// Check for player character/spawn, if not, place somewhere within camera range.
	Entity * e = map->GetEntity("Player");
	if (e == NULL){
		Model * m = ModelMan.GetModel("Sprite");
		Texture * t = TexMan.GetTexture("RuneRPG/Units/200");
	}		
	if (e)
		e->flags |= PLAYER_OWNED_ENTITY;
	StateMan.QueueState(RUNE_GAME_STATE_MAP);
}



void ScrollerEditor::TranslateActiveEntities(Vector3f distance){
	Physics.QueueMessage(new PMSetEntity(TRANSLATE, editorSelection, distance));
}
void ScrollerEditor::SetScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, editorSelection, scale));
}
void ScrollerEditor::ScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SCALE, editorSelection, scale));
}
void ScrollerEditor::RotateActiveEntities(Vector3f rotation){
	for (int i = 0; i < editorSelection.Size(); ++i){
		if (!editorSelection[i])
			continue;
		editorSelection[i]->rotate(rotation);
	}
}



/// Saves the map to set mapFilePath. Assumes any file-checks have been done beforehand. Pauses input and physics while saving.
bool ScrollerEditor::SaveMap(){
	Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/saving_map.png"));
	bool physicsWasPaused = Physics.IsPaused();
	Physics.Pause();
	// Pause input too?
	Input.acceptInput = false;
	/*
	if (!filename.Contains(ROOT_MAP_DIR)){
		filename = ROOT_MAP_DIR + filename;
	}*/
	assert(mapFilePath.Length());
	bool success = MapMan.SaveMap(mapFilePath);
	if (success){
		Graphics.QueueMessage(new GMSetUIs("OutputData", GMUI::TEXT, "Saved to file "+mapFilePath+" succeeded"));
	}
	
	// Close the menu if it's open
	Graphics.QueueMessage(new GMPopUI("MainMenu"));
	// Resume control
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	if (!physicsWasPaused)
	Physics.Resume();
	Input.acceptInput = true;
	return true;
}

/// Attempts to load target map
bool ScrollerEditor::LoadMap(String fromFile)
{
	if (!fromFile.Contains(".map"))
		fromFile += ".map";
	Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/loading_map.png"));
	bool physicsWasPaused = Physics.IsPaused();
	Physics.Pause();
	// Pause input too?
	Input.acceptInput = false;
	// Clear selection if any
	editorSelection.Clear();
	// Load the new map.
	Map * loadMap = MapMan.LoadMap(fromFile);
	if (loadMap){
		MapMan.MakeActive(loadMap);
		map = (TileMap2D*)loadMap;
	}
	// Close the menu if it's open
	Graphics.QueueMessage(new GMPopUI("EditorMenu"));
	// Resume control
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	if (!physicsWasPaused)
	Physics.Resume();
	Input.acceptInput = true;
	return true;
}


void ScrollerEditor::SetMapSize(int xSize, int ySize){
	Graphics.PauseRendering();
	MapMan.DeleteEntities();
	MapMan.DeleteEvents();
	Graphics.ResumeRendering();
}

#endif
