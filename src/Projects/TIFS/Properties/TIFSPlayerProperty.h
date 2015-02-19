/// Emil Hedemalm
/// 2014-08-06
/// Entity property for player controllability and camera control.

#include "Entity/Properties/FirstPersonPlayerProperty.h"

class ToolParticleEmitter;

class TIFSDroneProperty;
class TIFSTurretProperty;

class TIFSPlayerProperty : public FirstPersonPlayerProperty
{
public:
	TIFSPlayerProperty(Entity * owner);
	static int ID();

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);

	static float defaultMovementSpeed;
	static float defaultFrictionOnStop;
	static float defaultFrictionOnRun;
	static float defaultJumpSpeed;

	/// 0 - repair, 1 - activate, 2 - targetting 
	void SetToolMode(int mode);
private:
	void UpdateHUDTargetInfo();

	int toolMode;

	Vector3f targetPositionSmoothed;

	ToolParticleEmitter * toolParticleEmitter;

	TIFSDroneProperty * targetDrone;
	TIFSTurretProperty * targetTurret;
	TIFSPlayerProperty * targetPlayer;
};