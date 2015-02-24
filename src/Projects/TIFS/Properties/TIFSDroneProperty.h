/// Emil Hedemalm
/// 2014-07-30
/// Dronely drone.

#ifndef TIFS_DRONE_PROPERTY_H
#define TIFS_DRONE_PROPERTY_H

#include "TIFSProperties.h"
#include "MathLib.h"

enum 
{
	FLYING_DRONE,
	HOVER_DRONE,
};

#define Drone TIFSDroneProperty

class TIFSDroneProperty : public EntityProperty
{
public:
	TIFSDroneProperty(Entity * owner);

	/// Setup physics here.
	void OnSpawn();
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	virtual void ProcessHoverDrone(int timeInMs);
	virtual void ProcessFlyingDrone(int timeInMs);

	virtual void ProcessMessage(Message * message);

	static int ID();
	
	void Damage(int amount);
	/// Default HOVER_DRONE
	int type;
	/// o-o
	Entity * target;

	int currentHP, maxHP;
	// Active and alive.
	bool isActive;

	// default 1.0?
	float acceleration;

	enum {
		DESCENDING_ONTO_CITY,
		RANDOM_ROAM,
		CHASE_PLAYER,
		ATTACK_TURRET,
		DRONE_STATES,
	};
	int state;
private:
	void SetState(int newState);
	void UpdateDestination();
	void UpdateVelocity();
	Vector3f destination;
	int timeInStateMs;
	int timeSinceLastVelUpdate;
};

#endif
