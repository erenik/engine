/// Emil Hedemalm
/// 2015-05-10
/// An interactive room-builder program.

#include "RoomBuilder.h"

#include "Application.h"
#include "UI/UIUtil.h"

Room room;

void Application::SetApplicationDefaults()
{	
}

RoomBuilder::RoomBuilder()
{	
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void RoomBuilder::OnEnter(AppState * previousState)
{
	// Open main UI.
	PushUI("gui/MainMenu");

	/// Create default room.
	room.Create();
}

/// Main processing function, using provided time since last frame.
void RoomBuilder::Process(int timeInMs)
{	
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void RoomBuilder::OnExit(AppState * nextState)
{
}

void RoomBuilder::ProcessMessage(Message * message)
{
	bool autoSave = false;
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "NewRoom")
			{
				PushUI("gui/NewRoom.gui");
				// Delete old entities.
				MapMan.DeleteAllEntities();
				// Reset all variables.
				room = Room();
			}
			break;
		}
		
	}
	
	if (autoSave)
		room->Save();	
}

/// Input functions for the various states
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void RoomBuilder::MouseClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked)
{
	
	
}

/// Interprets a mouse-move message to target position.
void RoomBuilder::MouseMove(AppWindow * AppWindow, int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL)
{
	
	
}


/// Creates default key-bindings for the state.
void RoomBuilder::CreateDefaultBindings()
{
	
	
}



