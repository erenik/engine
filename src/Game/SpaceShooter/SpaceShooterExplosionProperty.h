/// Emil Hedemalm
/// 2014-07-31
/// An explosion o-o

#include "Entity/EntityProperty.h"
#include "Time/Time.h"

class SpaceShooterExplosionProperty : public EntityProperty 
{
public:
	SpaceShooterExplosionProperty(Entity * owner);
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	Time startTime;

	/// o-o
	bool sleeping;
private:
};