//#include "Pathfinding.h"
//#include "../AI/StateProperty.h"
//#include "../Graphics/Messages/GMUI.h"
//#include <iomanip>
//
//extern UserInterface * ui[MAX_GAME_STATES];
//#define UI ui[StateMan.currentGameState];
//
///// The editor camera! :D
//Camera pathfindingCamera;
//// Current editor selection, defined in Input/Bindings/Editor.cpp atm
//Selection pathfindingSelection;
//
//void Pathfinding::OnEnter(GameState * previousState){
//	/// Disable physics rendering
//	Graphics.renderPhysics = false;
//	Graphics.renderGrid = false;
//	Graphics.renderNavMesh = true;
//
//	// And set it as active
//	Graphics.cameraToTrack = &pathfindingCamera;
//	Graphics.selectionToRender = &pathfindingSelection;
//	pathfindingCamera.SetRatio(Graphics.width, Graphics.height);
//
//	// First pause physics
//	Physics.Pause();
//
//	// Set physics and AI playback-speed.
//	Physics.QueueMessage(new PMSetSpeed(1.0f));
//
//	// Set 0 gravity for now.
//	Physics.QueueMessage(new PMSetGravity(Vector3f(0,0,0)));
//	// Load the AI scene
//	if (!MapMan.MakeActive("Pathfinding")){
//		MapMan.CreateMap("Pathfinding");
//		MapMan.MakeActive("Pathfinding");
//
//		/*
//		Lighting * light = MapMan.GetLighting();
//
//		int lights = light->ActiveLights();
//		for (int i = 0; i < lights; ++i){
//			light->SelectLight(i);
//			light->SetDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
//			light->SetSpecular(0.1f, 0.1f, 0.1f, 1.0f);
//			light->SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
//		}
//*/
//		/// Load all default pathfinding files into the WaypointManager
//
//
//
//		/// Make the first one active and create entities for it to be renderable
//
//
//
//	//	Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
//	//	Graphics.QueueMessage(new PM
//	//	Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
//	}
//	else
//		MapMan.MakeActive("Pathfinding");
//
//
//	// Begin loading textures here for the UI
//	Graphics.QueueMessage(new GMSet(ui));
//
//	// Play physics
//	Physics.Resume();
//
//	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
//}
//
//void Pathfinding::OnExit(GameState * nextState){
//	Physics.QueueMessage(new PhysicsMessage(PM_UNREGISTER_ENTITIES));
//	std::cout<<"\nLeaving Pathfinding state.";
//}
//
//void Pathfinding::Process(float time){
//
//
//	// Calculate time since last update
//	clock_t newTime = clock();
//	int timeDiff = newTime - lastTime;
//	lastTime = newTime;
//
//
//
//	/// Fly! :D
//	/// Rotate first, yo o.O
//	/// Rotation multiplier.
//	float rotMultiplier = 0.05f;
//	pathfindingCamera.rotation += pathfindingCamera.rotationVelocity * pathfindingCamera.rotationSpeedMultiplier * timeDiff;
//	// Check input for moving camera
//	if (pathfindingCamera.velocity.Length() > 0){
//		Vector4d moveVec;
//		moveVec = Vector4d(pathfindingCamera.velocity);
//		/// Flight-speed multiplier.
//		float multiplier = 0.5 * pathfindingCamera.flySpeedMultiplier;
//		moveVec = moveVec * multiplier * (float)timeDiff;
//		Matrix4d rotationMatrix;
//		rotationMatrix.InitRotationMatrixY(-pathfindingCamera.rotation.y);
//		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-pathfindingCamera.rotation.x));
//		moveVec = rotationMatrix.product(moveVec);
//		pathfindingCamera.position += Vector3f(moveVec);
//	}
//
//	/// Update gui
//	UserInterface * gui = StateMan.ActiveState()->GetUI();
//	Entity * selected = pathfindingSelection[0];
//	if (!gui || !selected){
//		return;
//	}
//	StateProperty * ai = selected->state;
//	if (ai && ai->globalState->stateID == AI_STATE::CHARACTER){
//		/*
//		CharacterState * cs = (CharacterState*)ai->globalState;
//		/// Link the relevant slider-handle's values to the selected entity
//		UIElement * e = gui->GetElementByName("hungerSlider");
//		((UISlider*)e)->SetLevel(cs->hunger);
//		e = gui->GetElementByName("fatigueSlider");
//		((UISlider*)e)->SetLevel(cs->fatigue);
//		e = gui->GetElementByName("thirstSlider");
//		((UISlider*)e)->SetLevel(cs->thirst);
//		e = gui->GetElementByName("socialDeprivationSlider");
//		((UISlider*)e)->SetLevel(cs->socialDeprivation);
//
//		/// Copy over entity name onto UIElement!
//		e = gui->GetElementByName("selectedEntityDiv");
//		if (!e->text)
//			e->text = new char[MAX_PATH];
//		strcpy(e->text, selected->name);
//		strcat(e->text, " ");
//		strcat(e->text, ai->currentState->name);
//		return;
//		*/
//	}
//};
//
//
///// Returns a ray in 3D space using the given mouse and
//
//#include "../UI/UIElement.h"
///// Input functions for the various states
//void Pathfinding::MouseClick(bool down, int x, int y, UIElement * elementClicked){
//	/// Return straight away if we clicked anything whatsoever in particular...!
//	if (elementClicked){
//		return;
//	}
//	/// If new state (should be, but yeah)
//	if (down != Input.lButtonDown){
//		/// Mouse press
//		if (down){
//			Input.startMouseX = x;
//			Input.startMouseY = y;
//		}
//		/// Mouse release
//		else {
//			// Simple "click"-selection if the coordinates haven't moved more than 2 pixels in either direction ^.^
//			if (abs(Input.startMouseX - Input.mouseX) < 2.0f &&
//				abs(Input.startMouseY - Input.mouseY) < 2.0f)
//			{
//				if (!Input.KeyPressed(KEY::CTRL))
//					pathfindingSelection.Clear();
//
//				Ray clickRay = GetRayFromScreenCoordinates(Input.mouseX, Input.mouseY, pathfindingCamera);
//			//	std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;
//
//				// Sort, then select closest entity that is within the ray.
//				Selection entities = MapMan.GetEntities();
//				Vector3f camPos = pathfindingCamera.Position();
//				entities.SortByDistance(camPos);
//				for (int i = 0; i < entities.Size(); ++i){
//					Entity * entity = entities[i];
//					Vector3f camToEntity = entity->positionVector - camPos;
//					float dotProductEntityToVector = clickRay.direction.DotProduct(camToEntity);
//					if (dotProductEntityToVector < 0){
//			//			std::cout<<"\nCulling entity "<<entity->name<<" as it is beind the camera direction.";
//						continue;
//					}
//					Vector3f projectedPointOnVector = camPos + dotProductEntityToVector * clickRay.direction;
//					float distanceToVector = (entity->positionVector - projectedPointOnVector).Length();
//					if (distanceToVector > entity->radius){
//		//				std::cout<<"\nEntity not intersecting, distance: "<<distanceToVector<<" radius: "<<std::setw(6)<<entity->radius<<" "<<entity->name;
//						continue;
//					}
//					pathfindingSelection.Add(entity);
//					break;
//				}
//				// Do selection behavior
//				OnSelect(pathfindingSelection);
//			}
//			else {
//
//				/// Find entities in a frustum in target location.
//				// Begin by extracting the end point in the pathfindingCamera frustum for where we clicked
//				Vector4f startPoint = pathfindingCamera.Position();
//				Vector4f endPoint = Vector4f(0, 0, -pathfindingCamera.farPlane, 1);
//				endPoint = Matrix4f(pathfindingCamera.Projection()) * endPoint;
//				endPoint = Matrix4f(pathfindingCamera.View()) * endPoint;
//
//
//
//		//		std::cout<<"\nStartPoint: "<<startPoint<<" EndPoint1: "<<endPoint;
//		//		std::cout<<" EndPoint2: "<<endPoint2;
//			}
//		}
//
//	}
//	Input.mouseX = x;
//	Input.mouseY = y;
//	Input.lButtonDown = down;
//}
//void Pathfinding::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
//
//	Input.mouseX = x;
//	Input.mouseY = y;
//	Input.rButtonDown = down;
//}
//
//void Pathfinding::MouseMove(float x, float y, bool lDown, bool rDown){
//	Camera * camera = Graphics.cameraToTrack;
//	float diffX = Input.prevMouseX - x;
//	float diffY = Input.prevMouseY - y;
////	std::cout<<"\nDiff: "<<diffX<<" "<<diffY;
//	if (lDown){
//		if(camera){
//			if (Input.KeyPressed(KEY::CTRL)){
//				camera->position += camera->UpVector().CrossProduct(camera->LookingAt()) * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
//				camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
//			}
//			else {
//				camera->rotation.x += diffY / 100.0f;
//				camera->rotation.y -= diffX / 100.0f;
//			}
//		}
//	}
//	else if (rDown){
//		if(camera){
//			if (Input.KeyPressed(KEY::CTRL)){
//				camera->distanceFromCentreOfMovement += diffY;
//				if (camera->distanceFromCentreOfMovement > 0)
//					camera->distanceFromCentreOfMovement = 0;
//			}
//			else {
//				camera->position += camera->UpVector().CrossProduct(camera->LookingAt()) * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
//				camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
//			}
//		}
//	}
//
//	Input.prevMouseX = Input.mouseX = x;
//	Input.prevMouseY = Input.mouseY = y;
//}
//
//void Pathfinding::MouseWheel(float delta){
//	Camera * camera = Graphics.cameraToTrack;
//	camera->distanceFromCentreOfMovement += delta / 100.0f;
//	if (delta > 0){
//		camera->distanceFromCentreOfMovement *= 0.8f;
//	}
//	else if (delta < 0){
//		camera->distanceFromCentreOfMovement *= 1.25f;
//	}
//	if (camera->distanceFromCentreOfMovement > 0)
//		camera->distanceFromCentreOfMovement = 0;
//}
//
//
///// Interpret selection queries for additional processing (updating UI for example).
//void Pathfinding::OnSelect(Selection &selection){
//	/// Link the relevant slider-handle's values to the selected entity
//	UserInterface * gui = StateMan.ActiveState()->GetUI();
//	Entity * selected = selection[0];
//	if (!gui){
//		std::cout<<"\nINFO: No active GUI available.";
//		return;
//	}
//	if (selection.Size() < 1){
//		/// Hide entity details window
//		Graphics.QueueMessage(new GMSetUIVisibility("selectedEntityDiv", false));
//		return;
//	}
//
//	StateProperty * ai = selected->state;
//	if (ai->globalState->stateID == AI_STATE::CHARACTER){
//		/*CharacterState * cs = (CharacterState*)ai->globalState;
//		/// Link the relevant slider-handle's values to the selected entity
//		UIElement * e = gui->GetElementByName("hungerSlider");
//		((UISlider*)e)->SetLevel(cs->hunger);
//		e = gui->GetElementByName("fatigueSlider");
//		((UISlider*)e)->SetLevel(cs->fatigue);
//		e = gui->GetElementByName("thirstSlider");
//		((UISlider*)e)->SetLevel(cs->thirst);
//		e = gui->GetElementByName("socialDeprivationSlider");
//		((UISlider*)e)->SetLevel(cs->socialDeprivation);
//		/// Copy over entity name onto UIElement!
//		e = gui->GetElementByName("selectedEntityDiv");
//		if (!e->text)
//			e->text = new char[MAX_PATH];
//		strcpy(e->text, selected->name);
//		strcat(e->text, " ");
//		strcat(e->text, ai->currentState->name);
//		/// Reveal window
//		Graphics.QueueMessage(new GMSetUIVisibility("selectedEntityDiv", true));
//		return;*/
//	}
//	else {
//		/// Hide entity details window
//		Graphics.QueueMessage(new GMSetUIVisibility("selectedEntityDiv", false));
//		return;
//	}
//}
//
//
