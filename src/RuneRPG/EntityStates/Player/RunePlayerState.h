// Emil Hedemalm
// 2013-06-15

#ifndef RUNE_PLAYER_STATE_H
#define RUNE_PLAYER_STATE_H

#include "Maps/2D/EntityStateTile2D.h"

namespace Direction{
enum directions {
	NONE, LEFT, RIGHT, UP, DOWN,
};
};

class EntityStateTile2D;

class RunePlayerState : public EntityStateTile2D {
public:
	RunePlayerState(Entity * owner);
	/// Function when entering this state.
	void OnEnter();
	/// Main processing function
	virtual void Process(float timePassed);
	/// Function when leaving this state
	void OnExit();
	/// Function for handling messages sent to the entity.
	void ProcessMessage(Message * message);

	/// Wosh.
	void DisableMovement();
	void EnableMovement();

	/// Direction the entity is walking!
	int Direction();
	/// Seconds per tile 
	float movementSpeed;
private:
	EntityStateTile2D * entityTile2D;
	/// One step in the looking-direction.
	Vector2i GetInteractionTile();
	/// Based on the requested direction..!
	void UpdateQueuedMovement();

	/// Handle movement based on queued movement..!
	void HandleMovement(float & timePassed);

	/// Time per tile, in seconds.
	static const float DEFAULT_MOVEMENT_SPEED;
	
	/// In seconds
	float timePassedSinceLastMovement;
	int tileX, tileY;
	bool left, right, up, down;
	/// Movement will be queued...!
	bool movementEnabled;
	bool isMoving;
	int direction, queuedDirection, previousDirection, lastQueuedDirection;
	/// Should go from 0 to 1.0 upon completion.
	float movementProgress;
	Vector3f previousPosition, 
		newPosition;
};

#endif
