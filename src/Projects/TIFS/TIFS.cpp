/// Emil Hedemalm
/// 2014-07-29
/** Reboot of the TIFS/Virtus project as was conducted during 10 weeks in the spring of 2013 with the following members:
	- Emil Hedemalm
	- Aksel Kornesjö
	- Cheng Wu
	- Andreas Söderberg
	- Michaela Sjöström
	- Fredric Lind

	Old dev blog:	http://focus.gscept.com/gp13-3/
	Old facebook page:	https://www.facebook.com/VirtusLTU
	Our final release for that first iteration of the project: http://svn.gscept.com/gp13-3/public/Virtus.zip
*/

/// The main/global Application state for the game.

#include "TIFS.h"
#include "TIFSMapEditor.h"

#include "TIFS/Physics/TIFSIntegrator.h"

#include "TIFS/Properties/TIFSTurretProperty.h"
#include "TIFS/Properties/TIFSDroneProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Camera/Camera.h"


#include "Message/Message.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Input/InputManager.h"

#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"


#include "Random/Random.h"

TIFS * tifs = NULL;
TIFSMapEditor * mapEditor = NULL;

Camera * freeFlyCamera = NULL;

void SetApplicationDefaults()
{
	Application::name = "The Invader from Space / VIRTUS";
	TextFont::defaultFontSource = "font3";
}

void RegisterStates()
{
	tifs = new TIFS();
	StateMan.RegisterState(tifs);
	mapEditor = new TIFSMapEditor();
	StateMan.RegisterState(mapEditor);
	StateMan.QueueGlobalState(tifs);
}


TIFS::TIFS()
{
	
}

TIFSIntegrator * integrator = 0;

/// Function when entering this state, providing a pointer to the previous StateMan.
void TIFS::OnEnter(AppState * previousState)
{
	// Setup integrator.
	if (!integrator)
		integrator = new TIFSIntegrator();

	if (!freeFlyCamera)
		freeFlyCamera = CameraMan.NewCamera();

	// Set free form camera as active.
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera));
	ResetCamera();

	Physics.QueueMessage(new PMSet(integrator));

	// Remove shit.
	
	// Do shit.

	if (!ui)
		CreateUserInterface();
	// Set ui as active?
	Graphics.QueueMessage(new GMSetUI(ui));

	// Remove shit
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	Input.ForceNavigateUI(true);

}

/// Main processing function, using provided time since last frame.
void TIFS::Process(int timeInMs)
{
	// Sleep.
	Sleep(100);
}
/// Function when leaving this state, providing a pointer to the next StateMan.
void TIFS::OnExit(AppState * nextState)
{

}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessPacket(Packet * packet)
{

}
/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessMessage(Message * message)
{	
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "Editor" || msg == "GoToEditor")
			{
				// Go there?
				StateMan.QueueState(mapEditor);
			}
			else if (msg == "OpenMainMenu")
			{
				Graphics.QueueMessage(new GMSetUI(ui));
				StateMan.QueueState(NULL);
			}
			else if (msg == "CreateTurrets")
			{
				CreateTurrets();
			}
			else if (msg == "SpawnDrones")
			{
				SpawnDrones();
			}
			else if (msg == "ResetCamera")
				ResetCamera();
			break;	
		}
	}
}

/// Creates default key-bindings for the state.
void TIFS::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	mapping->CreateBinding("ResetCamera", KEY::HOME);
}


/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void TIFS::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/TIFS.gui");

}

void TIFS::ResetCamera()
{
	// Reset the freeFlyCamera?
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera, CT_POSITION, Vector3f(0,40,30)));
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera, CT_ROTATION, Vector3f(-0.4f, 0, 0)));
}


Random droneRandom;

/// Randomly!!!! o-=o
void TIFS::SpawnDrones()
{
	MapMan.DeleteEntities(drones);
	drones.Clear();

	int numDronesToSpawn = 5;
	for (int i = 0; i < 5; ++i)
	{
		Vector3f pos;
		pos.x = droneRandom.Randf(50.f) - 25.f;
		pos.z = droneRandom.Randf(50.f) - 25.f;
		pos.y = droneRandom.Randf(50.f);
	
		SpawnDrone(pos);
	}
}


void TIFS::SpawnDrone(Vector3f atLocation)
{
	Entity * drone = MapMan.CreateEntity("Drone", ModelMan.GetModel("Sphere"), TexMan.GetTexture("Cyan"), atLocation);
	TIFSDroneProperty * droneProp = new TIFSDroneProperty(drone);
	drone->properties.Add(droneProp);
	drones.Add(drone);
	// Setup physics and other stuff.
	droneProp->OnSpawn();
}

void TIFS::CreateTurrets()
{
	MapMan.DeleteEntities(turrets);
	turrets.Clear();

	Random turretRandom;
	int turretsToCreate = 5;
	for (int i = 0; i < turretsToCreate; ++i)
	{
		Vector3f position;
		position.x = turretRandom.Randf(50.f) - 25.f;
		position.z = turretRandom.Randf(50.f) - 25.f;
		CreateTurret(0, position);
	}
}


/// Creates a turret!
void TIFS::CreateTurret(int ofSize, Vector3f atLocation)
{
	Entity * turretBase = MapMan.CreateEntity("TurretBase", ModelMan.GetModel("Turrets/LargeBase"), TexMan.GetTexture("Green"));
	Physics.QueueMessage(new PMSetEntity(turretBase, PT_POSITION, atLocation));
	turrets.Add(turretBase);


	/// Add a child-mesh-part to the first turret-part!
	Model * swivel = ModelMan.GetModel("Turrets/LargeSwivel");
	Entity * swivelEntity = MapMan.CreateEntity("TurretSwivel", swivel, TexMan.GetTexture("Blue"));
	
	/// Make the swivel's transformation depend on the base'.
	Graphics.QueueMessage(new GMSetEntity(swivelEntity, GT_PARENT, turretBase)); 
	turrets.Add(swivelEntity);

	// Move it up a bit.
	Model * underBarrel = ModelMan.GetModel("Turrets/LargeUnderBarrel");
	Entity * underBarrelEntity = MapMan.CreateEntity("TurretUnderBarrel", underBarrel, TexMan.GetTexture("Red"), Vector3f(0, 2, -1.f));
	Graphics.QueueMessage(new GMSetEntity(underBarrelEntity, GT_PARENT, swivelEntity));
	turrets.Add(underBarrelEntity);

	// Add barrel.
	Model * barrel = ModelMan.GetModel("Turrets/LargeBarrel");
	Entity * barrelEntity = MapMan.CreateEntity("TurretBarrel", barrel, TexMan.GetTexture("White"));
	Graphics.QueueMessage(new GMSetEntity(barrelEntity, GT_PARENT, underBarrelEntity));
	turrets.Add(barrelEntity);

	// Create the ... Turret Property.

	TIFSTurretProperty * prop = new TIFSTurretProperty(turretBase, swivelEntity, underBarrelEntity, barrelEntity);
	turretBase->properties.Add(prop);
}
