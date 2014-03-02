// Emil Hedemalm
// 2013-06-28

#include "StreamerTestState.h"

#include "Graphics/GraphicsManager.h"
#include "Maps/MapManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include "Physics/PhysicsProperty.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "UI/UserInterface.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/Message.h"
#include "Actions.h"
#include "OS/Sleep.h"
extern UserInterface * ui[MAX_GAME_STATES];

#include <iomanip>
#include "Graphics/Messages/GMNavMesh.h"
#include "Graphics/GraphicsProperty.h"
#include "EntityStates/StateProperty.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"

#define EDITOR_MAP "EditorMap"
#define SHIP_EDITOR_MAP	"ShipEditor"

#define LOG(s)	Graphics.QueueMessage(new GMSetUIs("Log", GMUI::TEXT, s));				

StreamerTestState::StreamerTestState()
: GameState(){
	id = GAME_STATE_STREAM_TEST;
	mouseCameraState = 0;
	editMode = ENTITIES;
	activePath = NULL;
	checkPointWaypointInterval = 5;
	checkpointSize = 10.0f;
    stateName = "Editor state";

	activeShipEntity = NULL;
	activeShip = NULL;
	lastActiveMap = NULL;

	forceMagnitude = 1.0f;
	multiplyForceByMass = false;

	renderClickRays = false;
}
StreamerTestState::~StreamerTestState(){
	std::cout<<"\nEdhitor Destruchtor :3";
}

Selection StreamerTestState::GetActiveSelection(){
    return editorSelection;
};

void StreamerTestState::CreateUserInterface(){
	if (ui)
		delete ui;
	this->ui = new UserInterface();
	ui->Load("gui/DemoProject/StreamerTest.gui");
}

void StreamerTestState::OnEnter(GameState * previousState){
	// Load initial texture and set it to render over everything else
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSet(ACTIVE_USER_INTERFACE, ui));

	if (!MapMan.Exists(EDITOR_MAP))
		MapMan.CreateMap(EDITOR_MAP);

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

	// And set it as active
	Graphics.cameraToTrack = &editorCamera;
	editorCamera.SetRatio(Graphics.width, Graphics.height);

	// Load the AI scene
	// Don't load anytheing o-o;
/*	if (!MapMan.MakeActive(&editorMap)){
		Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
		Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
	}
*/
	/// Reset physics settings
	Physics.QueueMessage(new PhysicsMessage(PM_RESET_SETTINGS));
	// Kill gravity...
	float defaultGravity = -1.0f;
	Physics.QueueMessage(new PMSet(GRAVITY, defaultGravity));
	// Update ui with gravity afterwards.. hm..

    /// Wosh!
	Physics.checkType = AABB_SWEEP;

	Graphics.QueueMessage(new GMSet(FOG_BEGIN, 5000.0f));
	Graphics.QueueMessage(new GMSet(FOG_END, 50000.0f));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));


	/// Update the UI to display correctly the global physics-settings.
	Graphics.QueueMessage(new GMSetUIb("PauseOnCollission", GMUI::TOGGLED, Physics.PauseOnCollission()));
#define STRING(p) String::ToString(p)
	Graphics.QueueMessage(new GMSetUIs("Gravity", GMUI::TEXT, STRING(defaultGravity)));
	Graphics.QueueMessage(new GMSetUIs("DefaultDensity", GMUI::TEXT, STRING(Physics.defaultDensity)));
}

void StreamerTestState::OnExit(GameState *nextState){
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	MapMan.MakeActive(NULL);
	std::cout<<"\nLeaving Editor state.";
}

extern String ToString(Vector3f vec);

/*
String ToString(Vector3f vec){
	return String::ToString(vec.x) + ", " + String::ToString(vec.y) + ", " + String::ToString(vec.z);
};
*/

void StreamerTestState::Process(float time){
	/// Process key input for navigating the 3D - Space
	Sleep(20);

	// Calculate time since last update
	clock_t newTime = Timer::GetCurrentTimeMs();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;

	/// Print data of current selection.
	if (editorSelection.Size()){
		Entity * primaryEntity = editorSelection[0];
		PhysicsProperty * pp = primaryEntity->physics;
		if (pp){
			String positionString = "Position: "+ToString(primaryEntity->positionVector);
			Graphics.QueueMessage(new GMSetUIs("Position", GMUI::TEXT, positionString));
			
			String linearMomentumString = "LinearMomentum: "+ToString(pp->linearMomentum);
			Graphics.QueueMessage(new GMSetUIs("LinearMomentum", GMUI::TEXT, linearMomentumString));

			String angularVelocityString = "AngularMomentum: "+ToString(pp->angularMomentum);
			Graphics.QueueMessage(new GMSetUIs("AngularMomentum", GMUI::TEXT, angularVelocityString));
		}
	}
			

	/// Fly! :D
	/// Rotate first, yo o.O
	/// Rotation multiplier.
	float rotMultiplier = 0.05f;
	editorCamera.rotation += editorCamera.rotationVelocity * editorCamera.rotationSpeedMultiplier * (float)timeDiff;
	// Check input for moving camera
	if (editorCamera.velocity.Length() > 0){
		Vector4d moveVec;
		moveVec = Vector4d(editorCamera.velocity);
		/// Flight-speed multiplier.
		float multiplier = 0.5f * editorCamera.flySpeedMultiplier;
		moveVec = moveVec * multiplier * (float)timeDiff;
		Matrix4d rotationMatrix;
		rotationMatrix.InitRotationMatrixY(-editorCamera.rotation.y);
		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-editorCamera.rotation.x));
		moveVec = rotationMatrix.product(moveVec);
		editorCamera.position += Vector3f(moveVec);
	}
};

/// Callback function that will be triggered via the MessageManager when messages are processed.
void StreamerTestState::ProcessMessage(Message * message){
	std::cout<<"\nStreamerTestState::ProcessMessage: ";
	switch(message->type){
		case MessageType::STRING: {
			String string = message->msg;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
				return;
			}
#define DEFAULT_TEXTURE  TexMan.GetTexture("80gray.png")
			else if (string == "SpawnEntities"){
			    const int entities = 20;
                for (int i = 0; i < entities; ++i){
                    Vector3f startPos = Vector3f(i, i*2, i*3);
                    Entity * e = MapMan.CreateEntity(ModelMan.GetModel("ships/ship1.obj"), DEFAULT_TEXTURE, startPos);
                    Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
                    Physics.QueueMessage(new PMSetPhysicsType(e, PhysicsType::DYNAMIC));
                }
			}
			else if (string == "SpawnEntity"){
			    const Camera * camera = Graphics.ActiveCamera();
                Model * model = ModelMan.GetModel("cube.obj");
               // Model * model = ModelMan.GetModel("ships/ship1.obj");
				Vector3f startPos = camera->Position() + camera->LookingAt() * (model->radius + AbsoluteValue(camera->nearPlane));
                Entity* e = MapMan.CreateEntity(model, DEFAULT_TEXTURE, startPos);
                Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
                Physics.QueueMessage(new PMSetPhysicsType(e, PhysicsType::DYNAMIC));
			}
			else if (string == "SpawnBrick"){
				const Camera * camera = Graphics.ActiveCamera();
                Model * model = ModelMan.GetModel("cube.obj");
               // Model * model = ModelMan.GetModel("ships/ship1.obj");
				Vector3f startPos = camera->Position() + camera->LookingAt() * (model->radius + AbsoluteValue(camera->nearPlane));
                Entity* e = MapMan.CreateEntity(model, DEFAULT_TEXTURE, startPos);
				Physics.QueueMessage(new PMSetEntity(SCALE, e, Vector3f(2.0f,3.0f,1.0f)));
                Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
                Physics.QueueMessage(new PMSetPhysicsType(e, PhysicsType::DYNAMIC));
			}
			else if (string == "CreatePlane"){
                Entity * e = MapMan.CreateEntity(ModelMan.GetModel("cube.obj"), DEFAULT_TEXTURE);
                Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
                float scale = 200.0f;
                Physics.QueueMessage(new PMSetEntity(SCALE, e, Vector3f(scale,1.0f,scale)));
			}
			else if (string == "CreateSlidingPlaneScenario"){
				LOG("Creating sliding plane scenario...");
				/// Fetch position
				const Camera * camera = Graphics.ActiveCamera();
                Model * model = ModelMan.GetModel("cube.obj");
				float angle = 0.12f;
				Vector3f startPos = camera->Position() + camera->LookingAt() * (model->radius + AbsoluteValue(camera->nearPlane) + 10.0f);
                /// Create a plane
				Entity * e = MapMan.CreateEntity(ModelMan.GetModel("cube.obj"), DEFAULT_TEXTURE, startPos);
				Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
                float scale = 20.0f;
                Physics.QueueMessage(new PMSetEntity(SCALE, e, Vector3f(scale,1.0f,scale)));
				Physics.QueueMessage(new PMSetEntity(SET_ROTATION, e, Vector3f(angle,0,0)));
				
				/// Add a box.
				// Model * model = ModelMan.GetModel("ships/ship1.obj");
				for (int i = 0; i < 5; ++i){
					e = MapMan.CreateEntity(model, DEFAULT_TEXTURE, startPos + Vector3f(0,2.0f + i * 2.0f,0 + i - 2));
					Physics.QueueMessage(new PMSetPhysicsShape(e, ShapeType::MESH));
					Physics.QueueMessage(new PMSetPhysicsType(e, PhysicsType::DYNAMIC));
					Physics.QueueMessage(new PMSetEntity(SET_ROTATION, e, Vector3f(angle,0,0)));
				}
			}
			else if (string == "AttachSpring"){
				if (editorSelection.Size() < 2){
					Graphics.QueueMessage(new GMSetUIs("Log", GMUI::TEXT, "ERROR: Not enough entities selected. Hold CTRL and left-click to select more."));
					return;
				}
				LOG("Attaching spring to current selection of size "+String::ToString(editorSelection.Size())+".");
				Entity * one = editorSelection[0];
				Entity * two = editorSelection[1];
				/// Use current length.
				Physics.QueueMessage(new PMCreateSpring(editorSelection, 5.f));	
			}
			else if (string == "DeleteEntities"){
                MapMan.DeleteEntities();
				LOG("Deleting entities..");
			}
			else if (string == "ToggleLightRendering"){
                Graphics.renderLights = !Graphics.renderLights;
			}
			else if (string == "ToggleAABBRendering"){

			}
			else if (string == "ToggleRenderClickRays"){
				renderClickRays = !renderClickRays;
				return;
			}
			else if (string == "ToggleRenderSeparatingAxes"){
				Graphics.renderSeparatingAxes = !Graphics.renderSeparatingAxes;
				return;
			}
			else if (string == "OnPhysicsPaused"){
                Graphics.QueueMessage(new GMSetUIb("PausePhysics", GMUI::TOGGLED, true, NULL));
				return;
			}
			else if (string == "TogglePauseOnCollission"){
				Physics.QueueMessage(new PMSet(PAUSE_ON_COLLISSION, !Physics.PauseOnCollission()));
			}
			else if (string == "ToggleForceMassRelativity"){
                multiplyForceByMass = !multiplyForceByMass;
			}
			else if (string == "SetForceMagnitude"){
                forceMagnitude = message->element->text.ParseFloat();
			}
			else if (string.Contains("SetDefaultDensity(this)")){
				float density = message->element->text.ParseFloat();
				Physics.QueueMessage(new PMSet(DEFAULT_DENSITY, density));
			}
			else if (string == "LockLinearPosition"){
			    std::cout<<"\nUIElement: "<<message->element->name;
			    bool locked = message->element->toggled;
			    std::cout<<"\n  locked: "<<locked;
			    if (editorSelection.Size()){
                    Entity * e = editorSelection[0];
                    Physics.QueueMessage(new PMSetEntity(LOCK_POSITION, editorSelection, locked));
			    }
			}
			else if (string.Contains("SetAngAccX")){
                float value = message->element->text.ParseFloat();
                if (!editorSelection.Size())
                    return;
                Entity * entity = editorSelection[0];
                Vector3f angAcc = entity->physics->angularAcceleration;
                angAcc.x = value;
                Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, entity, angAcc));
			}
			else if (string.Contains("SetAngAccY")){
                float value = message->element->text.ParseFloat();
                if (!editorSelection.Size())
                    return;
                Entity * entity = editorSelection[0];
                Vector3f angAcc = entity->physics->angularAcceleration;
                angAcc.y = value;
                Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, entity, angAcc));
			}
			else if (string.Contains("SetAngAccZ")){
                float value = message->element->text.ParseFloat();
                if (!editorSelection.Size())
                    return;
                Entity * entity = editorSelection[0];
                Vector3f angAcc = entity->physics->angularAcceleration;
                angAcc.z = value;
                Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, entity, angAcc));
			}
			else if (string == "TogglePhysicsPaused"){
			    if (Physics.IsPaused())
                    Physics.Resume();
                else
                    Physics.Pause();
                return;
			}
			else if (string == "SetPhysicsSpeed(this)"){
                UIElement * element = message->element;
                float speed = element->text.ParseFloat();
                Physics.QueueMessage(new PMSetSpeed(speed));
                return;
			}
			else if (string == "SetGravity(this)"){
                UIElement * element = message->element;
                float gravity = element->text.ParseFloat();
                Physics.QueueMessage(new PMSetGravity(Vector3f(0,gravity,0)));
                return;
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
			//// Paths n navmesh below

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
				Sleep(50);
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
						mode == 1? f : e->positionVector.x,
						mode == 2? f : e->positionVector.y,
						mode == 3? f : e->positionVector.z);
					Physics.QueueMessage(new PMSetEntity(POSITION, e, newPosition));
				}
				OnSelectionUpdated();
			}
		}
	}
	GameState::ProcessMessage(message);
}

/// Called every time the current selection is updated.
void StreamerTestState::OnSelectionUpdated(){
	LOG("Entities selected: "+String::ToString(editorSelection.Size()));

//	std::cout<<"\nStreamerTestState::OnSelectionUpdated()";
	if (editorSelection.Size() == 0){
//		Graphics.QueueMessage(new GMSetUIb("EntityManipWindow", GMUI::VISIBILITY, false));
		return;
	}
	Entity * entity = editorSelection[0];

	// Reveal the UI
//	Graphics.QueueMessage(new GMSetUIb("EntityManipWindow", GMUI::VISIBILITY, true));
	Vector3f position = entity->positionVector;
	String floatString;
	char buf[50];
	sprintf(buf, "%.3f", position.x);
	Graphics.QueueMessage(new GMSetUIs("xPos", GMUI::TEXT, buf));
	sprintf(buf, "%.3f", position.y);
	Graphics.QueueMessage(new GMSetUIs("yPos", GMUI::TEXT, buf));
	sprintf(buf, "%.3f", position.z);
	Graphics.QueueMessage(new GMSetUIs("zPos", GMUI::TEXT, buf));

	if (entity->physics){
		PhysicsProperty * p = entity->physics;
		bool locked = p->locks & POSITION_LOCKED;
		Graphics.QueueMessage(new GMSetUIb("LockLinearPos", GMUI::TOGGLED, locked));
	}

}

/// Input functions for the various states
void StreamerTestState::MouseClick(bool down, int x, int y, UIElement * elementClicked){
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
			if (AbsoluteValue(startMouseX - mouseX) < 2.0f &&
				AbsoluteValue(startMouseY - mouseY) < 2.0f &&
				elementClicked == NULL)
			{
				if (!Input.KeyPressed(KEY::CTRL))
					editorSelection.Clear();

				Ray clickRay = GetRayFromScreenCoordinates(mouseX, mouseY, editorCamera);
				/// Add the ray to be rendered. Toggle this option somehow?
				if (renderClickRays)
					Graphics.QueueMessage(new GMRender(clickRay, 15.0f));
	//			std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;

				switch(editMode){
					case NAVMESH: case PATHS:{
						assert(false);
						break;
					}
					case ENTITIES:{

						// Sort, then select closest entity that is within the ray.
						Selection entities = MapMan.GetEntities();
                        Vector3f point;
						Entity * entity = MapMan.GetFirstEntity(clickRay, point);

                        if (entity){
							editorSelection.Add(entity);
							/// TODO: Add option that switches bettn continue and break here!
                        }
                        OnSelectionUpdated();
					}

				}
			}
			else {


				/// Find entities in a frustum in target location.
				// Begin by extracting the end point in the editorCamera frustum for where we clicked
				Vector4f startPoint = editorCamera.Position();
				Vector4f endPoint = Vector4f(0, 0, -editorCamera.farPlane, 1);
				endPoint = Matrix4f(editorCamera.Projection()) * endPoint;
				endPoint = Matrix4f(editorCamera.View()) * endPoint;

		//		std::cout<<"\nStartPoint: "<<startPoint<<" EndPoint1: "<<endPoint;
				//		std::cout<<" EndPoint2: "<<endPoint2;
			}
		}

	}
	mouseX = (float)x;
	mouseY = (float)y;
	lButtonDown = down;
}
void StreamerTestState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
    if (down != rButtonDown){
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
                mouseX = x;
                mouseY = y;
                rButtonDown = down;

                Ray clickRay = GetRayFromScreenCoordinates(mouseX, mouseY, editorCamera);
				if (renderClickRays)
					Graphics.QueueMessage(new GMRender(clickRay, 15.0f));
                std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;

                /// Get first entity along the ray in the map!
                Vector3f point;
                Entity * entity = MapMan.GetFirstEntity(clickRay, point);
                if (entity){
                    assert(entity->physics);
                    Vector3f force = clickRay.direction.NormalizedCopy() * forceMagnitude;
                    if (multiplyForceByMass)
                        force *= entity->physics->mass;
                    if (entity)
                        Physics.QueueMessage(new PMApplyImpulse(entity, force, point));
                }
            }
        }
    }
	mouseX = (float)x;
	mouseY = (float)y;
	rButtonDown = down;
}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void StreamerTestState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){
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

void StreamerTestState::MouseWheel(float delta){
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

// Loads target map o-o
void StreamerTestState::LoadMap(String fromFile){
	editorSelection.Clear();
	if (!fromFile.Contains(".map"))
		fromFile = fromFile + ".map";
	if (!fromFile.Contains("racing")){
		fromFile = "racing/" + fromFile;
	}
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loading_map.png")));
	std::cout<<"\nLoadMap called for file: "<<fromFile;
	Sleep(100);
	String filename = fromFile;
	Map * loadedMap = MapMan.LoadMap(filename.c_str());
	// Set map to be active!
	if (loadedMap)
		MapMan.MakeActive(loadedMap);
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	lastActiveMap = loadedMap;
}


void StreamerTestState::TranslateActiveEntities(Vector3f distance){
	Physics.QueueMessage(new PMSetEntity(TRANSLATE, editorSelection, distance));
}
void StreamerTestState::SetScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, editorSelection, scale));
}
void StreamerTestState::ScaleActiveEntities(Vector3f scale){
	Physics.QueueMessage(new PMSetEntity(SCALE, editorSelection, scale));
}
void StreamerTestState::RotateActiveEntities(Vector3f rotation){
	for (int i = 0; i < editorSelection.Size(); ++i){
		if (!editorSelection[i])
			continue;
		editorSelection[i]->rotate(rotation);
	}
}

/// Handle drag-n-drop files.
void StreamerTestState::HandleDADFiles(List<String> & files){
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
		}
	}
}

void StreamerTestState::ReturnToLastActiveMap(){
	// Last map if we were editing something earlier..!
	if (lastActiveMap)
		MapMan.MakeActive(lastActiveMap);
	else {
		MapMan.MakeActive(EDITOR_MAP);
		lastActiveMap = MapMan.ActiveMap();
	}
}

/// And update UI and stuff!
void StreamerTestState::SetMode(int mode){
	/// Clear UI of previous mode!
#define HIDE(b) Graphics.QueueMessage(new GMSetUIb(b, GMUI::VISIBILITY, false));
	switch(editMode){
		case ENTITIES:
			HIDE("EntitiesUI");
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
			std::cout<<"\nERROR: Bad mode supplied in StreamerTestState::SetMode, returning...";
			return;
	}
	editMode = mode;
};
