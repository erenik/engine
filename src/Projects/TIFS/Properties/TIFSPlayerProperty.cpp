/// Emil Hedemalm
/// 2014-08-06
/// Entity property for player controllability and camera control.

#include "TIFSPlayerProperty.h"

#include "TIFSDroneProperty.h"
#include "TIFSTurretProperty.h"
#include "TIFSProperties.h"

#include "TIFS/TIFS.h"

#include "TIFS/Graphics/ToolParticles.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMParticles.h"

#include "Input/InputManager.h"
#include "Input/Keys.h"

#include "StateManager.h"
#include "Entity/EntityManager.h"

Entity * targetCrossHair = NULL;

float TIFSPlayerProperty::defaultMovementSpeed = 5.f;
float TIFSPlayerProperty::defaultFrictionOnStop = 0.5f;
float TIFSPlayerProperty::defaultFrictionOnRun = 0.1f;
float TIFSPlayerProperty::defaultJumpSpeed = 5.f;

TIFSPlayerProperty::TIFSPlayerProperty(Entity * owner)
: FirstPersonPlayerProperty("TIFSPlayerProperty", TIFSProperty::PLAYER, owner)
{
	toolParticleEmitter = NULL;
	toolMode = 0;

	targetDrone = NULL;
	targetTurret = NULL;

	jumpSpeed = defaultJumpSpeed;
	frictionOnRun = defaultFrictionOnRun;
	frictionOnStop = defaultFrictionOnStop;
	movementSpeed = defaultMovementSpeed;
	lastRaycastTargetPosition = Vector3f();
	targetPositionSmoothed = Vector3f();
}

int TIFSPlayerProperty::ID()
{
	return TIFSProperty::PLAYER;
}

/// Time passed in seconds..! Will steer if inputFocus is true.
void TIFSPlayerProperty::Process(int timeInMs)
{
	// Process movemant and camera stuff.
	FirstPersonPlayerProperty::Process(timeInMs);

	// Update info based on what we are looking at.
	UpdateHUDTargetInfo();

	// If LMB or E is pressed, interact with the entity.
	if (InputMan.lButtonDown || Input.KeyPressed(KEY::E))
	{
		// beam!
		if (!toolParticleEmitter)
		{
			toolParticleEmitter	= new ToolParticleEmitter();
			// Attach it to the particle system
			Graphics.QueueMessage(new GMAttachParticleEmitter(toolParticleEmitter, tifs->toolParticles));
		}
		// Set position.
		toolParticleEmitter->SetPositionAndTarget(owner->worldPosition + Vector3f(0,1.2f, 0), lastRaycastTargetPosition);
		
		// Depending on our target and tool-type, do stuff.
		switch(toolMode)
		{
			// Repairing
			case 0:
				toolParticleEmitter->particleVelocity = 5.f;
				toolParticleEmitter->color = Vector4f(1.f, 1.f, 0.05f, 1.f);
				toolParticleEmitter->particleScale = 0.4f;
				if (targetTurret)
				{
					// Repair it! o-o
					targetTurret->currentHP = (targetTurret->currentHP + timeInMs) % targetTurret->maxHP;
					if (targetTurret->currentHP > targetTurret->maxHP)
					{
						// Play some SFX to notify completion?
					}
				}
				break;
			// Activate
			case 1:
				toolParticleEmitter->particleVelocity = 10.f;
				toolParticleEmitter->particleScale = 0.2f;
				toolParticleEmitter->color = Vector4f(0.1f, 1.f, 1.f, 1.f);
				if (targetTurret)
				{
				
				}
				break;
			// Targetting
			case 2:
				toolParticleEmitter->particleVelocity = 25.f;
				toolParticleEmitter->particleScale = 0.1f;
				toolParticleEmitter->color = Vector4f(1.f, 0.2f, 1.f, 1.f);
				if (targetDrone)
				{
				
				}
				break;
		}
		// Set enabled after all parameters and colors have been set!
		toolParticleEmitter->enabled = true;
	}
	else if (toolParticleEmitter)
	{
		toolParticleEmitter->enabled = false;
	}
}

/// 0 - repair, 1 - activate, 2 - targetting 
void TIFSPlayerProperty::SetToolMode(int mode)
{
	toolMode = mode % 3;
}



void TIFSPlayerProperty::UpdateHUDTargetInfo()
{
	targetDrone = NULL;
	targetTurret = NULL;
	if (primaryTarget == NULL)
	{
		// Hide target-UI, or something?
//		Graphics.QueueMessage(new GMSetUIb("TargetDetailsList", GMUI::VISIBILITY, false));
		Graphics.QueueMessage(new GMSetUIs("TargetName", GMUI::TEXT, "Mundane environment"));
		return;
	}
//	Graphics.QueueMessage(new GMSetUIb("TargetDetailsList", GMUI::VISIBILITY, true));

	if (!targetCrossHair)
	{
		/*
		targetCrossHair = EntityMan.CreateEntity("TargetCrossHairEntity", ModelMan.GetModel("obj/sphere.obj"), TexMan.GetTexture("0x2255AA77"));
		targetCrossHair->SetPosition(this->lastRaycastTargetPosition);
		GraphicsProperty * graphics = new GraphicsProperty(targetCrossHair);
		graphics->blendModeDest = GL_ONE;
		graphics->castsShadow = false;
		graphics->flags = RenderFlag::ALPHA_ENTITY;
		PhysicsProperty * pp = new PhysicsProperty();
		targetCrossHair->physics = pp;
		pp->fullyDynamic = false;
		pp->collisionsEnabled = false;
		pp->type = PhysicsType::KINEMATIC;
		// Only add it to graphics?
		GraphicsQueue.Add(new GMRegisterEntity(targetCrossHair));
		/// Smooth out position in physics system...
		PhysicsQueue.Add(new PMSetEntity(targetCrossHair, PT_ESTIMATION_ENABLED, true));
		PhysicsQueue.Add(new PMRegisterEntity(targetCrossHair));
*/
//		MapMan.AddEntity(targetCrossHair);
	}
	// Just set position.
	/*
	targetPositionSmoothed = targetPositionSmoothed * 0.8 + lastRaycastTargetPosition * 0.2f;
	Time tNow = Time::Now();
	tNow.AddMs(1000);
	PhysicsQueue.Add(new PMSetEntity(targetCrossHair, PT_POSITION, targetPositionSmoothed, tNow));
*/

	targetDrone = (TIFSDroneProperty *)primaryTarget->GetProperty(TIFSDroneProperty::ID());
	targetTurret = (TIFSTurretProperty *)primaryTarget->GetProperty(TIFSTurretProperty::ID());
	targetPlayer = (TIFSPlayerProperty *)primaryTarget->GetProperty(TIFSPlayerProperty::ID());
	/// Display depending on what's seen.
	if (targetDrone)
	{
		Graphics.QueueMessage(new GMSetUIs("TargetName", GMUI::TEXT, "Unidentified aerial object"));
		String repairInfo = "HP "+String::ToString(targetDrone->currentHP)+"/"+String::ToString(targetDrone->maxHP);
		Graphics.QueueMessage(new GMSetUIs("TargetHP", GMUI::TEXT, repairInfo));
		Graphics.QueueMessage(new GMSetUIs("TargetStatus", GMUI::TEXT, targetDrone->isActive? "Offline" : "Online"));
	}
	else if (targetTurret)
	{
		Graphics.QueueMessage(new GMSetUIs("TargetName", GMUI::TEXT, "Semi-Automatic Hybrid Defensive turret"));
		// Update repair-info in the additional info section.
		String repairInfo = "HP "+String::ToString(targetTurret->currentHP)+"/"+String::ToString(targetTurret->maxHP);
		Graphics.QueueMessage(new GMSetUIs("TargetHP", GMUI::TEXT, repairInfo));
		Graphics.QueueMessage(new GMSetUIs("TargetStatus", GMUI::TEXT, targetTurret->active? "Offline" : "Online"));
	}
	else if (targetPlayer)
	{
		Graphics.QueueMessage(new GMSetUIs("TargetName", GMUI::TEXT, "Fellow defender"));
	}
	else 
	{
		Graphics.QueueMessage(new GMSetUIs("TargetName", GMUI::TEXT, "Mundane environment"));
	}
	Vector3f pos = this->lastRaycastTargetPosition;
	String posString;
	posString = "x:"+String::ToString(pos.x,1) + " ";
	posString += "y:"+String::ToString(pos.y,1) + " ";
	posString += "z:"+String::ToString(pos.z,1);
	Graphics.QueueMessage(new GMSetUIs("TargetPosition", GMUI::TEXT, posString));
}
