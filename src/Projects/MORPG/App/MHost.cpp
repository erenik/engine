/// Emil Hedemalm
/// 2015-01-15
/// Host-specific interaction: World creation and such.

#include "MHost.h"
#include "MORPG.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "MORPG/World/WorldGenerator.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Message/MathMessage.h"
#include "Message/MessageTypes.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Maps/MapManager.h"

#include "Model/Model.h"
#include "Model/ModelManager.h"

#include "TextureManager.h"

UserInterface * worldEditor = NULL;

WorldGenerator * activeWorldGenerator = NULL;
List<WorldGenerator*> worldGenerators;

MHost::MHost()
: AppState()
{
}

MHost::~MHost()
{
	worldGenerators.ClearAndDelete();
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void MHost::OnEnter(AppState * previousState)
{
	if (enterMode == WORLD_CREATION)
	{
		EnterWorldCreation();
	}
}

/// Main processing function, using provided time since last frame.
void MHost::Process(int timeInMs)
{
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MHost::OnExit(AppState * nextState)
{
}



/// Callback function that will be triggered via the MessageManager when messages are processed.
void MHost::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetWaterLevel")
			{
				activeWorldGenerator->waterLevel = im->value;
				GenerateWorld();
			}
			break;	
		}
		case MessageType::FLOAT_MESSAGE:
		{
			FloatMessage * fm = (FloatMessage*) message;
			if (msg == "SetWaterSources")
			{
				activeWorldGenerator->water = fm->value;
				GenerateWorld();
			}
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			if (msg == "SetWorldSize")
			{
				Vector2i vec = vm->vec2i;
				if (vec.x < 2)
					vec.x = 2;
				if (vec.y < 2)
					vec.y = 2;
				activeWorldGenerator->size = vec;
				GenerateWorld();
			}
			break;
		}
		case MessageType::STRING:
		{
			if (msg == "NewWorld" || msg == "GenerateWorld")
			{
				GenerateWorld();
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
			break;
		}
	}
}


/// Creates default key-bindings for the state.
void MHost::CreateDefaultBindings()
{
	// Add camera controls and camera stuff as necessary?
}


void MHost::EnterWorldCreation()
{
	// Swap gui.
	if (!worldEditor)
	{
		worldEditor = new UserInterface();
		worldEditor->Load("gui/WorldEditor.gui");
	}
	GraphicsMan.QueueMessage(new GMSetUI(worldEditor));

	/// Create world-generators.
	if (worldGenerators.Size() == 0)
	{
		WorldGenerator * generator = new WorldGenerator();
		worldGenerators.Add(generator);
		activeWorldGenerator = generator;
	}
}

void MHost::GenerateWorld()
{
	world.Delete();
	activeWorldGenerator->GenerateWorld(world);
	Texture * tex = world.GeneratePreviewTexture();
	Model * model = world.GenerateWorldModel();

	if (!worldMapEntity)
		MapMan.CreateEntity("WorldMap", model, tex);
	else
	{	
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
}