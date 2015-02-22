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

	/// 1 repair point per millisecond?
	repairSpeed = 1000.f;
	capacitorTransferSpeed = 3000.f;

	repairCapacityConsumption = 100.f;

	capacitorValue = 1000;
	maxCapacitorValue = 5000;
	capacitorRegenPerSecond = 600.f;
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

	/// Regen.
	capacitorValue += capacitorRegenPerSecond * timeInMs * 0.001f;
	ClampFloat(capacitorValue, 0, (float)maxCapacitorValue);

	// Next possible target on camera/within sight.
	if (InputMan.KeyPressedThisFrame(KEY::TAB))
	{
		// Disable click-to-target and raycasting.
		raycast = false;
		// Get closest target based on tool-mode?
		float closestDist = 1000000.f;
		for (int i = 0; i < tifs->turrets.Size(); ++i)
		{
			Entity * turret = tifs->turrets[i];
			float dist = (turret->position - owner->position).LengthSquared();
			if (dist < closestDist)
			{
				closestDist = dist;
				primaryTarget = turret;
				lastRaycastTargetPosition = turret->position;
			}
		}
	}

	// Update info based on what we are looking at.
	UpdateHUDTargetInfo();

	if (debug == 9 && primaryTarget)
		std::cout<<"\nprimaryTarget: "<<primaryTarget->name;

	// If LMB or E is pressed, interact with the entity.
	if (InputMan.lButtonDown || Input.KeyPressed(KEY::E))
	{
		// beam!
		if (!toolParticleEmitter)
		{
			toolParticleEmitter	= new ToolParticleEmitter();
			// Attach it to the particle system
			GraphicsQueue.Add(new GMAttachParticleEmitter(toolParticleEmitter, tifs->toolParticles));
		}
		// Set position.
		Vector3f particleDestination = targetTurret? targetTurret->position : lastRaycastTargetPosition;
		toolParticleEmitter->SetPositionAndTarget(owner->worldPosition + Vector3f(0,1.2f, 0), particleDestination);
		
		// Depending on our target and tool-type, do stuff.
		switch(toolMode)
		{
			// Repairing
			case REPAIR:
				toolParticleEmitter->emissionVelocity = 11.f;
				toolParticleEmitter->color = Vector4f(1.f, 1.f, 0.05f, 0.3f);
				toolParticleEmitter->scale = 0.25f;
				if (targetTurret)
				{
					// Repair it! o-o
					float amount = timeDiffS * repairSpeed;
					targetTurret->Repair(amount);
					capacitorValue -= repairCapacityConsumption * timeDiffS;
				}
				break;
			// Activate
			case ACTIVATE:
				toolParticleEmitter->emissionVelocity = 17.f;
				toolParticleEmitter->scale = 0.16f;
				toolParticleEmitter->color = Vector4f(0.1f, 1.f, 1.f, 0.2f);
				if (targetTurret)
				{
					// Activate it?
					float amount = timeDiffS * capacitorTransferSpeed;
					ClampFloat(amount, 0, capacitorValue);
					capacitorValue -= amount;
					targetTurret->Activate(amount);
				}
				break;
			// Targetting
			case REDIRECT_FIRE:
				toolParticleEmitter->emissionVelocity = 32.f;
				toolParticleEmitter->scale = 0.14f;
				toolParticleEmitter->color = Vector4f(0.9f, 0.1f, 0.5f, 0.35f);
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
	// Clamp value before display?
	ClampFloat(capacitorValue, 0, (float)maxCapacitorValue);
	// Update capacitor value
	Vector3f capacitorColor;
	capacitorColor = Vector3f(0,1,1) * capacitorValue / (float)maxCapacitorValue + Vector3f(1,0,0) * (1 - capacitorValue / (float)maxCapacitorValue);
	GraphicsQueue.Add(new GMSetUIv3f("CapacitorValue", GMUI::TEXT_COLOR, capacitorColor));
	GraphicsQueue.Add(new GMSetUIs("CapacitorValue", GMUI::TEXT, String((int)capacitorValue)));
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
		GraphicsQueue.Add(new GMSetUIs("TargetName", GMUI::TEXT, "Mundane environment"));
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
	// Update HUD with our own data. tool-mode switch here.
	if (previousToolMode != toolMode)
	{
		String modeText;
		Vector4f modeColor;
		switch(toolMode)
		{
			case REPAIR:
			{
				modeText = "REPAIR";
				modeColor = Vector4f(1,0.9f,0.1f,1);
				break;		
			}
			case ACTIVATE:
				modeText = "ACTIVATE";
				modeColor = Vector4f(0,1,1,1);
				break;
			case REDIRECT_FIRE:
				modeText = "REDIRECT FIRE";
				modeColor = Vector4f(1,0.1f,0.6f,1);
				break;
		}
		GraphicsQueue.Add(new GMSetUIs("ToolMode", GMUI::TEXT, modeText ));
		GraphicsQueue.Add(new GMSetUIv4f("ToolMode", GMUI::TEXT_COLOR, modeColor));
		previousToolMode = toolMode;
	}
	targetDrone = (TIFSDroneProperty *)primaryTarget->GetProperty(TIFSDroneProperty::ID());
	targetTurret = (TIFSTurretProperty *)primaryTarget->GetProperty(TIFSTurretProperty::ID());
	targetPlayer = (TIFSPlayerProperty *)primaryTarget->GetProperty(TIFSPlayerProperty::ID());

	if (primaryTarget != previousTarget)
	{
		previousTarget = primaryTarget;
		// Remove old UIs.
		GraphicsQueue.Add(new GMSetUIb("TurretDetails", GMUI::VISIBILITY, false));
		GraphicsQueue.Add(new GMSetUIb("DroneDetails", GMUI::VISIBILITY, false));
		GraphicsQueue.Add(new GMSetUIb("DefenderDetails", GMUI::VISIBILITY, false));
		GraphicsQueue.Add(new GMSetUIb("MothershipDetails", GMUI::VISIBILITY, false));
		// Depending on new target..
		if (targetTurret)
		{
			GraphicsQueue.Add(new GMSetUIb("TurretDetails", GMUI::VISIBILITY, true));
		}
	}

	//	targetBuilding = 
	/// Display depending on what's seen.
	if (targetDrone)
	{
		GraphicsQueue.Add(new GMSetUIs("TargetName", GMUI::TEXT, "Unidentified aerial object"));
		String repairInfo = "HP "+String::ToString(targetDrone->currentHP)+"/"+String::ToString(targetDrone->maxHP);
		GraphicsQueue.Add(new GMSetUIs("TargetHP", GMUI::TEXT, repairInfo));
		GraphicsQueue.Add(new GMSetUIs("TargetStatus", GMUI::TEXT, targetDrone->isActive? "Offline" : "Online"));
	}
	else if (targetTurret)
	{
		// Old info.
		GraphicsQueue.Add(new GMSetUIs("TargetName", GMUI::TEXT, "Semi-Automatic Hybrid Defensive turret"));
		// Update repair-info in the additional info section.
		String repairInfo = "HP "+String::ToString(targetTurret->currentHP)+"/"+String::ToString(targetTurret->maxHP);
		GraphicsQueue.Add(new GMSetUIs("TargetHP", GMUI::TEXT, repairInfo));
		GraphicsQueue.Add(new GMSetUIs("TargetStatus", GMUI::TEXT, targetTurret->active? "Online" : "Offline"));

		/// New info...
		GraphicsQueue.Add(new GMSetUIs("TurretName", GMUI::TEXT, targetTurret->name));
		GraphicsQueue.Add(new GMSetUIs("TurretHP", GMUI::TEXT, String(targetTurret->currentHP)));
		// Set color based on HP-ratio.
		Vector4f color;
		float ratio = targetTurret->currentHP / (float) targetTurret->maxHP;
		if (ratio < 0.25)
			color = Vector4f(0.75,0.5f,0.25f,1.f);
		else if (ratio < 0.50f)
			color = Vector4f(0.8f,0.6f,0.4f,1.f);
		else if (ratio < 0.75f)
			color = Vector4f(0.9f,0.8f,0.7f,1.f);
		// High enough to activate properly.
		else 
			color = Vector4f(1,1,1,1);
		GraphicsQueue.Add(new GMSetUIv4f("TurretHP", GMUI::TEXT_COLOR, color));
		GraphicsQueue.Add(new GMSetUIs("TurretCapacitor", GMUI::TEXT, String((int)targetTurret->currentCapacitorValue)));
		float turrRelCapac = targetTurret->currentCapacitorValue / targetTurret->maxCapacitorValue;
		Vector3f cTextColor = Vector3f(0,1,1) * turrRelCapac + Vector3f(0.5,0.5,0.5) * (1 - turrRelCapac);
		GraphicsQueue.Add(new GMSetUIv3f("TurretCapacitor", GMUI::TEXT_COLOR, cTextColor));
		String stateText;
		if (!targetTurret->active && !targetTurret->activatable)
		{
			stateText = "Requiring repairs";
		}
		else if (!targetTurret->active)
		{
			stateText = "Capacitor empty/Offline";
		}
		else if (targetTurret->target)
		{
			stateText = "Tracking target";
		}
		else 
			stateText = "Idle";

		GraphicsQueue.Add(new GMSetUIs("TurretState", GMUI::TEXT, stateText));
	}
	else if (targetPlayer)
	{
		GraphicsQueue.Add(new GMSetUIs("TargetName", GMUI::TEXT, "Fellow defender"));
	}
	else 
	{
		GraphicsQueue.Add(new GMSetUIs("TargetName", GMUI::TEXT, "Mundane environment"));
	}
	Vector3f pos = this->lastRaycastTargetPosition;
	String posString;
	posString = "x:"+String::ToString(pos.x,1) + " ";
	posString += "y:"+String::ToString(pos.y,1) + " ";
	posString += "z:"+String::ToString(pos.z,1);
	Graphics.QueueMessage(new GMSetUIs("TargetPosition", GMUI::TEXT, posString));
}
