/// Emil Hedemalm
/// 2014-07-25
/// A space-shooter game based on the input from computer vision imaging

#include "Pong.h"

#include "PongIntegrator.h"
#include "PongCR.h"
#include "PongCD.h"

#include "Graphics/Particles/Sparks.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMParticles.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"

#include "Message/Message.h"
#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "PongPlayerProperty.h"
#include "PongBallProperty.h"

#include "OS/Sleep.h"

Pong::Pong()
	: Game2D("SpaceShooter")
{
	player1 = NULL;
	score1Entity = NULL;
	player1Properties = 0;
	
	numBalls = 1;
	/// o-o
	gameState = SETTING_UP_PLAYFIELD;

	aiSpeed = 1.f;
	initialBallSpeed = 1.f;
	paddleScale = 1.f;
	ballScale = 5;
	ballSpeedIncreasePerCollision = 1.f;

	distanceFromCenter = 0.8f;

	paddleScale = 5.f;

	useMouseInput = false;

	pongIntegrator = 0;
	pongCR = 0;
	pongCD = 0;

	particleSystem = NULL;
}

Pong::~Pong()
{
	// Reset integrator and stuff before we delete any entities, or wat?
	Physics.QueueMessage(new PMSeti(PT_PHYSICS_INTEGRATOR, 0));
	Physics.QueueMessage(new PMSeti(PT_COLLISION_DETECTOR, 0));
	Physics.QueueMessage(new PMSeti(PT_COLLISION_RESOLVER, 0));
	SleepThread(50);
	ballProperties.Clear();
	MapMan.DeleteEntities(GetEntities());
	// Unregister particle system!
	if (GraphicsManager::GraphicsProcessingActive())
	{
		Graphics.QueueMessage(new GMUnregisterParticleSystem((ParticleSystem*)particleSystem, true));
	}
	else if (particleSystem)
		delete particleSystem;
	particleSystem = NULL;
}

	/// Allocates and sets things up.
void Pong::Initialize()
{

	pongRand.Init(Time());

	Texture * white = TexMan.GetTexture("White"),
		* red = TexMan.GetTexture("Red"),
		* alpha = TexMan.GetTexture("Alpha"),
		* blue = TexMan.GetTexture("Blue"),
		* green = TexMan.GetTexture("Green"),
		* gray = TexMan.GetTexture("Grey"),
		* horizontalBarColor = TexMan.GetTexture("0x88"),
		* verticalBarColor = TexMan.GetTexture("0xAA");
	Model * sphere = ModelMan.GetModel("Sphere");
	Model * pad = ModelMan.GetModel("Pong/Paddle");
	
	player1 = MapMan.CreateEntity("Player1", pad, white);
	player1Properties = new PongPlayerProperty(player1, Vector2f(1, 0), 15.f);
	player1->properties.Add(player1Properties);
	player2 = MapMan.CreateEntity("Player2", pad, white);
	player2Properties = new PongPlayerProperty(player2, Vector2f(-1, 0), 15.f);
	player2->properties.Add(player2Properties);
	players.Add(player1, player2);

	score1Entity = MapMan.CreateEntity("Player1Score", NULL, alpha);
	score2Entity = MapMan.CreateEntity("Player2Score", NULL, alpha);
	scoreEntities.Add(score1Entity, score2Entity);
		


	horizontalBars = MapMan.CreateEntity("Walls", ModelMan.GetModel("Pong/HorizontalBars"), horizontalBarColor);
	goals = MapMan.CreateEntity("Goals", ModelMan.GetModel("Pong/VerticalBars"), verticalBarColor);
	frame.Add(horizontalBars, goals);

	/// Add all entities.
	entities.Add(player1, player2);
	entities.Add(horizontalBars, goals, score1Entity, score2Entity);

	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		std::cout<<"\nEntity"<<entity;
	}

	particleSystem = new Sparks(true);

	gameState = SETTING_UP_PLAYFIELD;
}


void Pong::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			OnGoal(vm->GetVector3f());		
			break;
		}
		case MessageType::STRING:
		{
			// Ball reset message with x-coordinate. Sent when its velocity reaches 0 (goal!)
			if (msg.Contains(GOAL_MESSAGE))
			{
				String arg = msg.Tokenize(":")[1];
			
			}
		}
	}
}

Lighting pongLighting;

/// Call on a per-frame basis.
void Pong::Process()
{
	// 
	switch(gameState)
	{
		case SETTING_UP_PLAYFIELD:
		{
			// Register particles system if not already done so.
			Graphics.QueueMessage(new GMRegisterParticleSystem(particleSystem, true));

			// Load lighting
			pongLighting.LoadFrom("lighting/Pong");
			Graphics.QueueMessage(new GMSetLighting(&pongLighting));


			RecalculatePlayfieldLines();

			// Set up general stuff.
			SetupPhysics(entities);

			// Move stuff to edges of field.
			Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION, Vector3f(playerLines[0], 0, z)));
			Physics.QueueMessage(new PMSetEntity(player2, PT_POSITION, Vector3f(playerLines[1], 0, z)));
			Physics.QueueMessage(new PMSetEntity(frame, PT_POSITION, Vector3f(0, 0, z)));

			// Score-boards..
			Physics.QueueMessage(new PMSetEntity(score1Entity, PT_POSITION, Vector3f(scoreColumns[0], 0, z - 2)));
			Physics.QueueMessage(new PMSetEntity(score2Entity, PT_POSITION, Vector3f(scoreColumns[1], 0, z - 2)));
			Physics.QueueMessage(new PMSetEntity(scoreEntities, PT_COLLISIONS_ENABLED, false));

			// Set scale of score-board text relative to screen size.
			Graphics.QueueMessage(new GMSetEntityf(scoreEntities, GT_TEXT_SIZE_RATIO, gameSize[1] * 0.25f));
			Graphics.QueueMessage(new GMSetEntityVec4f(scoreEntities, GT_TEXT_COLOR, Vector4f(0.5,0.5f,0.5f,1)));

			// Make players kinematic.
			Physics.QueueMessage(new PMSetEntity(players, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));

			// Set scale of all to same?
			Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, paddleScale));
			
			// Scale the frame.
			Physics.QueueMessage(new PMSetEntity(frame, PT_SET_SCALE, Vector2f(gameSize)));

			// Set restitution and stuff for all.
	//		Physics.QueueMessage(new PMSetEntity(PT_RESTITUTION, players, 1.05f)); // Increase bounce as we go on? :3
			Physics.QueueMessage(new PMSetEntity(goals, PT_RESTITUTION, 0.0f)); // 0 bounce!
			
			Physics.QueueMessage(new PMSetEntity(goals, PT_FRICTION, 1.f)); // 0 bounce!

			// Set all to use mesh collissions.
			Physics.QueueMessage(new PMSetEntity(entities, PT_PHYSICS_SHAPE, PhysicsShape::MESH));
			
			// Disable gravity for the game entities.
			Physics.QueueMessage(new PMSetEntity(entities, PT_GRAVITY_MULTIPLIER, 0.f));

			// Disable collisions between the entities and the wall?
			
			// Reset score.
			player1Properties->score = player2Properties->score = 0;
			OnScoreUpdated();

			gameState = GAME_BEGUN;

			// Let the game begin!
			SpawnBalls();

			// Start ze music!
	//		AudioMan.PlayBGM("PongSong.ogg", .5f);

			break;
		}
		case GAME_BEGUN:
		{
			break;
		}
	}
}

/// Fetches all entities concerning this game.
List<Entity*> Pong::GetEntities()
{
	return players + scoreEntities + frame + balls;
}

/// Fetches all static entities.
List<Entity*> Pong::StaticEntities()
{
	return scoreEntities + frame;
}


void Pong::SetupPhysics()
{
	if (!pongIntegrator)
	{
		pongIntegrator = new PongIntegrator(z);
		Physics.QueueMessage(new PMSet(pongIntegrator));
	}
	if (!pongCR)
	{
		pongCR = new PongCR();
		Physics.QueueMessage(new PMSet(pongCR));
	}
	if (!pongCD)
	{
		pongCD = new PongCD();
		Physics.QueueMessage(new PMSet(pongCD));
	}
}


/// Sets up stuff specific for the entities in this little game.
void Pong::SetupPhysics(List<Entity*> forEntities)
{
	/// Add all pong-entities to the Pong-specific collision filter.
#define PONG_PHYSICS_CATEGORY 0x0000002

	Physics.QueueMessage(new PMSetEntity(forEntities, PT_COLLISION_CATEGORY, PONG_PHYSICS_CATEGORY));
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_COLLISION_FILTER, PONG_PHYSICS_CATEGORY));
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_RESTITUTION, 1.0f)); // All bounce!
	Physics.QueueMessage(new PMSetEntity(forEntities, PT_FRICTION, 0.f)); // 0 bounce!		
}
	



void Pong::SpawnBalls()
{
	AudioMan.QueueMessage(new AMPlaySFX("PongWin.ogg"));
//	AudioMan.PlaySFX("PongWin.ogg", 1.f);

	Random pongBallRand;

	Texture * red = TexMan.GetTextureByHex24(0xFF5522);
	Texture * white = TexMan.GetTexture("White");
	Model * sphere = ModelMan.GetModel("Sphere");
	
	// Pause/remove existing balls that are not to be used.
	for (int i = 0; i < balls.Size(); ++i)
	{
		Entity * ball = balls[i];//
		PongBallProperty * pbp = ball->GetProperty<PongBallProperty>();
		pbp->Sleep();
	}

	for (int i = 0; i < numBalls; ++i)
	{
		Entity * ball = 0;
		PongBallProperty * pbp = 0;

		if (balls.Size() > i)
			ball = balls[i];
		if (!ball)
		{
			ball = MapMan.CreateEntity("Ball", sphere, white);
			Graphics.QueueMessage(new GMSetEntityTexture(ball, SPECULAR_MAP, white));
			pbp = new PongBallProperty(this, ball, 50.f);
			ball->properties.Add(pbp);
			balls.Add(ball);
			ballProperties.Add(pbp);
		}
		else 
		{
			// Register it for rendering.
			Graphics.QueueMessage(new GMRegisterEntity(ball));
		}
		assert(ball);
		if (!pbp)
			pbp = (PongBallProperty*)ball->GetProperty("PongBallProperty");

		// Set up general stuff
		SetupPhysics(ball);

		Physics.QueueMessage(new PMSetEntity(ball, PT_SET_SCALE, ballScale));
		Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_SHAPE, PhysicsShape::SPHERE));
	
		// Set balls to be dynamic.
		Physics.QueueMessage(new PMSetEntity(ball, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));		

		
		pbp->defaultMinHorizontalVel = initialBallSpeed;
		std::cout<<"\nResetting default/starting min velocity: "<<pbp->defaultMinHorizontalVel;
		pbp->defaultMinHorizontalVel *= pow(0.85f, i);
		pbp->minimumHorizontalVelocity = pbp->defaultMinHorizontalVel;
		
		pbp->velocityIncreasePerBounce = ballSpeedIncreasePerCollision;
		assert(pbp);

		// Reset its position.
		// Center it.
		Physics.QueueMessage(new PMSetEntity(ball, PT_POSITION, Vector3f(0, 0, 0)));

		// Start the balllllllllllllllllllllll!
		int r = rand()%2;
		Vector2f initialVelocity(r * 2 - 1, 0);
		// Move it slightly up or downnnn.
		float upNDown = pongBallRand.Randf() - 0.5f;
		initialVelocity[1]  = upNDown;
		initialVelocity.Normalize();
		initialVelocity *= pbp->minimumHorizontalVelocity;
		std::cout<<"\nInitial velocity: "<<initialVelocity[0];
		Physics.QueueMessage(new PMSetEntity(ball, PT_VELOCITY, initialVelocity));

		// Reset stuff!
		pbp->OnSpawn();
	}
}


// Update text on both entities displaying the scores.
void Pong::OnScoreUpdated()
{
	Graphics.QueueMessage(new GMSetEntitys(score1Entity, GT_TEXT, String::ToString(player1Properties->score)));
	Graphics.QueueMessage(new GMSetEntitys(score2Entity, GT_TEXT, String::ToString(player2Properties->score)));
	

	// Check if game over, and update score if not!
	bool gameOver = true;
	for (int i = 0; i < balls.Size() && i < numBalls; ++i)
	{
		Entity * ball = balls[i];
		PongBallProperty * pbp = (PongBallProperty*)ball->GetProperty("PongBallProperty");
		if (!pbp->sleeping)
			gameOver = false;
	}
	if (gameOver)
	{
		SpawnBalls();
	}
}



void Pong::UpdateBallProperties()
{
	for (int i = 0; i < ballProperties.Size(); ++i)
	{
		PongBallProperty * pbp = ballProperties[i];
		pbp->defaultMinHorizontalVel = pbp->minimumHorizontalVelocity = initialBallSpeed;
		pbp->velocityIncreasePerBounce = ballSpeedIncreasePerCollision;
	}
}

/// o.o Called from ball maybe.
void Pong::OnGoal(ConstVec3fr atPosition)
{
	int posX = atPosition[0];
	if (posX < 0)
	{
		++player2Properties->score;
	}
	else 
		++player1Properties->score;
				
	// Check if game over, and update score if not!
	OnScoreUpdated();
}

// Sets new amount of balls and resets the game.
void Pong::SetNumBalls(int num)
{
	numBalls = num;
	SpawnBalls();
}

/*
void Pong::SetZ(float newZ)
{
	this->z = newZ;
	pongIntegrator->constantZ = z;
	Physics.QueueMessage(new PMSetEntity(frame, PT_POSITION, Vector3f(0, 0, z)));
}*/

void Pong::SetInitialBallSpeed(float f)
{
	this->initialBallSpeed = f;
	UpdateBallProperties();
}


void Pong::SetAISpeed(float sp)
{
	aiSpeed = sp;
	for (int i = 0; i < players.Size(); ++i)
	{
		Entity * player = players[i];
		PongPlayerProperty * ppp = (PongPlayerProperty *) player->GetProperty("PongPlayerProperty");
		if (!ppp)
			continue;
		ppp->aiSpeed = aiSpeed;
	}
}

void Pong::SetPaddleScale(float f)
{
	paddleScale = f;
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, paddleScale));
}

void Pong::SetZ(float newZ)
{
	this->z = newZ;
	pongIntegrator->constantZ = z;
	gameState = SETTING_UP_PLAYFIELD;
}


void Pong::SetPlayer1PositionY(float y)
{
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION_Y, y));
	player1Properties->lastUserInput = Time::Now();
}

void Pong::SetPlayer2PositionY(float y)
{
	Physics.QueueMessage(new PMSetEntity(player2, PT_POSITION_Y, y));
	player2Properties->lastUserInput = Time::Now();
}


void Pong::Reset()
{
	gameState = SETTING_UP_PLAYFIELD;
}


// Including player lines and score columns, based on frameSize
void Pong::RecalculatePlayfieldLines()
{
	// Min and max of the playing field. Calculate them accordingly.
	Vector2f halfSize = gameSize * 0.5f;
	Vector3f min, max;
	min = - halfSize;
	max = halfSize;

	// Set 
	pongIntegrator->frameMin = min;
	pongIntegrator->frameMax = max;


	// X-values, left and right.
	playerLines = Vector2f(min[0], max[0]);
	playerLines *= distanceFromCenter;

	// Update player positions?
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION_X, playerLines[0]));
	Physics.QueueMessage(new PMSetEntity(player2, PT_POSITION_X, playerLines[1]));

	// Middle!
	scoreColumns = Vector2f(min[0], max[0]);
	scoreColumns *= 0.5f;

}
