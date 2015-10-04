/// Emil Hedemalm
/// 2014-07-31
/// An explosion o-o

#include "Entity/EntityProperty.h"
#include "Time/Time.h"

namespace ExplosionType
{
	enum 
	{
		PROJECTILE,
		SHIP,
	}; 
};

class SpaceShooterExplosionProperty : public EntityProperty 
{
public:
	SpaceShooterExplosionProperty(Entity * owner);
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	/// Should be called before OnSpawn so that triggers and timers are setup correctly!
	void SetType(int type);
	/// Call to reset time alive to 0.
	void OnSpawn();

	/// ID of this class, also assigned to all created properties of this type.
	static int ID();

	int timeAliveMs;

	/// o-o
	bool sleeping;
private:
	int type;
};