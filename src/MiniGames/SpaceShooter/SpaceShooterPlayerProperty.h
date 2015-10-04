/// Emil Hedemalm
/// 2014-07-25
/// Player properties for a space-shooter game.
/// Applicable to both human and "enemy"/AI-players.


#include "MathLib.h"
#include "Entity/EntityProperty.h"
#include "Time/Time.h"

#include "SpaceShooterProjectileProperty.h"

class SpaceShooter;

class SpaceShooterPlayerProperty : public EntityProperty
{
public:
	/// Reference to the game and this property's owner.
	SpaceShooterPlayerProperty(SpaceShooter * game, Entity * owner);
	// Static version.
	static int ID();

	void Remove();
	/// D:
	void Destroy();
	// Reset sleep.
	void OnSpawn();
	
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);
	void Damage(SpaceShooterWeaponType & type);

	/// Determines type of projectile spawned.
	SpaceShooterWeaponType weaponType;

	// player score
	int score;

	// Since enemies go from right to left..
	bool isPlayer;
	bool enemy;

	// For transformations.
	Vector3f initialScale;
	/// For switching on AI
	Time lastUserInput;


	/// True for players and allies.
	bool allied;

	/// When deaded...
	bool sleeping;


	// False by default. If true will use default behaviour of following the mouse.
	bool useMouseInput;

	/// o.o
	int hp;
	int maxHP;
private:

	long millisecondsPassedSinceLastFire;
	
	SpaceShooter * game;
};











