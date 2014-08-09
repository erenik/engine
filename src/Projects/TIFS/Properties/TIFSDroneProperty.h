/// Emil Hedemalm
/// 2014-07-30
/// Dronely drone.

#include "TIFSProperties.h"
#include "MathLib.h"

class TIFSDroneProperty : public EntityProperty
{
public:
	TIFSDroneProperty(Entity * owner);

	/// Setup physics here.
	void OnSpawn();
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	static int ID();
	/// o-o
	Entity * target;

	int currentHP, maxHP;
	bool active;

private:
	Vector3f destination;

};