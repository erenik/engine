/// Emil Hedemalm
/// 2014-07-16
/// The classic Pong integrated as a filter into the IPM CVPipeline.

#include "CVBreakout.h"
#include "CV/CVPipeline.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Message/Message.h"

#include "Game/Breakout/Breakout.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"
				

void TestEntities(List<Entity*> entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		std::cout<<"\nEntity "<<entities[i]->name;
	}
}

CVBreakout::CVBreakout() 
	: CVGame(CVFilterID::BREAKOUT)
{
	ballScale = new CVFilterSetting("Ball scale", 5.f);
	paddleScale = new CVFilterSetting("Paddle scale", 5.f);
	distanceFromCenter = new CVFilterSetting("Player distance from center", 0.8f);
	initialBallSpeed = new CVFilterSetting("Initial ball speed", 50.f);
	aiSpeed = new CVFilterSetting("AI speed", 200.f);
	numBalls = new CVFilterSetting("Balls", 1);
	ballSpeedIncreasePerCollision = new CVFilterSetting("Ball speed increase", 1.f);
	lives = new CVFilterSetting("Lives", 3);
	level = new CVFilterSetting("Level", 0);

	// Add all settings so they get added to the UI.
	settings.Add(9, 
		ballScale, paddleScale, distanceFromCenter, 
		initialBallSpeed, aiSpeed, numBalls, 
		ballSpeedIncreasePerCollision, lives, level);

	game = breakout = new Breakout();
}

CVBreakout::~CVBreakout()
{
	/// Actual game hould have been deleted in CVGame OnDelete
}

int CVBreakout::Process(CVPipeline * pipe)
{
	// Check changes via CV settings, forward them to the game.
	CVGame::Process(pipe);

	if (initialBallSpeed->HasChanged())
	{
		breakout->SetBallSpeed(initialBallSpeed->GetFloat());
	}
	if (aiSpeed->HasChanged())
	{
		breakout->SetAISpeed(aiSpeed->GetFloat());
	}
	if (numBalls->HasChanged())
	{
		breakout->SetNumBalls(numBalls->GetInt());
	}
	if (ballSpeedIncreasePerCollision->HasChanged())
	{
		breakout->SetSpeedIncreasePerBounce(ballSpeedIncreasePerCollision->GetFloat());
	}
	if (level->HasChanged())
	{
		breakout->SetLevel(level->GetInt());
	}
	if (paddleScale->HasChanged())
		breakout->SetPaddleScale(paddleScale->GetFloat());
	if (ballScale->HasChanged())
		breakout->SetBallScale(ballScale->GetFloat());

	return CVReturnType::RENDER;
}
