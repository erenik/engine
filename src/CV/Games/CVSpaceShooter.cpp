/// Emil Hedemalm
/// 2014-07-25
/// A space-shooter game based on the input from computer vision imaging

#include "CVSpaceShooter.h"
#include "CV/CVPipeline.h"

#include "Game/SpaceShooter/SpaceShooter.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Message/Message.h"

#include "Physics/PhysicsManager.h"

CVSpaceShooter::CVSpaceShooter()
	: CVGame(CVFilterID::SPACE_SHOOTER)
{
	aiSpeed = new CVFilterSetting("AI speed", 200.f);
	lives = new CVFilterSetting("Lives", 3);
	level = new CVFilterSetting("Level", 0);
	shipScale = new CVFilterSetting("Ship scale", 20.f);

	yOnly = new CVFilterSetting("Control Y only", true);
	flipX = new CVFilterSetting("Flip X", false);

	// Add all settings so they get added to the UI.
	settings.Add(6, 
		aiSpeed, lives, level, 
		shipScale, yOnly, flipX);

	game = spaceShooter = new SpaceShooter();
}

CVSpaceShooter::~CVSpaceShooter()
{
	/// Actual game hould have been deleted in CVGame OnDelete
}


int CVSpaceShooter::Process(CVPipeline * pipe)
{

	/// This game literally spams messages, so sleep a bit. <- No. Messages were just being processed too slowly elsewhere! o.o
//	SleepThread(50);
	if (aiSpeed->HasChanged())
	{
	
	}
	if (level->HasChanged())
	{
		spaceShooter->SetLevel(level->GetInt());
	}
	if (shipScale->HasChanged())
	{
		spaceShooter->SetShipScale(shipScale->GetFloat());
	}
	if (flipX->HasChanged())
	{
		if (flipX->GetBool())
			spaceShooter->SetFlipX(-1.f);
		else 
			spaceShooter->SetFlipX(1.f);
	}

	CVGame::Process(pipe);

	// Track the mouse instead.
	if (yOnly->HasChanged())
		spaceShooter->yOnly = yOnly->GetBool();

	/// Just a split depending on input we want to use.
	if (game->useMouseInput)
	{

	}
	// Track the best hand.
	else 
	{
		// Find best hand.
		CVHand * bestHand = 0;
		float bestHandRating = 0.0f;
		float handRating = 0;
		for (int i = 0; i < pipe->hands.Size(); ++i)
		{
			CVHand & hand = pipe->hands[i];
			handRating = 0;

			Vector3f pictureCenter = pipe->initialInputSize;
			pictureCenter *= 0.5f;
			/*
			if (hand.center.x > pictureCenter.x &&  hand.center.y > pictureCenter.y)
				// Bad hand!
				handRating -= 0.5f;
			else
				handRating += 0.5f;
			*/
			handRating = 1.f;

			// If has many fingers, is good.
			handRating += hand.fingers.Size() * 0.2f;
			if (handRating > bestHandRating)
			{
				bestHandRating = handRating;
				bestHand = &hand;
			}
		}
		// o-o
		if (bestHand)
		{
			CVHand & hand = *bestHand;
			Vector3f position;
			if (hand.fingers.Size() == 0)
				position = hand.center;
			else 
				position = hand.fingers[0].point;

		//	position = hand.center;

			static Vector3f averagedPosition;
			averagedPosition = averagedPosition * 0.8f + position * 0.2f;

			Vector3f fingerPositionWorldSpace = pipe->InputSpaceCoordToOutputSpaceCoord(averagedPosition);
			/*
			position.x -= pipe->initialInput->cols * 0.5f;
			position.y = pipe->initialInput->rows * 0.5 - position.y;
			*/
			spaceShooter->SetPlayerPosition(fingerPositionWorldSpace);
		}
	}

	// Process the game? Maybe these objects should be processed from some other loop? 
	spaceShooter->Process();

	// ..
//	pipe->previousFilter->Paint(pipe);

	return CVReturnType::CV_IMAGE;
}

