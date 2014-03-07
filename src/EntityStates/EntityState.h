/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_STATE_H
#define AI_STATE_H

#include "../Entity/Entity.h"
#include "StateProperty.h"
#include "EntityStates.h"

class AI;
class Entity;
struct Message;

// Base class for States within the AI state machine. Subclass and override methods! Do not change much here!
class EntityState {
	friend class AIManager;
public:
	String name;
	EntityState(int stateType, Entity * owner);
	virtual ~EntityState();

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

	/// Assigns this entity an own AI which will be deleted when this state is destructed.
	void AssignAI(AI * ai);

	/// If it should have an AI o-o;
	AI * ai;

	/// For checking what state we're currently in.
	int stateID;
protected:
	/// Owner thingy
	Entity * entity;
};

#endif