/// Emil Hedemalm
/// 2015-01-21
/// Ship property

#ifndef SHIP_PROPERTY_H
#define SHIP_PROPERTY_H

#include "../Base/Ship.h"
#include "MathLib.h"
#include "Entity/EntityProperty.h"
#include "Time/Time.h"

class ShipProperty : public EntityProperty
{
	friend class Ship;
public:
	/// Reference to the game and this property's owner.
	ShipProperty(Ship * ship, Entity * owner);
	// Static version.
	static int ID();

	void Remove();
	// Reset sleep.
	void OnSpawn();
	
	virtual void Process(int timeInMs);
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	/// If reacting to collisions...
	virtual void OnCollision(Entity * withEntity);

	// Since enemies go from right to left..
	bool IsAllied();
	/// When deaded...
	bool sleeping;
	/// hm..
	bool shouldDelete;

	/// o.o
//	void LoadDataFrom(Ship * ship);


	// False by default. If true will use default behaviour of following the mouse.
	bool useMouseInput;

	/// o.o
//	Ship * ship;

private:
	
	Ship * ship;

	bool spawnInvulnerability; // Default true at start.
	long millisecondsPassedSinceLastFire;	
};

#endif









