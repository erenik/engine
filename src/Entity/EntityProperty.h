/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_PROPERTY_H
#define AI_PROPERTY_H

#include "String/AEString.h"
class Entity;
class EntityPropertyState;
class Message;
struct Collision;

/// The EntityProperty is pretty much a StateMachine that can be attached to the entities.... wat?
class EntityProperty 
{
//	friend class AIManager;
//	friend class World;
	friend class EntityPropertyState;
public:
	/// Default annulizing constructor.
	EntityProperty(String name, Entity * owner);
	virtual ~EntityProperty();

	/// Should correspond to class-name.
	String name;

	/// Sets global entity state!
	void SetGlobalState(EntityPropertyState * globalState);

	/// Swaps state straight away, keeping the queued state.
	void EnterState(EntityPropertyState * newState);
	/// Enters queued state. Returns false if no state was queued.
	bool EnterQueuedState();
	/// Enters the previous state again.
	void RevertToPreviousState();

	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);


	/// Callback for when one or more paths have been invalidated to due changes in the map.
	void PathsInvalidated();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	EntityPropertyState * GlobalState() { return globalState; };

	/// Sent to both global and current state
	void ProcessMessage(Message * message);

	/// Reference, should not be altered.
	Entity * owner;

private:
	/// AppState-control variables
	EntityPropertyState * currentState;
	EntityPropertyState * previousState;
	EntityPropertyState * queuedState;

	/// Global state that is run
	EntityPropertyState * globalState;


	/// General/multi-purpose variables common to all states.
	/// Should be declared in the GlobalState!

	/// Extra data variables
};

#endif
