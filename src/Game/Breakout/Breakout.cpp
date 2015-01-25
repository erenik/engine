/// Emil Hedemalm
/// 2014-07-16
/// The classic Pong integrated as a filter into the IPM CVPipeline.

#include "Breakout.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Message/Message.h"

#include "BreakoutBallProperty.h"
#include "BreakoutPaddleProperty.h"
#include "BreakoutBrickProperty.h"
#include "BreakoutPowerupProperty.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "BreakoutIntegrator.h"
#include "BreakoutCR.h"
#include "BreakoutCD.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"
				
Breakout::Breakout() 
	: Game2D("Breakout")
{
	useMouseInput = false;

	breakoutIntegrator = 0;
	breakoutCR = 0;
	breakoutCD = 0;
}

Breakout::~Breakout()
{
	// Reset integrator and stuff before we delete any entities, or wat?
//	Sleep(50);
	ballProperties.Clear();
	MapMan.DeleteEntities(GetEntities());
}

/// Allocates and sets things up.
void Breakout::Initialize()
{
	paddleScale = 1.f;
	ballScale = 1.f;
	aiSpeed = 1.f;
	ballMinVerticalSpeed = 5.f;
	ballSpeedIncreasePerBounce = .1f;
	currentLevel = 0;
	numBalls = 1;

	breakoutRand.Init(Time());

	Texture * white = TexMan.GetTexture("White"),
		* red = TexMan.GetTexture("Red"),
		* alpha = TexMan.GetTexture("Alpha"),
		* blue = TexMan.GetTexture("Blue"),
		* green = TexMan.GetTexture("Green"),
		* gray = TexMan.GetTexture("Grey"),
		* wallColor = TexMan.GetTextureByHex32(0x000077FF),
		* verticalBarColor = TexMan.GetTextureByHex24(0x999999);
	Model * sphere = ModelMan.GetModel("Sphere");
	Model * pad = ModelMan.GetModel("Breakout/Paddle");
	
	player1 = MapMan.CreateEntity("Player1", pad, white);
	player1Properties = new BreakoutPaddleProperty(player1, Vector2f(0, 1), 15.f);
	player1->properties.Add(player1Properties);
	players.Add(player1);

	score1Entity = MapMan.CreateEntity("Player1Score", NULL, alpha);
	scoreEntities.Add(score1Entity);

	Model * cube = ModelMan.GetModel("Cube");
	frame.Add(		
		topBar = MapMan.CreateEntity("Topbar", cube, red),
		leftBar = MapMan.CreateEntity("LeftBar", cube, green),
		rightBar = MapMan.CreateEntity("RightBar", cube, blue),
		base = MapMan.CreateEntity("Goals", cube, verticalBarColor)
	);

	/// Add all entities.
}

/// Resets the entire game. Similar to a hardware reset on old console games.
void Breakout::Reset()
{
	gameState = SETTING_UP_PLAYFIELD;
}

void Breakout::SetPaddleScale(float scale)
{
	paddleScale = scale;
	player1Properties->initialScale = Vector3f(1,1,1) * scale;
	PhysicsMan.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, paddleScale));
}

void Breakout::SetBallScale(float scale)
{
	ballScale = scale;
	PhysicsMan.QueueMessage(new PMSetEntity(balls, PT_SET_SCALE, ballScale));
}

void Breakout::SetBallSpeed(float unitsPerSecond)
{
	ballMinVerticalSpeed = unitsPerSecond;
	UpdateBallProperties();
}

void Breakout::SetSpeedIncreasePerBounce(float unitsPerSecond)
{
	ballSpeedIncreasePerBounce = unitsPerSecond;
	UpdateBallProperties();
}

void Breakout::SetAISpeed(float newSpeed)
{
	this->aiSpeed = newSpeed;
	for (int i = 0; i < players.Size(); ++i)
	{
		Entity * player = players[i];
		BreakoutPaddleProperty * bpp = (BreakoutPaddleProperty *) player->GetProperty("BreakoutPaddleProperty");
		if (!bpp)
			continue;
		bpp->aiSpeed = aiSpeed;
	}
}	

void Breakout::SetNumBalls(int num)
{
	this->numBalls = num;
	// Reset level.
	gameState = SETTING_UP_PLAYFIELD;
}


void Breakout::SetZ(float newZ)
{
	this->z = newZ;
	// Set constant Z in the integrator.
	if (breakoutIntegrator)
		breakoutIntegrator->constantZ = newZ;
	this->gameState = SETTING_UP_PLAYFIELD;
}

void Breakout::SetLevel(int level)
{
	Physics.Pause();
	this->currentLevel = level;
	// Load it.
	SpawnBricks(currentLevel);
	SpawnBalls();
	Physics.Resume();
}

/// For setting position manually o.o
void Breakout::SetPlayerPositionX(float x)
{
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION_X, x));
	player1Properties->lastUserInput = Time::Now();
}

List<Entity*> Breakout::GetEntities()
{
	return players + balls + bricks + powerUpEntities + frame + scoreEntities;
}

/// Fetches all static entities.
List<Entity*> Breakout::StaticEntities()
{
	return bricks + frame;
}

void Breakout::SetupPhysics()
{
	if (!breakoutIntegrator)
	{
		breakoutIntegrator = new BreakoutIntegrator(0);
		breakoutIntegrator->constantZ = z;
	}
	if (Physics.physicsIntegrator != breakoutIntegrator)
		Physics.QueueMessage(new PMSet(breakoutIntegrator));

	if (!breakoutCR)
		breakoutCR = new BreakoutCR();
	if (Physics.collisionResolver != breakoutCR)
		Physics.QueueMessage(new PMSet(breakoutCR));
	if (!breakoutCD)
		breakoutCD = new BreakoutCD();
	if (Physics.collisionDetector != breakoutCD)
		Physics.QueueMessage(new PMSet(breakoutCD));
}


/// Sets up stuff specific for the entities in this little game.
void Breakout::SetupPhysics(List<Entity*> forEntities)
{
	/// Add all pong-entities to the Pong-specific collision filter.
#define BREAKOUT_PHYSICS_CATEGORY (1 << 2)
#define BREAKOUT_PLAYER_CATEGORY (1 << 3)
	int physicsFilter = BREAKOUT_PHYSICS_CATEGORY | BREAKOUT_PLAYER_CATEGORY;
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_COLLISION_CATEGORY, BREAKOUT_PHYSICS_CATEGORY));
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_COLLISION_FILTER, physicsFilter));
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_RESTITUTION, 1.0f)); // All bounce!
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_FRICTION, 0.f)); // 0 bounce!		
}
	

void Breakout::Process()
{
	/// Set up physics as needed.
	SetupPhysics();
	
	// 
	switch(gameState)
	{
		case SETTING_UP_PLAYFIELD:
		{
			SetupPlayingField();
			break;
		}
		case GAME_BEGUN:
		{
			break;
		}
	}
}


// Call to re-create the playing field as it started out.
void Breakout::SetupPlayingField()
{
	Physics.Pause();

	
	// Min and max of the playing field. Calculate them accordingly.
	Vector2f halfSize = gameSize * 0.5f;
	Vector3f min, max;
	min = - halfSize;
	max = halfSize;

	// o-o Store it for future use.
	top = gameSize[1] * 0.5f;
	bottom = -top;
	right = gameSize[0] * 0.5f;
	left = -right;

	// Set 
	breakoutIntegrator->frameMin = min;
	breakoutIntegrator->frameMax = max;
	
	// For setting up the playing field.
	bottomSpacing = 0.3f;
	topSpacing = 0.1f;
	sideSpace = 0.1f;
	totalYSpacing = bottomSpacing + topSpacing;

	// Set up general stuff.
	SetupPhysics(GetEntities());

//			AudioMan.PlayBGM("PongSong.ogg", 1.f);
	AudioMan.QueueMessage(new AMPlayBGM("Breakout/2014-07-22_Growth.ogg", 0.3f));
//	AudioMan.PlayBGM("Breakout/Breakout.ogg", 1.f);

	// Move stuff to edges of field.
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION, Vector3f(0, -gameSize[1] * (0.5f - bottomSpacing * 0.1f), z)));
	Physics.QueueMessage(new PMSetEntity(frame, PT_POSITION, Vector3f(0, 0, z)));

	// Score-boards..
	Physics.QueueMessage(new PMSetEntity(score1Entity, PT_POSITION, Vector3f(0, 0, z + 2)));
//			Physics.QueueMessage(new PMSetEntity(PT_POSITION, score2Entity, Vector3f(scoreColumns[1], 0, z->GetFloat() - 2)));
	Physics.QueueMessage(new PMSetEntity(scoreEntities, PT_COLLISIONS_ENABLED, false));

	// Set scale of score-board text relative to screen size.
	Graphics.QueueMessage(new GMSetEntityf(scoreEntities, GT_TEXT_SIZE_RATIO, gameSize[1] * 0.25f));
	Graphics.QueueMessage(new GMSetEntityVec4f(scoreEntities, GT_TEXT_COLOR, Vector4f(0.5,0.5f,0.5f,1)));

	// Make players kinematic.
	Physics.QueueMessage(new PMSetEntity(players, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));

	// Set scale of all to same?
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, paddleScale));

	player1Properties->initialScale = Vector3f(1, 1, 1) * paddleScale;
			
	// Scale the frame.
	float colWidth = gameSize[0] * 0.05f, 
		colHeight = gameSize[1], 
		rowWidth = gameSize[0], 
		rowHeight = gameSize[1] * 0.05f;
	Physics.QueueMessage(new PMSetEntity(topBar, PT_SET_SCALE, Vector2f(rowWidth, rowHeight)));
	Physics.QueueMessage(new PMSetEntity(base, PT_SET_SCALE, Vector2f(rowWidth, rowHeight)));
	Physics.QueueMessage(new PMSetEntity(leftBar, PT_SET_SCALE, Vector2f(colWidth, colHeight)));
	Physics.QueueMessage(new PMSetEntity(rightBar, PT_SET_SCALE, Vector2f(colWidth, colHeight)));

	// Place 'em.
	Physics.QueueMessage(new PMSetEntity(topBar, PT_POSITION, Vector2f(0,top)));
	Physics.QueueMessage(new PMSetEntity(leftBar, PT_POSITION, Vector2f(left,0)));
	Physics.QueueMessage(new PMSetEntity(rightBar, PT_POSITION, Vector2f(right, 0)));
	Physics.QueueMessage(new PMSetEntity(base, PT_POSITION, Vector2f(0,bottom)));

	// Set restitution and stuff for all.
	Physics.QueueMessage(new PMSetEntity(base, PT_RESTITUTION, 0.0f)); // 0 bounce!
			
	Physics.QueueMessage(new PMSetEntity(base, PT_FRICTION, 1.f)); // 0 bounce!

	// Set all to use mesh collissions.
	Physics.QueueMessage(new PMSetEntity(GetEntities(), PT_PHYSICS_SHAPE, PhysicsShape::MESH));
			
	// Disable gravity for the game entities.
	Physics.QueueMessage(new PMSetEntity(GetEntities(), PT_GRAVITY_MULTIPLIER, 0.f));

	// Disable collisions between the entities and the wall?
			
	// Reset score.
	player1Properties->score = 0;
	//player2Properties->score = 0;
	OnScoreUpdated();
	// o-o
	SpawnBricks(currentLevel);
	// Let the game begin!
	SpawnBalls();
	gameState = GAME_BEGUN;
	Physics.Resume();
}



/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
void Breakout::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::BREAKOUT_POWERUP:	
		{
			BreakoutPowerupMessage * bpm = (BreakoutPowerupMessage*) message;
			
			Powerup & power = bpm->power;
			switch(power.type)
			{
				case BreakoutPowerup::MORE_BALLS:
				{
					// Need to re-write the spawn-balls function probably..
					SpawnNewBall();
					break;
				}
				case BreakoutPowerup::FASTER_BALLS:
				case BreakoutPowerup::SLOWER_BALLS:
				case BreakoutPowerup::WIDER_PADDLE:
				{
					player1Properties->AddPowerup(power);
					break;
				}
				case 55:
				{
					break;
				}
			}
		}
		case MessageType::STRING:
		{
			// Ball reset message with x-coordinate. Sent when its velocity reaches 0 (goal!)
			if (msg.Contains("BreakoutBallGoal:"))
			{
				// Check if game over, and update score if not!
				OnScoreUpdated();
			}
			else if (msg.Contains("BrickBroken:"))
			{
				int score = msg.Tokenize(":")[1].ParseInt();
				player1Properties->score += score;
				OnScoreUpdated();
			}
			else if (msg.Contains("SpawnPowerup"))
			{
				String vector = msg.Tokenize(":")[1];
				List<String> vectorParts = vector.Tokenize(",");
				Vector3f position;
				position[0] = vectorParts[0].ParseFloat();
				position[1] = vectorParts[1].ParseFloat();
				position[2] = z + 2.f;

				Entity * newPowerup = MapMan.CreateEntity("Powerup", ModelMan.GetModel("Sprite"), TexMan.GetTexture("Green"));
				
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_COLLISION_CATEGORY, BREAKOUT_PHYSICS_CATEGORY_MAIN));
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_COLLISION_FILTER, BREAKOUT_PHYSICS_CATEGORY_PLAYER));

				// Set so the powerups don't actually push stuff either.
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_NO_COLLISSION_RESOLUTION, true));
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_VELOCITY, Vector3f(0,-1,0)));

				// Set Z of it.
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_POSITION, position));
				Physics.QueueMessage(new PMSetEntity(newPowerup, PT_SET_SCALE, 20.f));

				SetupPhysics(newPowerup);
				
				BreakoutPowerupProperty * bpp = new BreakoutPowerupProperty(newPowerup);
				newPowerup->properties.Add(bpp);

				powerUpEntities.Add(newPowerup);
			}
		}
	}
}

// Update text on both entities displaying the scores.
void Breakout::OnScoreUpdated()
{	
	// Check if game over, and update score if not!
	bool gameOver = true;
	for (int i = 0; i < balls.Size(); ++i)
	{
		Entity * ball = balls[i];
		BreakoutBallProperty * pbp = (BreakoutBallProperty*)ball->GetProperty("BreakoutBallProperty");
		if (!pbp->sleeping)
			gameOver = false;
	}
	if (gameOver)
	{
		// Deduct some score!
		player1Properties->score *= 0.8f;
		SpawnBalls();
		goto updateScore;
	}

	// Check if level completed.
	bool levelComplete = true;
	for (int i = 0; i < bricks.Size(); ++i)
	{
		Entity * brick = bricks[i];
		BreakoutBrickProperty * bbp = (BreakoutBrickProperty*) brick->GetProperty("BreakoutBrickProperty");
		if (bbp->sleeping)
			continue;
		levelComplete = false;
	}
	if (levelComplete)
	{
		Physics.Pause();
		currentLevel++;
		SpawnBricks(currentLevel);
		SpawnBalls();
		Physics.Resume();
	}
updateScore:
	// Update score-board.
	Graphics.QueueMessage(new GMSetEntitys(score1Entity, GT_TEXT, String::ToString(player1Properties->score)));

}


void Breakout::CreateUniformBrickMatrix(Vector2i withBrickColumnsAndRows)
{
	Model * cube = ModelMan.GetModel("SmoothedCube");
	
	// First level..
	Vector2i partitions = withBrickColumnsAndRows;
	Vector2f partitionsF = partitions - Vector2i(1,1);
	Vector2f blockArea = this->gameSize * 0.8f;
		
	blockArea = gameSize.ElementMultiplication(Vector2f(1.f - sideSpace * 2, 1.f - totalYSpacing)); 
	Vector2f gameCenter = Vector2f();
	Vector2f blockCenter = Vector2f(0, top - blockArea[1] * 0.5f - topSpacing * gameSize[1]);
		
	// Block size, relative to 
	Vector2f newBlockSize = blockArea.ElementDivision(partitions);
		
	// Startposition being the lower-left most block's position!
	Vector2f startPosition = blockCenter - (partitionsF * 0.5f).ElementMultiplication(newBlockSize);
	Texture * color = TexMan.GetTexture("White");
	for (int x = 0; x < partitions[0]; ++x)
	{
		for (int y = 0; y < partitions[1]; ++y)
		{
			Entity * newBrick = MapMan.CreateEntity("Brick", cube, color);
			// Setup common physics stuff for each brick.
			SetupPhysics(newBrick);
			newBrick->properties.Add(new BreakoutBrickProperty(newBrick));
			// Set it at some good position.
			Vector3f position = startPosition + Vector2f(newBlockSize[0] * x, newBlockSize[1] * y);
			position[2] = z;
			Physics.QueueMessage(new PMSetEntity(newBrick, PT_POSITION, position));
			Physics.QueueMessage(new PMSetEntity(newBrick, PT_SET_SCALE, newBlockSize));
			Physics.QueueMessage(new PMSetEntity(newBrick, PT_PHYSICS_SHAPE, PhysicsShape::MESH));
			bricks.Add(newBrick);
		}
	}
}

/// Spawns bricks for a level. Pattern will be decided later.
void Breakout::SpawnBricks(int level)
{
	// Delete all old bricks?
	MapMan.DeleteEntities(bricks);
	bricks.Clear();
	// Default level, lots of bricks.
	switch(level)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		default:
		{
			int size = level + 5;
			CreateUniformBrickMatrix(Vector2i(size, size));
			break;
		}
	};
}
	

void Breakout::SpawnBalls()
{
	Random pongBallRand;

//	AudioMan.PlaySFX("PongWin.ogg", 1.f);

	Texture * red = TexMan.GetTexture("Red");
	Model * sphere = ModelMan.GetModel("Sphere");
	
	// Pause/remove existing balls that are not to be used.
	for (int i = 0; i < balls.Size(); ++i)
	{
		Entity * ball = balls[i];//
		Physics.QueueMessage(new PMUnregisterEntity(ball));
		Graphics.QueueMessage(new GMUnregisterEntity(ball));
		// Set them as sleeping too.
		BreakoutBallProperty * bbp = (BreakoutBallProperty*) ball->GetProperty("BreakoutBallProperty");
		bbp->sleeping = true;
	}

	for (int i = 0; i < numBalls; ++i)
	{
		Entity * ball = 0;
		BreakoutBallProperty * bbp = 0;

		if (balls.Size() > i)
			ball = balls[i];
		if (!ball)
		{
			ball = MapMan.CreateEntity("Ball", sphere, red);
			bbp = new BreakoutBallProperty(ball, 50.f);
			ball->properties.Add(bbp);
			
			// Setup collision filter.
			SetupPhysics(ball);
			
			balls.Add(ball);
			ballProperties.Add(bbp);
		}
		else 
		{
			// Register it for rendering.
			Graphics.QueueMessage(new GMRegisterEntity(ball));
		}
		assert(ball);
		if (!bbp)
			bbp = ball->GetProperty<BreakoutBallProperty>();

		// Set up general stuff
		SetupPhysics(ball);

		Physics.QueueMessage(new PMSetEntity(ball, PT_SET_SCALE, ballScale));
		Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_SHAPE, PhysicsShape::SPHERE));
	
		// Set balls to be dynamic.
		Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));		

		
		bbp->defaultMinVerticalVel = bbp->minimumVerticalVelocity = ballMinVerticalSpeed;
		bbp->defaultMinVerticalVel = ballMinVerticalSpeed;
		bbp->defaultMinVerticalVel *= pow(0.85f, i);
		bbp->minimumVerticalVelocity = bbp->defaultMinVerticalVel;
		bbp->velocityIncreasePerBounce = ballSpeedIncreasePerBounce;
		
		// Reset its position.
		// Center it.
		Vector3f startPos;
		startPos[1] = -(0.5f - bottomSpacing * 0.9f) * gameSize[1];
		Physics.QueueMessage(new PMSetEntity(ball, PT_POSITION, startPos));

		// Start the balllllllllllllllllllllll!
		Vector2f initialVelocity(0, -1);
		// Move it slightly up or downnnn.
		float leftNRight = pongBallRand.Randf() - 0.5f;
		initialVelocity[0]  = leftNRight;
		initialVelocity.Normalize();
		initialVelocity *= bbp->defaultMinVerticalVel;
		Physics.QueueMessage(new PMSetEntity(ball, PT_VELOCITY, initialVelocity));

		// Reset stuff!
		bbp->OnSpawn();
	}
}

/// Triggered by the power-up.
void Breakout::SpawnNewBall()
{
	Random pongBallRand;

//	AudioMan.PlaySFX("PongWin.ogg", 1.f);

	Texture * red = TexMan.GetTexture("Red");
	Model * sphere = ModelMan.GetModel("Sphere");
	

	Entity * ball = 0;
	BreakoutBallProperty * bbp = 0;

	// See if we can re-use one of the sleeping/dead balls.
	for (int i = 0; i < balls.Size(); ++i)
	{
		Entity * oldBall = balls[i];
		bbp = (BreakoutBallProperty*) oldBall->GetProperty("BreakoutBallProperty");
		assert(bbp);
		if (bbp && bbp->sleeping)
		{
			ball = oldBall;
		}
	}
	// Nope? Then create a new one.		
	if (!ball)
	{
		ball = MapMan.CreateEntity("Ball", sphere, red);
		bbp = new BreakoutBallProperty(ball, 50.f);
		ball->properties.Add(bbp);
			
		// Setup collision filter.
		SetupPhysics(ball);
			
		balls.Add(ball);
		ballProperties.Add(bbp);
	}
	else 
	{
		// Register it for rendering.
		Graphics.QueueMessage(new GMRegisterEntity(ball));
	}
	assert(ball);
	if (!bbp)
		bbp = (BreakoutBallProperty*)ball->GetProperty("BreakoutBallProperty");

	// Set up general stuff
	SetupPhysics(ball);

	Physics.QueueMessage(new PMSetEntity(ball, PT_SET_SCALE, ballScale));
	Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_SHAPE, PhysicsShape::SPHERE));
	
	// Set balls to be dynamic.
	Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));		

		
	bbp->minimumVerticalVelocity = bbp->defaultMinVerticalVel = ballMinVerticalSpeed;
		
	bbp->velocityIncreasePerBounce = ballSpeedIncreasePerBounce;
	assert(bbp);

	// Reset its position.
	// Center it.
	Vector3f startPos;
	startPos[1] = -(0.5f - bottomSpacing * 0.5f) * gameSize[1];
	// Start it x-wise same as the player.
	startPos[0] = player1->position[0];
	Physics.QueueMessage(new PMSetEntity(ball, PT_POSITION, startPos));

	// Start the balllllllllllllllllllllll!
	Vector2f initialVelocity(0, 1);
	// Move it slightly up or downnnn.
	float leftNRight = pongBallRand.Randf() - 0.5f;
	initialVelocity[0]  = leftNRight;
	initialVelocity.Normalize();
	initialVelocity *= bbp->defaultMinVerticalVel;
	Physics.QueueMessage(new PMSetEntity(ball, PT_VELOCITY, initialVelocity));

	// Ensure it becomes visible too, yo.
	Graphics.QueueMessage(new GMRegisterEntity(ball));
	Physics.QueueMessage(new PMRegisterEntity(ball));

	// Reset stuff!
	bbp->OnSpawn();
}
	


void Breakout::UpdateBallProperties()
{
	for (int i = 0; i < ballProperties.Size(); ++i)
	{
		BreakoutBallProperty * pbp = ballProperties[i];
		pbp->defaultMinVerticalVel = pbp->minimumVerticalVelocity = ballMinVerticalSpeed;
		pbp->velocityIncreasePerBounce = ballSpeedIncreasePerBounce;
	}
}
