/// Emil Hedemalm
/// 2014-07-26
/// Simple flying aircraft game/simulator to test physics and possibly rendering 

#include "Aircraft.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Maps/MapManager.h"

#include "Random/Random.h"

#include "Message/Message.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/Integrators/Aircraft/AircraftIntegrator.h"

#include "Input/InputManager.h"
#include "Input/Keys.h"

#include "Window/AppWindowManager.h"

#include "Viewport.h"

#include "ModelManager.h"
#include "TextureManager.h"

#include "Application/Application.h"
void SetApplicationDefaults()
{
	// Uh... stuff?
	Application::name = "Aircraft simulator";
}


#include "StateManager.h"
void RegisterStates()
{
	AircraftState * as = new AircraftState();
	StateMan.RegisterState(as);
	StateMan.QueueState(as);
}



AircraftState::AircraftState()
{
	/// Boolean, set to true if you want the KeyPressed function to be called.
	keyPressedCallback = true;
}
/// Virtual destructor to discard everything appropriately
AircraftState::~AircraftState()
{

}
/// Function when entering this state, providing a pointer to the previous StateMan.
void AircraftState::OnEnter(AppState * previousState)
{
	// Set custom integrator
	Physics.QueueMessage(new PMSet(new AircraftIntegrator()));

	// Clear overlay texture
	Graphics.QueueMessage(new GMSetOverlay(NULL));


// aircraft:
//	Aircraft
// 	F-14_Tomcat_echo
	// Create ze ship.
	ship = MapMan.CreateEntity("Ship", ModelMan.GetModel("Aircraft/F-14_Tomcat_echo"), TexMan.GetTexture("White"));


	CreateStars();


	/// Create our cameras.
	firstPersonCamera = CameraMan.NewCamera();
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, ENTITY_TO_TRACK, ship));
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, RELATIVE_POSITION, Vector3f(0, 5, 0)));

	thirdPersonCamera = CameraMan.NewCamera();
	Graphics.QueueMessage(new GMSetCamera(thirdPersonCamera, ENTITY_TO_TRACK, ship));
	Graphics.QueueMessage(new GMSetCamera(thirdPersonCamera, RELATIVE_POSITION, Vector3f(0, 20, 55)));

	topDownCamera = CameraMan.NewCamera();
	Graphics.QueueMessage(new GMSetCamera(topDownCamera, ENTITY_TO_TRACK, ship));
	Graphics.QueueMessage(new GMSetCamera(topDownCamera, INHERIT_ROTATION, false));
	Graphics.QueueMessage(new GMSetCamera(topDownCamera, RELATIVE_POSITION, Vector3f(0, 20, 55)));


	freeFlyCamera = CameraMan.NewCamera();
	cameras.Add(4, firstPersonCamera, thirdPersonCamera, topDownCamera, freeFlyCamera);
}

/// Main processing function, using provided time since last frame.
void AircraftState::Process(int timeInMs)
{

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void AircraftState::OnExit(AppState * nextState)
{

}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void AircraftState::ProcessPacket(Packet * packet)
{

}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void AircraftState::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "Create stars")
			{
				CreateStars();
			}
			else if (msg == "UpdateAcceleration")
				UpdateAcceleration();
			else if (msg == "UpdateLocalRotationX")
				UpdateLocalRotationX();
			else if (msg == "UpdateLocalRotationY")
				UpdateLocalRotationY();
			else if (msg == "UpdateLocalRotationZ")
				UpdateLocalRotationZ();
			else if (msg == "ChangeCamera")
				NextCamera();
			break;
		}
	}
}


/// Chat :3 Means that it has already been posted to the ChatManager.
void AircraftState::OnChatMessageReceived(ChatMessage * cm)
{

}


/// Input functions for the various states
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void AircraftState::MouseClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked)
{

}

/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void AircraftState::MouseRightClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked)
{

}

/// Interprets a mouse-move message to target position.
void AircraftState::MouseMove(AppWindow * AppWindow, int x, int y, bool lDown, bool rDown, UIElement * elementOver)
{

}

/** Handles mouse wheel input.
	Positive delta signifies scrolling upward or away from the user, negative being toward the user.
*/
void AircraftState::MouseWheel(AppWindow * AppWindow, float delta)
{

}

/// Callback from the Input-manager, query it for additional information as needed.
void AircraftState::KeyPressed(int keyCode, bool downBefore)
{

}


/** Function to handle custom actions defined per state.
	This function is called by the various bindings that the state defines.
*/
void AircraftState::InputProcessor(int action, int inputDevice /*= 0*/)
{

}

/// Creates default key-bindings for the state.
void AircraftState::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	mapping->CreateBinding("Create stars", KEY::CTRL, KEY::C, KEY::S);

	String accelMsg = "UpdateAcceleration";
	mapping->CreateBinding(accelMsg, KEY::W)->stringStopAction = accelMsg;
	mapping->CreateBinding(accelMsg, KEY::S)->stringStopAction = accelMsg;
	mapping->CreateBinding(accelMsg, KEY::CTRL)->stringStopAction = accelMsg;

	String locYMsg = "UpdateLocalRotationY";
	mapping->CreateBinding(locYMsg, KEY::A)->stringStopAction = locYMsg;
	mapping->CreateBinding(locYMsg, KEY::D)->stringStopAction = locYMsg;

	String locXMsg = "UpdateLocalRotationX";
	mapping->CreateBinding(locXMsg, KEY::UP)->stringStopAction = locXMsg;
	mapping->CreateBinding(locXMsg, KEY::DOWN)->stringStopAction = locXMsg;

	String locZMsg = "UpdateLocalRotationZ";
	mapping->CreateBinding(locZMsg, KEY::LEFT)->stringStopAction = locZMsg;
	mapping->CreateBinding(locZMsg, KEY::RIGHT)->stringStopAction = locZMsg;

	mapping->CreateBinding("ChangeCamera", KEY::C);
}

/// Creates the user interface for this state
void AircraftState::CreateUserInterface()
{

}

/** Attempts to free the resources used by the user interface before deleting it.
	Aborts and returns false if any errors occur along the way.
*/
bool AircraftState::DeallocateUserInterface()
{

	return true;
}


/// For handling drag-and-drop files.
void AircraftState::HandleDADFiles(List<String> & files)
{

}


/// What happens.. when we rendar?!
void AircraftState::Render(GraphicsState * graphicsState)
{

}




void AircraftState::CreateStars()
{
	Random starRandom; 
	int requestedStars = 1000;
	
	int numStars = stars.Size();
	int starsToCreate = requestedStars - numStars;

	for (int i = 0; i < starsToCreate; ++i)
	{
		
		Entity * star = MapMan.CreateEntity("Star", ModelMan.GetModel("sphere"), TexMan.GetTexture("Red"));
		stars.Add(star);
	}

	for (int i = 0; i < requestedStars; ++i)
	{
		Entity * star = stars[i];
		// Randomize location
		float r;
		Vector3f pos;
		r = starRandom.Randf(10000.f);
		pos.x = r - 5000.f;
		r = starRandom.Randf(10000.f);
		pos.y = r - 5000.f;
		r = starRandom.Randf(10000.f);
		pos.z = r - 5000.f;
		
		Physics.QueueMessage(new PMSetEntity(POSITION, star, pos));

		int red = starRandom.Randi(255),
			blue = starRandom.Randi(255),
			green = starRandom.Randi(255);
		int bitRed = red << 16;
		int bitBlue = blue << 8;
		int color = bitRed + bitBlue + green;
		Texture * tex = TexMan.GetTextureByHex24(color);
		Graphics.QueueMessage(new GMSetEntityTexture(star, DIFFUSE_MAP, tex));

		// Scale them depending on their distance from origin!
		float distance = pos.Length();
		float scaleMax = 1 + distance * 0.1f;
		float scale = starRandom.Randf(scaleMax);
		Physics.QueueMessage(new PMSetEntity(SET_SCALE, star, scale));
	}
	std::cout<<"\nStars created.";
}

void AircraftState::UpdateAcceleration()
{
	bool wDown = InputMan.KeyPressed(KEY::W);
	bool sDown = InputMan.KeyPressed(KEY::S);
	float relativeAcceleration = 0.f;
	if (wDown)
		relativeAcceleration += 1.f;
	if (sDown)
		relativeAcceleration -= 1.f;
	
	Vector3f acceleration(0,0,relativeAcceleration);


	float thrust = 10.f;
	acceleration *= 10.f;

	// Send it to ze entity.
	Physics.QueueMessage(new PMSetEntity(RELATIVE_ACCELERATION, ship, acceleration));
}

void AircraftState::UpdateLocalRotationX()
{
	bool upDown = InputMan.KeyPressed(KEY::UP);
	bool downDown = InputMan.KeyPressed(KEY::DOWN);
	float relativeRotX = 0.f;
	if (upDown)
		relativeRotX += 1.f;
	if (downDown)
		relativeRotX -= 1.f;

	/// o-o
	relativeRot.x = relativeRotX;
	// Rotations should be something like... degrees per meter traveled?
	UpdateLocalRotation();
}

void AircraftState::UpdateLocalRotationY()
{
	bool aDown = InputMan.KeyPressed(KEY::A);
	bool dDown = InputMan.KeyPressed(KEY::D);
	float relativeRotY = 0.f;
	if (aDown)
		relativeRotY -= 1.f;
	if (dDown)
		relativeRotY += 1.f;

	/// o-o
	relativeRot.y = relativeRotY;
	// Rotations should be something like... degrees per meter traveled?
	UpdateLocalRotation();
}
void AircraftState::UpdateLocalRotationZ()
{	
	bool leftDown = InputMan.KeyPressed(KEY::LEFT);
	bool rightDown = InputMan.KeyPressed(KEY::RIGHT);
	float relativeRotZ = 0.f;
	if (leftDown)
		relativeRotZ += 1.f;
	if (rightDown)
		relativeRotZ -= 1.f;

	/// o-o
	relativeRot.z = relativeRotZ;
	// Rotations should be something like... degrees per meter traveled?
	UpdateLocalRotation();
}

void AircraftState::UpdateLocalRotation()
{
	// Send it to ze entity.
	Physics.QueueMessage(new PMSetEntity(RELATIVE_ROTATION, ship, relativeRot));
}

void AircraftState::NextCamera()
{
	// Check active camera.
	int activeCameraIndex = -1;
	AppWindow * mainWindow = WindowMan.MainWindow();
	Camera * currentCamera = 0;
	if (mainWindow)
	{
		// Get current camera.
		Viewport * mainVp = mainWindow->MainViewport();
		if (mainVp)
			currentCamera = mainVp->camera;
	}
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * c = cameras[i];
		if (c == currentCamera)
		{
			activeCameraIndex = i;
		}
	}

	activeCameraIndex++;

	// Assert it's within range.
	if (activeCameraIndex >= cameras.Size())
		activeCameraIndex = 0;
	// ... 

	// Set next
	Graphics.QueueMessage(new GMSetCamera(cameras[activeCameraIndex]));

}
