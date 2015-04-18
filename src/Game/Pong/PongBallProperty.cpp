/// Emil Hedemalm
/// 2014-07-17
/// Property for the ball and other entities in a simple Pong-game.

#include "PongBallProperty.h"
#include "PongPlayerProperty.h"
#include "Pong.h"

#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"

#include "Time/Time.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Audio/AudioManager.h"

#include "Graphics/Particles/ParticleEmitter.h"

#include "OS/Sleep.h"

PongBallProperty::PongBallProperty(Pong * game, Entity * owner, float defaultMinHorizontalVel)
	: EntityProperty("PongBallProperty", ID(), owner), defaultMinHorizontalVel(defaultMinHorizontalVel), game(game)
{
	minimumHorizontalVelocity = 1.f;
	velocityIncreasePerBounce = 0.f;
	maxYVel = 1000.f;
	lastSFX = Time::Now();
	pe = new ParticleEmitter(EmitterType::POINT_DIRECTIONAL);
	pe->SetColor(Vector4f(0.01f, 0.01f, 0.01f, 1.f));
	pe->SetScale(5.f);
	pe->SetParticleLifeTime(0.5f);
	pe->SetEmissionVelocity(0.f);
	pe->entityToTrack = owner;
	sleeping = false;
};

PongBallProperty::~PongBallProperty()
{
	Graphics.QueueMessage(new GMDetachParticleEmitter(pe));
	SleepThread(10);
	delete pe;
}

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int PongBallProperty::ID()
{
	return EntityPropertyID::MINI_GAME_1 + 1;
}

/// Time passed in seconds..!
void PongBallProperty::Process(int timeInMs)
{
	if (sleeping)
	{
		if (owner->physics && owner->physics->velocity.Length() > 0)
		{
			std::cout<<"\nSleeping yet moving...?";
		}
		return;
	}
	if (!owner->physics)
		return;
	// Ball still a long time? Re-spawn it! o-o
	if (owner->physics->velocity.MaxPart() == 0)
	{
		if (!sleeping)
		{
			if (ballStoppedStartTime.intervals == 0)
				ballStoppedStartTime = Time::Now();
			Time currentTime = Time::Now();
			if ((currentTime - ballStoppedStartTime).Seconds() > 1)
			{
				// Reset it.
			//	OnSpawn();
				// Give points to players too.
//				MesMan.QueueMessages("PongBallGoal:"+String::ToString(owner->position[0]));
				sleeping = true;
				game->OnGoal(owner->position);
			}
		}
	}
	// Moving ball o.o
	else {
		Graphics.QueueMessage(new GMSetParticleEmitter(pe, GT_EMITTER_POSITION, owner->position));
	}
}


/// Resets position to default.
void PongBallProperty::OnSpawn()
{
	std::cout<<"\nOnSpawn called";

	// Attach emitter to self and the particle system.
	Graphics.QueueMessage(new GMAttachParticleEmitter(pe, (ParticleSystem*)game->particleSystem));
	Graphics.QueueMessage(new GMSetParticleEmitter(pe, GT_EMITTER_POSITION, owner->position));

	// Set ball propertiiiies!
	minimumHorizontalVelocity = defaultMinHorizontalVel;
	sleeping = false;
}



/// If reacting to collisions...
void PongBallProperty::OnCollision(Collision & data)
{
	if (sleeping)
		return;
	Entity * other = data.one == owner? data.two : data.one;

	// Play SFX!
	PongPlayerProperty * playerProp = other->GetProperty<PongPlayerProperty>();
	if (playerProp)
	{
		if (data.collissionPoint[0] < 0)
			PlaySFX("Pong.ogg");
		else
			PlaySFX("Pong2.ogg");
	}

	// Goal if ball and goal!
	if (other->name.Contains("Goal"))
	{	
		// Sleep! So we don't get double messages!
		this->Sleep();
		// SFX!
		PlaySFX("PongGoal.ogg");
		// Goal!
		MesMan.QueueMessage(new VectorMessage(GOAL_MESSAGE, owner->position));
	}
	else if (other->name.Contains("Wall"))
	{
		PlaySFX("PongWall.ogg");
	}
}

/// Makes this ball sleep, unregistering it from physics/graphics?
void PongBallProperty::Sleep()
{
	if (sleeping)
		return;
	ballStoppedStartTime = Time();
	sleeping = true;	
	GraphicsMan.QueueMessage(new GMUnregisterEntity(owner));
	PhysicsMan.QueueMessage(new PMUnregisterEntity(owner));
	if (pe && game->particleSystem)
		Graphics.QueueMessage(new GMDetachParticleEmitter(pe, (ParticleSystem*)game->particleSystem));
}

/// Plays target SFX, but only if a sufficient amount of time has elapsed since the last sfx, so that the colission system, if failing, doesn't spam the system.
void PongBallProperty::PlaySFX(String source, float volume /* = 1.f */)
{
	Time now = Time::Now();
	if ((now - lastSFX).Milliseconds() < 100)
		return;
	lastSFX = now;
	AudioMan.QueueMessage(new AMPlaySFX("Pong/"+source, volume));

}
