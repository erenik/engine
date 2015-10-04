/// Emil Hedemalm
/// 2015-07-09
/// o.o

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "SpaceShooter2D/Base/Weapon.h"

class Ship;

class Explosion
{
public:
	Explosion()
	{
		totalDamageInflicted = 0;
		currentRadius = 0;
	};
	int totalDamageInflicted;
	Weapon weapon;
	Vector3f position;
	float currentRadius;
	List<Ship*> affectedShips; // Only affect a ship once.
	Time startTime;
};

#endif
