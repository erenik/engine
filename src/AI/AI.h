// Emil Hedemalm
// 2013-07-28

#ifndef AI_H
#define AI_H

/// Placeholder enum, disregard if wanted.
enum defaultAITypes {
	AI_TYPE_NULL,
	AI_TYPE_STATE_MACHINE,
};

class AIState;

#include "Message/Message.h"

/// An AI. can be further defined in subclasses for how it should work!
class AI {
public:
	/// If type is AI_TYPE_STATE_MACHINE, this will act as a regular state machine, entering queued states as requested via the QueueState function.
	AI(int type);
	/// Virtual destructor so subclass destructor callback works correctly! IMPORTANT!
	virtual ~AI();
	// OnEnter/-Exit of the map?
	virtual void OnEnter();
	virtual void OnExit();
	/// Processur. o-o
	virtual void Process(float timeInSeconds);
	/// Queues a new current state.
	virtual void QueueState(AIState * newState);
	/// Queues a new global state.
	virtual void QueueGlobalState(AIState * newState);

	// WOshi.
	virtual void ProcessMessage(Message * message);

	/// If it's active or not.
	bool enabled;
protected:
	/// Will be NULL or perhaps STATE_MACHINE by default, see aiTypes above.
	int type;

	/// Assume  states can be of use no matter what time, so have pointers here for it straight away.
	AIState * globalState;
	AIState * previousState;
	AIState * currentState;
};


#endif
