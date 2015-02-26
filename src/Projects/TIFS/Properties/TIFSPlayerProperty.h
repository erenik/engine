/// Emil Hedemalm
/// 2014-08-06
/// Entity property for player controllability and camera control.

#ifndef TIFS_PLAYER_PROPERTY_H
#define TIFS_PLAYER_PROPERTY_H

#include "Entity/Properties/FirstPersonPlayerProperty.h"

class ToolParticleEmitter;

class TIFSDroneProperty;
class TIFSTurretProperty;

#define TPP TIFSPlayerProperty

class TIFSPlayerProperty : public FirstPersonPlayerProperty
{
	friend class TIFS;
public:
	TIFSPlayerProperty(Entity * owner);
	virtual ~TIFSPlayerProperty();
	static int ID();

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);


	static float defaultMovementSpeed;
	static float defaultFrictionOnStop;
	static float defaultFrictionOnRun;
	static float defaultJumpSpeed;

	/// 0 - repair, 1 - activate, 2 - targetting 
	void SetToolMode(int mode);
private:
	/// processing/activation for the tool.
	void Tool();
	void Damage(int amount);
	void UpdateHUDTargetInfo();

	int maxHP;
	float currentHP, regen;

	enum 
	{
		REPAIR,
		ACTIVATE,
		REDIRECT_FIRE,
	};
	int toolMode, previousToolMode;
	float repairSpeed;
	Vector3f targetPositionSmoothed;

	ToolParticleEmitter * toolParticleEmitter;

	TIFSDroneProperty * targetDrone;
	TIFSTurretProperty * targetTurret;
	TIFSPlayerProperty * targetPlayer;

	Entity * previousTarget;

	float repairCapacityConsumption;
	float capacitorTransferSpeed;

	float capacitorValue;
	int maxCapacitorValue;
	float capacitorRegenPerSecond;

};

#endif
