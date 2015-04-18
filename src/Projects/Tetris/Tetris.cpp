/// Emil Hedemalm
/// 2015-01-06
/// Our version of Tetris

#include "Tetris.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Random/Random.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Input/Keys.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

Vector2i fieldSize(10,20);

Random tetrisRand;
Random typeRand;

TetrisBrick::TetrisBrick()
{
	millisecondsAlive = 0;
	rotation = 0;
}

TetrisBrick::~TetrisBrick()
{
//	std::cout<<"\nBrick destructor.";
}	

List<Vector2f> TetrisBrick::GetLocalPartLocations()
{
	List<Vector2f> partLocs;
	switch(type)
	{
		case TetrisBrick::I:partLocs.Add(4, Vector2f(0,2), Vector2f(0, 1), Vector2f(0,0), Vector2f(0,-1)); break;
		case TetrisBrick::J: partLocs.Add(4, Vector2f(0,1), Vector2f(0, 0), Vector2f(0,-1), Vector2f(-1,-1)); break;
		case TetrisBrick::L: partLocs.Add(4, Vector2f(0,1), Vector2f(0, 0), Vector2f(0,-1), Vector2f(1,-1)); break;
		case TetrisBrick::O: partLocs.Add(4, Vector2f(0,1), Vector2f(1, 1), Vector2f(0,0), Vector2f(1,0)); break;
		case TetrisBrick::S: partLocs.Add(4, Vector2f(0,0), Vector2f(1, 0), Vector2f(0,-1), Vector2f(-1,-1)); break;
		case TetrisBrick::Z: partLocs.Add(4, Vector2f(0,0), Vector2f(-1, 0), Vector2f(0,-1), Vector2f(1,-1)); break;
		case TetrisBrick::T: partLocs.Add(4, Vector2f(0,0), Vector2f(-1, 0), Vector2f(+1,0), Vector2f(0,+1)); break;
		default: partLocs.Add(Vector2f());
	}
	return partLocs;
}

List<Vector2i> TetrisBlock::GetAbsPartLocations()
{
	List<Vector2f> locLocs = GetLocalPartLocations();
	List<Vector2i> absLocs;
	for (int i = 0; i < locLocs.Size(); ++i)
	{
		// Rotate as needed?
		Vector2f localLocation = locLocs[i];
		Vector2f rotated = Matrix2f::InitRotationMatrixZ(rotation * (PI * 0.5)).Product(localLocation);
		Vector2f absLoc = rotated + position;
		Vector2i absLoci = absLoc;
		absLocs.Add(absLoci);
	}
	return absLocs;
}


Tetris::Tetris()
: AppState()
{
	millisecondsPassed = 0;
	movingBrick = NULL;
}

/// Virtual destructor to discard everything appropriately.
Tetris::~Tetris()
{
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void Tetris::OnEnter(AppState * previousState)
{
	// Create the field?
	CreateField();
	SetupCamera();
	timeStepMs = 1000;
	score = 0;
	rowsCompletedTotal = 0;
	SleepThread(1000);
}

/// Main processing function, using provided time since last frame.
void Tetris::Process(int timeInMs)
{
	millisecondsPassed += timeInMs;
	// Check time-step
	if (movingBrick && millisecondsPassed > timeStepMs)
	{
		// Unable to move more?! o.o
		if (!MoveBrick(movingBrick, Vector2f(0,-1)))
		{
			// Check if it's sticking out on top, because if it is, it's game over.
			List<Vector2i> positions = movingBrick->GetAbsPartLocations();
			for (int i = 0; i < positions.Size(); ++i)
			{
				if (positions[i].y > fieldSize.y)
				{
					GameOver();
					return;
				}
			}
			// Delete the moving brick, this will spawn a new one.
			SAFE_DELETE(movingBrick);
			// Evaluate rows if any was completed.
			EvaluateRows();
		}
		millisecondsPassed -= timeStepMs;
	}

	// Spawn a brick if needed.
	if (movingBrick == NULL)
	{
		// Spawn a new brick and its segmented parts.
		std::cout<<"\nSpawwwnnn";
		Vector3f position;
		// Create brick.
		movingBrick = new TetrisBrick();
		movingBrick->type = typeRand.Randi(TetrisBrick::MAX_TYPES);
		// Place it somewhere in X, 4 units above?
		movingBrick->position.y = fieldSize.y + 1;
		movingBrick->position.x = tetrisRand.Randi(fieldSize.x);
		// Ensure it is within the accepted boundaries.
		EnsureBrickWithinBoundaries(movingBrick);
		CreateParts(movingBrick);
		// Place it.
		bool okToPlace = CanPlace(movingBrick);
		if (okToPlace)
		{
			Place(movingBrick);
		}
		else 
		{
			// Game over.
			GameOver();
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void Tetris::OnExit(AppState * nextState)
{
	SAFE_DELETE(movingBrick);

	for (int i = 0; i < fieldSize.x; ++i)
	{
		delete[] field[i];
	}
	delete[] field;
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void Tetris::ProcessMessage(Message * message)
{
	String msg = message->msg;
	if (msg == "Reset Camera")
		SetupCamera();
	if (msg == "Left")
	{
		MoveBrick(movingBrick, Vector2f(-1,0));
	}
	else if (msg == "Right")
	{
		MoveBrick(movingBrick, Vector2f(1,0));
	}
	else if (msg == "Down")
		MoveBrick(movingBrick, Vector2f(0,-1));
	else if (msg == "Rotate clockwise")
	{
		RotateBrick(movingBrick, 1);
	}
	else if (msg == "Rotate counter-clockwise")
	{
		RotateBrick(movingBrick, -1);	
	}
}


/// Creates default key-bindings for the state.
void Tetris::CreateDefaultBindings()
{
	InputMapping & input = inputMapping;
	input.CreateBinding("Reset Camera", KEY::HOME);
	input.CreateBinding("Left", KEY::LEFT)->activateOnRepeat = true;
	input.CreateBinding("Right", KEY::RIGHT)->activateOnRepeat = true;
	input.CreateBinding("Down", KEY::DOWN)->activateOnRepeat = true;
	input.CreateBinding("Rotate clockwise", KEY::CTRL)->SetActivateOnRepeat(true);
	input.CreateBinding("Rotate counter-clockwise", KEY::SPACE)->SetActivateOnRepeat(true);
}

/*
/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void Tetris::CreateUserInterface()
{ 

}
*/

#include "Application/Application.h"

void SetApplicationDefaults()
{
	Application::name = "Tetris";
}

#include "StateManager.h"

void RegisterStates()
{
	Tetris * tetris = new Tetris();
	StateMan.RegisterState(tetris);
	StateMan.QueueState(tetris);
}


void Tetris::CreateField()
{
	field = new Entity ** [fieldSize.x];
	for (int i = 0; i < fieldSize.x; ++i)
	{
		field[i] = new Entity * [fieldSize.y];
		for (int j = 0; j < fieldSize.y; ++j)
		{
			field[i][j] = NULL;
		}
	}
}

void Tetris::SetupCamera()
{
	// Grab current camera, or just create own..?
	Camera * tetrisCamera = CameraMan.NewCamera();
	tetrisCamera->rotation = Vector3f();
	tetrisCamera->position = Vector3f(fieldSize.x * 0.5,fieldSize.y * 0.5,20);
//	tetrisCamera->position = Vector3f();
	tetrisCamera->projectionType = Camera::ORTHOGONAL;
	tetrisCamera->zoom = 20.f;

	GraphicsMan.QueueMessage(new GMSetCamera(tetrisCamera));
}

void Tetris::EnsureBrickWithinBoundaries(TetrisBrick * brick)
{
	List<Vector2i> abs = movingBrick->GetAbsPartLocations();
	for (int i = 0; i < abs.Size(); ++i)
	{
		if (abs[i].x < 0)
		{
			movingBrick->position.x += 1;
			return EnsureBrickWithinBoundaries(brick);
		}
		if (abs[i].x >= fieldSize.x)
		{
			movingBrick->position.x -= 1;
			return EnsureBrickWithinBoundaries(brick);
		}
	}
}

void Tetris::CreateParts(TetrisBrick * brick)
{
	Texture * tex = NULL;
	int color = 0xFF0000FF;
	switch(brick->type)
	{
		case TetrisBrick::I: color = 0x00FF00FF; break;
		case TetrisBrick::J: color = 0x2255FFFF; break;
		case TetrisBrick::L: color = 0xFF0000FF; break;
		case TetrisBrick::O: color = 0xFFFF00FF; break;
		case TetrisBrick::S: color = 0xAAAAAAFF; break;
		case TetrisBrick::T: color = 0xFF00FFFF; break;
		case TetrisBrick::Z: color = 0x00FFAAFF; break;
	}
	tex = TexMan.GetTextureByHex32(color);

	List<Vector2f> partLocations = brick->GetLocalPartLocations();

	for (int i = 0; i < partLocations.Size(); ++i)
	{
		Vector2f partLoc = partLocations[i];
		// Create its parts.
		Entity * brickPart = MapMan.CreateEntity("Brick-part", ModelMan.GetModel("Sprite"), tex, partLoc + brick->position);
		brickParts.Add(brickPart);
		// Add part to brick.
		brick->parts.Add(brickPart);
	}
	// Scale 'em.
	PhysicsMan.QueueMessage(new PMSetEntity(brick->parts, PT_SET_SCALE, 0.9f));
}


/// Unlinks block parts from the field, used before movements.
void Tetris::Unlink(List<Entity*> blockParts)
{
	int blocksNulled = 0;
	for (int i = 0; i < fieldSize.x; ++i)
	{
		for (int j = 0; j < fieldSize.y; ++j)
		{
			if (blockParts.Exists(field[i][j]))
			{
//				std::cout<<"\nBlock removed at x"<<i<<" y"<<j;
				field[i][j] = NULL;
				++blocksNulled;
			}
		}
	}
	int partNulledInField = blocksNulled;
//	std::cout<<"\nBlock parts removed from "<<blocksNulled<<" locations on the field";
	// Take into consideration blocks beyond the upper border.
	for (int i = 0; i < blockParts.Size(); ++i)
	{
		if (blockParts[i]->position.y >= fieldSize.y)
			++blocksNulled;
	}
//	assert(blocksNulled >= 4);
}

/// Checks if the block can be placed in the field properly. Only Y+ may go beyond the field's limits.
bool Tetris::CanPlace(TetrisBlock * block)
{
	List<Vector2i> positions = block->GetAbsPartLocations();
	for (int i = 0; i < positions.Size(); ++i)
	{
		Vector2i pos = positions[i];
		if (pos.x < 0 || pos.y < 0 ||
			pos.x >= fieldSize.x)
			return false;
		/// Allow blocks above the Y+ border.
		if (pos.y >= fieldSize.y)
			continue;
		/// check vacancy in the field.
		if (field[pos.x][pos.y] != NULL)
			return false;
	}
	return true;
}

/// Places. Calls CanPlace automatically unless alreadyChecked is true.
bool Tetris::Place(TetrisBlock * block, bool alreadyChecked /* = false*/)
{
	if (!alreadyChecked)
		assert(CanPlace(block));
	
	List<Vector2i> absLocs = block->GetAbsPartLocations();
	for (int i = 0; i < absLocs .Size(); ++i)
	{
		Vector2i pos = absLocs[i];
		assert(pos.x >= 0 && pos.y >= 0);
		assert(pos.x < fieldSize.x);
		Entity * part = block->parts[i];
		/// Allow blocks above the Y+ border.
		if (pos.y >= fieldSize.y)
		{
			// Make invisible for the time being?
			GraphicsMan.QueueMessage(new GMSetEntityb(part, GT_VISIBILITY, false));
			continue;
		}
		// Make invisible for the time being?
		GraphicsMan.QueueMessage(new GMSetEntityb(part, GT_VISIBILITY, true));

		/// Set field to point to it.
		assert(field[pos.x][pos.y] == NULL);
		field[pos.x][pos.y] = part;

		/// Set position of rendered entity.
		Vector3f position3f = pos;
		PhysicsMan.QueueMessage(new PMSetEntity(part, PT_SET_POSITION, position3f));
	}
	return true;
}

/// End current game. Display stuff.
void Tetris::GameOver()
{
	std::cout<<"\nGame over!";
	
	// Delete all entities.
	MapMan.DeleteAllEntities();
	for (int i = 0; i < fieldSize.x; ++i)
	{
		for (int j = 0; j < fieldSize.y; ++j)
		{
			field[i][j] = NULL;
		}
	}
	// Delete the moving brick.
	SAFE_DELETE(movingBrick);

	timeStepMs = 1000;
	score = 0;
	rowsCompletedTotal = 0;
}

/// Returns false if it failed due to obstruction.
bool Tetris::MoveBrick(TetrisBrick * brick, Vector2f distance)
{
	Unlink(brick->parts);
	// Move brick?
	TetrisBrick hypoMove = *brick;
	// Check location of brick parts if we move it down one step.
	hypoMove.position += distance;
	bool ok = CanPlace(&hypoMove);
	if (ok)
	{
		Place(&hypoMove);
		*brick = hypoMove;
		return true;
	}	
	Place(brick);
	return false;
}


/// Returns false if it failed due to obstruction.
bool Tetris::RotateBrick(TetrisBrick * brick, int turnsClockwise)
{
	Unlink(brick->parts);
	// Move brick?
	TetrisBrick hypoMove = *brick;
	// Check location of brick parts if we move it down one step.
	hypoMove.rotation += turnsClockwise;
	// Round it out?
	if (hypoMove.rotation == 4)
		hypoMove.rotation = 0;
	// Force it to be positive, between 0 and 3 (inclusive) preferably.
	if (hypoMove.rotation < 0)
		hypoMove.rotation += 4;
	// Type-specific rotation.
	switch(brick->type)
	{
		case TetrisBrick::O: hypoMove.rotation = 0; break;
		case TetrisBrick::I:
		case TetrisBrick::S:
		case TetrisBrick::Z:
			// Clamp rotations to 0 and 1.
			hypoMove.rotation = hypoMove.rotation % 2;
			break;
	}

	bool ok = CanPlace(&hypoMove);
	if (ok)
	{
		Place(&hypoMove);
		*brick = hypoMove;
		return true;
	}	
	Place(brick);
	return false;
}

void Tetris::EvaluateRows()
{
	List<int> rowsComplete;
	for (int y = 0; y < fieldSize.y; ++y)
	{
		bool rowComplete = true;
		for (int x = 0; x < fieldSize.x; ++x)
		{
			if (field[x][y])
			{
			
			}
			// Nothing there.
			else 
			{
				// Break inner loop?
				rowComplete = false;
			}
		}
		if (rowComplete)
		{
			rowsComplete.Add(y);
		}
	}
	if (rowsComplete.Size() == 0)
		return;
	
	// Kill rows! o.o
	for (int i = 0; i < rowsComplete.Size(); ++i)
	{
		int row = rowsComplete[i];
		for (int x = 0; x < fieldSize.x; ++x)
		{
			MapMan.DeleteEntity(field[x][row]);
			field[x][row] = NULL;
		}
	}

	// Give score! o.o
	score += 1000 * pow(1.5f, (float)(rowsComplete.Size() - 1));
	
	std::cout<<"\nScore: "<<score;

	rowsCompletedTotal += rowsComplete.Size();
	// Time step..!
	this->timeStepMs = 1000 - rowsCompletedTotal * 10;

	// Animation...! o.o ?
	
	SleepThread(1000);

	// Move down shit! o.o
	// Begin with upper most row which was completed.
	for (int i = rowsComplete.Size() - 1; i >= 0; --i)
	{
		// And move all content above down one step.
		int clearedRow = rowsComplete[i];
		// 
		for (int y = clearedRow + 1; y < fieldSize.y; ++y)
		{
			for (int x = 0; x < fieldSize.x; ++x)
			{
				if (field[x][y])
				{
					// Move it down internally.
					field[x][y - 1] = field[x][y];
					Entity * entity = field[x][y];
					field[x][y] = NULL;
					// Move it down graphically.
					PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_SET_POSITION, Vector3f(x,y-1,0))); 
				}
			}
		}
	}
}
