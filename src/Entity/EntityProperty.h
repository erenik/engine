/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_PROPERTY_H
#define AI_PROPERTY_H

#include "String/AEString.h"

class Entity;
class EntityPropertyState;
class Message;
struct Collision;

/// Just some stuff.
const int propertiesPerGame = 100;
const int propertiesPerMiniGame = 10;
namespace EntityPropertyID 
{
	enum entityPropertyIDs
	{
		UTILITY_PROPERTIES, // E.g. EntityStateTile2D.

		CUSTOM_GAME_1 = UTILITY_PROPERTIES + 100,
		CUSTOM_GAME_2 = CUSTOM_GAME_1 + propertiesPerGame,
		CUSTOM_GAME_3 = CUSTOM_GAME_2 + propertiesPerGame,
		CUSTOM_GAME_4 = CUSTOM_GAME_3 + propertiesPerGame,
		CUSTOM_GAME_5 = CUSTOM_GAME_4 + propertiesPerGame,
		CUSTOM_GAME_6 = CUSTOM_GAME_5 + propertiesPerGame,
		LAST_GAME,
		MINI_GAME_1 = LAST_GAME,							// Reserved by: Pong
		MINI_GAME_2 = MINI_GAME_1 + propertiesPerMiniGame,	// Breakout
		MINI_GAME_3 = MINI_GAME_2 + propertiesPerMiniGame,	// SpaceShooter
		MINI_GAME_4 = MINI_GAME_3 + propertiesPerMiniGame,
		MINI_GAME_5 = MINI_GAME_4 + propertiesPerMiniGame,
		MINI_GAME_6 = MINI_GAME_5 + propertiesPerMiniGame,
	};
};

/// The EntityProperty is pretty much a StateMachine that can be attached to the entities.... wat?
class EntityProperty 
{
//	friend class AIManager;
//	friend class World;
	friend class EntityPropertyState;
public:
	/// Default annulizing constructor.
	EntityProperty(String name, int id, Entity * owner);
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

	/// Returns the ID of this specific property (used when identifying it within an entity later on).
	int GetID();
	
	/// Callback for when one or more paths have been invalidated to due changes in the map.
	void PathsInvalidated();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	EntityPropertyState * GlobalState() { return globalState; };

	/// Sent to both global and current state
	void ProcessMessage(Message * message);

	/// Reference, should not be altered.
	Entity * owner;
	// For properties with multiple owners.
	List<Entity*> owners;

protected:
	/// ID.
	const int id;

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
