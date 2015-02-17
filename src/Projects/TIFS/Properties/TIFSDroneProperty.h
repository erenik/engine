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
	virtual void ProcessMessage(Message * message);

	static int ID();
	/// o-o
	Entity * target;

	int currentHP, maxHP;
	// Active and alive.
	bool isActive;

	// default 1.0?
	float acceleration;
private:
	Vector3f destination;

};