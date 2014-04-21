// Emil Hedemalm
// 2013-06-28

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
#include "Actions.h"
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
#include "Graphics/Messages/GMLight.h"

#include <iomanip>
/// Flum Microsoft function...
#undef CreateEvent
#include "RuneEditor.h"
#include "Event/Event.h"
#include "../Map/MapState.h"
#include "EntityStates/StateProperty.h"
#include "../../EntityStates/RREntityState.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"
#include "../../RuneDirectories.h"
#include "Entity/EntityFlags.h"
#include "Graphics/Messages/GMSet.h"
#include "../RuneGameStatesEnum.h"

/// For sprintf in Linux
#include <cstdio>


RuneEditor::RuneEditor()
: GameState() {
	std::cout<<"\nRuneEditor constructor";
	id = GAME_STATE_EDITOR;
	stateName = "RuneEditorState";
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "RuneEditorState");
	mouseCameraState = 0;
	editMode = EditMode::TILES;
	tileBrushType = terrainBrushType = brushType = SQUARE;
	tileBrushSize = terrainBrushSize = brushSize = 0;
	terrainTypeSelected = 0;
	TileTypes.LoadTileTypes("data/tiles.txt");
	tileType = TileTypes.GetTileTypeByIndex(0);
	selectedEvent = NULL;
	runeEditorCamera = new Camera();
	runeEditorCamera->flySpeed = 20.0f;
	rootMapDir = "map/";
	// We want keypressed.
	keyPressedCallback = true;
	map = NULL;
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "RuneEditorState");
	objectType = NULL;
}
RuneEditor::~RuneEditor(){
	SAFE_DELETE(runeEditorCamera);
}

/// Set defaults!
void RuneEditor::ResetCamera(){
	std::cout<<"\nRuneEditor::ResetCamera";
	runeEditorCamera->projectionType = Camera::ORTHOGONAL;
	runeEditorCamera->rotation = Vector3f();
	TileMapLevel * level = map->ActiveLevel();
	/// Don't call resetCamera if there's nothing to look at...
	assert(level);
	Vector2i size = level->Size();
	runeEditorCamera->position = Vector3f(-size.x*0.5f, -size.y*0.5f, -10);
	runeEditorCamera->zoom = 10.f;
	runeEditorCamera->farPlane = -50.0f;
	runeEditorCamera->Update();
}

void RuneEditor::CreateUserInterface(){
	std::cout<<"\nRuneEditor::CreateUserInterface: "<<stateName;
	assert(stateName == "RuneEditorState");
	
	if (ui)
		delete ui;
	this->ui = new UserInterface();
	std::cout<<"\nState name: "<<stateName;
	assert(stateName == "RuneEditorState");
	
	String source = "gui/" + stateName + ".gui";
	std::cout<<"RuneEditor::CreateUserInterface: "<<source;
	ui->Load(source);
//	ui->Load("gui/RuneEditor.gui");
}

void RuneEditor::OnEnter(GameState * previousState){

	std::cout<<"\nRuneEditor::OnEnter";
	/// Create a map upon entering instead?
	if (!map){
		std::cout<<"\nCreating map";
		map = MapMan.CreateMap2D("default.2dmap");
		map->SetSize(10, 10);
		newSize = map->Size();
		ResetCamera();
	}
	
	// Load initial texture and set it to render over everything else
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));
	OnEditModeUpdated(-1);

	TileMapLevel * level = map->ActiveLevel();
	
	/// Turn off UI-navigation.
	Input.NavigateUI(false);
	/// Turn off any requirement to have an active UI element as it interferes with the mouse-hover-detection mechanism currently in place?
	
	// Close the menu if it's up.
	Graphics.QueueMessage(new GMPopUI("MainMenu"));

	/// Check amount of tiles.
	MapMan.processOnEnter = false;
	MapMan.MakeActive(map);
	MapMan.ClearEventSpawnedEntities();
	MapMan.ClearPlayerEntities();
	MapMan.processOnEnter = true;

	Graphics.QueueMessage(new GMSet(ACTIVE_2D_MAP_TO_RENDER, map));

	/// Disable AI-rendering
	Graphics.renderAI = false;
	Graphics.renderNavMesh = false;
	Graphics.renderGrid = false;
	Graphics.renderNavMesh = true;
	Graphics.renderPhysics = true;
	Graphics.renderFPS = true;
	Graphics.renderLights = false;

	std::cout<<"\nrrrr5";
	/// Set RuneEditor selection as the renderable one!
	Graphics.selectionToRender = &runeEditorSelection;

	// And set it as active
	Graphics.cameraToTrack = runeEditorCamera;
	runeEditorCamera->SetRatio(Graphics.width, Graphics.height);

	/// Make RuneEditor map active!
	assert(MapMan.MakeActive(map));

	std::cout<<"\nrrrr6";
	// Load the AI scene
	// Don't load anytheing o-o;
/*	if (!MapMan.MakeActive(&RuneEditorMap)){
		Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
		Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
	}
*/
	/// Reset physics settings
	Physics.QueueMessage(new PhysicsMessage(PM_RESET_SETTINGS));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));

	std::cout<<"\nrrrr";

	/// Keyboard navigation!
	Input.ForceNavigateUI(true);

	/// Update ui
	OnTileTypesUpdated();
	OnTileTypeSelected();
}

void RuneEditor::OnExit(GameState *nextState){
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	if (nextState->GetID() == GAME_STATE_MAIN_MENU)
		MapMan.MakeActive(NULL);
	std::cout<<"\nLeaving RuneEditor state.";
}


/// Last update time in ms
#include <ctime>
clock_t lastTime = 0;
void RuneEditor::Process(float time){
	/// Process key input for navigating the 3D - Space
	Sleep(20);

    time_t currentTime = Timer::GetCurrentTimeMs();

	///// Fly! :D
	///// Rotate first, yo o.O
	///// Rotation multiplier.
	//float rotMultiplier = 0.05f;
	//runeEditorCamera->rotation += runeEditorCamera->rotationVelocity * runeEditorCamera->rotationSpeed * (float)time;
	//// Check input for moving camera
	//if (runeEditorCamera->velocity.Length() > 0){
	//	Vector4d moveVec;
	//	moveVec = Vector4d(runeEditorCamera->velocity);
	//	/// Flight-speed multiplier.
	//	float multiplier = 0.5f * runeEditorCamera->flySpeed;
	//	moveVec = moveVec * multiplier * (float)time * runeEditorCamera->zoom;
	//	Matrix4d rotationMatrix;
	//	rotationMatrix.InitRotationMatrixY(-runeEditorCamera->rotation.y);
	//	rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-runeEditorCamera->rotation.x));
	//	moveVec = rotationMatrix.product(moveVec);
	//	runeEditorCamera->position += Vector3f(moveVec);
	//}
};

/// Callback function that will be triggered via the MessageManager when messages are processed.
void RuneEditor::ProcessMessage(Message * message){
	std::cout<<"\nRuneEditor::ProcessMessage: ";
	switch(message->type)
	{
		case MessageType::DATA_MESSAGE:
		{
			DataMessage * dm = (DataMessage*) message;
			String msg = message->msg;
			if (msg == "SetObjectOccupationMatrix"){
				for (int i = 0; i < objectType->passability.Size() && dm->binaryData.Size(); ++i){
					*objectType->passability[i] = dm->binaryData[i];
				}
				objectType->UpdatePivotPosition();
			}
			break;
		}
		case MessageType::TEXTURE_MESSAGE: 
		{
			TextureMessage * tm = (TextureMessage*) message;
			String msg = message->msg;
			if (msg == "SetObjectPrimaryTexture")
			{
				objectType->textureSource = tm->texSource;
				OnObjectTypesUpdated();
			}
			break;
		}
		case MessageType::SET_STRING: 
		{
			SetStringMessage * sm = (SetStringMessage*) message;
			String msg = message->msg;
			if (msg == "SetObjectName")
			{
				objectType->name = sm->value;
				OnObjectTypesUpdated();
			}
			else if (msg == "SetLightName")
				selectedLight->SetName(sm->value);
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			String msg = message->msg;
			if (msg == "SetObjectSizeXY"){
				Vector2i size = vm->vec2i;
				if (!objectType)
					return;
				objectType->size = size;
				objectType->UpdatePassabilityMatrix();
				objectType->UpdatePivotPosition();
				Graphics.QueueMessage(new GMSetUIv2i("ObjectOccupationMatrix", GMUI::MATRIX_SIZE, size));
			}
			else if (msg == "SetGlobalAmbient")
			{
				Graphics.QueueMessage(new GMSetAmbience(vm->vec3f));
			}
			else if (msg == "SetLightColor")
			{
				Graphics.QueueMessage(new GMSetLight(selectedLight, COLOR, vm->vec3f));
			}
			else if (msg == "SetLightAttenuation")
			{
				Graphics.QueueMessage(new GMSetLight(selectedLight, ATTENUATION, vm->vec3f));
			}
			else if (msg == "SetNewMapSize")
			{
				newSize = vm->vec2i;
				Vector2i size = map->Size();
				Vector2i diff = newSize - size;
				String modificationString;
				String diffString = String::ToString(AbsoluteValue(diff.x));
				if (diff.x > 0)
					modificationString = diffString+" columns will be added.";
				else if (diff.x < 0)
					modificationString = diffString+" columns will be removed.";
				else
					modificationString = "No change in columns.";
				Graphics.QueueMessage(new GMSetUIs("EffectsX", GMUI::TEXT, modificationString));
				diffString = String::ToString(AbsoluteValue(diff.y));
				if (diff.y > 0)
					modificationString = diffString+" rows will be added.";
				else if (diff.y < 0)
					modificationString = diffString+" rows will be removed.";
				else
					modificationString = "No change in rows.";
				Graphics.QueueMessage(new GMSetUIs("EffectsY", GMUI::TEXT, modificationString));
			}
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
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "NewObject")
			{
				objectType = GridObjectTypeMan.New();
				OnObjectTypesUpdated();
				Graphics.QueueMessage(new GMPushUI("Editor/EditObjectMenu.gui"));
				OnObjectTypeSelectedForEditing();
			}
			else if (string.Contains("EditObjectType("))
			{
				String typeString = string.Tokenize("()")[1];
				int id = typeString.ParseInt(); 
				objectType = GridObjectTypeMan.GetTypeByID(id);
				if (!objectType)
					return;
				Graphics.QueueMessage(new GMPushUI("Editor/EditObjectMenu.gui"));
				OnObjectTypeSelectedForEditing();
			}
			else if (string == "SetObjectType(this)")
			{
				String typeString = string.Tokenize("()")[1];
				typeString = message->element->text;
				objectType = GridObjectTypeMan.GetType(typeString);
			}
			else if (string == "SaveObjects"){
				OnObjectTypesUpdated();
				SaveObjects();
			}
			else if (string == "OnReloadUI"){
				Input.ForceNavigateUI(true);
				OnEditModeUpdated(-1);
				switch(this->editMode){
					case EditMode::TILES:
						OnTileTypesUpdated();
						break;
					case EditMode::OBJECTS:
						OnObjectTypesUpdated();
						break;
				}
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
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
			}
			else if (string == "TestLevel"){
				Playtest();
				return;
			}
			else if (string == "SquareBrush"){
				SetBrushType(SQUARE);
			}
			else if (string == "CircleBrush"){
				SetBrushType(CIRCLE);
			}
			else if (string == "DragRect"){
				SetBrushType(DRAG_RECT);
			}
			else if (string == "ApplyNewSize"){
				MesMan.QueueMessages("Query(SetMapSize)");
			}
			else if (string.Contains("SetMapSize")){
				this->SetMapSize(newSize.x, newSize.y);
				UpdateUISize();
				// TODO: Update relevant UI.
			//	Graphics.QueueMessage(new GMSetUIs("TilesInLevel", GMUI::TEXT, this->map->
			}
			else if (string.Contains("SetTileType(")){
				String typeString = string.Tokenize("()")[1];
				if (typeString == "this"){
					typeString = message->element->text;
					tileType = TileMan.GetTileType(typeString);
				}
				else {
					int index = typeString.ParseInt(); 
					tileType = TileMan.GetTileTypeByIndex(index);
				}
				OnTileTypeSelected();
			}
			else if (string.Contains("SetBrushSize(")){
				String sizeString = string.Tokenize("()")[1];
				int size = 0;
				if (sizeString == "this"){
					sizeString = message->element->text;
				}
				size = sizeString.ParseInt();
				SetBrushSize(size);
			}
			else if (string.Contains("SetMode")){
				int previousMode = editMode;
				String mode = string.Tokenize("()")[1];
				std::cout<<"\nSetMode message received..."<<mode;
				// Translate from UI if needed
				if (mode == "this"){
					assert(message->element);
					mode = message->element->name;
				}
				mode.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (mode.Contains("Size")){
					editMode = EditMode::SIZE;
				}
				else if (mode.Contains("Tiles")){
					editMode = EditMode::TILES;
				}
				else if (mode.Contains("Terrain")){
					editMode = EditMode::TERRAIN;
				}
				else if (mode.Contains("Objects")){
					editMode = EditMode::OBJECTS;
				}
				else if (mode.Contains("Events")){
					editMode = EditMode::EVENTS;
				}
				else if (mode.Contains("Lighting")){
					editMode = EditMode::LIGHTING;
				}
				OnEditModeUpdated(previousMode);
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
			else if (string == "load_map"){
				StateMan.ActiveState()->InputProcessor(LOAD_MAP);
			}
			else if (string == "save_map"){
				StateMan.ActiveState()->InputProcessor(SAVE_MAP);
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
		}
	}
	GameState::ProcessMessage(message);
}

/// Called every time the current selection is updated.
void RuneEditor::OnSelectionUpdated(){
	std::cout<<"\nRuneEditor::OnSelectionUpdated()";
	if (runeEditorSelection.Size() == NULL){
		Graphics.QueueMessage(new GMSetUIb("EntityManipWindow", GMUI::VISIBILITY, false));
		return;
	}
	Entity * entity = runeEditorSelection[0];

	// Reveal the UI
	Graphics.QueueMessage(new GMSetUIb("EntityManipWindow", GMUI::VISIBILITY, true));
	Vector3f position = entity->positionVector;
	String floatString;
	char buf[50];
	sprintf(buf, "%.3f", position.x);
	Graphics.QueueMessage(new GMSetUIs("xPos", GMUI::TEXT, buf));
	sprintf(buf, "%.3f", position.y);
	Graphics.QueueMessage(new GMSetUIs("yPos", GMUI::TEXT, buf));
	sprintf(buf, "%.3f", position.z);
	Graphics.QueueMessage(new GMSetUIs("zPos", GMUI::TEXT, buf));
}

/// Input functions for the various states
void RuneEditor::MouseClick(bool down, int x, int y, UIElement * elementClicked){
	/// Don't do anything if we're over a UI-element, yo.
	if (elementClicked)
		return;

	// If left mouse button was just clicked down...
	if (down){
		if (Input.KeyPressed(KEY::CTRL))
			return;
		switch(editMode)
		{
			case EditMode::TILES: 
			case EditMode::TERRAIN:
				// Draw
				switch(brushType)
				{
					case SQUARE:
					case CIRCLE:
						// Paint.
						Paint();
						break;
				}
				break;
			case EditMode::OBJECTS:
				CreateObject();
				break;
			case EditMode::EVENTS:
				if (!SelectEvent())
					CreateEvent();
				break;
			case EditMode::LIGHTING:
			{
				if (!SelectLight())
					CreateLight();
				break;
			}
		}
	}
	/// If released the left mouse button.
	else {
		switch(editMode)
		{
			case EditMode::TILES: 
			case EditMode::TERRAIN:
				// Draw
				switch(brushType)
				{
					case DRAG_RECT:
					case DRAG_CIRCLE:
						// Paint.
						Paint();
						break;
				}
				break;
		}
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
void RuneEditor::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
	/// Don't do anything if we're over a UI-element, yo.
	if (elementClicked)
		return;

	switch(editMode){
		case EditMode::TILES: {
		//	Paint();
			break;
		}
		case EditMode::EVENTS:{
			DeleteEvent();
			break;
		}
		case EditMode::OBJECTS:
		{
			DeleteObject();
			break;
		}
		case EditMode::LIGHTING:
			DeleteLight();
			break;

	}
	mouseX = (float)x;
	mouseY = (float)y;
	rButtonDown = down;
}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void RuneEditor::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){

	/// Don't do anything if we're over a UI-element, yo.
	if (elementOver)
		return;

	Camera * camera = Graphics.cameraToTrack;
	float diffX = mouseX - x;
	float diffY = mouseY - y;

	if (!map)
		return;
	TileMapLevel * level = map->ActiveLevel();
	if (!level)
		return;
	Vector2i size = level->Size();
	
	/// Get position in le welt.
//	std::cout<<"\nRetrieving plane coordinate in das welt.";
	Plane plane;
	plane.Set3Points(Vector3f(0,-1,0), Vector3f(1,0,0), Vector3f(0,0,0));
	Vector3f collissionPoint;
	Ray clickRay = GetRayFromScreenCoordinates(x, y, *runeEditorCamera);
	if (RayPlaneIntersection(clickRay, plane, &collissionPoint))
	{
		cursorPosition = cursorPositionPreRounding = collissionPoint;
		cursorPosition.Round();
	}
	/// Update cursor position!
	int xi = (int)cursorPosition.x, yi = (int)cursorPosition.y;
	String posString = "x: " + String::ToString(xi) +
		" y:" + String::ToString(yi);
	Graphics.QueueMessage(new GMSetUIs("CursorPosition", GMUI::TEXT, posString));
	/// If good coordinate, check info
	if (xi >= 0 && xi < size.x && yi >= 0 && yi < size.y){
		String info;
		switch(editMode){
			case EditMode::TILES: {
				const Tile * t = map->GetTile(Vector2i(xi,yi));
				if (!t)
					break;
				if (t->type)
					info = t->type->name;
				else
					info = "Empty tile";
				break;
			}
			default:
				break;
		}
		Graphics.QueueMessage(new GMSetUIs("CursorTargetInfo", GMUI::TEXT, info));
	}

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
			// Draw
			switch(editMode){
				case EditMode::TILES: case EditMode::TERRAIN:
					switch(brushType){
						case SQUARE:
						case CIRCLE:
							Paint();
							break;
					}
					break;
				case EditMode::OBJECTS:
					CreateObject();
					break;
			}
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
				switch(editMode){
					case EditMode::OBJECTS:
						DeleteObject();
						break;
					case EditMode::LIGHTING:
						DeleteLight();
						break;
				}
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

void RuneEditor::MouseWheel(float delta){

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
		switch(editMode){
			case EditMode::TILES: {
				if (delta > 0)
					tileType = TileMan.GetNext(tileType);
				else
					tileType = TileMan.GetPrevious(tileType);
				OnTileTypeSelected();
				Graphics.QueueMessage(new GMSetUIs("ActiveToolName", GMUI::TEXT, "Tile: "+tileType->name));
				break;
			}
		}
	}
}

/// Callback from the Input-manager, query it for additional information as needed.
void RuneEditor::KeyPressed(int keyCode, bool downBefore){
//	std::cout<<"\nOpHening Mhenu.";
	switch(keyCode){
		case KEY::ESCAPE: 
//			std::cout<<"\nOpHening Mhenu.";
			Graphics.QueueMessage(new GMPushUI("MainMenu"));
			break;
	}
}

// For rendering extra editor-specific content like the current cursor position and stuff... doesn't feel fitting to add that to TileMap2D nor in the GraphicsManager.
void RuneEditor::Render(GraphicsState & graphicsState){

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

	/// If tile or terrain mode, use cp-brushes
	bool brushes = true;
	int pBrushSize = brushSize;
	switch(editMode){
		case EditMode::EVENTS:
		case EditMode::LIGHTING:
			pBrushSize = 0;
		case EditMode::TILES:
		case EditMode::TERRAIN:
			brushes = true;
			break;
		case EditMode::OBJECTS:
			brushes = false;
			break;
		default:
			brushes = false;
	}

	float z = 0.1f;
	if (brushes){
		
		List<Vector2i> tilesToPaint = GetTilesToPaint(brushType, pBrushSize);

		/// Fetch color
		switch(editMode){
			case EditMode::TILES:
				if (tileType == 0)
					tileType = TileTypes.GetTileTypeByIndex(0);
				glColor4f(tileType->color.x, tileType->color.y, tileType->color.z, 0.5f);
				break;
			case EditMode::EVENTS:
				glColor4f(0.4f,0.4f,0.3f,0.5f);
				break;
			case EditMode::LIGHTING:
				glColor4f(0.6f,0.3f,0.1f,0.5f);
				break;
		};
		
		
		// Paint the given tiles
		glBlendFunc(GL_ONE, GL_ONE);
		glBegin(GL_QUADS);
		for (int i = 0; i < tilesToPaint.Size(); ++i){
			Vector3f position = tilesToPaint[i];
			glVertex3f(position.x - 0.5f, position.y - 0.5f,z);
			glVertex3f(position.x - 0.5f, position.y + 0.5f,z);
			glVertex3f(position.x + 0.5f, position.y + 0.5f,z);
			glVertex3f(position.x + 0.5f, position.y - 0.5f,z);
		}
		glEnd();
	}
	// /Custom rendering without "brushes"
	else {
		switch(editMode){
			case EditMode::OBJECTS:
			{
				Vector3f position = cursorPositionPreRounding;
				Vector2f pivotPosition;
				Vector2i objectSize;
				if (objectType == 0)
					objectType = GridObjectTypeMan.GetTypeByID(0);
				if (objectType)
				{
					objectSize = objectType->size;
					pivotPosition = objectType->pivotPosition;
					if (!objectType->texture && objectType->textureSource.Length()){
						objectType->texture = TexMan.GetTexture(objectType->textureSource);
					}
					if (objectType->texture){
						if (objectType->texture->glid == -1)
							objectType->texture->Bufferize();
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, objectType->texture->glid);
					}
				}
				else {
					glDisable(GL_TEXTURE_2D);
				}
				if (objectSize.x < 1)
					objectSize.x = 1;
				if (objectSize.y < 1)
					objectSize.y = 1;
				if (pivotPosition.MaxPart() == 0){
					pivotPosition = objectSize;
					pivotPosition /= 2.0f;
				}

				float pivotToLeft = -pivotPosition.x;
				float pivotToRight = objectSize.x - pivotPosition.x;
				float pivotToTop = objectSize.y - pivotPosition.y;
				float pivotToBottom = -pivotPosition.y;

				Vector2i doublePivot = pivotPosition * 2;

				/// Depending on the pivot position, adjust the anchor for where to center painting of it.
				if (doublePivot.x % 2 == 0){
					/// Even amount of cells in width, center it between the closest two cells X-wise.
					float x = position.x;
					float roundedX = RoundFloat(x);
					float otherX = 0;
					if (roundedX <= x)
						otherX = RoundFloat(x + 0.5f);
					else 
						otherX = RoundFloat(x - 0.5f);
					position.x = (RoundFloat(roundedX) + RoundFloat(otherX)) / 2;
				}
				else {
					position.x = RoundFloat(position.x);
				}
				if (doublePivot.y % 2 == 0){
					float y = position.y;
					float roundedY = RoundFloat(y);
					float otherY = 0;
					if (roundedY <= y){
						otherY = RoundFloat(y + 0.5f);
					}
					else {
						otherY = RoundFloat(y - 0.5f);
					}
					position.y = (RoundFloat(roundedY) + RoundFloat(otherY)) / 2;
				}
				else {
					position.y = RoundFloat(position.y);
				}

				bool canPaint = true;
				// Paint the given tiles
				if (canPaint){
					glColor4f(1.f,1.f,1.f,0.5f);
				}
				else{
					glColor4f(0.4f,0.0f,0.0f,0.3f);		
				}
				glEnable(GL_TEXTURE_2D);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_QUADS);
					glTexCoord2f(0,0);	
					glVertex3f(position.x + pivotToLeft, position.y + pivotToBottom,z);
					glTexCoord2f(0,1);				
					glVertex3f(position.x + pivotToLeft, position.y + pivotToTop,z);
					glTexCoord2f(1,1);				
					glVertex3f(position.x + pivotToRight, position.y + pivotToTop,z);
					glTexCoord2f(1,0);				
					glVertex3f(position.x + pivotToRight, position.y + pivotToBottom,z);
				glEnd();


				/// Fetch tiles required to place it, and render according to if they are occupied or not! 
				TileMapLevel * level = map->ActiveLevel();
				List<Tile*> tilesRequired = level->TilesRequired(objectType, cursorPositionPreRounding, NULL);
				for (int i = 0; i < tilesRequired.Size(); ++i){
					Tile * tile = tilesRequired[i];
					if (!tile)
						continue;
					// Painterly!
					if (tile->IsVacant())
						glColor4f(1.f,1.f,1.f,0.5f);
					else 
						glColor4f(1.0f,0.0f,0.0f,0.5f);		

					position = tile->position;
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glDisable(GL_DEPTH_TEST);
					glDisable(GL_TEXTURE_2D);
					glBegin(GL_QUADS);
						glVertex3f(position.x - 0.5f, position.y - 0.5f,z);
						glVertex3f(position.x - 0.5f, position.y + 0.5f,z);
						glVertex3f(position.x + 0.5f, position.y + 0.5f,z);
						glVertex3f(position.x + 0.5f, position.y - 0.5f,z);
					glEnd();

				}
	
				break;
			}
		}
	}
			
}

/// Sets appropriate variables and updates relevant GUI.
void RuneEditor::SetBrushType(int type)
{
	brushType = type;
	String brushName;
	switch(type){
		case SQUARE: brushName = "SquareBrush"; break;
		case CIRCLE: brushName = "CircleBrush"; break;
		case DRAG_RECT: brushName = "DragRect"; break;
		case DRAG_CIRCLE: brushName = "DragCircle"; break;
		default: assert(false && "Bad brushtype");
	}
	Graphics.QueueMessage(new GMSetUIb("TileBrushes", GMUI::CHILD_TOGGLED, false));
	Graphics.QueueMessage(new GMSetUIb(brushName, GMUI::TOGGLED, true));
}
void RuneEditor::SetBrushSize(int size)
{
	if (size < 0)
		size = 0;
	brushSize = size;
	String sizeString = String::ToString(size);
	Graphics.QueueMessage(new GMSetUIs("SetBrushSize", GMUI::TEXT, sizeString));
}

/// Attempts to delete object at current cursor position.
void RuneEditor::DeleteObject()
{
	TileMapLevel * level = map->ActiveLevel();
	List<Tile*> tiles =	level->TilesRequired(objectType, cursorPositionPreRounding, NULL);
	/// Delete 'em.
	Graphics.PauseRendering();
	level->DeleteObjects(tiles);
	Graphics.ResumeRendering();
}

/// Saves actively edited object, pushing it to list of all objects. calls OnObjectTypesUpdated on success.
void RuneEditor::SaveObjects()
{
	// Save object... wehweh.
	GridObjectTypeMan.Save();
}

/// Updates list of available objects.
void RuneEditor::OnObjectTypesUpdated()
{
	Graphics.QueueMessage(new GMClearUI("ObjectTypes"));
	List<GridObjectType*> objectTypes = GridObjectTypeMan.GetTypes();
	for (int i = 0; i < objectTypes.Size(); ++i)
	{
		GridObjectType * goType = objectTypes[i];
		/// Check search/filter string first.	
		String typeName = goType->name;
		if (!typeName.Length())
			typeName = "No name, D:";
		UICheckBox * cBox = new UICheckBox();
		cBox->text = typeName;
		cBox->name = typeName;
		cBox->textColor = ui->defaultTextColor;
		cBox->textureSource = ui->defaultTextureSource;
		cBox->sizeRatioY = 0.1f;
		cBox->activationMessage = "SetObjectType(this)";
		
		UIButton * editButton = new UIButton();
		editButton->activationMessage = "EditObjectType("+String::ToString(goType->ID())+")";
		editButton->text = "Edit";
		editButton->sizeRatioX = 0.2f;
		editButton->alignmentX = 0.7f;
		editButton->textureSource = ui->defaultTextureSource;
		cBox->AddChild(editButton);

		UIImage * image = new UIImage(goType->textureSource);
		image->alignmentX = 0.9f;
		image->sizeRatioX = 0.2f;
		cBox->AddChild(image);
		Graphics.QueueMessage(new GMAddUI(cBox, "ObjectTypes"));
	}
}

/// Called once the edit ui for objects has been pushed.
void RuneEditor::OnObjectTypeSelectedForEditing()
{
	assert(objectType);
	Graphics.QueueMessage(new GMSetUIs("ObjectName", GMUI::STRING_INPUT_TEXT, objectType->name));
	Graphics.QueueMessage(new GMSetUIs("ObjectPrimaryTexture", GMUI::TEXTURE_INPUT_SOURCE, objectType->textureSource));
	Graphics.QueueMessage(new GMSetUIv2i("ObjectSizeXY", GMUI::VECTOR_INPUT, objectType->size));
	Graphics.QueueMessage(new GMSetUIv2i("ObjectOccupationMatrix", GMUI::MATRIX_SIZE, objectType->size));	
	Graphics.QueueMessage(new GMSetUIvb("ObjectOccupationMatrix", GMUI::MATRIX_DATA, objectType->passability));
//	Graphics.QueueMessage(new GMSetUIv2f("PivotXY", GMUI::VECTOR_INPUT, objectType->pivot));
}	

/// Called to update ui.
void RuneEditor::OnTileTypesUpdated()
{
	Graphics.QueueMessage(new GMClearUI("TileTypes"));
	List<TileType*> tileTypes = TileTypes.GetTypes();
	for (int i = 0; i < tileTypes.Size(); ++i)
	{
		TileType * tt = tileTypes[i];
		UICheckBox * cBox = new UICheckBox();
		cBox->text = tt->name;
		cBox->name = "TileType:"+tt->name;
		cBox->sizeRatioY = 0.1f;
		cBox->textureSource = ui->defaultTextureSource;
		cBox->textColor = ui->defaultTextColor;
		cBox->activationMessage = "SetTileType(this)";
		// 
		UIImage * textureElement = new UIImage(tt->textureSource);
		textureElement->alignmentX	= 0.9f;
		cBox->AddChild(textureElement);

		Graphics.QueueMessage(new GMAddUI(cBox, "TileTypes"));
	}
}

/// Transitions to the level test state!
void RuneEditor::Playtest(){

	GameState * gs = StateMan.GetState(RUNE_GAME_STATE_MAP);
	MapState * ms = (MapState*) gs;
	ms->SetEnterMode(EnterMode::TESTING_MAP);
	ms->SetCamera(*runeEditorCamera);
	ms->activeMap = map;
	/// Check for player character/spawn, if not, place somewhere within camera range.
	Entity * e = map->GetEntity("Player");
	if (e == NULL){
		Model * m = ModelMan.GetModel("Sprite");
		Texture * t = TexMan.GetTexture("RuneRPG/Units/200");
		
		// Get middle of screen-tile, or where cursor is?
		const Tile * tile = map->GetClosestVacantTile(cursorPosition);
		if (tile == NULL){
			std::cout<<"\nERROR: No valid walkable tile in range.";
			return;
		}
		Vector3i position = tile->position;
		assert(position.x != -1);
		if (tile == NULL){
			assert(false && "No valid walkable tile in range!");
			StateMan.QueueState(GAME_STATE_EDITOR);
			return;
		}
		Entity * player = MapMan.CreateEntity(m,t,position);
		player->name = "Player";
		player->state = new StateProperty(player);
		// Lall.
	//	EntityStateTile2D * entityTile2D = ((TileMap2D*)MapMan.ActiveMap())->GetEntity2DByEntity(player);
	//	player->state->SetGlobalState(new RREntityState(player, entityTile2D));
		//	Physics.QueueMessage(new PMSetEntity(POSITION, player, Vector3f(tile->X(),tile->Y(),5)));
	}
	if (e)
		e->flags |= PLAYER_OWNED_ENTITY;
	StateMan.QueueState(RUNE_GAME_STATE_MAP);
}

/// Paint!
void RuneEditor::Paint(){
	if (!map)
		return;
	TileMapLevel * level = map->ActiveLevel();
	if (!level)
		return;
	Vector2i size = level->Size();
	
	assert(editMode == EditMode::TILES || editMode == EditMode::TERRAIN);
	Graphics.PauseRendering();
	TileType * t = tileType;
	assert(t);
	List<Vector2i> tilesToPaint = this->GetTilesToPaint(brushType, brushSize);
	for (int i = 0; i < tilesToPaint.Size(); ++i){
		Vector2i pos = tilesToPaint[i];
		switch(editMode){
			case EditMode::TILES:
				map->SetTileType(pos, t);
				break;
		}
	}
	// Post-drawing modifications, for example reset previous position when using the drag-tools.
	switch(brushType){
		case DRAG_RECT: case DRAG_CIRCLE:
			previousCursorPosition = cursorPosition;
	}
	Graphics.ResumeRendering();
}
/// Attempts to create an object on current mouse location. Returns object if it succeeded.
GridObject * RuneEditor::CreateObject()
{
	if (!map)
		return NULL;
	TileMapLevel * level = map->ActiveLevel();
	if (!level)
		return NULL;
	Vector2i size = level->Size();
	assert(editMode == EditMode::OBJECTS);
	if (!level->CanCreateObject(objectType, cursorPositionPreRounding)){
		return NULL;
	}
	Graphics.PauseRendering();
	/// Uh... create object.. at target location.. or try to.
	GridObject * go = new GridObject(objectType);
	go->position = cursorPositionPreRounding;
	level->AddObject(go);
	Graphics.ResumeRendering();	
}

/// SElect evvveettttnt
bool RuneEditor::SelectEvent()
{
	List<Event*> events = map->GetEvents();
	for (int i = 0; i < events.Size(); ++i){
		Event * event = events[i];
		float distance = (event->position - cursorPosition).LengthSquared();
		if (distance < 0.9f){
			selectedEvent = event;
			OnSelectedEventUpdated();
			return true;
		}
	}
	return false;
}

/// Create an event on the map! :)
void RuneEditor::CreateEvent()
{
	TileMapLevel * level = map->ActiveLevel();
	Vector2i size = level->Size();
	
	assert(editMode == EditMode::EVENTS);
	// Opt outs
	if (cursorPosition.x < 0 || cursorPosition.x >= size.x)
		return;
	if (cursorPosition.y < 0 || cursorPosition.y >= size.y)
		return;
	Graphics.PauseRendering();
	String eventName = "New event";
	String num;
	int i = 1;
	Event * event = new Event("New event");
	while (MapMan.AddEvent(event) == false){
		++i;
		event->name = eventName +" "+ String::ToString(i);
	}
	event->position = cursorPosition;
	selectedEvent = event;
	OnSelectedEventUpdated();
	assert(event);
	Graphics.ResumeRendering();
}

/// Attempts to delete an event at given location!
void RuneEditor::DeleteEvent(){
	TileMapLevel * level = map->ActiveLevel();
	Vector2i size = level->Size();
	
	assert(editMode == EditMode::EVENTS);
	// Opt outs
	if (cursorPosition.x < 0 || cursorPosition.x > size.x)
		return;
	if (cursorPosition.y < 0 || cursorPosition.y >= size.y)
		return;

	Graphics.PauseRendering();
	List<Event*> events = MapMan.GetEvents();
	for (int i = 0; i < events.Size(); ++i){
		Event * e = events[i];
		if ((e->position - cursorPosition).LengthSquared() < 0.9f){
			if (e == selectedEvent){
				selectedEvent = NULL;
				OnSelectedEventUpdated();
			}
			assert(MapMan.DeleteEvent(e) && "Event was not present in the map!!!!!");
		}
	}
	Graphics.ResumeRendering();
}

/// Tries to select light at current cursor position.
bool RuneEditor::SelectLight()
{
	List<Light*> lights = map->GetLighting()->GetLights();
	for (int i = 0; i < lights.Size(); ++i){
		Light * light = lights[i];
		float distance = (light->position - cursorPosition).LengthSquared();
		if (distance < 0.9f)
		{
			selectedLight = light;
			OnSelectedLightUpdated();
			return true;
		}
	}
	return false;
}
/// Creates a new light
void RuneEditor::CreateLight()
{
	if (SelectLight())
		return;
	TileMapLevel * level = map->ActiveLevel();
	Vector2i size = level->Size();
	assert(editMode == EditMode::LIGHTING);
	// Opt outs
	if (cursorPosition.x < 0 || cursorPosition.x >= size.x)
		return;
	if (cursorPosition.y < 0 || cursorPosition.y >= size.y)
		return;
	Graphics.PauseRendering();
	String lightName = "New light";
	String num;
	int i = 1;
	Lighting * lighting = this->map->GetLightingEditable();
	Light * light = lighting->CreateLight();
	if (!light)
		return;
	light->position = cursorPosition;
	light->attenuation = Vector3f(1,1,1);
	selectedLight = light;
	OnSelectedLightUpdated();
	assert(light);
	Graphics.ResumeRendering();	
}
void RuneEditor::DeleteLight()
{
	Light * lightToDelete = NULL;
	List<Light*> lights = map->GetLighting()->GetLights();
	for (int i = 0; i < lights.Size(); ++i){
		Light * light = lights[i];
		float distance = (light->position - cursorPosition).LengthSquared();
		if (distance < 0.9f){
			lightToDelete = light;
			break;
		}
	}
	if (lightToDelete)
	{
		Graphics.PauseRendering();
		Lighting * lighting = map->GetLightingEditable();
		lighting->DeleteLight(lightToDelete);
		Graphics.ResumeRendering();
	}
}

/// Queries the TileTypeManager to reload the available tiles.
void RuneEditor::ReloadTiles(){
	Graphics.PauseRendering();
	tileTypeIndex = TileTypes.Index(tileType);
	tileType = NULL;
	TileTypes.LoadTileTypes("data/RuneRPG/tiles.txt");
	tileType = TileTypes.GetTileTypeByIndex(tileTypeIndex);
	Graphics.ResumeRendering();
	OnTileTypesUpdated();
}

/// Fetches active/relevant set of tiles that will be painted if wished so now. Uses saves mouse coordinate-data to calculate the set.
List<Vector2i> RuneEditor::GetTilesToPaint(int brushType, int brushSize)
{
	List<Vector2i> tilesToPaint;
	switch(brushType){
		case SQUARE: 
		{
			Vector2i position = cursorPosition;
			Vector2i min = position - Vector2i(brushSize, brushSize);
			Vector2i max = position + Vector2i(brushSize, brushSize);
			// Fetch all between min and max!
			for (int y = min.y; y <= max.y; ++y){
				for (int x = min.x; x <= max.x; ++x){
					tilesToPaint.Add(Vector2i(x,y));
				}
			}
			break;
		}
		case CIRCLE:
		{
			Vector2i position = cursorPosition;
			Vector2i min = position - Vector2i(brushSize, brushSize);
			Vector2i max = position + Vector2i(brushSize, brushSize);
			// Fetch all between min and max!
			for (int y = min.y; y <= max.y; ++y){
				for (int x = min.x; x <= max.x; ++x){
					Vector2i pos = Vector2i(x,y);
					if ((pos - position).Length() > brushSize)
						continue;
					tilesToPaint.Add(pos);
				}
			}
			break;
		}
		case DRAG_RECT: 
		{
			Vector2i position = cursorPosition;
			Vector2i min = Vector2i::Minimum(cursorPosition, previousCursorPosition);
			Vector2i max = Vector2i::Maximum(cursorPosition, previousCursorPosition);
			// Fetch all between min and max!
			for (int y = min.y; y <= max.y; ++y){
				for (int x = min.x; x <= max.x; ++x){
					Vector2i pos = Vector2i(x,y);
					tilesToPaint.Add(pos);
				}
			}
			break;
		}
		default:
			assert(false && "Implement");
	}
	return tilesToPaint;
}

void RuneEditor::TranslateActiveEntities(Vector3f distance){
	Physics.QueueMessage(new PMSetEntity(TRANSLATE, runeEditorSelection, distance));
}
void RuneEditor::SetScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, runeEditorSelection, scale));
}
void RuneEditor::ScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SCALE, runeEditorSelection, scale));
}
void RuneEditor::RotateActiveEntities(Vector3f rotation){
	for (int i = 0; i < runeEditorSelection.Size(); ++i){
		if (!runeEditorSelection[i])
			continue;
		runeEditorSelection[i]->rotate(rotation);
	}
}

void RuneEditor::UpdateUISize()
{
	Vector2i mapSize = map->Size();
	Graphics.QueueMessage(new GMSetUIs("CurrentSize", GMUI::TEXT, "Current size: "+String::ToString(mapSize.x)+", "+String::ToString(mapSize.y)));
	Graphics.QueueMessage(new GMSetUIv2i("NewMapSize", GMUI::VECTOR_INPUT, mapSize));
	Graphics.QueueMessage(new GMSetUIs("EffectsY", GMUI::TEXT, ""));
	Graphics.QueueMessage(new GMSetUIs("EffectsX", GMUI::TEXT, ""));
}

void RuneEditor::UpdateUITiles()
{
	OnTileTypesUpdated();
	OnTileTypeSelected();
}

void RuneEditor::UpdateUIObjects()
{
	OnObjectTypesUpdated();
}

void RuneEditor::UpdateUILighting()
{
	const Lighting * lighting = Graphics.ActiveLighting();
	Graphics.QueueMessage(new GMSetUIv3f("GlobalAmbient", GMUI::VECTOR_INPUT, lighting->GetAmbient()));
}

/// Update GUI n stuff!
void RuneEditor::OnEditModeUpdated(int previousMode)
{
	String previousModeName = GetModeName(previousMode);
	/// Get new mode name
	String modeName = GetModeName(editMode);

	/// Make stuff invisible.
	Graphics.QueueMessage(new GMPopUI(previousModeName+"Menu"));
	/// Make visible new UI using given modeName :)
	Graphics.QueueMessage(new GMPushUI("Editor/"+modeName+"Menu.gui"));
	Graphics.QueueMessage(new GMSetUIb("Modes", GMUI::CHILD_TOGGLED, false));
	/// Update some label?
	Graphics.QueueMessage(new GMSetUIs("CurrentMode", GMUI::TEXT, modeName));
	// Highlight selected mode.
	Graphics.QueueMessage(new GMSetUIb(modeName, GMUI::TOGGLED, true));

	/// Load data specific to that UI.
	switch(editMode){
		case EditMode::SIZE:
			UpdateUISize();
			break;
		case EditMode::TILES:
			UpdateUITiles();
			break;
		case EditMode::OBJECTS:
			UpdateUIObjects();
			break;
		case EditMode::LIGHTING:
			UpdateUILighting();
			break;
	}
}

/// To update gui visualization?
void RuneEditor::OnTileTypeSelected()
{
	/// Remove toggle flag from all other tile-types.
	Graphics.QueueMessage(new GMSetUIb("TileTypes", GMUI::CHILD_TOGGLED, false));

	// Highlight selected mode.
	Graphics.QueueMessage(new GMSetUIb("TileType:"+tileType->name, GMUI::TOGGLED, true));
}

/// For the edit modes.
String RuneEditor::GetModeName(int editMode)
{
	String modeName;
	/// Set name of this mode if needed for display/debug?
	switch(editMode){
		case EditMode::SIZE: modeName = "Size"; break;
		case EditMode::TILES: modeName = "Tiles"; break;
		case EditMode::TERRAIN: modeName = "Terrain"; break;
		case EditMode::OBJECTS: modeName = "Objects"; break;
		case EditMode::EVENTS:	modeName = "Events"; break;
		case EditMode::LIGHTING: modeName = "Lighting"; break;
		default: modeName = ";__;"; break;
	}
	return modeName;
}	

// Wosh
void RuneEditor::OnSelectedEventUpdated(){
	if (selectedEvent){
		/// Update event sCHATS
		Graphics.QueueMessage(new GMSetUIs("EventName", GMUI::TEXT, selectedEvent->name));
		Graphics.QueueMessage(new GMSetUIs("EventSource", GMUI::TEXT, selectedEvent->source));
		// Open UI
		Graphics.QueueMessage(new GMSetUIb("EventDetails", GMUI::VISIBILITY, true));
	}
	else {
		// Hide UI
		Graphics.QueueMessage(new GMSetUIb("EventDetails", GMUI::VISIBILITY, false));
	};
}

/// Update more gui.
void RuneEditor::OnSelectedLightUpdated()
{
	if (selectedLight)
	{
		Graphics.QueueMessage(new GMPushUI("Editor/LightEditor.gui"));
		Graphics.QueueMessage(new GMSetUIs("LightName", GMUI::STRING_INPUT_TEXT, selectedLight->Name()));
		Graphics.QueueMessage(new GMSetUIv3f("LightColor", GMUI::VECTOR_INPUT, selectedLight->diffuse));
		Graphics.QueueMessage(new GMSetUIv3f("LightAttenuation", GMUI::VECTOR_INPUT, selectedLight->attenuation));
	}
	else 
	{
		Graphics.QueueMessage(new GMPopUI("LightEditor"));
	}
}


/// Saves the map to set mapFilePath. Assumes any file-checks have been done beforehand. Pauses input and physics while saving.
bool RuneEditor::SaveMap(){
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
	MapMan.SaveMap(mapFilePath);

	
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
bool RuneEditor::LoadMap(String fromFile)
{
	if (!fromFile.Contains(".tmap"))
		fromFile += ".tmap";
	Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/loading_map.png"));
	bool physicsWasPaused = Physics.IsPaused();
	Physics.Pause();
	// Pause input too?
	Input.acceptInput = false;
	// Clear selection if any
	runeEditorSelection.Clear();
	// Load the new map.
	Map * loadMap = MapMan.LoadMap(fromFile);
	if (loadMap){
		MapMan.MakeActive(loadMap);
		map = (TileMap2D*)loadMap;
	}
	newSize = map->Size();
	// Close the menu if it's open
	Graphics.QueueMessage(new GMPopUI("MainMenu"));
	// Resume control
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	if (!physicsWasPaused)
	Physics.Resume();
	Input.acceptInput = true;
	return true;
}


void RuneEditor::SetMapSize(int xSize, int ySize){
	Graphics.PauseRendering();
	MapMan.DeleteEntities();
	MapMan.DeleteEvents();
	map->SetSize(xSize, ySize);
	Graphics.ResumeRendering();
}
