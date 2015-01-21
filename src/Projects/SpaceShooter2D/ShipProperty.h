/// Emil Hedemalm
/// 2015-01-21
/// Ship property

#ifndef SHIP_PROPERTY_H
#define SHIP_PROPERTY_H

#include "Ship.h"
#include "MathLib.h"
#include "Entity/EntityProperty.h"
#include "Time/Time.h"

class ShipProperty : public EntityProperty
{
public:
	/// Reference to the game and this property's owner.
	ShipProperty(Ship * ship, Entity * owner);
	// Static version.
	static int ID();

	void Remove();
	/// D:
	void Destroy();
	// Reset sleep.
	void OnSpawn();
	
	virtual void Process(int timeInMs);
	virtual void ProcessWeapons(int timeInMs);
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	// Since enemies go from right to left..

	/// When deaded...
	bool sleeping;

	// False by default. If true will use default behaviour of following the mouse.
	bool useMouseInput;

	/// o.o
	Ship * ship;

private:

	void ProcessAI();
	long millisecondsPassedSinceLastFire;	
};

#endif









