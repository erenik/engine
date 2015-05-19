/// Emil Hedemalm
/// 2015-04-20
/// Player property.

#include "SideScroller.h"
#include "Luchador.h"

LuchadorProperty::LuchadorProperty(Entity * owner)
	: EntityProperty("LuchaProp", EP_LUCHA, owner)
{
	sleeping = false;
	lastJump = Time::Now();
	autoRun = false;
	timeSinceLastTaco = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
	timeSinceLastTaco.AddMs(1000 * 60); // 1 minute since last taco. Is OK.
	speedBonusDueToTacos = 1.0f;
}

void LuchadorProperty::OnCollision(Collision & data)
{
	Entity * other = NULL;
	if (data.one == owner)
		other = data.two;
	else
		other = data.one;
	if (other->physics->collisionCategory == CC_ENVIRONMENT)
	{
		// o.o
		if (AbsoluteValue(data.collisionNormal.y) > 0.8f)
		{
			inAir = false;
			/// If autorun disabled, stop tis animation.
			if (autoRun)
			{
				GMPlayAnimation anim("Run", owner);
				anim.Process(); // Process straight away, no use queueing it up.
			}
		}
	}
	/// Just ensure we're not moving in Z.
	owner->physics->velocity.z = 0;
	owner->position.z = 0;
}

void LuchadorProperty::Process(int timeInMs)
{
	if (sleeping) 
		return;

	/// Taco son muy importante.
	timeSinceLastTaco.AddMs(timeInMs);
	int minutes = timeSinceLastTaco.Minutes();
	int seconds = timeSinceLastTaco.Seconds();
	if (seconds < 5)
		SetSpeedBonusDueToTacos(1.2f);
	else if (seconds < 15)
		SetSpeedBonusDueToTacos(0.5f); // Very full!!
	else if (seconds < 30)
		SetSpeedBonusDueToTacos(0.8f); // Very full!!
	else if (minutes < 5)
		SetSpeedBonusDueToTacos(1.f);
	else 
		SetSpeedBonusDueToTacos(0.8f);

	distance = owner->position.x;
	sideScroller->UpdateDistance();
	if (owner->position.y < -2.f && false)
	{
		assert(false);
	}

	// IF falling or 'jumping' by ramp power
	if (owner->physics->lastGroundCollisionMs + 300 < physicsNowMs)
	{
		// Set jump animation! o.o
		if (!inAir)
			QueueGraphics(new GMPlayAnimation("Jump", owner));
		inAir = true;
	} 
}

void LuchadorProperty::ProcessMessage(Message * message)
{
	String & msg = message->msg;
	if (msg == "Run")
	{
		Run();
	}
	else if (msg == "Stop")
	{
		Stop();
	}
	else if (msg == "Jump")
	{
		if (PhysicsMan.IsPaused())
			return;
		if (sleeping)
			return;
		int jumpCooldownMs = 500;
		if ((now - lastJump).Milliseconds() < jumpCooldownMs)
			return;
		inAir = true;
		lastJump = now;
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, owner->Velocity() + Vector3f(0,5.f,0)));
		// Set jump animation! o.o
		QueueGraphics(new GMPlayAnimation("Jump", owner));
	}
	else if (msg == "UpdateVelocity")
	{
		UpdateVelocity();
	}
}

void LuchadorProperty::OnCollisionCallback(CollisionCallback * cc)
{
	Entity * other = NULL;
	if (cc->one == owner)
		other = cc->two;
	else
		other = cc->one;
	if (other->name.Contains("Hole"))
	{
		if (sideScroller->state != SideScroller::GAME_OVER)
		{
			// Play SFX
			List<String> deadSfx;
			deadSfx.Add("sfx/Noooo.wav", "sfx/Muy malo.wav", "sfx/Demasiado malo.wav");
			deadSfx.AddItem("sfx/Lastima.wav");
			QueueAudio(new AMPlaySFX(deadSfx[sfxRand.Randi(100) % deadSfx.Size()]));

			// Deaded.
			QueuePhysics(new PMSetEntity(owner, PT_PHYSICS_TYPE, PhysicsType::STATIC));
			attempts->iValue += 1;
			sideScroller->UpdateAttempts();
			// Add up total munny.
			totalMunny->iValue += munny;
			// Update completed X if the player has traverse farther than earlier.
			int playerX = playerEntity->position.x;
			if (playerX > completedX->GetInt())
			{
				completedX->SetInt(playerX);
			}
			/// Update start X for retries.
			while(playerX > startX + 1000)
				startX += 1000;
			// Auto-save?
			sideScroller->AutoSave();
			sleeping = true;
			QueueGraphics(new GMPlayAnimation("Idle", owner));
			sideScroller->GameOver();
		}
//		ScriptMan.PlayScript("scripts/OnDeath.txt");
		return;
	}
	if (other->physics->collisionCategory == CC_ENVIRONMENT)
	{
		// o.o
		if (cc->impactNormal.y > 0.8f)
		{
			if (autoRun)
			{
				GMPlayAnimation anim("Run", owner);
				anim.Process(); // Process straight away, no use queueing it up.
			}
		}
	}
}

void LuchadorProperty::Run()
{
	autoRun = true;
	UpdateVelocity();
}

/// Stops velocity and acceleration.
void LuchadorProperty::Stop()
{
	autoRun = false;
	state = STOPPED;
	QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f()));
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
	QueueGraphics(new GMPlayAnimation("Idle", owner));
}


void LuchadorProperty::BuyTaco()
{
	timeSinceLastTaco = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
}

void LuchadorProperty::UpdateVelocity()
{
	Vector3f totalVec;
	totalVec = Vector3f(1,0,0);
	/// o.o
	float playerSpeed = 15.f;
	if (equippedMask)
		playerSpeed += equippedMask->speedBonus;
	// Food modifier.
	playerSpeed *= speedBonusDueToTacos;
	totalVec *= playerSpeed;
	bool moving = autoRun;
	// Set acceleration?
	if (moving)
	{
		QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, totalVec));
	}
	else 
		QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
}


void LuchadorProperty::SetSpeedBonusDueToTacos(float newBonus)
{
	if (speedBonusDueToTacos == newBonus)
		return;
	speedBonusDueToTacos = newBonus;
	// Update vel.
	UpdateVelocity();
}
