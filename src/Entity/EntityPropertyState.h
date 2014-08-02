/// Emil Hedemalm
/// 2013-02-08

#ifndef ENTITY_PROPERTY_STATE_H
#define ENTITY_PROPERTY_STATE_H

#include "EntityProperty.h"

class Entity;
class Message;

// Base class for states within an EntityProperty
class EntityPropertyState 
{
	friend class AIManager;
public:
	String name;
	EntityPropertyState (int stateType, Entity * owner);
	virtual ~EntityPropertyState();

	/// Function when entering this state.
	virtual void OnEnter() = 0;
	/// Main processing function. Time passed in seconds.
	virtual void Process(int timeInMs) = 0;
	/// Function when leaving this state
	virtual void OnExit() = 0;

	/// Function for handling messages sent to the entity.
	virtual void ProcessMessage(Message * message) = 0;

	/// Callback function for when paths are invalidated.
	virtual void PathsInvalidated();

	/// Assigns this entity an own AI which will be deleted when this state is destructed.
//	void AssignAI(AI * ai);

	/// If it should have an AI o-o;
//	AI * ai;

	/// For checking what state we're currently in.
	int stateID;
protected:
	/// Owner thingy
	Entity * entity;
};

#endif