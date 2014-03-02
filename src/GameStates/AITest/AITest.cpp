///// Emil Hedemalm
///// 2013-03-01
//#include "AITest.h"
//
//extern UserInterface * ui[MAX_GAME_STATES];
//#define UI ui[StateMan.currentGameState];
//
//#include "../../Graphics/Messages/GMUI.h"
//#include "../../AI/World.h"
//#include "../../AI/States/CharacterState.h"
//#include <iomanip>
//
///// The editor camera! :D
//Camera aiTestCamera;
//// Current editor selection, definedi n Input/Bindings/Editor.cpp atm
//Selection aiTestSelection;
//
//
//void AITest::OnEnter(GameState * previousState){
//	/// Disable physics rendering
//	Graphics.renderPhysics = false;
//	Graphics.renderNavMesh = true;
//
//	// And set it as active
//	Graphics.cameraToTrack = &aiTestCamera;
//	Graphics.selectionToRender = &aiTestSelection;
//	aiTestCamera.SetRatio(Graphics.width, Graphics.height);
//
//	// First pause physics
//	Physics.Pause();
//
//	// Set physics and AI playback-speed.
//	Physics.QueueMessage(new PMSetSpeed(playbackSpeed));
//	AI.SetSpeed(playbackSpeed);
//
//	// Set 0 gravity for now.
//	Physics.QueueMessage(new PMSetGravity(Vector3f(0,0,0)));
//	// Load the AI scene
//	if (!MapMan.MakeActive("AITest")){
//		MapMan.CreateMap("AITest");
//		MapMan.MakeActive("AITest");
//
//		Lighting light = MapMan.GetLighting();
//
//		int lights = light.ActiveLights();
//		for (int i = 0; i < lights; ++i){
//			light.SelectLight(i);
//			light.SetDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
//			light.SetSpecular(0.1f, 0.1f, 0.1f, 1.0f);
//			light.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
//		}
//
//		AIWorld.CreateDefaultSetup();
//
//		// Swap all previous entities's types to dynamic
//	//	Physics.QueueMessage(new PMSetPhysicsType(MapMan.GetEntities(), PhysicsType::DYNAMIC));
//
//		// Register them all for AI-processing.
//	//	AI.RegisterEntities(MapMan.GetEntities());
//
//	//	Graphics.QueueMessage(new GMRegisterEntities(MapMan.GetEntities()));
//	//	Graphics.QueueMessage(new PM
//	//	Physics.QueueMessage(new PMRegisterEntities(MapMan.GetEntities()));
//	}
//	else
//		MapMan.MakeActive("AITest");
//
//	AIWorld.MapToNavMesh();
//
//	// Begin loading textures here for the UI
//	Graphics.QueueMessage(new GMSet(ui));
//
//	// Play physics
//	Physics.Resume();
//	// Play AI
//	AI.Resume();
//	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
//}
//
//void AITest::OnExit(GameState * nextState){
//	/// Unregister all entities from physics and AI
//	int result = AI.UnregisterAllEntities();
//	std::cout<<"\nUnregistered "<<result<<" entities from AI calculations";
//	Physics.QueueMessage(new PhysicsMessage(PM_UNREGISTER_ENTITIES));
//	std::cout<<"\nLeaving AI Test state.";
//	Graphics.renderNavMesh = false;
//}
//
//void AITest::Process(float time){
//
//
//	// Calculate time since last update
//	clock_t newTime = clock();
//	int timeDiff = newTime - lastTime;
//	lastTime = newTime;
//
//	/// Process AI
//	AI.Process();
//
//	/// Fly! :D
//	/// Rotate first, yo o.O
//	/// Rotation multiplier.
//	float rotMultiplier = 0.05f;
//	aiTestCamera.rotation += aiTestCamera.rotationVelocity * aiTestCamera.rotationSpeedMultiplier * timeDiff;
//	// Check input for moving camera
//	if (aiTestCamera.velocity.Length() > 0){
//		Vector4d moveVec;
//		moveVec = Vector4d(aiTestCamera.velocity);
//		/// Flight-speed multiplier.
//		float multiplier = 0.5 * aiTestCamera.flySpeedMultiplier;
//		moveVec = moveVec * multiplier * (float)timeDiff;
//		Matrix4d rotationMatrix;
//		rotationMatrix.InitRotationMatrixY(-aiTestCamera.rotation.y);
//		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-aiTestCamera.rotation.x));
//		moveVec = rotationMatrix.product(moveVec);
//		aiTestCamera.position += Vector3f(moveVec);
//	}
///*
//	/// Update gui
//	UserInterface * gui = StateMan.ActiveState()->GetUI();
//	if (aiTestSelection.Size()){
//		Entity * selected = aiTestSelection[0];
//		if (!gui || !selected){
//			return;
//		}
//		StateProperty * ai = selected->state;
//		if (ai->globalState->stateID == AI_STATE::CHARACTER){
//			CharacterState * cs = (CharacterState*)ai->globalState;
//			/// Link the relevant slider-handle's values to the selected entity
//			UIElement * e = gui->GetElementByName("hungerSlider");
//			((UISlider*)e)->SetLevel(cs->hunger);
//			e = gui->GetElementByName("fatigueSlider");
//			((UISlider*)e)->SetLevel(cs->fatigue);
//			e = gui->GetElementByName("thirstSlider");
//			((UISlider*)e)->SetLevel(cs->thirst);
//			e = gui->GetElementByName("socialDeprivationSlider");
//			((UISlider*)e)->SetLevel(cs->socialDeprivation);
//
//			/// Copy over entity name onto UIElement!
//			e = gui->GetElementByName("selectedEntityDiv");
//			if (!e->text)
//				e->text = new char[MAX_PATH];
//			strcpy(e->text, selected->name);
//			strcat(e->text, " ");
//			strcat(e->text, ai->currentState->name);
//			return;
//		}
//	}
//	*/
//};
//
///// Input functions for the various states
//void AITest::MouseClick(bool down, int x, int y, UIElement * elementClicked){
//	/*
//	/// Return straight away if we clicked anything whatsoever in particular...!
//	if (elementClicked){
//		// lazy..
//		UIElement * e = elementClicked;
//		switch(elementClicked->UIType){
//		case UI_TYPE_SLIDER_BAR:{
//			CharacterState * cs = ((CharacterState*)aiTestSelection[0]->state->globalState);
//			float value = (x - e->left) / e->sizeX;
//			if (strcmp(e->name, "hungerSlider") == 0)
//				cs->hunger = value;
//			if (strcmp(e->name, "thirstSlider") == 0)
//				cs->thirst = value;
//			if (strcmp(e->name, "fatigueSlider") == 0)
//				cs->fatigue = value;
//			if (strcmp(e->name, "socialDeprivationSlider") == 0)
//				cs->socialDeprivation = value;
//			}
//			break;
//		default: {
//			if (!e->name)
//				break;
//			if (strcmp(e->name, "pauseButton") == 0){
//				AI.Pause();
//				Physics.Pause();
//			}
//			else if (strcmp(e->name, "playButton") == 0){
//				AI.Resume();
//				Physics.Resume();
//			}
//			else if (strcmp(e->name, "slowButton") == 0){
//				DecreaseSpeed();
//			}
//			else if (strcmp(e->name, "speedButton") == 0){
//				IncreaseSpeed();
//			}
//			}
//			break;
//		}
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
//					aiTestSelection.Clear();
//
//				Ray clickRay = GetRayFromScreenCoordinates(Input.mouseX, Input.mouseY, aiTestCamera);
//			//	std::cout<<"\nStartPoint: "<<clickRay.start<<" \nDirection: "<<clickRay.direction;
//
//				// Sort, then select closest entity that is within the ray.
//				Selection entities = MapMan.GetEntities();
//				Vector3f camPos = aiTestCamera.Position();
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
//					aiTestSelection.Add(entity);
//					break;
//				}
//				// Do selection behavior
//				OnSelect(aiTestSelection);
//
//				/// Toggle tiles if no entity was selected.
//				if (aiTestSelection.Size() <= 0){
//					AI.Pause();
//					Physics.Pause();
//					/// Ray clickRay;
//					Plane plane;	/// Default plane should work... maybe?
//					Vector3f collissionPoint;
//					RayPlaneIntersection(clickRay, plane, &collissionPoint);
//					// Get waypoint closest to the clickposition and toggle it's walkability!
//					WaypointMan.ToggleWaypointWalkability(collissionPoint);
//					/// Don't forget to re-map the AIWorld...! Should maybe make callback for this?
//					if (WaypointMan.ActiveNavMesh()->optimized){
//						AIWorld.MapToNavMesh();
//					}
//					/// Should always be able to re-map to the navMesh assuming we've done everything right.
//					else {
//						WaypointMan.CleansePData();
//						AIWorld.MapToNavMesh();
//					}
//					/// Resume calculations..!
//					AI.Resume();
//					Physics.Resume();
//				}
//			}
//			else {
//
//				/// Find entities in a frustum in target location.
//				// Begin by extracting the end point in the aiTestCamera frustum for where we clicked
//				Vector4f startPoint = aiTestCamera.Position();
//				Vector4f endPoint = Vector4f(0, 0, -aiTestCamera.farPlane, 1);
//				endPoint = Matrix4f(aiTestCamera.Projection()) * endPoint;
//				endPoint = Matrix4f(aiTestCamera.View()) * endPoint;
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
//	*/
//}
//void AITest::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
//
//	Input.mouseX = x;
//	Input.mouseY = y;
//	Input.rButtonDown = down;
//}
//
//void AITest::MouseMove(float x, float y, bool lDown, bool rDown){
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
//void AITest::MouseWheel(float delta){
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
//void AITest::OnSelect(Selection &selection){
//	/*
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
//		/// Copy over entity name onto UIElement!
//		e = gui->GetElementByName("selectedEntityDiv");
//		if (!e->text)
//			e->text = new char[MAX_PATH];
//		strcpy(e->text, selected->name);
//		strcat(e->text, " ");
//		strcat(e->text, ai->currentState->name);
//		/// Reveal window
//		Graphics.QueueMessage(new GMSetUIVisibility("selectedEntityDiv", true));
//		return;
//	}
//	else {
//		/// Hide entity details window
//		Graphics.QueueMessage(new GMSetUIVisibility("selectedEntityDiv", false));
//		return;
//	}
//	*/
//}
//
//void AITest::IncreaseSpeed(){
//	if (playbackSpeed < 0.1f)
//		playbackSpeed += 0.01f;
//	else if (playbackSpeed < 1.0f)
//		playbackSpeed += 0.1f;
//	else if (playbackSpeed < 2.0f)
//		playbackSpeed += 0.2f;
//	else if (playbackSpeed < 5.0f)
//		playbackSpeed += 0.5f;
//	else if (playbackSpeed < 1000.0f)
//		playbackSpeed += 2.5f;
//
//	// Set physics and AI playback-speed.
//	Physics.QueueMessage(new PMSetSpeed(playbackSpeed));
//	AI.SetSpeed(playbackSpeed);
//}
//
//void AITest::DecreaseSpeed(){
//	if (playbackSpeed <= 0.01f)
//		return;
//	else if (playbackSpeed <= 0.1f)
//		playbackSpeed -= 0.01f;
//	else if (playbackSpeed < 1.01f)
//		playbackSpeed -= 0.1f;
//	else if (playbackSpeed < 2.02f)
//		playbackSpeed -= 0.2f;
//	else if (playbackSpeed < 5.05f)
//		playbackSpeed -= 0.5f;
//	else
//		playbackSpeed -= 2.5f;
//
//	// Set physics and AI playback-speed.
//	Physics.QueueMessage(new PMSetSpeed(playbackSpeed));
//	AI.SetSpeed(playbackSpeed);
//}
