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

	/// o-o
	Entity * target;

private:
	Vector3f destination;

};