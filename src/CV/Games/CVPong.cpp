/// Emil Hedemalm
/// 2014-07-16
/// The classic Pong integrated as a filter into the IPM CVPipeline.

#include "CVPong.h"
#include "CV/CVPipeline.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Message/Message.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Game/Pong/Pong.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"

CVPong::CVPong() 
	: CVGame(CVFilterID::PONG)
{

	ballScale = new CVFilterSetting("Ball scale", 5.f);
	paddleScale = new CVFilterSetting("Paddle scale", 5.f);
	distanceFromCenter = new CVFilterSetting("Player distance from center", 0.8f);
	initialBallSpeed = new CVFilterSetting("Initial ball speed", 50.f);
	aiSpeed = new CVFilterSetting("AI speed", 200.f);
	numBalls = new CVFilterSetting("Balls", 1);
	ballSpeedIncreasePerCollision = new CVFilterSetting("Ball speed increase", 2.f);

	// Add all settings so they get added to the UI.
	settings.Add(7, 
		ballScale, paddleScale, distanceFromCenter, 
		initialBallSpeed, aiSpeed, numBalls, 
		ballSpeedIncreasePerCollision);

	game = pong = new Pong();
}

CVPong::~CVPong()
{
	/// Actual game hould have been deleted in CVGame OnDelete
	// Reset lighting?
	Lighting lighting;
	lighting.SetAmbient(Vector3f(1,1,1));
	Graphics.QueueMessage(new GMSetLighting(lighting));
}

int CVPong::Process(CVPipeline * pipe)
{
	/// Set up physics as needed.
	pong->SetupPhysics();

	CVGame::Process(pipe);


	static Vector2i lastFrameSize;
	Vector2i frameSize = pipe->initialInputSize;
	if (frameSize != lastFrameSize)
	{
		pong->SetFrameSize(frameSize);
		lastFrameSize = frameSize;
	}
	if (numBalls->HasChanged())
		pong->SetNumBalls(numBalls->GetInt());
	if (initialBallSpeed->HasChanged())
	{
		pong->SetInitialBallSpeed(initialBallSpeed->GetFloat());
	}
	if (aiSpeed->HasChanged())
	{
		pong->SetAISpeed(aiSpeed->GetFloat());
	}
	else if (ballSpeedIncreasePerCollision->HasChanged())
	{
		// UpdateBallProperties();
	}
	if (paddleScale->HasChanged())
		pong->SetPaddleScale(paddleScale->GetFloat());

	

	// Min and max of the playing field. Calculate them accordingly.
	Vector2f halfSize = frameSize * 0.5f;
	Vector3f min, max;
	min = - halfSize;
	max = halfSize;

	// Go through the hands.
	for (int i = 0; i < pipe->hands.Size(); ++i)
	{
		CVHand & hand = pipe->hands[i];
		Vector3f handCenter = hand.center;
		Vector3f worldPos = pipe->InputSpaceToWorldSpace(handCenter);
//			handCenter.x -= pipe->initialInput->cols * 0.5f;
//		handCenter.y = pipe->initialInput->rows * 0.5 - handCenter.y;
		// Is left!
		if (worldPos.x < min.x * 0.2f)
		{
			pong->SetPlayer1PositionY(worldPos.y);
		}
		else if (worldPos.x > max.x * 0.2f)
		{
			pong->SetPlayer2PositionY(worldPos.y);
		}
	}
	return CVReturnType::RENDER;
}
