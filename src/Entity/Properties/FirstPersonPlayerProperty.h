/// Emil Hedemalm
/// 2014-08-06
/** A property which sets up movement and camera controls
	for an arbitrary first-person playable entity.
	
	Exact details how to sub-class this will be updated later on.
	Inspiration may also be taken to create your own tailored variant.

	If a game is to potentially give control over an entity, that property should have some subclass of this property attached to it.
*/

#include "Entity/EntityProperty.h"

#include "MathLib.h"

class FirstPersonPlayerProperty : public EntityProperty
{
public:
	FirstPersonPlayerProperty(String propertyName, int id, Entity * owner);

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);
	void ToggleAutorun();

	// Default false. Enable to steer this entity.
	bool inputFocus;

	// Set this or it won't be able to move.
	float movementSpeed;

protected:
	/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
		and then passed on as messages to the character with inputFocus turned on.
	*/
	void ProcessInput(); 
	void UpdateVelocity(ConstVec3fr newVel);
	// Checks mouse position and casts a ray. Will return all entities along the ray, starting with the closest one.
	void UpdateTargetsByCursorPosition();

	// Targets dictated by the latest call to UpdateTargets
	List<Entity*> targets;
	Entity * primaryTarget;
	/// Set to be the first raycast target position when calling UpdateTargetsByCursorPosition
	Vector3f lastRaycastTargetPosition;

	/// For handling movement.
	Vector3f lastAcc;
	float lastRight;
	bool autorun;

	Vector3f lastVelocity;
};



