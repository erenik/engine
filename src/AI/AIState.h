/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_STATE_H
#define AI_STATE_H

#include "../Entity/Entity.h"

class Entity;
class Message;

// Base class for States within the AI state machine. Subclass and override methods! Do not change much here!
class AIState {
	friend class AIManager;
public:
	String name;
	AIState(int stateType, Entity * owner);
	/// Virtual destructor so subclass destructor callback works correctly! IMPORTANT!
	virtual ~AIState(){};

	/// Function when entering this state.
	virtual void OnEnter() = 0;
	/// Main processing function. Time passed in seconds.
	virtual void Process(float timePassed) = 0;
	/// Function when leaving this state
	virtual void OnExit() = 0;

	/// Function for handling messages sent to the entity.
	virtual void ProcessMessage(Message * message) = 0;

	/// Callback function for when paths are invalidated.
	virtual void PathsInvalidated();

	/// For checking what state we're currently in.
	int stateID;
protected:
	/// Owner-pointer reference
	Entity * entity;
};

#endif