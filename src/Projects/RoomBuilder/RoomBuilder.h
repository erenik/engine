/// Emil Hedemalm
/// 2015-05-10
/// An interactive room-builder program.

#ifndef ROOM_BUILDER_H
#define ROOM_BUILDER_H

#include "AppStates/AppState.h"
#include "Maps/MapManager.h"
#include "Room.h"
#include "StateManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Message/Message.h"
#include "File/LogFile.h"
#include "File/FileUtil.h"

/// Active room.
extern Room room;

class RoomBuilder : public AppState
{
public:
	RoomBuilder();	
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState) = 0;
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs) = 0;
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState) = 0;


	virtual void ProcessMessage(Message * message);

	/// Input functions for the various states
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	virtual void MouseClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked);
	/// Interprets a mouse-move message to target position.
	virtual void MouseMove(AppWindow * AppWindow, int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
};

#endif
