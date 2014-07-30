/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#include "TIFSProperties.h"

class TIFSTurretProperty : public EntityProperty
{
public:
	// The base will become the owner... I think. Or all of them.. ? o.O
	TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel);

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);


	// Might not use
	Entity * target;
	// Parts
	Entity * base, * swivel, * underBarrel, * barrel;

	
};











