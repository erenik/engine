/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "MORPG.h"

#include "Application/Application.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Maps/MapManager.h"

#include "MORPG/World/WorldGenerator.h"


WorldGenerator * activeWorldGenerator = NULL;
List<WorldGenerator*> worldGenerators;

Camera * mapPreviewCamera = NULL;

void RegisterStates()
{
	MORPG * global = new MORPG();
	StateMan.RegisterState(global);
	StateMan.QueueGlobalState(global);
}

void SetApplicationDefaults()
{
	Application::name = "Time and time again - a MORPG sandbox";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}


MORPG::MORPG()
{
	worldMapEntity = NULL;
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void MORPG::OnEnter(AppState * previousState)
{

	if (!mapPreviewCamera)
		mapPreviewCamera = CameraMan.NewCamera();

	Graphics.QueueMessage(new GMSetCamera(mapPreviewCamera, CT_ROTATION, Vector3f()));

	/// World map... 
//	worldMapEntity = MapMan.CreateEntity("World map entity", NULL, NULL);
	Model * plane = ModelMan.GetModel("plane");
	Texture * white = TexMan.GetTexture("White");
	worldMapEntity = MapMan.CreateEntity("World map entity", plane, white);

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
				Texture * tex = world.GeneratePreviewTexture();
				Model * model = world.GenerateWorldModel();

				if (!worldMapEntity)
					MapMan.CreateEntity("WorldMap", model, tex);
				else
				{
		//			
					Model * plane = ModelMan.GetModel("plane");
					Texture * white = TexMan.GetTexture("White");
					Graphics.QueueMessage(new GMSetEntity(worldMapEntity, GT_MODEL, plane));
					Graphics.QueueMessage(new GMSetEntityTexture(worldMapEntity, DIFFUSE_MAP, white));

					// Re-bufferize the texture.
					Graphics.QueueMessage(new GMBufferTexture(tex));
					// Same for the model..
					Graphics.QueueMessage(new GMBufferMesh(model->GetTriangulatedMesh()));
					// Try our model..
					Graphics.QueueMessage(new GMSetEntity(worldMapEntity, GT_MODEL, model));

					Graphics.QueueMessage(new GMSetEntityTexture(worldMapEntity, DIFFUSE_MAP, tex));
					Physics.QueueMessage(new PMSetEntity(worldMapEntity, PT_SET_SCALE, Vector3f(15.f, 1.f, 15.f)));
				}
				return;
			}
			else if (msg == "MoarWater")
			{
				activeWorldGenerator->water += 0.05f;
				if (activeWorldGenerator->water > 0.95f)
					activeWorldGenerator->water = 0.95f;
				MesMan.QueueMessages("GenerateWorld");
			}
			else if (msg == "LessWater")
			{
				activeWorldGenerator->water -= 0.05f;
				if (activeWorldGenerator->water < 0.0f)
					activeWorldGenerator->water = 0.0f;
				MesMan.QueueMessages("GenerateWorld");
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


