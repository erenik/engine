/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_PROPERTY_H
#define AI_PROPERTY_H

#include "AI.h"
class AIState;
class Entity;
class EntityState;
struct Message;

/// The StateProperty is pretty much a StateMachine that can be attached to the entities.
struct StateProperty {
	friend class AIManager;
	friend class World;
	friend class EntityState;
public:
	/// Default annulizing constructor.
	StateProperty(Entity * entity);
	~StateProperty();

	/// Sets global entity state!
	void SetGlobalState(EntityState * globalState);

	/// Swaps state straight away, keeping the queued state.
	void EnterState(EntityState * newState);
	/// Enters queued state. Returns false if no state was queued.
	bool EnterQueuedState();
	/// Enters the previous state again.
	void RevertToPreviousState();

	/// Callback for when one or more paths have been invalidated to due changes in the map.
	void PathsInvalidated();

	/// Time passed in seconds..!
	void Process(float timePassed);
	EntityState * GlobalState() { return globalState; };

	/// Sent to both global and current state
	void ProcessMessage(Message * message);

private:
	/// GameState-control variables
	EntityState * currentState;
	EntityState * previousState;
	EntityState * queuedState;

	/// Global state that is run
	EntityState * globalState;

	/// Reference
	Entity * entity;

	/// General/multi-purpose variables common to all states.
	/// Should be declared in the GlobalState!

	/// Extra data variables
};

#endif
