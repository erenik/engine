/// Emil Hedemalm
/// 2014-07-25
/// Player properties for a space-shooter game.
/// Applicable to both human and "enemy"/AI-players.


#include "MathLib.h"
#include "Entity/EntityProperty.h"
#include "Time/Time.h"

#include "SpaceShooterProjectileProperty.h"

class SpaceShooterPlayerProperty : public EntityProperty
{
public:
	SpaceShooterPlayerProperty(Entity * owner);
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	
	/// Determines type of projectile spawned.
	SpaceShooterWeaponType type;

	// player score
	int score;

	// Since enemies go from right to left..
	bool player;
	bool enemy;

	// For transformations.
	Vector3f initialScale;
	/// For switching on AI
	Time lastUserInput;

private:

};











