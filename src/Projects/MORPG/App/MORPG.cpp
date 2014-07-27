/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "MORPG.h"

#include "Application/Application.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/Message.h"

#include "Maps/MapManager.h"

#include "MORPG/World/WorldGenerator.h"

WorldGenerator * activeWorldGenerator = NULL;
List<WorldGenerator*> worldGenerators;

void RegisterStates()
{
	MORPG * global = new MORPG();
	StateMan.RegisterState(global);
	StateMan.SetGlobalState(global);
}

void SetApplicationDefaults()
{
	Application::name = "Time and time again - a MORPG sandbox";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}



/// Function when entering this state, providing a pointer to the previous StateMan.
void MORPG::OnEnter(AppState * previousState)
{

	/// World map... 
	worldMapEntity = MapMan.CreateEntity("World map entity", NULL, NULL);

	// Set up ui.
	if (!ui)
		CreateUserInterface();

	Graphics.QueueMessage(new GMSetUI(ui));
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	/// Create world-generators.
	if (worldGenerators.Size() == 0)
	{
		WorldGenerator * generator = new WorldGenerator();
		worldGenerators.Add(generator);
		activeWorldGenerator = generator;
	}
}

/// Main processing function, using provided time since last frame.
void MORPG::Process(int timeInMs)
{

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MORPG::OnExit(AppState * nextState)
{

}

/// Creates the user interface for this state
void MORPG::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MORPG.gui");
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MORPG::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "NewWorld" || msg == "GenerateWorld")
			{
				world.Delete();
				activeWorldGenerator->GenerateWorld(world);
				world.GeneratePreviewTexture();
				world.GenerateWorldModel();
			}
			else if (msg == "SaveWorld")
			{
				std::fstream file;
				file.open("tmp.world", std::ios_base::out);
				if (!file.is_open())
					return;
				world.WriteTo(file);
				file.close();			
			}
			else if (msg == "LoadWorld")
			{
				std::fstream file;
				file.open("tmp.world", std::ios_base::in);
				if (!file.is_open())
					return;
				world.ReadFrom(file);
				file.close();
			}
			break;
		}
	}
}


