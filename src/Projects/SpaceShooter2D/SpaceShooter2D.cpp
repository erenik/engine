/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SpaceShooter2D.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Entity/EntityManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Input/InputManager.h"
#include "Input/Action.h"

#include "Model/Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "Model/ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "File/FileUtil.h"

#include "Maps/MapManager.h"

#include "Network/NetworkManager.h"

#include "Script/Script.h"
#include "Script/ScriptManager.h"

#include "Game/SpaceShooter/SpaceShooterIntegrator.h"
#include "Game/SpaceShooter/SpaceShooterCD.h"
#include "Game/SpaceShooter/SpaceShooterCR.h"

void SetApplicationDefaults()
{
	Application::name = "SpaceShooter2D";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}

void RegisterStates()
{
	SpaceShooter2D * ss = new SpaceShooter2D();
	StateMan.RegisterState(ss);
	StateMan.QueueState(ss);
}

Ship::Ship()
{
	spawned = false;
	entity = NULL;
}
Ship::~Ship()
{
	
}


/// Creates new ship of specified type.
Ship Ship::New(String type)
{
	// For now, just add a default one.
	Ship ship;
	ship.type = type;
	return ship;
}


SpaceShooter2D::SpaceShooter2D()
{
	playerShip = NULL;
	levelCamera = NULL;
}
SpaceShooter2D::~SpaceShooter2D()
{

}
/// Function when entering this state, providing a pointer to the previous StateMan.
void SpaceShooter2D::OnEnter(AppState * previousState)
{
	// Remove overlay.
	// Set up ui.
	if (!ui)
		CreateUserInterface();
	GraphicsMan.QueueMessage(new GMSetUI(ui));
	GraphicsMan.QueueMessage(new GMSetOverlay(NULL));

	mode = IN_MENU;

	// Load Space Race integrator
	integrator = new SpaceShooterIntegrator(0.f);
	PhysicsMan.QueueMessage(new PMSet(integrator));
	cd = new SpaceShooterCD();
	PhysicsMan.QueueMessage(new PMSet(cd));
	cr = new SpaceShooterCR();
	PhysicsMan.QueueMessage(new PMSet(cr));

	// Run OnEnter.ini if such a file exists.
	Script * script = new Script();
	script->Load("OnEnter.ini");
	ScriptMan.PlayScript(script);
}

/// Main processing function, using provided time since last frame.
void SpaceShooter2D::Process(int timeInMs)
{
	switch(mode)
	{
		case PLAYING_LEVEL:
		{
			// Check if we want to spawn any enemy ships.
			// Add all enemy ships too?
			for (int i = 0; i < level.ships.Size(); ++i)
			{
				Ship & ship = level.ships[i];
				if (ship.spawned)
					continue;
				if (AbsoluteValue(levelCamera->position.x - ship.position.x) > 20.f)
					continue;
				Entity * entity = EntityMan.CreateEntity(ship.type, ModelMan.GetModel("sphere.obj"), TexMan.GetTextureByColor(Color(0,255,0,255)));
				entity->position = ship.position;
				entity->RecalculateMatrix();
				ship.entity = entity;
				ship.spawned = true;
				MapMan.AddEntity(entity);
			}
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SpaceShooter2D::OnExit(AppState * nextState)
{

}


/// Creates the user interface for this state
void SpaceShooter2D::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MainMenu.gui");
}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void SpaceShooter2D::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "NewGame")
				NewGame();
			if (msg.Contains("StartMoveShip"))
			{
				String dirStr = msg - "StartMoveShip";
				int dir = Direction::Get(dirStr);
				movementDirections.Add(dir);
				UpdatePlayerVelocity();
			}
			else if (msg.Contains("StopMoveShip"))
			{
				String dirStr = msg - "StopMoveShip";
				int dir = Direction::Get(dirStr);
				while(movementDirections.Remove(dir));
				UpdatePlayerVelocity();
			}
			break;
		}
	}
}


/// Creates default key-bindings for the state.
void SpaceShooter2D::CreateDefaultBindings()
{
	List<Binding*> & bindings = this->inputMapping.bindings;
	bindings.Add(4,
		new Binding(Action::CreateStartStopAction("MoveShipUp"), KEY::W),
		new Binding(Action::CreateStartStopAction("MoveShipDown"), KEY::S),
		new Binding(Action::CreateStartStopAction("MoveShipLeft"), KEY::A),
		new Binding(Action::CreateStartStopAction("MoveShipRight"), KEY::D)
		);
}


/// Starts a new game. Calls LoadLevel
void SpaceShooter2D::NewGame()
{
	LoadLevel("Levels/Stage 1/Level 1-1");
}

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};

/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SpaceShooter2D::LoadLevel(String levelSource)
{
	/// Clear old stuff.
	level.ships.Clear();

	level.millisecondsPerPixel = 200;

	String sourceTxt = levelSource + ".txt";
	String sourcePng = levelSource + ".png";

	List<ShipColorCoding> colorCodings;
	List<String> lines = File::GetLines(sourceTxt);
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (line.StartsWith("ShipType"))
		{
			List<String> tokens = line.Tokenize(" ");
			ShipColorCoding newCode;
			if (tokens.Size() < 2)
				continue;
			newCode.ship = tokens[1];
			assert(tokens[2] == "RGB");
			if (tokens.Size() < 6)
			{
				std::cout<<"ERrror";
				continue;
			}
			newCode.color.x = tokens[3].ParseInt();
			newCode.color.y = tokens[4].ParseInt();
			newCode.color.z = tokens[5].ParseInt();
			colorCodings.Add(newCode);
		}
	}

	List<String> files ;
	GetFilesInDirectory("Levels/Stage 1", files);  
	Texture * tex = TexMan.LoadTexture(sourcePng, true);
	tex->releaseOnBufferization = false;
	assert(tex);
	// Parse it.
	for (int x = 0; x < tex->width; ++x)
	{
		// Get top.
		int topY = tex->height - 1;
		for (int y = topY; y > topY - 20; --y)
		{
			Vector3i color = tex->GetPixelVec4i(x,y);
			// Skip if white.
			if (color.x == 255 && color.y == 255 && color.z == 255)
				continue;
			std::cout<<"\nColor "<<color;
			for (int i = 0; i < colorCodings.Size(); ++i)
			{
				ShipColorCoding & coding = colorCodings[i];
				if (coding.color == color)
				{
					Ship newShip = Ship::New(coding.ship);
					newShip.position.x = x;
					newShip.position.y = 20 - (topY - y);
					// Create ship.
					level.ships.Add(newShip);
				}
			}
		}
	}
	if (level.ships.Size() == 0)
	{	
		std::cout<<"\nError: No Ships in level.";
		return;
	}
	mode = PLAYING_LEVEL;
	// Hide main menu
	GraphicsMan.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, false, ui));
	GraphicsMan.QueueMessage(new GMSetUIb("HUD", GMUI::VISIBILITY, true, ui));
	// Add player?
	if (!playerShip)
	{
		playerShip = EntityMan.CreateEntity("Player ship", ModelMan.GetModel("sphere.obj"), TexMan.GetTextureByColor(Color(255,0,0,255)));
		shipEntities.Add(playerShip);
	}
	// Set player to mid position.
	PhysicsMan.QueueMessage(new PMSetEntity(playerShip, PT_SET_POSITION, Vector3f(0, 10.f, 0)));
	// Register player for rendering.
	MapMan.AddEntity(playerShip);
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera();
	// Set it up.
	levelCamera->position = Vector3f(0,10,10);
	levelCamera->rotation = Vector3f(0,0,0);
//	levelCamera->Begin(Direction::RIGHT); // Move it..!
//	levelCamera->
	GraphicsMan.QueueMessage(new GMSetCamera(levelCamera));

	levelCamera->velocity = Vector3f(1,0,0);
	// Begin movement of player too?
	PhysicsMan.QueueMessage(new PMSetEntity(playerShip, PT_VELOCITY, Vector3f(1.f, 0, 0)));
	// No gravity
	PhysicsMan.QueueMessage(new PMSet(PT_GRAVITY, Vector3f(0,0,0)));
}



void SpaceShooter2D::UpdatePlayerVelocity()
{
	Vector3f totalVec;
	for (int i = 0; i < movementDirections.Size(); ++i)
	{
		Vector3f vec = Direction::GetVector(movementDirections[i]);
		totalVec += vec;
	}
	// Set player speed.
	if (playerShip)
	{
		PhysicsMan.QueueMessage(new PMSetEntity(playerShip, PT_VELOCITY, totalVec + Vector3f(1,0,0)));
	}
}