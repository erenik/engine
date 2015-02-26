/// Emil Hedemalm
/// 2014-08-06
/** A property which sets up movement and camera controls
	for an arbitrary first-person playable entity.
	
	Exact details how to sub-class this will be updated later on.
	Inspiration may also be taken to create your own tailored variant.

	If a game is to potentially give control over an entity, that property should have some subclass of this property attached to it.
*/

#ifndef FIRST_PERSON_PLAYER_PROPERTY_H
#define FIRST_PERSON_PLAYER_PROPERTY_H

#include "Entity/EntityProperty.h"
#include "MathLib.h"
#include "Time/Time.h"

class FirstPersonPlayerProperty : public EntityProperty
{
public:
	FirstPersonPlayerProperty(String propertyName, int id, Entity * owner);

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);

	void ToggleAutorun();

	// Default false. Enable to steer this entity.
	bool inputFocus;

	/// Default true. Will raycast mouse position each frame.
	bool raycast;
	// Set this or it won't be able to move.
	float movementSpeed;
	// 2 frictions, one when moving, other when (trying to) stand still.
	float frictionOnStop;
	float frictionOnRun;
	float jumpSpeed;
	int jumpCooldownMs;

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


	float forward, right;
	Vector3f lastVelocity;
	
	/// o.o
	bool jumping;
	Time lastJump;
};

#endif
