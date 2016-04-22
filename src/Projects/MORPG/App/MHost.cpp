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
#include "Graphics/Camera/CameraUtil.h"

#include "MORPG/World/WorldGenerator.h"
#include "MORPG/World/WorldMap.h"
#include "MORPG/World/Zone.h"

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

#include "Input/InputManager.h"
#include "Input/Action.h"

UserInterface * worldEditor = NULL;

WorldGenerator * activeWorldGenerator = NULL;
List<WorldGenerator*> worldGenerators;

Camera * worldCamera = NULL;

MHost::MHost()
: AppState()
{
	mode = WORLD_EDITOR;
	settlementIndex = 0;
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
			if (msg == "SetSmoothing")
			{
				activeWorldGenerator->smoothing = im->value;
				GenerateWorld();
			}
			else if (msg == "SetNumSettlements")
			{
				activeWorldGenerator->numSettlements = im->value;
				GenerateSettlements();
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
			else if (msg == "SetWaterDepth")
			{
				activeWorldGenerator->waterDepth = fm->value;
				GenerateWorld();
			}
			else if (msg == "SetMountains")
			{
				activeWorldGenerator->mountains = fm->value;
				GenerateWorld();
			}
			else if (msg == "SetMountainHeight")
			{
				activeWorldGenerator->mountainHeight = fm->value;
				GenerateWorld();
			}
			else if (msg == "SetSmoothingMultiplier")
			{
				activeWorldGenerator->smoothingMultiplier = fm->value;
				GenerateWorld();
			}
			else if (msg == "SetOceanElevation")
			{
				world.oceanElevation = fm->value;
				worldMap.UpdateOcean();
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
			else if (msg == "SetOceanColor")
			{
				Vector4f vec = vm->vec4f;
				world.oceanColor = vec;
				worldMap.UpdateOcean();
			}
			break;
		}
		case MessageType::STRING:
		{
			if (msg == "NewWorld" || msg == "GenerateWorld")
			{
				GenerateWorld(true);
				return;
			}
			if (msg == "GenerateSettlements")
			{
				GenerateSettlements(true);
			}
			if (msg == "WorldEditor")
			{
				EnterWorldCreation();
			}
			if (msg == "SettlementEditor")
			{
				// Enter first settlement in the list.
				if (world.settlements.Size())
				{
					if (mode == SETTLEMENT_EDITOR)
					{
						if (InputMan.KeyPressed(KEY::SHIFT))
						{
							--settlementIndex;
							if (settlementIndex < 0)
								settlementIndex += world.settlements.Size();
						}
						else
							++settlementIndex;
					}
					Zone * zone = world.settlements[settlementIndex % world.settlements.Size()];
					zone->MakeActive();
					mode = SETTLEMENT_EDITOR;
				}
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
				file.open("tmp.world", std::ios_base::out | std::ios_base::binary);
				if (!file.is_open())
					return;
				world.WriteTo(file);
				file.close();			
			}
			else if (msg == "LoadWorld")
			{
				std::fstream file;
				file.open("tmp.world", std::ios_base::in | std::ios_base::binary);
				if (!file.is_open())
					return;
				world.ReadFrom(file);
				OnWorldUpdated();
				file.close();
			}
			HandleCameraMessages(msg);
			break;
		}
	}
}


/// Creates default key-bindings for the state.
void MHost::CreateDefaultBindings()
{
	// Add camera controls and camera stuff as necessary?
	InputMapping & mapping = this->inputMapping;
	mapping.bindings.Add(new Binding(Action::FromString("WorldEditor"), KEY::F1));
	mapping.bindings.Add(new Binding(Action::FromString("SettlementEditor"), KEY::F2));
	mapping.bindings.Add(new Binding(Action::FromString("SettlementEditor"), KEY::SHIFT, KEY::F2));
	// Add default camera bindings.
	mapping.bindings.Add(CreateDefaultCameraBindings());
}


void MHost::HandleCameraMessages(String msg)
{
	Camera * camera = CameraMan.ActiveCamera();
	if (!camera)
		return;
	ProcessCameraMessages(msg, camera);
}


void MHost::EnterWorldCreation()
{
	mode = WORLD_EDITOR;
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

	// Make the world-map and its camera active for display and thus interaction. o.o
	worldMap.MakeActive();
}

void MHost::GenerateWorld(bool newRandomSeed)
{
//	world.Delete();
	activeWorldGenerator->GenerateWorld(world, newRandomSeed);
	OnWorldUpdated();
}

void MHost::GenerateSettlements(bool newRandomSeed)
{
	activeWorldGenerator->GenerateSettlements(world, newRandomSeed);
	// Actually generate the contents of them too?
	for (int i = 0; i < world.settlements.Size(); ++i)
	{
		Zone * zone = world.settlements[i];
		activeWorldGenerator->GenerateSettlement(zone);
	}
	worldMap.UpdateSettlements();
}

void MHost::OnWorldUpdated()
{
	worldMap.Update();
	worldMap.UpdateSettlements();
	worldMap.CenterCamera();
}
