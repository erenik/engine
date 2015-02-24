/// Emil Hedemalm
/// 2015-02-23
/// o.o

#ifndef TIFS_MOTHERSHIP_PROPERTY_H
#define TIFS_MOTHERSHIP_PROPERTY_H

#include "TIFSProperties.h"
#include "MathLib.h"

#define Mothership TIFSMothershipProperty

class TIFSMothershipProperty : public EntityProperty
{
public:
	TIFSMothershipProperty(Entity * owner);

	/// Setup physics here.
	void OnSpawn();
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);

	static int ID();
	
	void Damage(int amount);
	/// o-o
	Entity * target;

	int currentHP, maxHP;
	// Active and alive.
	bool isActive;

	Vector3f position;

	// default 1.0?
	float acceleration;

	enum {
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
